#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
//file operation
#include <linux/fs.h>
#include <linux/cdev.h>
//devce file
#include <linux/device.h>
#include <linux/kdev_t.h>
//sysfs
#include <linux/sysfs.h>
#include <linux/kobject.h>
//interrupt
#include <linux/interrupt.h>
//workqueue
#include <linux/workqueue.h>

#define IRQ_NO 51

volatile int sys_val_dynamic = 0;
dev_t dev_num = 0;
static struct class* dev_class;
static struct device* dev_file;
static struct cdev my_cdev;
static struct kobject* my_kobj;
static struct work_struct workqueue;
void work_fn(struct work_struct* work);

static int __init create_dynamic_workqueue(void);
static void __exit remove_dynamic_workqueue(void);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off);
//sysfs function
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count);
struct kobj_attribute my_attr = __ATTR(val_sysfs_dynamic_workqueue, 0660, sys_show, sys_store);

//function interrupt
static irqreturn_t irqhandler(int irq, void* dev_id);

//file operations
struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .read = read_fops,
    .write = write_fops,
    .open = open_fops,
    .release = release_fops,
};

static int open_fops(struct inode* inode, struct file* file)
{
    pr_info("OPEN\n");
    return 0;
}
static int release_fops(struct inode* inode, struct file* file)
{
    pr_info("RELEASE\n");
    return 0;
}
static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off)
{
    pr_info("READ\n");
    generic_handle_irq(IRQ_NO);
    return 0;
}
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n");
    return len;
}

static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    pr_info("SYS_SHOW: READ\n");
    return sprintf(buf, "Value = %d\n", sys_val_dynamic);
}
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    pr_info("SYS_STORE: WRITE\n");
    sscanf(buf, "%d", &sys_val_dynamic);
    return count;
}

static irqreturn_t irqhandler(int irq, void* dev_id)
{
    pr_info("Call irq\n");
    schedule_work(&workqueue);
    return IRQ_HANDLED;
}
void work_fn(struct work_struct* work)
{
    pr_info("Exetute dynamic workqueue function\n");
}
static int __init create_dynamic_workqueue(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "dynamic_workqueue_num") < 0)
    {
        pr_err("Cannot allocate major minor number\n");
        return -1;
    }
    pr_info("Major(%d)\nMinor(%d)", MAJOR(dev_num), MINOR(dev_num));

    dev_class = class_create(THIS_MODULE, "dynamic_workqueue_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }
    dev_file = device_create(dev_class, NULL, dev_num, NULL, "dynamic_workqueue_device");
    if(IS_ERR(dev_file))
    {
        pr_err("Err create device file\n");
        goto r_device;
    }

    cdev_init(&my_cdev, &my_fops);
    if(cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("Err add device to system\n");
        goto r_class;
    }

    my_kobj = kobject_create_and_add("ex_dynamic_workqueue", kernel_kobj);
    if(sysfs_create_file(my_kobj, &my_attr.attr))
    {
        pr_err("Create sysfs file err\n");
        goto r_sysfs;
    }
    if(request_irq(IRQ_NO, irqhandler, IRQF_SHARED, "dynamic_workqueue_itr", (void*)irqhandler))
    {
        pr_err("Create itr err\n");
        goto r_sysfs;
    }
    INIT_WORK(&workqueue, work_fn);
    pr_info("Insert dynamic workqueue done\n");
    return 0;
r_sysfs:
    kobject_put(my_kobj);
    sysfs_remove_file(my_kobj, &my_attr.attr);
r_device:
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
r_class:
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return -1;
}

static void __exit remove_dynamic_workqueue(void)
{
    free_irq(IRQ_NO, (void*)irqhandler);
    kobject_put(my_kobj);
    sysfs_remove_file(my_kobj, &my_attr.attr);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove dynamic workqueue done\n");
}

module_init(create_dynamic_workqueue);
module_exit(remove_dynamic_workqueue);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
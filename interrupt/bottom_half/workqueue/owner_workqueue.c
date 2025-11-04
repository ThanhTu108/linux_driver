#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>

//device file
#include <linux/device.h>
#include <linux/kdev_t.h>

//file operation
#include <linux/cdev.h>
#include <linux/fs.h>
//Sysfs
#include <linux/sysfs.h>
#include <linux/kobject.h>
//interrupt
#include <linux/interrupt.h>
//workqueue
#include <linux/workqueue.h>

#define IRQ_NO 51

volatile int sys_own_wq = 0;
dev_t dev_num = 0;
static struct class* dev_class;
static struct device* dev_file;
static struct cdev my_cdev;
static struct kobject* my_kobj;

static void work_fn(struct work_struct* work);
static struct workqueue_struct* own_workqueue;
static irqreturn_t irqhandler(int irq, void* dev_id);


static int __init create_own_wq(void);
static void __exit remove_own_wq(void);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off);
static ssize_t  write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off);

//sysfs
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count);
static struct kobj_attribute my_attr = __ATTR(val_sys_own_wq, 0660, sys_show, sys_store);

static struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = open_fops,
    .read = read_fops,
    .write = write_fops,
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

static void work_fn(struct work_struct* work)
{
    pr_info("Execute function wq\n");
}

static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    pr_info("\nSYS_SHOW: READ\n");
    return sprintf(buf, "Value = %d\n", sys_own_wq);
}
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    pr_info("\nSYS_STORE: WRITE\n");
    sscanf(buf, "%d", &sys_own_wq);
    return count;
}

DECLARE_WORK(work, work_fn);
static irqreturn_t irqhandler(int irq, void* dev_id)
{
    pr_info("Call irq\n");
    queue_work(own_workqueue, &work);
    return IRQ_HANDLED;
}
static int __init create_own_wq(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "owner_workqueue_num") < 0)
    {
        pr_err("Cannot allocate major and minor number\n");
        return -1;
    }
    pr_info("Major(%d)\nMinor(%d)", MAJOR(dev_num), MINOR(dev_num));
    dev_class = class_create(THIS_MODULE, "owner_workqueue_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Create struct class err\n");
        goto r_class;
    }
    dev_file = device_create(dev_class, NULL, dev_num, NULL, "owner_workqueue_file");
    if(IS_ERR(dev_file))
    {
        pr_err("Create device file err\n");
        goto r_device;
    }
    cdev_init(&my_cdev, &my_fops);
    if(cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("Cannot add device to system\n");
    }
    my_kobj = kobject_create_and_add("owner_wq_sys", kernel_kobj);
    if(sysfs_create_file(my_kobj, &my_attr.attr))
    {
        pr_err("Create sysfs file err\n");
        goto r_sysfs;
    }
    if(request_irq(IRQ_NO, irqhandler, IRQF_SHARED, "own_wq_itr", (void*)irqhandler))
    {
        pr_err("Cannot register irq flags\n");
        goto r_irq;
    }
    own_workqueue = create_workqueue("own_wq");
    pr_info("Insert device done\n");
    return 0;
r_irq:
    kobject_put(my_kobj);
    sysfs_remove_file(my_kobj, &my_attr.attr);
r_sysfs:
    device_destroy(dev_class, dev_num);
r_device:
    class_destroy(dev_class);
r_class:
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return -1;
}   
static void __exit remove_own_wq(void)
{
    destroy_workqueue(own_workqueue);
    free_irq(IRQ_NO, (void*)irqhandler);
    kobject_put(my_kobj);
    sysfs_remove_file(my_kobj, &my_attr.attr);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove done\n");
}

module_init(create_own_wq);
module_exit(remove_own_wq);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
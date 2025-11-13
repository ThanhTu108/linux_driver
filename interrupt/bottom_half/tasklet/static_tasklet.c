#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>

//file operations
#include <linux/cdev.h>
#include <linux/fs.h>
//device file
#include <linux/kdev_t.h>
#include <linux/device.h>
//sysfs
#include <linux/kobject.h>
#include <linux/sysfs.h>
//itr
#include <linux/interrupt.h>

#define IRQ_NO 51

volatile int sys_val = 0;
static dev_t dev_num = 0;
static struct class* dev_class;
static struct device* dev_file;
static struct kobject* my_kobj;
static struct cdev my_cdev;

//tasklet
void my_tasklet_fn(struct tasklet_struct* t);
//kernel 6.1.83 DECLARE_TASKLET(name, func);
DECLARE_TASKLET(my_tasklet, my_tasklet_fn);  //0 is arg: passing 0 to this function

//interrupt
static irqreturn_t irq_handler(int irq, void* dev_id)
{
    pr_info("Call irq\n");
    tasklet_schedule(&my_tasklet);
    return IRQ_HANDLED;
}

static int __init ex_create_tasklet(void);
static void __exit ex_remove_tasklet(void);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off);
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr,const char* buf, size_t count);
static struct kobj_attribute my_attr = __ATTR(val_stt_tasklet, 0664, sys_show, sys_store);


static struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = open_fops,
    .read = read_fops, 
    .write = write_fops,
    .release = release_fops,
};
void my_tasklet_fn(struct tasklet_struct* t)
{
    pr_info("Tasklet function execute");
}
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
    return sprintf(buf, "%d\n", sys_val);
}
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr,const char* buf, size_t count)
{
    pr_info("SYS_STORE: WRITE\n");
    sscanf(buf, "%d\n", &sys_val);
    return count;
}
static int __init ex_create_tasklet(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "tasklet_nn_num") < 0)
    {
        pr_err("Cannot allocate major minor number\n");
        return -1;
    }
    pr_info("Major(%d)\nMinor(%d)", MAJOR(dev_num), MINOR(dev_num));
    cdev_init(&my_cdev, &my_fops);
    if(cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("Cannot add device to system\n");
        goto r_class;
    }

    dev_class = class_create(THIS_MODULE, "tasklet_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }
    dev_file = device_create(dev_class, NULL, dev_num, NULL, "tasklet_device");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    my_kobj = kobject_create_and_add("sys_tasklet", kernel_kobj);
    if(sysfs_create_file(my_kobj, &my_attr.attr))
    {
        pr_err("Cannot create sysfs file\n");
        goto r_sysfs;
    }

    if(request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "irq_tasklet", (void*)irq_handler))
    {
        pr_err("Cannot register irq\n");
        goto r_sysfs;
    }
    pr_info("INSERT TASKLET DONE\n");
    return 0;
r_sysfs:
    sysfs_remove_file(my_kobj, &my_attr.attr);
    kobject_put(my_kobj);
    device_destroy(dev_class, dev_num);
r_device:
    class_destroy(dev_class);
r_class:
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return -1;
}
static void __exit ex_remove_tasklet(void)
{
    tasklet_kill(&my_tasklet);
    free_irq(IRQ_NO, (void*)irq_handler);
    sysfs_remove_file(my_kobj, &my_attr.attr);
    kobject_put(my_kobj);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove done\n");
}
module_init(ex_create_tasklet);
module_exit(ex_remove_tasklet);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
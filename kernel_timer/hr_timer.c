#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>

//device file
#include <linux/device.h>
#include <linux/kdev_t.h>
//fops
#include <linux/cdev.h>
#include <linux/fs.h>

//hrtimer (high resolution timer)
#include <linux/hrtimer.h>
//sysfs
#include <linux/kobject.h>
#include <linux/sysfs.h>

//Timer variable
#define TIMEOUT_SEC 4
#define TIMEOUT_NSEC 1000000000L

//Golobal variabal
volatile int val_sys = 0;
static int count = 0;

static dev_t dev_num;
static struct hrtimer my_hrtimer; 

static struct class* dev_class;
static struct device* dev_file; 
static struct cdev my_cdev; 
static struct kobject* my_kobj;

//Golobal function

// enum hrtimer_restart		(*function)(struct hrtimer *);
enum hrtimer_restart hrtimer_callback(struct hrtimer* timer);

static int __init cre_hrtimer(void);
static void __exit rmv_hrtimer(void);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off);
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count);
static struct kobj_attribute my_attr = __ATTR(val_hrhrtimer, 0660, sys_show, sys_store);

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
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off)
{
    pr_info("READ\n");
    return 0;
}
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n");
    return 0;
}
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    pr_info("SYS_SHOW: READ \n");
    return sprintf(buf, "%d\n", val_sys);
}
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    pr_info("SYS_STORE: WRITE\n");
    sscanf(buf, "%d", &val_sys);
    return count;
}
enum hrtimer_restart hrtimer_callback(struct hrtimer* timer)
{
    pr_info("Call hr_timer: %d\n", count++);
    hrtimer_forward_now(timer, ktime_set(TIMEOUT_SEC, TIMEOUT_NSEC));
    return HRTIMER_RESTART;
}

static int __init cre_hrtimer(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "hrtimer_num") < 0)
    {
        pr_err("Cannot create struct class \n");
        return -1;
    }
    pr_info("Major(%d), Minor(%d)", MAJOR(dev_num), MINOR(dev_num));

    cdev_init(&my_cdev, &my_fops);
    if(cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("Cannot add device to system\n");
        goto r_class;
    }

    dev_class = class_create(THIS_MODULE, "hrtimer_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }
    dev_file = device_create(dev_class, NULL, dev_num, NULL, "hrtimer_device");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create struct class\n");
        goto r_device;
    }
    my_kobj = kobject_create_and_add("sys_hrhrtimer", kernel_kobj);
    if(sysfs_create_file(my_kobj, &my_attr.attr))
    {
        pr_err("Cannot create sysfs file\n");
        goto r_sysfs;
    }
    hrtimer_init(&my_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    my_hrtimer.function = &hrtimer_callback;
    hrtimer_start(&my_hrtimer, ktime_set(TIMEOUT_SEC, TIMEOUT_NSEC), HRTIMER_MODE_REL);

    pr_info("Create hrtime done\n");
    return 0;
r_sysfs:
    sysfs_remove_file(kernel_kobj, &my_attr.attr);
    kobject_put(my_kobj);
    device_destroy(dev_class, dev_num);
r_device:
    class_destroy(dev_class);
r_class:
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return -1;
}
static void __exit rmv_hrtimer(void)
{
    hrtimer_cancel(&my_hrtimer);
    sysfs_remove_file(kernel_kobj, &my_attr.attr);
    kobject_put(my_kobj);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove hrtimer done\n");
}

module_init(cre_hrtimer);
module_exit(rmv_hrtimer);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu108303@gmail.com");
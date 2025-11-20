#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>

//device file
#include <linux/device.h>
#include <linux/kdev_t.h>
//fops
#include <linux/fs.h>
#include <linux/cdev.h>
//sysfs
#include <linux/sysfs.h>
#include <linux/kobject.h>
//timer
#include <linux/timer.h>
#include <linux/jiffies.h>
//timer
#define TIMEOUT 5000
static struct timer_list my_timer;

volatile int count = 0;
volatile int val_sys = 0;
dev_t dev_num;
struct class* dev_class; 
struct device* dev_file;
struct cdev my_cdev; 
struct kobject* my_kobj;

//function prototype
void timer_callback(struct timer_list* timer);
static int __init ex_cre_kerneltimer(void);
static void __exit rm_kerneltimer(void);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off); 
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count);
static struct kobj_attribute my_attr = __ATTR(val_kerneltimer, 0660, sys_show, sys_store);

struct file_operations my_fops = 
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
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n");
    return len;
} 
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off)
{
    pr_info("READ\n");
    return 0;
}
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    pr_info("SYS_SHOW\n");
    return sprintf(buf, "%d\n", val_sys);
}
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    pr_info("SYS_STORE\n");
    sscanf(buf, "%d", &val_sys);
    return count;
}
void timer_callback(struct timer_list* timer)
{
    pr_info("Timer execute: %d\n", count++);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(TIMEOUT));
}

static int __init ex_cre_kerneltimer(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "kernel_timer_num") < 0)
    {
        pr_err("Cannot create major and minor number\n");
        return -1;
    }
    pr_info("MAJOR(%d), MINOR(%d)\n", MAJOR(dev_num), MINOR(dev_num));
    cdev_init(&my_cdev, &my_fops);
    if(cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("Cannot add device to system\n");
        goto r_class;
    }
    dev_class = class_create(THIS_MODULE, "kernel_timer_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }

    dev_file = device_create(dev_class, NULL, dev_num, NULL, "kernel_timer_device");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    my_kobj = kobject_create_and_add("sysfs_kerneltimer", kernel_kobj);
    if(sysfs_create_file(my_kobj, &my_attr.attr))
    {
        pr_err("Cannot create sys file\n");
        goto r_sysfs;
    }
    timer_setup(&my_timer, timer_callback, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(TIMEOUT));
    pr_info("Insert device done\n");
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
static void __exit rm_kerneltimer(void)
{
    del_timer(&my_timer);
    sysfs_remove_file(kernel_kobj, &my_attr.attr);
    kobject_put(my_kobj);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove device done\n");
}

module_init(ex_cre_kerneltimer);
module_exit(rm_kerneltimer);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
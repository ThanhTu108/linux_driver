#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/err.h>
//device file
#include <linux/device.h>
#include <linux/kdev_t.h>

//file operations
#include <linux/cdev.h>

//sysfs
#include <linux/sysfs.h>
#include <linux/kobject.h> 

//file operations
static int __init create_itr_ex(void);
static void __exit remove_itr_ex(void);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off);
static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off);


volatile int val = 0;
dev_t dev_num;
static struct class* dev_class;
static struct device* dev_file;
static struct cdev my_cdev;
static struct kobject* my_kobj;

//sysfs function
        //ssize_t (*show)(struct kobject *kobj, struct kobj_attribute *attr,
        //			char *buf);
        // ssize_t (*store)(struct kobject *kobj, struct kobj_attribute *attr,
        // 		 const char *buf, size_t count);
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count);


//create attribute
//kobject_parent = kernel_kobj -> /sys/kernel/****/val_itr_sys
struct kobj_attribute attr_sysfs = __ATTR(val_itr_sys, 0660, sys_show, sys_store);
struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = open_fops,
    .release = release_fops,
    .read = read_fops,
    .write = write_fops,
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
    pr_info("Read function\n");
    return 0;
}
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off)
{
    pr_info("Write function\n");
    return 0;
}

static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    pr_info("\nSYS_SHOW - READ\n");
    return sprintf(buf, "Value = %d", val);
}
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    pr_info("\nSYS_STORE - READ\n");
    sscanf(buf, "%d", &val);
    return count;
}

static int __init create_itr_ex(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "itrrupt_num") < 0)
    {
        pr_err("Cannot allocate major minor number\n");
        return -1;
    }
    pr_info("Major(%d)\nMinor(%d)\n", MAJOR(dev_num), MINOR(dev_num));
    cdev_init(&my_cdev, &my_fops);
    if(cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("Cannot add device to system\n");
        goto r_class;
    }
    dev_class = class_create(THIS_MODULE, "itrrupt_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Create struct class err\n");
        goto r_class;
    }

    dev_file = device_create(dev_class, NULL, dev_num, NULL, "itrrupt_device");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    my_kobj = kobject_create_and_add("ex_itr", kernel_kobj);
    if(sysfs_create_file(my_kobj, &attr_sysfs.attr))
    {
        pr_err("Cannot create sysfs file......\n");
        goto r_sysfs;
    }

    pr_info("Interrupt ex done\n");
    return 0;
r_sysfs:
    kobject_put(my_kobj);
    sysfs_remove_file(kernel_kobj, &attr_sysfs.attr);
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev_num, 1);
    cdev_del(&my_cdev);
    return -1;
}

static void __exit remove_itr_ex(void)
{
    kobject_put(my_kobj);
    sysfs_remove_file(kernel_kobj, &attr_sysfs.attr);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove done\n");
}


module_init(create_itr_ex);
module_exit(remove_itr_ex);

MODULE_LICENSE("GPL");
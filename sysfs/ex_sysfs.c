#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>

//device file
#include <linux/device.h>
#include <linux/kdev_t.h>
//file operation
#include <linux/fs.h>
#include <linux/cdev.h>
//kmalloc/kfree
#include <linux/slab.h>
//copy_from/to_user
#include <linux/uaccess.h>

//sysfs
#include <linux/kobject.h>
#include <linux/sysfs.h> 

volatile int value = 0;
dev_t dev_num = 0;
static struct class* dev_class;
static struct device* dev_file;
static struct cdev cdev_fops;
static struct kobject* kobj_sysfs;


static int __init create_sysfs_function(void);
static void __exit remove_sysfs_function(void);

//File operations
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off);


//sysfs functions (kobject attribute)
static ssize_t sysfs_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t sysfs_store(struct kobject* kobj, struct kobj_attribute* attr,const char* buf, size_t count);
//kobject_parent = kernel_kobj -> /sys/kernel/****/val_sysfs
struct kobj_attribute attr_sysfs = __ATTR(val_sysfs, 0660, sysfs_show, sysfs_store);


struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = open_fops,
    .release = release_fops,
    .read = read_fops,
    .write = write_fops,
};

static ssize_t sysfs_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    pr_info("SYSFS_SHOW -- READ\n");
    return sprintf(buf, "Value = %d", value);
}
static ssize_t sysfs_store(struct kobject* kobj, struct kobj_attribute* attr,const char* buf, size_t count)
{
    pr_info("SYSFS_STORE -- WRITE\n");
    //buf: user
    sscanf(buf, "%d", &value);
    return count;
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
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off)
{
    pr_info("READ\n");
    return 0;
}
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n");
    return len;
}

static int __init create_sysfs_function(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "sysfs_mm") < 0)
    {
        pr_err("Cannot allocate major and minor number\n");
        return -1;
    }
    pr_info("MAJOR(%d)\nMINOR(%d)", MAJOR(dev_num), MINOR(dev_num));

    dev_class = class_create(THIS_MODULE, "sysfs_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }
    dev_file = device_create(dev_class, NULL, dev_num, NULL, "sysfs_file");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }

    cdev_init(&cdev_fops, &my_fops);
    cdev_fops.owner = THIS_MODULE;
    if(cdev_add(&cdev_fops, dev_num, 1) < 0)
    {
        pr_err("Error add fops to system\n");
        goto r_device;
    }
    //create directory in /sys/kernel/***/
    kobj_sysfs = kobject_create_and_add("ex_sysfs", kernel_kobj);
    //create sys file for val_sysfs
    if(sysfs_create_file(kobj_sysfs, &attr_sysfs.attr))
    {
        pr_err("Cannot create sysfs file\n");
        goto r_sysfs;
    }

    pr_info("Insert sysfs done\n");
    return 0;
r_sysfs:
    kobject_put(kobj_sysfs);
    sysfs_remove_file(kobj_sysfs, &attr_sysfs.attr);
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev_num, 1);
    return -1;
}
static void __exit remove_sysfs_function(void)
{
    kobject_put(kobj_sysfs);
    sysfs_remove_file(kobj_sysfs, &attr_sysfs.attr);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&cdev_fops);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove done\n");
}
module_init(create_sysfs_function);
module_exit(remove_sysfs_function);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");


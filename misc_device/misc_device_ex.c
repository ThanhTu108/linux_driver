#include <linux/fs.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/init.h>
//misc device
#include <linux/miscdevice.h>

//global value
static int val_sys_1 = 0;
static int val_sys_2 = 0;

//global prototype
static int __init cre_misc_device(void);
static void __exit rmv_misc_device(void);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off);


//function in sys
static ssize_t value1_show(struct device* dev, struct device_attribute* attr, char* buf);
static ssize_t value1_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count);
static ssize_t value2_show(struct device* dev, struct device_attribute* attr, char* buf);
static ssize_t value2_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count);


//sys
//create sys value
DEVICE_ATTR_RW(value1); // dev_attr_value1
/*
#define DEVICE_ATTR_RW(_name) \
	struct device_attribute dev_attr_##_name = __ATTR_RW(_name)

#define __ATTR_RW(_name) __ATTR(_name, 0644, _name##_show, _name##_store)
=> function must be name_show or name_store


struct device_attribute my_attr = __ATTR(name, 0644, name_show, name_store);
*/
DEVICE_ATTR_RW(value2);

static struct attribute *my_attr[] =
{
    &dev_attr_value1.attr,
    &dev_attr_value2.attr,
    NULL,
};

static const struct attribute_group my_group = 
{
    .attrs = my_attr,
};
static const struct attribute_group *my_groups[] = 
{
    &my_group,
    NULL,
};

static struct file_operations my_fops =
{
    .owner = THIS_MODULE,
    .open = open_fops, 
    .release = release_fops,
    .read = read_fops, 
    .write = write_fops,
};

struct miscdevice my_miscdev = 
{
    .minor = MISC_DYNAMIC_MINOR, 
    .name = "misc_device_example",
    .fops = &my_fops,
    .groups = my_groups,
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
    return len;
}

// static int val_sys_1 = 0;
// static int val_sys_2 = 0;
static ssize_t value1_show(struct device* dev, struct device_attribute* attr, char* buf)
{
    pr_info("Show 1\n");
    return sprintf(buf, "Value 1: %d\n", val_sys_1);
}
static ssize_t value1_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
    pr_info("Store 1\n");
    sscanf(buf, "%d", &val_sys_1);
    return count;
}
static ssize_t value2_show(struct device* dev, struct device_attribute* attr, char* buf)
{
    pr_info("Show_2\n");
    return sprintf(buf, "Value 1: %d\n", val_sys_2);
}
static ssize_t value2_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
    
    pr_info("Store 2\n");
    sscanf(buf, "%d", &val_sys_2);
    return count;
}

static int __init cre_misc_device(void)
{
    int ret = 0;
    ret = misc_register(&my_miscdev);
    if(ret)
    {
        pr_err("Cannot register misc device\n");
        return ret;
    }
    pr_info("Init misc device done\n");
    return 0;
}
static void __exit rmv_misc_device(void)
{
    misc_deregister(&my_miscdev);
    pr_info("Remove misc device done\n");
}

module_init(cre_misc_device);
module_exit(rmv_misc_device);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu108303@gmail.com");
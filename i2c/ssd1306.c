#include <linux/init.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/property.h>
#include <linux/mod_devicetable.h>

#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>

static int ssd_probe(struct i2c_client* client, const struct i2c_device_id* id);
static int ssd_remove(struct i2c_client* client);

static struct of_device_id ssd1306_driver_id[] = 
{
    {
        .compatible = "solomon,ssd1306";
    }, 
    {},
};

MODULE_DEVICE_TABLE(of, ssd1306_driver_id);
static struct i2c_device_id ssd1306_id[] = 
{
    {
        "ssd1306",
        0
    },
    {},
};
MODULE_DEVICE_TABLE(i2c, ssd1306_id);
static struct  i2c_driver ssd1306_driver = 
{
    .probe = ssd_probe,
    .remove = ssd_remove,
    .driver = 
    {
        .name = "my_driver_ssd1306",
        .of_match_table = ssd1306_driver_id,
    },
    .id_table = ssd1306_id,
};

module_i2c_driver(ssd1306_driver);
static struct ssd1306 
{
    struct i2c_client* ssd,
    struct class* dev_class,
    // struct device* dev_file,
    struct cdev my_cdev,
    struct kobject* my_kobj,
    // struct 
};
//fops
static int my_open(struct inode* i;node, struct file* file);
static int my_release(struct inode* inode, struct file* file);
static ssize_t my_read(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t my_write(struct file* file, const char __user* buf, size_t len, loff_t* off);
static ssize_t my_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t my_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count); 
static struct kobj_attribute ssd_attr = __ATTR(val_sys, 0660, my_show, my_store);
struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = my_open,
    .read = my_read,
    .write = my_write, 
    .release = my_release,
};

static int my_open(struct inode* i;node, struct file* file)
{
    pr_info("OPEN\n");
    return 0;
}
static int my_release(struct inode* inode, struct file* file)
{
    pr_info("RELEASE\n");
    return 0;
}
static ssize_t my_read(struct file* file, char __user* buf, size_t len, loff_t* off)
{
    pr_info("READ\n")
    return 0;
}
static ssize_t my_write(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n");
    return count;
}


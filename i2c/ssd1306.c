#include <linux/init.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/property.h>
#include <linux/mod_devicetable.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>

//define cmd ssd1306
#define SINGLE_CMD 0x00    //dc = 1, co = 0
#define SINGLE_DATA 0x40   //dc = 1, co = 1


static int ssd_probe(struct i2c_client* client, const struct i2c_device_id* id);
static void ssd_remove(struct i2c_client* client);

static struct of_device_id ssd1306_driver_id[] = 
{
    {
        .compatible = "solomon,ssd1306",
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

struct ssd1306_t
{
    struct i2c_client* client;
    dev_t dev_num;
    struct class* dev_class;
    // struct device* dev_file,
    struct cdev my_cdev;
    struct kobject* my_kobj;
    // struct 
};
//fops
static int my_open(struct inode* inode, struct file* file);
static int my_release(struct inode* inode, struct file* file);
static ssize_t my_read(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t my_write(struct file* file, const char __user* buf, size_t len, loff_t* off);
// static ssize_t my_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
// static ssize_t my_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count); 
// static struct kobj_attribute ssd_attr = __ATTR(val_sys, 0660, my_show, my_store);
struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = my_open,
    .read = my_read,
    .write = my_write, 
    .release = my_release,
};

static int my_open(struct inode* inode, struct file* file)
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
    pr_info("READ\n");
    return 0;
}
static ssize_t my_write(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n");
    return len;
}

// //function write
// static void i2c_write(bool is_cmd, )

static int ssd_probe(struct i2c_client* client, const struct i2c_device_id* id)
{   
    struct device* dev = &(client->dev);
    uint32_t height, width;
    struct ssd1306_t* ssd;
    ssd = devm_kmalloc(dev, sizeof(struct ssd1306_t), GFP_KERNEL);
    if(ssd == NULL)
    {
        pr_err("Cannot allocate memory for ssd\n");
        return -1;
    }
    i2c_set_clientdata(client, ssd);
    int ret;
    if(device_property_present(dev, "height") == false)
    {
        dev_err(dev, "'height' not found\n");
        return -1;
    }
    if(device_property_present(dev, "width") == false)
    {
        dev_err(dev,"'width' not found\n");
        return -1;
    }
    ret = device_property_read_u32(dev, "height", &height);
    if(ret)
    {
        dev_err(dev, "Cannot read height\n");
        return -1;
    }
    ret = device_property_read_u32(dev, "width", &width);
    if(ret)
    {
        dev_err(dev, "Cannot read width\n");
        return -1;
    }
    dev_info(dev, "Height = %d\tWidth = %d\n", height, width);
    if(alloc_chrdev_region(&ssd->dev_num, 0, 1, "ssd_num") < 0)
    {
        pr_err("Cannot create major minor number\n");
        return -1;
    }
    pr_info("Major(%d) \t Minor(%d)\n", MAJOR(ssd->dev_num), MINOR(ssd->dev_num));
    ssd->dev_class = class_create(THIS_MODULE, "ssd_class");
    if(IS_ERR(ssd->dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }
    if(IS_ERR(device_create(ssd->dev_class, NULL, ssd->dev_num, NULL, "ssd_device")))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    cdev_init(&ssd->my_cdev, &my_fops);
    if(cdev_add(&ssd->my_cdev, ssd->dev_num, 1) < 0)
    {
        pr_err("Cannot add device to system\n");
        goto r_class;
    }
    pr_info("Insert done\n");
    return 0;
r_device:
    class_destroy(ssd->dev_class);
r_class:
    cdev_del(&ssd->my_cdev);
    unregister_chrdev_region(ssd->dev_num, 1);
    return -1;
}
static void ssd_remove(struct i2c_client* client)
{
    struct ssd1306_t* ssd = i2c_get_clientdata(client);
    device_destroy(ssd->dev_class, ssd->dev_num);
    class_destroy(ssd->dev_class);
    cdev_del(&ssd->my_cdev);
    unregister_chrdev_region(ssd->dev_num, 1);
    pr_info("remove i2c ssd1306 done\n");
}

module_i2c_driver(ssd1306_driver);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
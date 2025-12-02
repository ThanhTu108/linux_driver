#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/property.h>
#include <linux/mod_devicetable.h>

//gpiod
#include <linux/gpio/consumer.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/proc_fs.h>

//global variables
    //gpiod
uint8_t gpio_status = 0;
static struct gpio_desc* my_led = NULL;
dev_t dev_num = 0;
static struct class* dev_class;
static struct cdev my_cdev;
static struct kobject* my_kobj;
static struct proc_dir_entry *proc_file;

//global protocol
static int gpiod_probe(struct platform_device* pdev);
static int gpiod_remove(struct platform_device* pdev);
static struct of_device_id my_driver_id[] = 
{
    {
        .compatible = "gpiod, gpio8_11",
    },
    {}
};
MODULE_DEVICE_TABLE(of, my_driver_id);
static struct platform_driver my_driver = 
{
    .probe = gpiod_probe,
    .remove = gpiod_remove,
    .driver = 
    {
        .name = "my_driver_gpiod",
        .of_match_table = my_driver_id,
    },
};

    //fops
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off);
    //proc_fs
static int open_proc(struct inode* inode, struct file* file);
static int release_proc(struct inode* inode, struct file* file);
static ssize_t read_proc(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t write_proc(struct file* file, const char __user* buf, size_t len, loff_t* off);

    //kobj
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count);
struct kobj_attribute my_attr = __ATTR(val_sys_dt, 0660, sys_show, sys_store);

static struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = open_fops,
    .read = read_fops, 
    .write = write_fops,
    .release = release_fops, 
};

static struct proc_ops my_proc = 
{
    .proc_open = open_proc,
    .proc_read = read_proc, 
    .proc_write = write_proc,
    .proc_release = release_proc, 
};



static int open_fops(struct inode* inode, struct file* file)
{
    pr_info("OPEN FOPS\n");
    return 0;
}
static int release_fops(struct inode* inode, struct file* file)
{
    pr_info("RELEASE FOPS\n");
    return 0;
}
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off)
{
    len = 1;
    gpio_status = gpiod_get_value(my_led);
    pr_info("Status led fops: %d", gpio_status);
    if(copy_to_user(buf, &gpio_status, len))
    {
        pr_err("Cannot read\n");
    }
    return 0;
}
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    uint8_t value[10] = {0};
    if(copy_from_user(&value[0], buf, len))
    {
        pr_err("Cannot write\n");
    }
    pr_info("Set gpio fops: %c\n", value[0]);
    if(value[0] == '1')
    {
        gpiod_set_value(my_led, 1);
    }
    else if(value[0] == '0')
    {
        gpiod_set_value(my_led, 0);
    }
    else 
    {
        pr_info("Value err\n");
    }
    return len;
}
    //proc_fs
static int open_proc(struct inode* inode, struct file* file)
{
    pr_info("OPEN PROC\n");
    return 0;
}
static int release_proc(struct inode* inode, struct file* file)
{
    pr_info("RELEASE PROC\n");
    return 0;
}
static ssize_t read_proc(struct file* file, char __user* buf, size_t len, loff_t* off)
{
    len = 1;
    gpio_status = gpiod_get_value(my_led);
    pr_info("Status led proc: %d", gpio_status);
    if(copy_to_user(buf, &gpio_status, len))
    {
        pr_err("Cannot read\n");
    }
    return 0;
}
static ssize_t write_proc(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    uint8_t value[10] = {0};
    if(copy_from_user(&value[0], buf, len))
    {
        pr_err("Cannot write\n");
    }
    pr_info("Set gpio proc: %c\n", value[0]);
    if(value[0] == '1')
    {
        gpiod_set_value(my_led, 1);
    }
    else if(value[0] == '0')
    {
        gpiod_set_value(my_led, 0);
    }
    else 
    {
        pr_info("Value err\n");
    }
    return len;
}

static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{

    pr_info("SYS_SHOW: READ\n");
    gpio_status = gpiod_get_value(my_led);
    return sprintf(buf, "Gpio_status_sys: %d\n", gpio_status);
}
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    pr_info("SYS_STORE: WRITE\n");
    sscanf(buf, "%hhd", &gpio_status);
    gpiod_set_value(my_led, gpio_status);
    return count;
}

static int gpiod_probe(struct platform_device* pdev)
{
    struct device* dev = &pdev->dev;
    const char* label;
    int my_value, ret;
    pr_info("Probe function\n");
    if(device_property_present(dev, "label") == false)
    {
        pr_err("'Label' not found\n");
        return -1;
    }
    if(device_property_present(dev, "val") == false)
    {
        pr_err("'Val' not found");
        return -1;
    }
    ret = device_property_read_string(dev, "label", &label);
    if(ret)
    {
        pr_err("Cannot read string 'label'\n");
        return -1;
    }
    ret = device_property_read_u32(dev, "val", &my_value);
    if(ret)
    {
        pr_err("Cannot read value 'val'\n");
        return -1;
    }
    pr_info("Labels: %s\n", label);
    pr_info("Value: %d\n", my_value);
    //init gpio
    my_led = gpiod_get(dev, "led", GPIOD_OUT_LOW);
    if(IS_ERR(my_led))
    {
        pr_info("Cannot get descriptor gpio\n");
        goto r_gpio;
    }
    //create cdev, device, class, sysfs, proc
    if(alloc_chrdev_region(&dev_num, 0, 1, "gpiod_num") < 0)
    {
        pr_info("Cannot create major, minor\n");
        return -1;
    }
    pr_info("MAJOR(%d), MINOR(%d) \n", MAJOR(dev_num), MINOR(dev_num));
    cdev_init(&my_cdev, &my_fops);
    if(cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("Cannot add device to system\n");
        goto r_class;
    }
    dev_class = class_create(THIS_MODULE, "gpiod_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }
    if(IS_ERR(device_create(dev_class, NULL, dev_num, NULL, "gpiod_device")))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    my_kobj = kobject_create_and_add("gpiod_sysfs", kernel_kobj);
    if(sysfs_create_file(my_kobj, &my_attr.attr))
    {
        pr_err("Cannot create sysfs file\n");
        goto r_sysfs;
    }
    proc_file = proc_create("my_led", 0660, NULL, &my_proc);
    if(proc_file == NULL)
    {
        pr_err("Cannot create /proc/my_led\n");
        goto r_gpio;
    }
    pr_info("Create probe done\n");
    return 0;
r_gpio:
    gpiod_put(my_led);
r_sysfs:
    sysfs_remove_file(my_kobj, &my_attr.attr);
    kobject_put(my_kobj);
    device_destroy(dev_class, dev_num);
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev_num, 1);
    return -1;
}
static int gpiod_remove(struct platform_device* pdev)
{
    gpiod_put(my_led);
    sysfs_remove_file(my_kobj, &my_attr.attr);
    kobject_put(my_kobj);
    proc_remove(proc_file);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    unregister_chrdev_region(dev_num, 1);
    return 0;
}
module_platform_driver(my_driver);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
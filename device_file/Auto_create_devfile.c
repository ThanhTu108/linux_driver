#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/err.h>

//Device file
#include <linux/device.h>
#include <linux/kdev_t.h>


dev_t dev_number = 0;
static struct class* dev_class;
static struct device* dev_file;


static int __init create_devfile(void)
{
    if(alloc_chrdev_region(&dev_number, 0, 1, "auto_create_devfile") < 0)
    {
        pr_err("Cannot allocate major minor number");
        return -1;  
    }
    pr_info("Major(%d), Minor(%d)", MAJOR(dev_number), MINOR(dev_number));

    dev_class = class_create(THIS_MODULE, "Class_dev");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }
    dev_file = device_create(dev_class, NULL, dev_number, NULL, "dev_file");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create device file");
        goto r_device;
    }


    return 0;
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev_number, 1);
    return -1;
}

static void __exit device_func_exit(void)
{
    device_destroy(dev_class, dev_number);
    class_destroy(dev_class);
    unregister_chrdev_region(dev_number, 1);
    pr_info("Clean device");
}

module_init(create_devfile);
module_exit(device_func_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("simple driver for create device file automatically create device file");
MODULE_AUTHOR("GPL");
MODULE_VERSION("1.0");
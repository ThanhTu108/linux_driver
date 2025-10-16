#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>   //major minor number
#include <linux/init.h>


dev_t dev = 0;

static int __init create_dev_file(void)
{
    if(alloc_chrdev_region(&dev, 0, 1, "static_device_mm") < 0)
    {
        printk(KERN_ERR "Cannot allocate major minor");
        return -1;
    }
    printk(KERN_INFO "MAJOR(%d) MINOR(%d)", MAJOR(dev), MINOR(dev));
    return 0;
}

static void __exit exit_create_dev_file(void)
{
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "Remove major minor done");
}

module_init(create_dev_file);
module_exit(exit_create_dev_file);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
MODULE_DESCRIPTION("simple driver for create device file");

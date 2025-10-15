#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

dev_t dev = 0;

static int __init allocate_MM(void)
{
    if(alloc_chrdev_region(&dev, 0, 1, "dynamic_MM") < 0)
    {
        printk(KERN_INFO "Cannot allocate major minor number");
        return -1;
    }
    printk(KERN_INFO "MAJOR(%d), MINOR(%d)", MAJOR(dev), MINOR(dev));
    printk(KERN_INFO "init complete");
    return 0;
}

static void __exit exit_MM(void)
{
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "Unregister major minor number");
}

module_init(allocate_MM);
module_exit(exit_MM);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
MODULE_DESCRIPTION("simple driver (dynamic allocate mm number)");

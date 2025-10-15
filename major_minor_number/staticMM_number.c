#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>   

dev_t dev = MKDEV(235,0);

static int __init static_mm(void)
{
    register_chrdev_region(dev, 1, "static_driver");
    printk(KERN_INFO "MAJOR = %d, MINOR = %d", MAJOR(dev), MINOR(dev));
    printk(KERN_INFO "init done");
    return 0;
}


static void __exit exit_static(void)
{
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "exit funtion static mm number");
}

module_init(static_mm);
module_exit(exit_static);

MODULE_AUTHOR("thanhtu10803@gmail.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("simple driver (static allocation major and minor number)");

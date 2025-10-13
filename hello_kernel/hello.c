
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>

static int __init hello_world_init(void)
{
    printk(KERN_INFO "hello world\n");
    return 0;
}

static void __exit hello_world_exit(void)
{
    printk(KERN_INFO "bye bye\n");
}
 
module_init(hello_world_init);
module_exit(hello_world_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
MODULE_DESCRIPTION("A simple driver");
MODULE_VERSION("1.0");
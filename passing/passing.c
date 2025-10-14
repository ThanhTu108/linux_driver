#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

int val;
int arr_val[10] = {0};
char* name;
int cb_val = 0;

module_param(val, int, S_IRUSR|S_IWUSR);
module_param_array(arr_val, int, NULL, S_IRUSR|S_IWUSR);
module_param(name, charp, S_IRUSR|S_IWUSR);

int my_param_set_int(const char* value,const struct kernel_param* kp)
{
    int res;
    res = param_set_int(value, kp);
    if(res == 0)
    {
        printk(KERN_INFO "Callback function is call");
        printk(KERN_INFO "Cb_val = %d\n", *(int* )kp->arg);
        return 0;
    }
    return -1;
}

const struct kernel_param_ops ops = 
{
    .get = &param_get_int,
    .set = &my_param_set_int,
};

module_param_cb(cb_val, &ops, &cb_val, S_IRUSR|S_IWUSR);

static int __init passing_arguments(void)
{
    int i;
    printk(KERN_INFO "Value = %d\n", val);
    printk(KERN_INFO "Name: %s\n", name);
    printk(KERN_INFO "Callback: %d\n", cb_val);
    for(i = 0; i < sizeof(arr_val)/sizeof(int); i++)
    {
        printk(KERN_INFO "Arr[%d]: %d\n", i, arr_val[i]);
    }
    return 0;
}

static void __exit exit_passing(void)
{
    printk(KERN_INFO "Exit function");
}

module_init(passing_arguments);
module_exit(exit_passing);

MODULE_AUTHOR("thanhtu10803@gmail.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("simple driver for passing arguments");
MODULE_VERSION("1.0");
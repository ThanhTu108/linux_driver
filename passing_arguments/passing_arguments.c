#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

int value;
int arr_val[4];
int cb_val = 0;
char* name_val;

module_param(value, int, S_IRUSR|S_IWUSR);

//charp: is meaning ....pointer
module_param(name_val, charp, S_IRUSR|S_IWUSR);
module_param_array(arr_val, int, NULL, S_IRUSR|S_IWUSR);

int notify_param_cb(const char* val, const struct kernel_param* kp)
{
    int res = param_set_int(val, kp);
    if(res == 0)
    {
        printk(KERN_INFO "call back is call\n");
        printk(KERN_INFO "New value cb_val = %d\n", cb_val);
    }
    return -1;
}

const struct kernel_param_ops ops =
{
    .set = notify_param_cb,
    .get = &param_get_int,
};

module_param_cb(cb_val, &ops, &cb_val, S_IRUGO|S_IWUSR);

static int __init param_func(void)
{
    int i; 

    printk(KERN_INFO "Value = %d\n", value);
    printk(KERN_INFO "Value_cb = %d\n", cb_val);
    printk(KERN_INFO "Name = %s\n", name_val);
    for(i = 0; i < sizeof(arr_val)/sizeof(int); i++)
    {
        printk(KERN_INFO "Arr[%d]: = %d\n", i, *(arr_val+i));
    }
    printk(KERN_INFO "Done");
    return 0;
}

static void __exit exit_func(void)
{
    printk(KERN_INFO "exit func.........\n");
}

module_init(param_func);
module_exit(exit_func);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("simple param function");


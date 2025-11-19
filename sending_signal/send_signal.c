#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>

//device file
#include <linux/device.h>
#include <linux/kdev_t.h>
//fops
#include <linux/cdev.h>
#include <linux/fs.h>
//interrupts
#include <linux/interrupt.h>
//ioctl
#include <linux/ioctl.h>
//sysfs
#include <linux/sysfs.h>
#include <linux/kobject.h>
//copy_from/to_user
#include <linux/uaccess.h>
#include <linux/slab.h>
//task struct
#include <linux/sched.h>
//signal
#include <linux/signal.h>

//num interrupts
#define IRQ_NO 51

//ioctl magic
#define TASK_CURRENT _IOW('a', 'a', int32_t*)

//decide signal to send
#define SIGTX 44 

static struct task_struct *task = NULL;
static int signum = 0;

volatile int val_sys = 0;
dev_t dev_num;
static struct class* dev_class;
static struct device* dev_file; 
static struct cdev my_cdev;
static struct kobject* my_kobj; 


static int __init ex_create_sendsignal(void);
static void __exit ex_rmv_sendsignal(void);
static int open_fops(struct inode* inode, struct file*file); 
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off);
static long ioctl_fops(struct file* file, unsigned int cmd, unsigned long arg);

static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count);
static struct kobj_attribute my_attr = __ATTR(val_sys_sendsignal, 0660, sys_show, sys_store);

//itr function
static irqreturn_t irq_handler(int irq, void* dev_id);
static struct file_operations my_fops =
{
    .owner = THIS_MODULE,
    .open = open_fops,
    .release = release_fops,
    .read = read_fops,
    .write = write_fops,
    .unlocked_ioctl = ioctl_fops,
};

static int open_fops(struct inode* inode, struct file* file)
{
    pr_info("OPEN\n");
    return 0;
}
static int release_fops(struct inode* inode, struct file* file)
{
    pr_info("RELEASE\n");
    struct task_struct* _temp = get_current();
    if(_temp == task)
    {
        task = NULL;
    }
    return 0;
}

static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off)
{
    pr_info("READ\n");
    generic_handle_irq(IRQ_NO);
    return 0;
}
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n");
    return len;
}
static long ioctl_fops(struct file*file ,unsigned int cmd, unsigned long arg)
{
    if(cmd == TASK_CURRENT)
    {
        pr_info("TASK CURRENT: \n");
        task = get_current();
        signum = SIGTX;
    }
    return 0;
}

static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    pr_info("SYS_SHOW: READ\n");
    return sprintf(buf, "%d\n", val_sys);
}
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    pr_info("SYS_STORE: WRITE\n");
    sscanf(buf, "%d", &val_sys);
    return count;
}

static irqreturn_t irq_handler(int irq, void* dev_id)
{
    struct kernel_siginfo info;
    memset(&info, 0, sizeof(struct kernel_siginfo));
    pr_info("Call irq\n");
    info.si_signo = SIGTX;
    info.si_code = SI_QUEUE,
    info.si_int = val_sys;
    if(task != NULL)
    {
        pr_info("Send signal to application\n");
        if(send_sig_info(SIGTX, &info, task) < 0)
        {
            pr_err("Cannot send signal to app\n");
        }
    }
    return IRQ_HANDLED;
}


static int __init ex_create_sendsignal(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "send_sig_num") < 0)
    {
        pr_err("Cannot create major, minor\n");
        return -1;
    }
    pr_info("MAJOR(%d), MINOR(%d)", MAJOR(dev_num), MINOR(dev_num));

    cdev_init(&my_cdev, &my_fops);
    if(cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("Cannot add device to the system\n");
        goto r_class;
    }
    dev_class = class_create(THIS_MODULE, "send_sig_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }
    dev_file = device_create(dev_class, NULL, dev_num, NULL, "send_sig_device");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    my_kobj = kobject_create_and_add("sys_send", kernel_kobj);
    if(sysfs_create_file(my_kobj, &my_attr.attr))
    {
        pr_err("Cannot create sys file\n");
        goto r_sysfs;
    }
    if(request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "irq_sendsignal", (void*)irq_handler))
    {
        pr_err("Cannot register irq\n");
        goto r_sysfs;
    }
    pr_info("Insert send signal from linux kernel to app done\n");
    return 0;
r_sysfs:
    sysfs_remove_file(kernel_kobj, &my_attr.attr);
    kobject_put(my_kobj);
    device_destroy(dev_class, dev_num);
r_device:
    class_destroy(dev_class);
r_class:
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return -1;
}
static void __exit ex_rmv_sendsignal(void)
{
    free_irq(IRQ_NO, (void*) irq_handler);
    sysfs_remove_file(kernel_kobj, &my_attr.attr);
    kobject_put(my_kobj);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove send signal from linux kernel to app done\n");
}

module_init(ex_create_sendsignal);
module_exit(ex_rmv_sendsignal);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
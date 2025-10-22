#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
//file operation
#include <linux/fs.h>
#include <linux/cdev.h>
//device file
#include <linux/kdev_t.h>
#include <linux/device.h>
//kmalloc, kfree
#include <linux/slab.h>
#include <linux/uaccess.h>  //copy

//Waitqueue
#include <linux/wait.h>

//thread
#include <linux/kthread.h>
//create thread, queue
DECLARE_WAIT_QUEUE_HEAD(wait_queue_task);
static struct task_struct* wait_thread;
int flag_queue = 0;

uint32_t count;

dev_t dev_num = 0;
static struct class* dev_class;
static struct device* dev_file;
static struct cdev cdev_fops;

char kernel_buff[20];

static int __init create_waitqueue(void);
static void __exit remove_waitqueue(void);
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static long ioctl_fops(struct file* file, unsigned int cmd, unsigned long arg);

static int wait_function(void* unused)
{
    while(1)
    {
        pr_info("Wait event....\n");
        wait_event_interruptible(wait_queue_task, flag_queue!=0);
        if(flag_queue == 3)
        {
            pr_info("Go to exit function");
            return 0;
        } 
        if(flag_queue == 2)
        {
            pr_info("Write function event");
        }
        pr_info("Read data: %d", ++count);
        if(count > 255) count = 0;
        flag_queue = 0;
    }
    do_exit(0);
    return 0;
}

static struct file_operations my_fops = 
{
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
    return 0;
}

static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off)
{
    pr_info("Read function\n");
    flag_queue = 1;
    wake_up_interruptible(&wait_queue_task);
    return 0;
}

static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("String to write: \n");
    if(copy_from_user(kernel_buff, buf, sizeof(kernel_buff)))
    {
        pr_err("Write err!!!");
    }
    pr_info("%s ", kernel_buff);
    flag_queue = 2;
    wake_up_interruptible(&wait_queue_task);
    return len;
}

static long ioctl_fops(struct file* file, unsigned int cmd, unsigned long arg)
{
    pr_info("ioctl\n");
    return 0;
}

static int __init create_waitqueue(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "wait_queue_mm") < 0)
    {
        pr_err("Cannot allocate major, minor number\n");
        return -1;
    }
    pr_info("\tMajor(%d)\n\tMinor(%d)", MAJOR(dev_num), MINOR(dev_num));
    dev_class = class_create(THIS_MODULE, "wait_queue_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }

    dev_file = device_create(dev_class, NULL, dev_num, NULL, "wait_queue_devfile");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    cdev_init(&cdev_fops, &my_fops);
    cdev_fops.owner = THIS_MODULE;
    cdev_fops.ops = &my_fops;
    if(cdev_add(&cdev_fops, dev_num, 1) < 0)
    {
        pr_err("Err add device file to system\n");
        goto r_class;
    }


    //create kernel thread
    wait_thread = kthread_create(wait_function,NULL, "Wait_queue_thread");
    if(wait_thread)
    {
        pr_info("Thread create successfully\n");
        wake_up_process(wait_thread);
    }
    else
    {
        pr_err("Thread create fail\n");
    }
    pr_info("Done \n");
    return 0;

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev_num, 1);
    return -1;
}

static void __exit remove_waitqueue(void)
{
    flag_queue = 2;
    wake_up_interruptible(&wait_queue_task);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&cdev_fops);
    unregister_chrdev_region(dev_num, 1);
    pr_info("remove done\n");
}

module_init(create_waitqueue);
module_exit(remove_waitqueue);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803");
MODULE_DESCRIPTION("Simple driver wait queue and thread process");
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>

//file operations
#include <linux/fs.h>
#include <linux/cdev.h>


//device file
#include <linux/device.h>
#include <linux/kdev_t.h>

//kmalloc/ kfree
#include <linux/slab.h>
//copy_from/to_user
#include <linux/uaccess.h>
//waitqueue
#include <linux/wait.h>
#include <linux/kthread.h>

int count = 0;
static struct task_struct* dynamic_queue_thread;

dev_t dev_num = 0;
struct class* dev_class;
struct device* dev_file;
struct cdev cdev_fops;
// - wait_queue_head_t name;
// - initialize using `init_waitqueue_head(&name)`
wait_queue_head_t dynamic_queue;

char data[20];
int flag_queue = 0;

static int __init create_dynamic_waitqueue(void);
static void __exit remove_dynamic_waitqueue(void);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file,const char __user* buf, size_t len, loff_t* off);

struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = open_fops,
    .read = read_fops,
    .write = write_fops,
    .release = release_fops,
};

static int wait_function(void* )
{
    while(1)
    {
        pr_info("Sleep..............\n");
        count++;
        
        wait_event_interruptible(dynamic_queue, flag_queue!=0);
        if(flag_queue == 3)
        {
            pr_info("Count = %d\n", count);
            pr_info("Event rmmod is call !!!\n");
            return 0;
        }
        else if(flag_queue == 1)
        {
            pr_info("Count = %d\n", count);
            pr_info("EVENT READ\n");      
        }
        else if(flag_queue == 2)
        {
            pr_info("Count = %d\n", count);
            pr_info("EVENT WRITE\n");
        }
        flag_queue = 0;
    }
    do_exit(0);
    return 0;
}
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
    if(copy_to_user(buf, data, sizeof(data)))
    {
        pr_err("Read : Err\n");
    }
    flag_queue = 1;
    wake_up_interruptible(&dynamic_queue);
    return 0;
}
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    if(copy_from_user(buf, data, sizeof(data)))
    {
        pr_err("Write: Err\n");
    }
    flag_queue = 2;
    wake_up_interruptible(&dynamic_queue);
    return sizeof(data);
}

static int __init create_dynamic_waitqueue(void)
{

    
    if(alloc_chrdev_region(&dev_num, 0, 1, "dynamic_queue_mm") < 0)
    {
        pr_err("Cannot allocate major minor number (dynamic queue mm)");
        return - 1;
    }

    pr_info("\tMajor(%d)\n\tMinor(%d)", MAJOR(dev_num), MINOR(dev_num));
    
    dev_class = class_create(THIS_MODULE, "dynamic_queue_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;q
    }

    dev_file = device_create(dev_class, NULL, dev_num, NULL, "dynamic_waitqueue_file");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }

    cdev_init(&cdev_fops, &my_fops);
    if(cdev_add(&cdev_fops, dev_num, 1))
    {
        pr_err("Err add cdev to system\n");
        goto r_class;
    }
    init_waitqueue_head(&dynamic_queue);
    dynamic_queue_thread = kthread_create(wait_function, NULL, "Dynamic_Queue_Thread");
    if(dynamic_queue_thread)
    {
        pr_info("Create thread successfull\n");
        wake_up_process(dynamic_queue_thread);
    }
    else
    {
        pr_err("Thread create fail\n");
    }
    pr_info("Init dynamic waitqueue done\n");
    return 0;
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev_num, 1);
    return -1;
}

static void __exit remove_dynamic_waitqueue(void)
{
    flag_queue = 3;
    wake_up_interruptible(&dynamic_queue);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&cdev_fops);
    unregister_chrdev_region(dev_num, 1);
    pr_info("remove done\n");
}

module_init(create_dynamic_waitqueue);
module_exit(remove_dynamic_waitqueue);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
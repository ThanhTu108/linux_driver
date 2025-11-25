#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
//device file
#include <linux/kdev_t.h>
#include <linux/device.h>
//fops
#include <linux/fs.h>
#include <linux/cdev.h>
//kthread
#include <linux/kthread.h>
#include <linux/sched.h>    //task_struct
#include <linux/delay.h>
#include <linux/atomic.h>

//global variable
dev_t dev_num = 0;
struct class* dev_class;
struct cdev my_cdev;
//atomic
atomic_t val_atomic = ATOMIC_INIT(0);
unsigned int check_bit = 0;
//thread
struct task_struct* thread_1;
struct task_struct* thread_2;

//global function
static int __init cre_atomic(void);
static void __exit rmv_atomic(void);

static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off);

static int thread_1_fn(void* pv);
static int thread_2_fn(void* pv);

struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = open_fops,
    .release = release_fops,
    .read = read_fops,
    .write = write_fops,
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
    pr_info("READ\n");
    return 0;
}
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n");
    return len;
}

static int thread_1_fn(void* pv)
{
    unsigned int prev_val = 0;
    while(!kthread_should_stop())
    {
        atomic_inc(&val_atomic);
        prev_val = test_and_change_bit(1, (void*)&check_bit);
        pr_info("THREAD 1:");
        pr_info("\tValue atomic = %u\t", atomic_read(&val_atomic));
        pr_info("Bit: %u\n", prev_val);
        msleep(1000);
    }
    return 0;
}
static int thread_2_fn(void* pv)
{
    unsigned int prev_val = 0;
    while(!kthread_should_stop())
    {
        atomic_inc(&val_atomic);
        prev_val = test_and_change_bit(1, (void*)&check_bit);
        pr_info("THREAD 2:");
        pr_info("\tValue atomic = %u\t", atomic_read(&val_atomic));
        pr_info("Bit: %u\n", prev_val);
        msleep(1000);
    }
    return 0;
}


static int __init cre_atomic(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "atomic_num") < 0)
    {
        pr_err("Cannot create major minor number\n");
        return -1;
    }
    pr_info("MAJOR(%d), MINOR(%d)\n", MAJOR(dev_num), MINOR(dev_num));

    cdev_init(&my_cdev, &my_fops);
    if(cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("Cannot add device to system\n");
        goto r_class;
    }
    dev_class = class_create(THIS_MODULE, "atomic_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }
    if(IS_ERR(device_create(dev_class, NULL, dev_num, NULL, "atomic_device")))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    thread_1 = kthread_run(thread_1_fn, NULL, "thread1");
    if(thread_1)
    {
        pr_info("Thread 1 successful\n");
    }
    else
    {
        pr_err("Create thread 1 fail\n");
        goto r_device;
    }

    thread_2 = kthread_run(thread_2_fn, NULL, "thread2");
    if(thread_2)
    {
        pr_info("Thread 2 successful\n");
    }
    else
    {
        pr_err("Create thread 2 fail\n");
        goto r_device;
    }
    pr_info("Insert done\n");
    return 0;
r_device: 
    device_destroy(dev_class, dev_num); 
    class_destroy(dev_class);
r_class:
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return -1;
}
static void __exit rmv_atomic(void)
{
    kthread_stop(thread_1);
    kthread_stop(thread_2);
    device_destroy(dev_class, dev_num); 
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove done\n");
}

module_init(cre_atomic);
module_exit(rmv_atomic);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
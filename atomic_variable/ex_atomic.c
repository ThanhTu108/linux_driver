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
        pr_info("Value atomic = %d\t", atomic_read(&val_atomic));
        pr_info("Bit: %u\n", prev_val);
        msleep(1000);
    }
    return 0;
}
static int thread_2_fn(void* pv);


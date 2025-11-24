#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>
//device file
#include <linux/device.h>
#include <linux/kdev_t.h>
//sysfs
#include <linux/kobject.h>
#include <linux/sysfs.h>
//fops
#include <linux/fs.h>
#include <linux/cdev.h>

//completion
#include <linux/completion.h>
//thread
#include <linux/kthread.h>
#include <linux/sched.h>    //task_struct

//global variable
dev_t dev_num;
static struct class* dev_class;
static struct cdev my_cdev;
static struct kobject* my_kobj;

//Complete
DECLARE_COMPLETION(my_complete);
int complete_flag = 0;
volatile int val_sys = 0;
volatile int val_complete = 0;
//thread
static struct task_struct* thread1;
int thread_1_fn(void* pv);
static int __init cre_completion(void);
static void __exit rmv_completion(void);
static int open_fops(struct inode* inode,  struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off);
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count);
static struct kobj_attribute my_attr = __ATTR(val_static_completion, 0660, sys_show, sys_store);

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
static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off)
{
    pr_info("READ\n");
    complete_flag = 1;
    complete(&my_complete);
    return 0;
}
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n");
    return len;
}

static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    pr_info("SYS_SHOW: read\n");
    return sprintf(buf, "%d\n", val_sys);
}
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    pr_info("SYS_STORE: write\n");
    sscanf(buf, "%d", &val_sys);
    return count;
}

int thread_1_fn(void* pv)
{
    while(!kthread_should_stop())
    {
        pr_info("Thread function wait for completion\n");
        wait_for_completion(&my_complete);
        if(complete_flag == 2)
        {
            pr_info("Exit\n");
            return 0;
        }
        pr_info("Read is: %d\n", val_complete++);
        complete_flag = 0;
        reinit_completion(&my_complete);
    }
    // do_exit(0);
    return 0;
}

static int __init cre_completion(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "completion_num") < 0)
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
    dev_class = class_create(THIS_MODULE, "completion_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }
    if(IS_ERR(device_create(dev_class, NULL, dev_num, NULL, "completion_device")))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    my_kobj = kobject_create_and_add("sys_complete", kernel_kobj);
    if(sysfs_create_file(my_kobj, &my_attr.attr))
    {
        pr_err("Cannot create sysfs file\n");
        goto r_sys;
    }
    thread1 = kthread_run(thread_1_fn, NULL, "thread1");
    if(thread1)
    {
        pr_info("Thread 1 successful\n");
    }
    else
    {
        pr_err("Create thread 1 fail\n");
        goto r_device;
    }
    pr_info("Insert device done \n");
    return 0;
r_sys:
    sysfs_remove_file(my_kobj, &my_attr.attr);
    kobject_put(my_kobj);
    device_destroy(dev_class, dev_num);
r_device:  
    class_destroy(dev_class);
r_class:
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return -1;
}
static void __exit rmv_completion(void)
{
    complete_flag = 2;
    complete(&my_complete);
    kthread_stop(thread1);
    sysfs_remove_file(my_kobj, &my_attr.attr);
    kobject_put(my_kobj);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove done\n");
}

module_init(cre_completion);
module_exit(rmv_completion);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
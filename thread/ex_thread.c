#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>
//file operation
#include <linux/fs.h>
#include <linux/cdev.h>
//device file
#include <linux/device.h>
#include <linux/kdev_t.h>
//kernel thread
#include <linux/kthread.h>
#include <linux/sched.h>    //task_struct
//copy_from/to_user
#include <linux/slab.h>
#include <linux/delay.h>
//sysfs
#include <linux/kobject.h>
#include <linux/sysfs.h>
//workqueue
#include <linux/workqueue.h>
//kmalloc
#include <linux/slab.h>
//interrupt
#include <linux/interrupt.h>
//kernel thread
#include <linux/kthread.h>
#define IRQ_NO 51
volatile int value_itr = 0;
volatile int value_sys = 0;
dev_t dev_num = 0;
static struct class* dev_class;
static struct device* dev_file;
static struct cdev my_cdev;
static struct kobject* my_kobj;

//thread
static struct task_struct* my_thread;
//Linklist
LIST_HEAD(my_list);
struct my_node
{
    int data;
    struct list_head list; 
};
static int __init _create_thread_function(void);
static void __exit __remove_thread_function(void);

//function prototypes
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off);
static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off);
//sysfs
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count);
struct kobj_attribute my_attr = __ATTR(val_ex_thread, 0660, sys_show, sys_store);

//workqueue 
static struct workqueue_struct* own_wq;
static void work_fn(struct work_struct* work);
DECLARE_WORK(my_work, work_fn);

//thread function
static int thread_fn(void* pv);
struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = open_fops,
    .read = read_fops, 
    .write = write_fops,
    .release = release_fops,
};

static void work_fn(struct work_struct* work)
{
    struct my_node* _temp = NULL;
    pr_info("Add new node with interrupt\n");
    _temp = kmalloc(sizeof(struct my_node), GFP_KERNEL);
    _temp->data = value_itr;
    INIT_LIST_HEAD(&_temp->list);
    list_add_tail(&_temp->list, &my_list);
}

static irqreturn_t irq_handler(int irq, void* dev_id)
{
    pr_info("Call irq\n");
    queue_work(own_wq, &my_work);
    return IRQ_HANDLED;
}
static int thread_fn(void* pv)
{
    //thread_should_stop return true if call thread_stop
    while(!kthread_should_stop())
    {
        pr_info("Thread is running\n");
        msleep(2000);
    }
    pr_info("Thread is stopping\n");
    return 0;
}

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
    struct my_node* _temp;
    int cnt = 0;
    list_for_each_entry(_temp, &my_list, list)
    {
        pr_info("Node[%d]: %d\n", cnt++, _temp->data);
    }
    pr_info("Number of node: %d\n", cnt);
    return 0;
}
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off)
{
    pr_info("Write data of new node: ");
    char kbuf[10] = {0};
    if(copy_from_user(kbuf, user_buf, len))
    {
        pr_info("\nWrite fail\n");
    }
    else
    {
        sscanf(kbuf, "%d", &value_itr);
        pr_info("%d", value_itr);
        generic_handle_irq(IRQ_NO);
    }
    return len;
}
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    pr_info("SYS_SHOW: READ\n");
    return sprintf(buf, "%d\n", value_sys);
}
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr,const char* buf, size_t count)
{
    pr_info("SYS_STORE: WRITE\n");
    sscanf(buf, "%d", &value_sys);
    return count;
}

static int __init _create_thread_function(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "thread_mm_num") < 0)
    {
        pr_err("Cannot create major, minor number\n");
        return -1;
    }
    pr_info("MAJOR(%d)\nMINOR(%d)", MAJOR(dev_num), MINOR(dev_num));
    dev_class = class_create(THIS_MODULE, "thread_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }

    dev_file = device_create(dev_class, NULL, dev_num, NULL, "thread_device");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create struct class\n");
        goto r_device;
    }

    cdev_init(&my_cdev, &my_fops);
    if(cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("Cannot add device to system\n");
        goto r_class;
    }
    //create direction sysfs
    my_kobj = kobject_create_and_add("thread_sys", kernel_kobj);
    if(sysfs_create_file(my_kobj, &my_attr.attr))
    {
        pr_info("Cannot create sysfs file\n");
        goto r_sysfs;
    }
    if(request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "thread_irq", (void*)irq_handler))
    {
        pr_info("Cannot register irq\n");
        goto r_sysfs;
    }
    own_wq = create_workqueue("own_wq");

    my_thread = kthread_run(thread_fn, NULL, "my_thread_ex");
    if(IS_ERR(my_thread))
    {
        pr_err("Create thread error! \n");
        goto r_device;
    }

    // my_thread = kthread_create(thread_fn, NULL, "my_thread_ex");
    // if(my_thread)
    // {
    //     wake_up_process(my_thread);
    // }
    // else
    // {
    //     pr_err("Create thread error! \n");
    //     goto r_device;
    // }
    pr_info("insert linklist irq thread done");
    return 0;

r_sysfs:
    sysfs_remove_file(my_kobj, &my_attr.attr);
    kobject_put(my_kobj);
r_device:
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
r_class:
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return -1;
}

static void __exit __remove_thread_function(void)
{
    struct my_node* _temp;
    struct my_node* cur;
    list_for_each_entry_safe(cur, _temp, &my_list, list)
    {
        pr_info("Remove: Node = %d", cur->data);
        list_del(&cur->list);
        kfree(cur);
    }
    kthread_stop(my_thread);
    destroy_workqueue(own_wq);
    free_irq(IRQ_NO, (void*)irq_handler);
    sysfs_remove_file(my_kobj, &my_attr.attr);
    kobject_put(my_kobj);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
}


module_init(_create_thread_function);
module_exit(__remove_thread_function);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
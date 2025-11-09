#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
//device file
#include <linux/device.h>
#include <linux/kdev_t.h>
//file operations
#include <linux/fs.h>
#include <linux/cdev.h>
//interrupts
#include <linux/interrupt.h>
//workqueue
#include <linux/workqueue.h>
//sysfs
#include <linux/kobject.h>
#include <linux/sysfs.h>

#include <linux/slab.h> //kmalloc
#define IRQ_NO 51

volatile int value = 0;

dev_t dev_num;
static struct class* dev_class;
static struct device* dev_file;
static struct cdev my_cdev;
static struct kobject* my_kobj;


//interrupts 
static struct workqueue_struct* own_workqueue;
static void work_fn(struct work_struct* work);
DECLARE_WORK(link_list_work, work_fn);

//Linklist (bottom half)
LIST_HEAD(my_list);
struct my_node 
{
    int data;
    struct list_head list;
};
static int __init create_link_list_irq(void);
static void __exit remove_link_list_irq(void);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off);
static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off);

//sysfs
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count);
static struct kobj_attribute my_attr = __ATTR(val_sys_linklist, 0660, sys_show, sys_store);

//Interrupt handler for IRQ 51. 
static irqreturn_t irq_handler(int irq, void* dev_id)
{
    pr_info("Call irq\n");
    queue_work(own_workqueue, &link_list_work);
    return IRQ_HANDLED;
}

void work_fn(struct work_struct* work)
{
    struct my_node* _temp = NULL;
    pr_info("Execute work fn\n");

    _temp = kmalloc(sizeof(struct my_node), GFP_KERNEL);

    _temp->data = value;
    INIT_LIST_HEAD(&_temp->list);
    list_add_tail(&_temp->list, &my_list);
}

struct file_operations my_fops =
{
    .owner = THIS_MODULE,
    .read = read_fops,
    .open = open_fops,
    .write = write_fops,
    .release = release_fops,
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
static int write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off)
{
    pr_info("WRITE_FUNCTION\n");
    char kernel_buf[100];
    if(copy_from_user(kernel_buf, user_buf, len))
    {
        pr_err("Copy fail\n");
    }
    sscanf(kernel_buf, "%d", &value);
    generic_handle_irq(IRQ_NO);
    return len;
}
static int read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off)
{
    pr_info("READ\n");
    struct my_node* _temp;
    int count = 0;
    list_for_each_entry(_temp, &my_list, list)
    {
        pr_info("Node[%d] = %d\n", count++, _temp->data);
    }

    pr_info("Number of node: %d\n", count);
    return 0;
}
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    pr_info("SYSFS_SHOW: READ\n");
    return 0;
}
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    pr_info("SYSFS_STORE: WRITE\n");
    return count;
}
static int __init create_link_list_irq(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "link_list_mm") < 0)
    {
        pr_info("Cannot create majoj and minor number\n");
        return -1;
    }

    pr_info("MAJOR(%d)\nMINOR(%d)\n", MAJOR(dev_num), MINOR(dev_num));

    dev_class = class_create(THIS_MODULE, "link_list_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }

    dev_file = device_create(dev_class, NULL, dev_num, NULL, "link_list_device");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    cdev_init(&my_cdev, &my_fops);
    if(cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("Add device to system errr!!\n");
        goto r_class;
    }
    //create directory /sys/kernel/EX_linklist
    my_kobj = kobject_create_and_add("EX_linklist", kernel_kobj);
    if(sysfs_create_file(my_kobj, &my_attr.attr))
    {
        pr_info("Cannot create sysfs file\n");
        goto r_sysfs;
    }
    if(request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "linklist_itr", (void*)irq_handler))
    {
        pr_info("Cannot register irq \n");
        // goto 
        goto r_sysfs;
    }

    own_workqueue = create_workqueue("own_wq");
    pr_info("insert linklist itr done\n");
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
static void __exit remove_link_list_irq(void)
{
    struct my_node* _temp;
    struct my_node* _cur;
    list_for_each_entry_safe(_cur, _temp, &my_list, list)
    {
        list_del(&_cur->list);
        kfree(_cur);
    }
    destroy_workqueue(own_workqueue);
    free_irq(IRQ_NO, (void*) irq_handler);
    sysfs_remove_file(my_kobj, &my_attr.attr);
    kobject_put(my_kobj); 
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove linklist done\n");
}


module_init(create_link_list_irq);
module_exit(remove_link_list_irq);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
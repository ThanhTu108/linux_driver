#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/mod_devicetable.h>

//gpiod
#include <linux/gpio/consumer.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
//global protocol
static int gpiod_button_probe(struct platform_device* pdev);
static int gpiod_button_remove(struct platform_device* pdev);
static int gpiod_led_probe(struct platform_device* pdev);
static int gpiod_led_remove(struct platform_device* pdev);

static struct of_device_id driver_led_ids[] = 
{
    {
        .compatible = "gpiod, gpio8_11",
    },
    {}
};
MODULE_DEVICE_TABLE(of, driver_led_ids);
static struct of_device_id driver_button_ids[] = 
{
    {
        .compatible = "gpiod, gpio8_12",
    },
    {}
};
MODULE_DEVICE_TABLE(of, driver_button_ids);

static struct platform_driver gpiod_button_driver = 
{
    .probe = gpiod_button_probe,
    .remove = gpiod_button_remove,
    .driver = 
    {
        .name = "my_gpiod_button_irq", //sys/bus/platform/driver
        .of_match_table = driver_button_ids,
    },
};

static struct platform_driver gpiod_led_driver = 
{
    .probe = gpiod_led_probe,
    .remove = gpiod_led_remove,
    .driver = 
    {
        .name = "my_gpiod_led", //sys/bus/platform/driver
        .of_match_table = driver_led_ids,
    },
};

//global variable
static struct gpio_desc* led_8_11;
static struct gpio_desc* button_8_12;
uint8_t led_status;
int count_itr = 0;
int sys_val = 0;
static dev_t btn_num = 0;
static dev_t led_num = 0;
static struct class* btn_class;
static struct class* led_class;
static struct cdev btn_cdev;
static struct cdev led_cdev;
static struct kobject* btn_kobj;
static struct kobject* led_kobj;
static struct proc_dir_entry* led_proc;
static struct proc_dir_entry* btn_proc;
//function 
static int my_open(struct inode* inode, struct file* file);
static int my_release(struct inode* inode, struct file* file);
static ssize_t my_read(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t my_write(struct file* file, const char __user* buf, size_t len, loff_t* off);

static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count);
static struct kobj_attribute my_attr = __ATTR(val_gpiod_itr, 0660, sys_show, sys_store);

static struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = my_open,
    .read = my_read,
    .write = my_write,
    .release = my_release,
};

static struct proc_ops my_proc = 
{
    .proc_open = my_open,
    .proc_release = my_release,
    .proc_write = my_write,
    .proc_read = my_read,
};

static int my_open(struct inode* inode, struct file* file)
{
    pr_info("OPEN\n");
    return 0;
}
static int my_release(struct inode* inode, struct file* file)
{
    pr_info("RELEASE\n");
    return 0;
}
static ssize_t my_read(struct file* file, char __user* buf, size_t len, loff_t* off)
{
    len = 1;
    led_status = gpiod_get_value(led_8_11);
    if(led_status == 0)
    {
        pr_info("Led: off\n");
    }
    else 
    {
        pr_info("Led: on\n");
    }
    if(copy_to_user(buf, &led_status, len))
    {
        pr_err("Read_fail\n");
    }
    return 0;
}
static ssize_t my_write(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("Write\n");
    return 0;
}

static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    pr_info("SYS_SHOW: READ\n");
    led_status = gpiod_get_value(led_8_11);
    return sprintf(buf, "Led status: %d\n Count itr = %d\n Sys value = %d\n", led_status, count_itr, sys_val);
}
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    pr_info("SYS_STORE: WRITE\n");
    sscanf(buf, "%d", &sys_val);
    return count;
}

static int gpiod_led_probe(struct platform_device* pdev)
{
    struct device* dev = &pdev->dev;
    const char* labels;
    int my_value, ret;
    if(device_property_present(dev, "label") == false)
    {
        pr_err("'Label' not found\n");
        return -1;
    }
    if(device_property_present(dev, "val") == false)
    {
        pr_err("'val' not found\n");
        return -1;
    }
    ret = device_property_read_string(dev, "label", &labels);
    if(ret)
    {
        pr_err("Cannot read string 'label'\n");
        return -1;
    }
    pr_info("Labels: %s", labels);
    ret = device_property_read_u32(dev, "val", &my_value);
    if(ret)
    {
        pr_err("Cannot read string 'label'\n");
        return -1;
    }
    pr_info("Value: %d", my_value);
    led_8_11 = devm_gpiod_get(dev, "led", GPIOD_OUT_LOW);
    alloc_chrdev_region(&led_num, 0, 1, "led_num");
    led_class = class_create(THIS_MODULE, "led_class");
    device_create(led_class, NULL, btn_num, NULL, "led_device");
    cdev_init(&led_cdev, &my_fops);
    cdev_add(&led_cdev, led_num, 1);
    led_kobj = kobject_create_and_add("led_kobj", kernel_kobj);
    if(sysfs_create_file(led_kobj, &my_attr.attr))
    {
        pr_err("Cannot create sysfs\n");
        return -1;
    }
    led_proc = proc_create("led_proc", 0660, NULL, &my_proc);
    pr_info("Create led done\n");   
    return 0;
}

static irqreturn_t gpiod_irq(int irq, void* dev_id)
{
    count_itr++;
    pr_info(">>> IRQ Button Pressed! Count: %d <<<\n", count_itr);
    int val = gpiod_get_value(led_8_11);
    gpiod_set_value(led_8_11, !val);
    return IRQ_HANDLED;
}

static int gpiod_button_probe(struct platform_device* pdev)
{
    struct device* dev = &pdev->dev;
    int debounce, ret;
    int btn_irq;
    if(device_property_present(dev, "debounce-ms") == false)
    {
        pr_err("'debounce-ms' not found\n");
        return -1;
    }
    ret = device_property_read_u32(dev, "debounce-ms", &debounce);
    if(ret)
    {
        pr_err("Cannot read value 'debounce'\n");
        return -1;
    }
    pr_info("debounce: %d", debounce);
    button_8_12 = devm_gpiod_get(dev, "button", GPIOD_IN);
    gpiod_set_debounce(button_8_12, debounce);
    btn_irq = gpiod_to_irq(button_8_12);
    //request irq
    if(devm_request_irq(dev, btn_irq, gpiod_irq, IRQF_TRIGGER_FALLING | IRQF_SHARED, "my_gpiod_irq",(void*)gpiod_irq))
    {
        pr_err("Request irq fail\n");
        return -1;
    }
    alloc_chrdev_region(&btn_num, 0, 1, "btn_num");
    btn_class = class_create(THIS_MODULE, "btn_class");
    device_create(btn_class, NULL, btn_num, NULL, "btn_device");
    cdev_init(&btn_cdev, &my_fops);
    cdev_add(&btn_cdev, btn_num, 1);
    btn_kobj = kobject_create_and_add("btn_kobj", kernel_kobj);
    if(sysfs_create_file(btn_kobj, &my_attr.attr))
    {
        pr_err("Cannot create sysfs\n");
        return -1;
    }
    btn_proc = proc_create("btn_proc", 0660, NULL, &my_proc);

    pr_info("Create btn done\n");   
    return 0;
}

static int gpiod_led_remove(struct platform_device* pdev)
{
    proc_remove(led_proc);
    sysfs_remove_file(led_kobj, &my_attr.attr);
    kobject_put(led_kobj);
    cdev_del(&led_cdev);
    device_destroy(led_class, led_num);
    class_destroy(led_class);
    unregister_chrdev_region(led_num, 1);
    pr_info("Remove led\n");
    return 0;
}
static int gpiod_button_remove(struct platform_device* pdev)
{
    proc_remove(btn_proc);
    sysfs_remove_file(btn_kobj, &my_attr.attr);
    kobject_put(btn_kobj);
    cdev_del(&btn_cdev);
    device_destroy(btn_class, btn_num);
    class_destroy(btn_class);
    unregister_chrdev_region(btn_num, 1);
    return 0;
}
static int __init cre_function(void)
{
    platform_driver_register(&gpiod_led_driver);
    platform_driver_register(&gpiod_button_driver);
    pr_info("init done\n");
    return 0;
}
static void __exit rmv_function(void)
{
    platform_driver_unregister(&gpiod_led_driver);
    platform_driver_unregister(&gpiod_button_driver);
    pr_info("exit done\n");
}

module_init(cre_function);
module_exit(rmv_function);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
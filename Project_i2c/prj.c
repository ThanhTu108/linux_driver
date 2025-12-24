#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>    //interrupt for button

#include "ssd1306.h"
#include "font5x7.h"
#include <linux/sched.h>
#include <linux/gpio/consumer.h>
#include <linux/completion.h>
#include <linux/atomic.h>
//probe, remove i2c
static int ssd1306_ui_probe(struct i2c_client* client, const struct i2c_device_id *id);
static void ssd1306_ui_remove(struct i2c_client *client);

// static int button_probe(struct platform_device* pdev);
// static int button_remove(struct platform_device* pdev);

static struct of_device_id ssd1306_driver_id[] = 
{
    {
        .compatible = "solomon,ssd1306_ui",
    },
    {},
};
MODULE_DEVICE_TABLE(of, ssd1306_driver_id);
static struct i2c_device_id ssd1306_id[] =
{
    {"ssd1306_ui", 0}, 
    {},
};
MODULE_DEVICE_TABLE(i2c, ssd1306_id);

static struct i2c_driver my_ssd1306_ui = 
{
    .probe = ssd1306_ui_probe,
    .remove = ssd1306_ui_remove,
    .driver = 
    {
        .name = "my_ssd1306_ui",
        .of_match_table = ssd1306_driver_id
    },
    .id_table = ssd1306_id,
};
// static struct of_device_id button_id[] = 
// { 
//     {
//         compatible = "solomon,button_ssd1306_ui",
//     },
//     {},
// };

// MODULE_DEVICE_TABLE(of, button_id);

// static struct platform_driver btn_ssd1306_ui = 
// {
//     .probe = button_probe,
//     .remove = button_remove,
//     .driver = 
//     {
//         .name= "btn_ssd1306_ui",
//         .of_match_table = button_id,
//     },
// };


static struct task_struct* thread_ssd1306_ui;
int thread_ssd1306_ui_fn(void* data);
static struct completion wait_event;
atomic_t btn_val = ATOMIC_INIT(0);

static int ssd_open(struct inode* inode, struct file* file);
static int ssd_release(struct inode* inode, struct file* file);
static ssize_t ssd_read(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t ssd_write(struct file* file, const char __user* buf, size_t len, loff_t* off);

struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = ssd_open, 
    .release = ssd_release, 
    .read = ssd_read,
    .write = ssd_write,
};

// static struct btn_dev 
// {
//     struct device* dev;
//     struct 
// };

// static int button_probe(struct platform_device* pdev)
// {
//     struct device* dev = &pdev->dev;
//     char* label;
//     if(device_property_present(dev, "label") == false)
//     {
//         pr_err("'label' not found\n");
//         return -1;
//     }
//     if(device_property_present(dev, "debounce-ms") == false)
//     {
//         pr_err("'debounce-ms' not found\n");
//         return -1;
//     }
// }

static int ssd_open(struct inode* inode, struct file* file)
{
    pr_info("OPEN\n");
    struct ssd1306_t* ssd;
    ssd = container_of(inode->i_cdev, struct ssd1306_t, my_cdev);
    file->private_data = ssd;
    return 0;
}

static int ssd_release(struct inode* inode, struct file* file)
{
    pr_info("RELEASE\n");
    return 0;
}
static ssize_t ssd_read(struct file* file, char __user* buf, size_t len, loff_t* off)
{
    pr_info("READ\n");
    struct ssd1306_t* ssd = file->private_data;
    pr_info("Prev_mode: %d\n", ssd->mode);
    atomic_set(&btn_val, ((ssd->mode) + 1) % 5);
    // pr_info("BTN_VAL: %d\n", (ssd->mode+1) % 5);
    pr_info("BTN_VAL atomic: %d\n", atomic_read(&btn_val));
    complete(&wait_event);
    return 0;
}
static ssize_t ssd_write(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n\n");
    return len;
}
int thread_ssd1306_ui_fn(void* data)
{
    struct ssd1306_t* ssd = (struct ssd1306_t*)data;
    if (!ssd) 
    {
        pr_err("Error: Thread data is NULL\n");
        return -1;
    }
    pr_info("Wait read function\n");
    enum menu_mode last = ssd->mode; 
    while(!kthread_should_stop())
    {
        wait_for_completion(&wait_event);
        enum menu_mode cur_mode = atomic_read(&btn_val);
        if(cur_mode != last)
        {
            ssd1306_draw_mode(ssd, cur_mode);
        }
        if (cur_mode == 10)
            return 0;
        // atomic_set(&btn_val, 0);
    }
    return 0;
}
static int ssd1306_ui_probe(struct i2c_client* client, const struct i2c_device_id *id)
{
    struct ssd1306_t* ssd;
    struct device* dev = &client->dev;
    ssd = devm_kmalloc(dev, sizeof(struct ssd1306_t), GFP_KERNEL);
    ssd->client = client;
    i2c_set_clientdata(client, ssd);
    if(alloc_chrdev_region(&ssd->dev_num, 0, 1, "ssd_ui_num") < 0)
    {
        pr_err("Cannot create major minor number\n");
        return -1;
    }
    pr_info("Major(%d) \t Minor(%d)\n", MAJOR(ssd->dev_num), MINOR(ssd->dev_num));
    ssd->dev_class = class_create(THIS_MODULE, "ssd_ui_class");
    if(IS_ERR(ssd->dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }
    if(IS_ERR(device_create(ssd->dev_class, NULL, ssd->dev_num, NULL, "ssd_ui_device")))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    cdev_init(&ssd->my_cdev, &my_fops);
    if(cdev_add(&ssd->my_cdev, ssd->dev_num, 1) < 0)
    {
        pr_err("Cannot add device to system\n");
        goto r_class;
    }
    init_completion(&wait_event);
    thread_ssd1306_ui = kthread_run(thread_ssd1306_ui_fn, (void*)ssd, "thread_draw_ui");
    if(IS_ERR(thread_ssd1306_ui))
    {
        pr_err("Create thread fail\n");
        goto r_device;
    }
    ssd1306_init(ssd);
    ssd1306_set_page_col(ssd, 0, 0);
    // ssd1306_draw_bitmap(ssd, 0, 0, bitmap_sawtooth, 128, 8);
    // ssd1306_draw_bitmap(ssd, 0, 2, bitmap_turtle, 32, 32);
    // ssd1306_draw_bitmap(ssd, 33, 2, bitmap_cat, 32, 32);
    // ssd1306_draw_bitmap(ssd, 66, 2, bitmap_cow, 32, 32);
    // ssd1306_draw_bitmap(ssd, 97, 2, bitmap_hotdog, 32, 32);
    // ssd1306_set_page_col(ssd, 10, 7);
    // ssd1306_write_integer_8x8(ssd, 0);
    ssd1306_draw_menu(ssd);
    return 0;
r_device:
    class_destroy(ssd->dev_class);
r_class:
    cdev_del(&ssd->my_cdev);
    unregister_chrdev_region(ssd->dev_num, 1);
    return -1;
}
static void ssd1306_ui_remove(struct i2c_client *client)
{
    struct ssd1306_t* ssd = i2c_get_clientdata(client);
    atomic_set(&btn_val, 10);
    complete(&wait_event);
    kthread_stop(thread_ssd1306_ui);
    ssd1306_clear(ssd);
    ssd1306_send_cmd(ssd, SSD1306_DISPLAY_OFF);
    device_destroy(ssd->dev_class, ssd->dev_num);
    class_destroy(ssd->dev_class);
    cdev_del(&ssd->my_cdev);
    unregister_chrdev_region(ssd->dev_num, 1);
    pr_info("remove i2c ssd1306_ui done\n");
}
module_i2c_driver(my_ssd1306_ui);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
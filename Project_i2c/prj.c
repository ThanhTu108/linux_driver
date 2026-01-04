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
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/delay.h>
static int __init create_function(void);
static void __exit rmv_function(void);
//probe, remove i2c
static int ssd1306_ui_probe(struct i2c_client* client, const struct i2c_device_id *id);
static void ssd1306_ui_remove(struct i2c_client *client);
static irqreturn_t btn_irq(int irq, void* dev_id);
static int button_probe(struct platform_device* pdev);
static int button_remove(struct platform_device* pdev);

#define NUMBER_BUTTON 4
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
static struct of_device_id button_id[] = 
{ 
    {
        .compatible = "solomon,button_ssd1306_ui",
    },
    {},
};

MODULE_DEVICE_TABLE(of, button_id);

static struct platform_driver btn_ssd1306_ui = 
{
    .probe = button_probe,
    .remove = button_remove,
    .driver = 
    {
        .name = "btn_ssd1306_ui",
        .of_match_table = button_id,
    },
};


static struct task_struct* thread_ssd1306_ui;
int thread_ssd1306_ui_fn(void* data);

static int ssd_open(struct inode* inode, struct file* file);
static int ssd_release(struct inode* inode, struct file* file);
static ssize_t ssd_read(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t ssd_write(struct file* file, const char __user* buf, size_t len, loff_t* off);
static void button_handler(struct button_t* button);
struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = ssd_open, 
    .release = ssd_release, 
    .read = ssd_read,
    .write = ssd_write,
};
static irqreturn_t btn_irq(int irq, void* dev_id)
{
    struct button_t* btn = dev_id;
    struct ssd1306_t* ssd = btn->data;
    if (unlikely(!btn || !ssd))
    {
        pr_info("Cannot get irq\n");
        return IRQ_HANDLED;
    }
    atomic_set(&ssd->last_btn, btn->type);
    complete(&ssd->event);
    return IRQ_HANDLED;
}
static int button_probe(struct platform_device* pdev)
{
    struct device* dev = &pdev->dev;
    const char* label;
    int debounce_ms;
    int ret; 
    pr_info("Button probe\n");
    if(device_property_present(dev, "label") == false)
    {
        pr_err("'label' not found\n");
        return -1;
    }
    ret = device_property_read_string(dev, "label", &label);
    if(ret)
    {
        pr_err("Cannot read string 'label'\n");
        return -1;
    }
    if(device_property_present(dev, "debounce-ms") == false)
    {
        pr_err("'debounce-ms' not found\n");
        return -1;
    }

    ret = device_property_read_u32(dev, "debounce-ms", &debounce_ms);
    if(ret)
    {
        pr_err("Cannot read debounce\n");
        return -1;
    }
    pr_info("Debounce - ms: %d\n", debounce_ms);
    pr_info("Label: %s\n", label);

    dev_info(dev, "Get info done\n");
    int i;
    for(i = 0; i < NUMBER_BUTTON; i++)
    {
        btn[i].btn_ssd = devm_gpiod_get_index(dev, "button", i, GPIOD_IN);
        if(IS_ERR(btn[i].btn_ssd))
        {
            pr_info("Cannot get index: %d\n", i);
            return -1;
        }
        gpiod_set_debounce(btn[i].btn_ssd, debounce_ms);
        btn[i].irq = gpiod_to_irq(btn[i].btn_ssd);
        btn[i].type = i;
        if(devm_request_irq(dev, 
                        btn[i].irq, 
                        //func
                        btn_irq,
                        IRQF_TRIGGER_FALLING | IRQF_SHARED, 
                        "ssd_irq", 
                        &btn[i]))
        {
            pr_err("Request irq fail\n");
            return -1;
        }
    }
    return 0;
}
static int button_remove(struct platform_device* pdev)
{
    pr_info("Remove button\n");
    return 0;
}
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
    // struct ssd1306_t* ssd = file->private_data;
    // pr_info("Prev_mode: %d\n", ssd->mode);
    // atomic_set(&btn_type, ((ssd->mode) +5 - 1) % 5);
    // // pr_info("btn_type: %d\n", (ssd->mode+1) % 5);
    // pr_info("btn_type atomic: %d\n", atomic_read(&btn_type));
    // complete(&wait_event);
    return 0;
}
static ssize_t ssd_write(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n\n");
    return len;
}

static void button_handler(struct button_t* btn)
{
    struct ssd1306_t* ssd = btn->data;
    pr_info("Check function\n");
    pr_info("State: %d\n", ssd->state);
    switch(ssd->state)
    {
        case(LOGO):
            if(btn->type == BTN_SEL)
            {
                pr_info("Select\n");
                ssd->state = SEL_MENU;
                ssd1306_draw_menu(ssd);
                msleep(100);
            }
            break;
        case(SEL_MENU):
            switch(btn->type)
            {
                case(BTN_UP):
                    ssd1306_draw_mode(ssd, (ssd->mode+1)%5);
                    break;
                case(BTN_DW):
                    ssd1306_draw_mode(ssd, (ssd->mode+ 5 - 1)%5);
                    break;
                case(BTN_BACK):
                    ssd1306_draw_logo(ssd);
                    ssd->state = LOGO;
                    break;
                case(BTN_SEL):
                    ssd->state = ADJ_VAL;
                    break;
                    
            }
            // if(btn->tyo)
            break;
        default:
            pr_info("DEFAULT\n");
    }
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
    
    while(!kthread_should_stop())
    {
        // enum menu_mode last = ssd->mode; 
        wait_for_completion(&ssd->event);
        // enum menu_mode cur_mode = atomic_read(&btn_type);
        // if(cur_mode != last)
        // {
        //     ssd1306_draw_mode(ssd, cur_mode);
        // }
        int val = atomic_read(&ssd->last_btn);
        pr_info("Val: %d\n", val);
        if(val <= NUMBER_BUTTON)
        {
            pr_info("Go to: \n");
            button_handler(&btn[val]);
        }
        if (val == 10)
            return 0;
        // atomic_set(&btn_type, 0);
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
    ssd->state = LOGO;
    for(int i = 0; i< NUMBER_BUTTON; i++)
    {
        btn[i].data = ssd;
    }
    init_completion(&ssd->event);
    atomic_set(&ssd->last_btn, LOGO);
    thread_ssd1306_ui = kthread_run(thread_ssd1306_ui_fn, (void*)ssd, "thread_draw_ui");
    if(IS_ERR(thread_ssd1306_ui))
    {
        pr_err("Create thread fail\n");
        goto r_device;
    }
    ssd1306_init(ssd);
    // ssd1306_draw_menu(ssd);
    ssd1306_draw_logo(ssd);
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
    atomic_set(&ssd->last_btn, 10);
    complete(&ssd->event);
    kthread_stop(thread_ssd1306_ui);
    ssd1306_clear(ssd);
    ssd1306_send_cmd(ssd, SSD1306_DISPLAY_OFF);
    device_destroy(ssd->dev_class, ssd->dev_num);
    class_destroy(ssd->dev_class);
    cdev_del(&ssd->my_cdev);
    unregister_chrdev_region(ssd->dev_num, 1);
    pr_info("remove i2c ssd1306_ui done\n");
}

// module_i2c_driver(my_ssd1306_ui);

static int __init create_function(void)
{
    int ret;

    ret = i2c_add_driver(&my_ssd1306_ui);
    if (ret) {
        pr_err("Failed to register i2c driver: %d\n", ret);
        return ret;
    }
    ret = platform_driver_register(&btn_ssd1306_ui);
    if (ret) {
        pr_err("Failed to register platform driver: %d\n", ret);
        i2c_del_driver(&my_ssd1306_ui);
        return ret;
    }
    pr_info("Init done\n");
    return 0;
}

static void __exit rmv_function(void)
{
    i2c_del_driver(&my_ssd1306_ui);
    for (int i = 0; i < NUMBER_BUTTON; i++)
        disable_irq(btn[i].irq);
    platform_driver_unregister(&btn_ssd1306_ui);
    pr_info("Exit done\n");
}
module_init(create_function);
module_exit(rmv_function);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
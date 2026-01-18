#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/atomic.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include "ssd1306.h"
#include <linux/delay.h>
int ssd1306_probe(struct i2c_client *client, const struct i2c_device_id *id);
void ssd1306_remove(struct i2c_client *client);
int thread_ui(void* data);
static struct button_ops_ssd ops;
extern void button_set_callback(struct button_ops_ssd* ops);
extern void button_unregister_callback(struct button_ops_ssd* ops);
static struct of_device_id ssd1306_id[] = 
{
    {
        .compatible = "solomon,ssd1306_ui",
    },
    {},
};
MODULE_DEVICE_TABLE(of, ssd1306_id);
static struct i2c_device_id ssd1306_i2c_id[] = 
{
    {"ssd1306", 0},
    {},
};
MODULE_DEVICE_TABLE(i2c, ssd1306_i2c_id);

static struct i2c_driver ssd1306_i2c_driver= 
{
    .probe = ssd1306_probe,
    .remove = ssd1306_remove,
    .driver = 
    {
        .name = "ssd1306_driver",
        .of_match_table = ssd1306_id,
    },
    .id_table = ssd1306_i2c_id,
};
int thread_ui(void* data)
{
    struct ssd1306_t* ssd = (struct ssd1306_t*)(data);
    pr_info("Thread_ui, wait event\n");
    while(!kthread_should_stop())
    {
        wait_for_completion(&ssd->event);
        int type = atomic_read(&ssd->last_btn);
        switch(type)
        {
            case 0:
                ssd->cur_state->up(ssd);
                break;
            case 1:
                ssd->cur_state->dw(ssd);
                break;
            case 2:
                ssd->cur_state->back(ssd);
                break;
            case 3:
                ssd->cur_state->sel(ssd);
                break;
        }
        if( type == 10) return 0;
    }
    return 0;
}
int ssd1306_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct ssd1306_t* ssd = devm_kzalloc(&client->dev, (sizeof(struct ssd1306_t)), GFP_KERNEL);
    if(!ssd)
    {
        pr_err("Cannot allocate memory for ssd\n");
        return -1;
    }
    ssd->dev = &client->dev;
    ssd->client = client;
    
    atomic_set(&ssd->last_btn, -1);
    init_completion(&ssd->event);
    ops.is_press = button_ssd_handler;
    ops.data = (void*)ssd;
    ssd->thread_ui = kthread_run(thread_ui, ssd, "ssd1306_thread");
    if(IS_ERR(ssd->thread_ui))
    {
        pr_err("Run thread fail\n");
        return -1;
    }
    button_set_callback(&ops);
    ssd1306_init(ssd);
    // ssd1306_draw_logo(ssd);
    // msleep(1000);
    ssd->cur_state = fsm_get_struct_fsm(LOGO);
    ssd->cur_state->enter(ssd);
    ssd->val_contrast = 100;
    ssd->inverse = 0;
    
    // msleep(1000);
    // fsm_set_state(ssd, SEL_MENU);
    i2c_set_clientdata(client, ssd);
    return 0;
}
void ssd1306_remove(struct i2c_client *client)
{
    struct ssd1306_t *ssd = i2c_get_clientdata(client);
    ssd1306_clear(ssd);
    button_unregister_callback(&ops);
    atomic_set(&ssd->last_btn, 10);
    complete(&ssd->event);
    kthread_stop(ssd->thread_ui);
    pr_info("SSD1306 Driver Removed\n");
}
module_i2c_driver(ssd1306_i2c_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
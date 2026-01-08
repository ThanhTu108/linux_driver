#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>

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
    {ssd1306, 0},
    {},
};
MODULE_DEVICE_TABLE(i2c, ssd1306_i2c_id);

int ssd1306_probe(struct i2c_client *client, const struct i2c_device_id *id);
void ssd1306_remove(struct i2c_client *client);
static struct i2c_driver my_ssd1306_driver = 
{
    .probe = ssd1306_probe,
    .remove = ssd1306_remove,
    .driver =
    {
        .name = my_ssd1306_driver,
        .of_match_table = ssd1306_id,
    }
    .id_table = ssd1306_i2c_id,
};

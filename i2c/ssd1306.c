#include "ssd1306.h"
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include "font5x7.h"
static int i2c_write(struct ssd1306_t* ssd ,unsigned char* buf, unsigned int len)
{
    int ret = i2c_master_send(ssd->client, buf, len);
    return ret;
}
void ssd1306_send_cmd(struct ssd1306_t* ssd, enum ssd1306_cmd cmd)
{
    unsigned char buf[2] = {SINGLE_CMD, cmd};
    int ret = i2c_write(ssd, buf, 2);
    if(ret < 0)
    {
        pr_err("Cannot send\n");
    }
}
void ssd1306_send_data(struct ssd1306_t* ssd, uint8_t data)
{
    unsigned char buf[2] = {SINGLE_DATA, data};
    int ret = i2c_write(ssd, buf, 2);
    if(ret < 0)
    {
        pr_err("Cannot send\n");
    }
}

void ssd1306_init(struct ssd1306_t* ssd)
{
    msleep(100);
    ssd1306_send_cmd(ssd, SSD1306_DISPLAY_OFF);
    //set retio
    ssd1306_send_cmd(ssd, SSD1306_SET_MULTIPLEX_RATIO);
    ssd1306_send_cmd(ssd, 0x3F);
    //set display off set
    ssd1306_send_cmd(ssd, SSD1306_SET_DISPLAY_OFFSET);
    ssd1306_send_cmd(ssd, 0x00);
    //Set display start line
    ssd1306_send_cmd(ssd, SSD1306_SET_DISPLAY_START_LINE);
    //memory mode 
    ssd1306_send_cmd(ssd, SSD1306_MEMORY_MODE);
    ssd1306_send_cmd(ssd, 0x00);
    ssd1306_set_page_col(ssd, 0, 0);
    // Remap disable
    ssd1306_send_cmd(ssd, SSD1306_REMAP_NORMAL);
    //scan com
    ssd1306_send_cmd(ssd, SSD1306_SCAN_DIRECTION_NORMAL);
    //set compin hw
    ssd1306_send_cmd(ssd, SSD1306_SET_COM_PIN);
    ssd1306_send_cmd(ssd, 0x12);
    //set contrast
    ssd1306_send_cmd(ssd, SSD1306_SET_CONTRAST);
    ssd1306_send_cmd(ssd, 0x7F);
    // entire display off
    ssd1306_send_cmd(ssd, SSD1306_ENTIRE_DISPLAY_OFF);
    //set normal display
    ssd1306_send_cmd(ssd, SSD1306_NORMAL_DISPLAY);
    // set osc frequency
    ssd1306_send_cmd(ssd, SSD1306_SET_CLOCK_DIV_RATIO);
    ssd1306_send_cmd(ssd, 0x80);
    //charge pump
    ssd1306_send_cmd(ssd, SSD1306_SET_CHARGE_PUMP);
    ssd1306_send_cmd(ssd, 0x14);    //enable
    //when use charge pump, must use pre charge
    ssd1306_send_cmd(ssd, SSD1306_SET_PRE_CHARGE);
    ssd1306_send_cmd(ssd, 0xF1);
    int page, col;
    for(page = 0; page <=7; page++)
    {
        for(col = 0; col <=127; col++)
        {
            ssd1306_send_data(ssd, 0x00);
        }
    }
    ssd1306_send_cmd(ssd, SSD1306_DISPLAY_ON);
}

void ssd1306_set_page_col(struct ssd1306_t* ssd, uint8_t x, uint8_t y)
{
    
    if(x > 127)
    {
        x = 127;
    }
    if(y > 7)
    {
        y = 7;
    }
    ssd1306_send_cmd(ssd, SSD1306_SET_COLUMN_ADDR);
    ssd1306_send_cmd(ssd, x);
    ssd1306_send_cmd(ssd, 127);
    ssd1306_send_cmd(ssd, SSD1306_SET_PAGE_ADDR);
    ssd1306_send_cmd(ssd, y);
    ssd1306_send_cmd(ssd, 7);
}
void ssd1306_clear(struct ssd1306_t* ssd)
{
    int page, col;
    for(page = 0; page <=7; page++)
    {
        for(col = 0; col <=127; col++)
        {
            ssd1306_send_data(ssd, 0x00);
        }
    }
    ssd1306_set_page_col(ssd, 0 , 0);
}
void ssd1306_write_integer(struct ssd1306_t* ssd, int num)
{
    if(num < 0 || num > 9)
    {
        return;
    }
    char id = num + '0';
    id = char_to_idx(id);
    for(int i = 0; i<5; i++)
    {
        pr_info("Index = %d",(id + i));
        ssd1306_send_data(ssd, Font5x7[i + id]);
    }
}
void ssd1306_write_integer_8x8(struct ssd1306_t* ssd, int num)
{
    if(num < 0 || num > 9)
    {
        return;
    }
    char id = num + '0';
    id = char_to_idx8x8(id);
    for(int i = 0; i<8; i++)
    {
        pr_info("Index = %d",(id + i));
        ssd1306_send_data(ssd, Font8x8[i + id]);
    }
}

void ssd1306_write_string(struct ssd1306_t* ssd, char* str)
{   
    char* new_str;
    new_str = kzalloc(strlen(str), GFP_KERNEL); 
    if(new_str == NULL)
    {
        pr_err("Cannot alocate memory\n");
    }
    strcpy(new_str, str);
    pr_info("%s\n", new_str);
    for(int i = 0; new_str[i] != '\0'; i++)
    {
        for(int j = 0; j < 5; j++)
        {
            int id = char_to_idx(new_str[i]);
            pr_info("id_write string: %d\n", id);
            ssd1306_send_data(ssd, Font5x7[j + id]);
        }
    }
    kfree(new_str);
}
void ssd1306_write_string_8x8(struct ssd1306_t* ssd, char* str)
{
    char* new_str; 
    new_str = kzalloc(strlen(str), GFP_KERNEL);
    if(new_str == NULL)
    {
        pr_err("Cannot allocate memory\n");
        return;
    }
    strcpy(new_str, str);
    pr_info("%s\n", new_str);
    for(int i = 0; new_str[i] != '\0'; i++)
    {
        int idx = char_to_idx8x8(str[i]);
        for(int j = 0; j < 8; j++)
        {
            ssd1306_send_data(ssd, Font8x8[idx + j]);
        }
    }
    kfree(new_str);
}
void ssd1306_write_space(struct ssd1306_t* ssd)
{
    for(int i = 0; i < 5; i++)
        {
            ssd1306_send_data(ssd, Font5x7[i]);
        }
}

void ssd1306_draw_bitmap(struct ssd1306_t* ssd, uint8_t col, uint8_t page, const char* bitmap, uint8_t w, uint8_t h)
{
    int _tmp_page = h/8;
    if(h%8 != 0)
    {
        _tmp_page++;
    }
    for(int i = 0; i < _tmp_page; i++)
    {
        ssd1306_set_page_col(ssd, col, page + i);
        for(int j = 0; j < w; j++)
        {
            int idx = (i * w) + j; //get index
            ssd1306_send_data(ssd, bitmap[idx]);
        }
    }
}
#ifndef __SSD1306_H
#define __SSD1306_H

#include <linux/types.h>
#include <linux/cdev.h>
//define cmd ssd1306
#define SINGLE_CMD 0x00    //dc = 0, co = 0
#define SINGLE_DATA 0x40   //dc = 1, co = 1
enum ssd1306_cmd 
{
    SSD1306_DISPLAY_ON = 0xAF,  //normal mode
    SSD1306_DISPLAY_OFF = 0xAE, //sleep mode
    SSD1306_SET_CLOCK_DIV_RATIO = 0XD5, //default = 0x80
    SSD1306_SET_MULTIPLEX_RATIO = 0xA8, //default 128x64 = 63, 128x32 = 31
    SSD1306_SET_DISPLAY_OFFSET = 0xD3,  //00 - 63
    SSD1306_SET_DISPLAY_START_LINE = 0x40,  //0x40 - 0x7F (00-63)
    SSD1306_SET_CHARGE_PUMP = 0x8D, //and send 0x14 to enable, 0x10: disable
    SSD1306_MEMORY_MODE = 0x20, //0x00: Horizontal, 0x01: Vertical
    SSD1306_SET_COLUMN_ADDR = 0x21,
    SSD1306_SET_PAGE_ADDR = 0X22,
    //left <-> right
    SSD1306_REMAP_NORMAL = 0xA0,    
    SSD1306_REMAP_REVERSE = 0xA1,
    // top <-> bottom
    SSD1306_SCAN_DIRECTION_NORMAL = 0xC0,
    SSD1306_SCAN_DIRECTION_REVERSE = 0xC8,
    SSD1306_SET_COM_PIN = 0xDA, //64: 0x12, 32: 0x02
    SSD1306_SET_CONTRAST = 0x81,
    SSD1306_SET_PRE_CHARGE = 0xD9, //normally 0xF1 (1111: phase 2 pre-charge, 0001: phase 1 Discharge)
    SSD1306_SET_VCOMH_DESELECT = 0xDB,   //A[6:4] (USE 0X20)
    SSD1306_ENTIRE_DISPLAY_ON  = 0xA5, // Entire display on (don't care Ram)
    SSD1306_ENTIRE_DISPLAY_OFF = 0xA4, //Entire display off
    SSD1306_NORMAL_DISPLAY = 0xA6,  //Ram = 1 -> pixel on
    SSD1306_INVERSE_DISPLAY = 0xA7, //ram = 0 -> pixel off
};

struct ssd1306_t
{
    struct i2c_client* client;
    dev_t dev_num;
    struct class* dev_class;
    // struct device* dev_file,
    struct cdev my_cdev;
    struct kobject* my_kobj;
    // struct 
};
// extern const char bitmap[];
// //function write
void ssd1306_send_cmd(struct ssd1306_t* ssd, enum ssd1306_cmd cmd);
void ssd1306_send_data(struct ssd1306_t* ssd, uint8_t data);

void ssd1306_init(struct ssd1306_t* ssd);
void ssd1306_set_page_col(struct ssd1306_t* ssd, uint8_t x, uint8_t y);
void ssd1306_clear(struct ssd1306_t* ssd);
void ssd1306_write_integer(struct ssd1306_t* ssd, int num);
void ssd1306_write_string(struct ssd1306_t* ssd, char* str);
void ssd1306_write_space(struct ssd1306_t* ssd);
void ssd1306_draw_sawtooth(struct ssd1306_t* ssd);
void ssd1306_draw_smile_icon(struct ssd1306_t* ssd, uint8_t col, uint8_t page);
void ssd1306_draw_bitmap(struct ssd1306_t* ssd, uint8_t col, uint8_t page, const char* bitmap, uint8_t h, uint8_t w);
#endif
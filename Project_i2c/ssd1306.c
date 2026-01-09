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
    ssd1306_set_page_col(ssd, 0 , 0);
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
    for(int i = 0; new_str[i] != '\0'; i++)
    {
        for(int j = 0; j < 5; j++)
        {
            int id = char_to_idx(new_str[i]);
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
void ssd1306_draw_menu(struct ssd1306_t *ssd)
{
    /* Title */
    ssd1306_clear(ssd);
    ssd1306_set_page_col(ssd, 0, 0);
    ssd1306_write_string_8x8(ssd, "   SSD1306 UI   ");
    ssd1306_set_page_col(ssd, 0, 1);
    ssd1306_write_string_8x8(ssd, "----------------");
    /* Menu items */
    ssd1306_set_page_col(ssd, 0, 2);
    ssd1306_write_string_8x8(ssd, "|  Contrast:   |");
    ssd1306_set_page_col(ssd, 0, 3);
    ssd1306_write_string_8x8(ssd, "|  Inverse:    |");
    ssd1306_set_page_col(ssd, 0, 4);
    ssd1306_write_string_8x8(ssd, "|  Rotate:     |");
    ssd1306_set_page_col(ssd, 0, 5);
    ssd1306_write_string_8x8(ssd, "|  Display:    |");
    ssd1306_set_page_col(ssd, 0, 6);
    ssd1306_write_string_8x8(ssd, "|     Exit     |");
    ssd1306_set_page_col(ssd, 0, 7);
    ssd1306_write_string_8x8(ssd, "----------------");
}
void ssd1306_set_contrast(struct ssd1306_t *ssd, uint8_t contrast)
{
    ssd1306_send_cmd(ssd, SSD1306_SET_CONTRAST);
    ssd1306_send_cmd(ssd, contrast);
}
void ssd1306_inverse(struct ssd1306_t* ssd, bool is_inverse)
{
    if(is_inverse)
    {
        ssd1306_send_cmd(ssd, SSD1306_SCAN_DIRECTION_NORMAL);
        ssd1306_send_cmd(ssd, SSD1306_REMAP_NORMAL);
    }
    else
    {
        ssd1306_send_cmd(ssd, SSD1306_SCAN_DIRECTION_REVERSE);
        ssd1306_send_cmd(ssd, SSD1306_REMAP_REVERSE);
    }
}
void ssd1306_set_rotate(struct ssd1306_t* ssd, uint16_t rotate)
{
    // ssd1306_cmd(ssd, )
}
void ssd1306_draw_logo(struct ssd1306_t *ssd)
{
    ssd1306_clear(ssd);
    ssd1306_set_page_col(ssd, 0, 0);
    ssd1306_draw_bitmap(ssd, 0, 0, bitmap_sawtooth, 128, 8);
    ssd1306_draw_bitmap(ssd, 0, 2, bitmap_turtle, 32, 32);
    ssd1306_draw_bitmap(ssd, 33, 2, bitmap_cat, 32, 32);
    ssd1306_draw_bitmap(ssd, 66, 2, bitmap_cow, 32, 32);
    ssd1306_draw_bitmap(ssd, 95, 2, bitmap_hotdog, 32, 32);
    ssd1306_set_page_col(ssd, 10, 7);
    ssd1306_write_string_8x8(ssd, "PRESS SEL");
}

static int mode_to_page(enum menu_mode mode)
{
    switch(mode)
    {
        case MODE_CONTRAST:
            return 2;
        case MODE_INVERSE:
            return 3;
        case MODE_ROTATE:
            return 4;
        case MODE_DISPLAY:
            return 5;
        case MODE_EXIT:
            return 6;
        default:
            return -1;
    }
}

void ssd1306_draw_mode(struct ssd1306_t *ssd, enum menu_mode mode)
{
    if(mode == ssd->mode)
    {
        return;
    }
    int prev_mode = mode_to_page(ssd->mode);
    // if(mode == 0)
    // {
    //     ssd->mode = mode;
    //     ssd1306_set_page_col(ssd, 8, prev_mode);
    //     ssd1306_write_string_8x8(ssd, "  ");
    //     return;
    // }   
    if(prev_mode >= 0)
    {
        ssd1306_set_page_col(ssd, 8, prev_mode);
        ssd1306_write_string_8x8(ssd, "  ");
    }
    int new_mode = mode_to_page(mode);
    ssd1306_set_page_col(ssd, 8, new_mode);
    ssd1306_write_string_8x8(ssd, "->");
    ssd->mode = mode;
}
//start: 10, end: 110
void ssd1306_draw_menu_contrast(struct ssd1306_t* ssd)
{
    ssd1306_clear(ssd);
    ssd1306_set_page_col(ssd, 32, 0);
    ssd1306_write_string_8x8(ssd, "CONTRAST");

    int fill = (ssd->val_contrast * (END_COL_CONTRAST - START_COL_CONTRAST)) / 230;
    ssd1306_set_page_col(ssd, START_COL_CONTRAST - 1, TOP_PAGE);
    ssd1306_send_data(ssd, 0xFF);
    ssd1306_set_page_col(ssd, START_COL_CONTRAST - 1, BOT_PAGE);
    ssd1306_send_data(ssd, 0xFF);
    for(int i = 0; i < 100; i++)
    {
        if(i < fill)
        {
            ssd1306_set_page_col(ssd, START_COL_CONTRAST+i, TOP_PAGE);
            ssd1306_send_data(ssd, 0xff);
            ssd1306_set_page_col(ssd, START_COL_CONTRAST+i, BOT_PAGE);
            ssd1306_send_data(ssd, 0xff);
        }
        else 
        {
            ssd1306_set_page_col(ssd, START_COL_CONTRAST+i, TOP_PAGE);
            ssd1306_send_data(ssd, 0x01);
            ssd1306_set_page_col(ssd, START_COL_CONTRAST+i, BOT_PAGE);
            ssd1306_send_data(ssd, 0x80);
        }
    } 
    ssd1306_set_page_col(ssd, END_COL_CONTRAST, TOP_PAGE);
    ssd1306_send_data(ssd, 0xFF);
    ssd1306_set_page_col(ssd, END_COL_CONTRAST, BOT_PAGE);
    ssd1306_send_data(ssd, 0xFF);
}
void ssd1306_draw_contrast(struct ssd1306_t* ssd)
{
    int fill = (ssd->val_contrast * (END_COL_CONTRAST - START_COL_CONTRAST)) / 230;
    ssd1306_set_page_col(ssd, START_COL_CONTRAST, TOP_PAGE);
    for(int i = 0; i < 100; i++)
    {
        if(i < fill)
        { 
            ssd1306_send_data(ssd, 0xFF);
        }
        else 
        {
            ssd1306_send_data(ssd, 0x01);
        }
    } 
    ssd1306_set_page_col(ssd, START_COL_CONTRAST, BOT_PAGE);
    for(int i = 0; i < 100; i++)
    {
        if(i < fill)
        { 
            ssd1306_send_data(ssd, 0xFF);
        }
        else 
        {
            ssd1306_send_data(ssd, 0x80);
        }
    } 
}

const char* name[] = {"up", "down", "back", "sel"};
// typedef void (*button_callback_t)(enum btn_type type, void* data);
void button_ssd_handler(int type, void* data)
{
    struct ssd1306_t* ssd = (struct ssd1306_t*)data;
    if(ssd)
    {
        pr_info("SSD_handler: %s", name[type]);
        atomic_set(&ssd->last_btn, type);
        complete(&ssd->event);
    }
    else
    {
        pr_info("ERRR\n");
    }
}
void logo_on_enter(struct ssd1306_t* ssd)
{
    ssd1306_draw_logo(ssd);
}

void do_noop(struct ssd1306_t* ssd)
{

}

static struct fsm_state state_logo = 
{
    .state = LOGO,
    .name = "LOGO",
    .enter = logo_on_enter,
    .exit = logo_exit,
    
    .back = do_noop,
    .sel = logo_sel,
    .up = do_noop,
    .dw = do_noop
};
static struct fsm_state state_menu = 
{
    .state = SEL_MENU,
    .name = "MENU",
    .enter = menu_on_enter,
    .exit = menu_exit,
    
    .back = menu_back,
    .sel = menu_sel,
    .up = menu_up,
    .dw = menu_dw
};
static struct fsm_state state_adj = 
{
    .state = ADJ_VAL,
    .name = "ADJ",
    .enter = adj_on_enter,
    .exit = adj_exit,
    
    .back = adj_back,
    .sel = adj_sel,
    .up = adj_up,
    .dw = adj_dw
};

static struct fsm_state* state[] = 
{
    &state_logo,
    &state_menu,
    &state_adj,
};
// This function is used to get stuct state from library
struct fsm_state* fsm_get_struct_fsm(e_menu_state e_state)
{
    if(e_state >= STATE_COUNT)
    {
        return NULL;
    }
    return state[e_state];
}
void fsm_set_state(struct ssd1306_t* ssd, e_menu_state new_state)
{
    struct fsm_state *old_state;
    struct fsm_state* temp_state = NULL;
    if(!ssd)
    {
        return;
    }   
    temp_state = fsm_get_struct_fsm(new_state);
    if(!temp_state)
    {
        return;
    }
    old_state = ssd->cur_state;
    if(old_state == temp_state)
    {
        return;
    }
    if(old_state && old_state->exit)
    {
        old_state->exit(ssd);
    }
    ssd->cur_state = temp_state;
    if(temp_state->enter)
    {
        temp_state->enter(ssd);
    }
}
void logo_sel(struct ssd1306_t* ssd)
{
    if(ssd->display == 0)
    {
        ssd1306_send_cmd(ssd, SSD1306_DISPLAY_ON);
    }
    fsm_set_state(ssd, SEL_MENU);
}
void logo_exit(struct ssd1306_t* ssd)
{
    ssd->mode = MODE_NONE;
}
void menu_on_enter(struct ssd1306_t* ssd)
{
    ssd1306_draw_menu(ssd);
}
void menu_back(struct ssd1306_t* ssd)
{
    fsm_set_state(ssd, LOGO);
}
void menu_up(struct ssd1306_t* ssd)
{
    ssd1306_draw_mode(ssd, (ssd->mode + 1) % MODE_COUNT);
}

void menu_dw(struct ssd1306_t* ssd)
{
    ssd1306_draw_mode(ssd, (ssd->mode + MODE_COUNT - 1) % (MODE_COUNT));
}
void menu_exit(struct ssd1306_t* ssd)
{
    ssd->mode = ssd->mode;
}
void menu_sel(struct ssd1306_t* ssd)
{
    pr_info("Mode in menu first: %d\n", ssd->mode);
    if(ssd->mode == MODE_NONE)
    {
        return;
    }
    switch(ssd->mode)
    {
        case MODE_DISPLAY:
            ssd->display = 0;
            fsm_set_state(ssd, LOGO);
            ssd1306_send_cmd(ssd, SSD1306_DISPLAY_OFF);
            return;
        case MODE_EXIT: 
            fsm_set_state(ssd, LOGO);
            return;
        default:
            fsm_set_state(ssd, ADJ_VAL);
            break;
    }
}

void adj_on_enter(struct ssd1306_t* ssd)
{
    switch(ssd->mode)
    {
        case MODE_CONTRAST:
            ssd1306_draw_menu_contrast(ssd);
            break;
        default:
            break;
    }
}
void adj_exit(struct ssd1306_t* ssd)
{
    ssd->mode = - 1;
}
void adj_up(struct ssd1306_t* ssd)
{
    pr_info("Into adject_val_down\n");
    pr_info("Mode : %d\n", ssd->mode);
    switch(ssd->mode)
    {
        case MODE_CONTRAST:
            ssd->val_contrast = ssd->val_contrast + 10;
            if(ssd->val_contrast >= 240)
            {
                ssd->val_contrast = 240;
            }
            pr_info("Contrast: %d\n", (ssd->val_contrast));
            ssd1306_set_contrast(ssd, ssd->val_contrast);
            ssd1306_draw_contrast(ssd);
            break;
        case MODE_INVERSE:
            ssd->inverse = !ssd->inverse;
            ssd1306_inverse(ssd, ssd->inverse);
            break;
        default:
            break;
    }
}
void adj_back(struct ssd1306_t* ssd)
{
    fsm_set_state(ssd, SEL_MENU);
}
void adj_dw(struct ssd1306_t* ssd)
{
    pr_info("Into adject_val_down\n");
    pr_info("Mode : %d\n", ssd->mode);
    switch(ssd->mode)
    {
        case MODE_CONTRAST:
            ssd->val_contrast = ssd->val_contrast - 10;
            if(ssd->val_contrast <= 10)
            {
                ssd->val_contrast = 10;
            }
            pr_info("Contrast: %d\n", (ssd->val_contrast));
            ssd1306_set_contrast(ssd, ssd->val_contrast);
            ssd1306_draw_contrast(ssd);
            break;
        case MODE_INVERSE:
            ssd->inverse = !ssd->inverse;
            ssd1306_inverse(ssd, ssd->inverse);
            break;
        case MODE_ROTATE:
            break;
        default:
            break;
    }
}
void adj_sel(struct ssd1306_t* ssd)
{
    
}
#ifndef __BUTTON_H_
#define __BUTTON_H_

#define NUMBER_BUTTON 4
enum btn_type
{
    BTN_UP = 0,
    BTN_DW,
    BTN_SEL,
    BTN_BACK,
};
typedef void (*button_callback_t)(enum btn_type type, void* data);
// struct button_t 
// {
//     struct gpio_desc* btn_ssd;
//     enum btn_type type;
//     int irq;
//     // enum menu_mode mode;
//     void* data;
// };
struct button_operation
{
    button_callback_t is_press;
    void* data;
};

void button_set_callback(struct button_operation* btn_ops);
void button_send_type(enum btn_type type);
void button_unregister_callback(struct button_operation* btn_ops);
#endif

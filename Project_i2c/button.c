#include "button.h"
#include <linux/module.h>

static struct button_operation* g_btn_ops = NULL;

void button_set_callback(struct button_operation* btn_ops)
{
    if(btn_ops)
    {
        g_btn_ops = btn_ops;
    }
}
EXPORT_SYMBOL_GPL(button_set_callback);

void button_send_type(enum btn_type type)
{
    if(g_btn_ops && g_btn_ops->is_press)
    {
        g_btn_ops->is_press(type, g_btn_ops->data)
    }
}
EXPORT_SYMBOL_GPL(button_send_type);
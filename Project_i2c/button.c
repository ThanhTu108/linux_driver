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

void button_unregister_callback(struct button_operation* btn_ops)
{
    if (g_btn_ops == btn_ops) {
        g_btn_ops = NULL;
    }
}
EXPORT_SYMBOL_GPL(button_unregister_callback);

void button_send_type(enum btn_type type)
{
    struct button_operation* ops = g_btn_ops;
    if(ops && ops->is_press)
    {
        ops->is_press(type, ops->data);
    }
}
EXPORT_SYMBOL_GPL(button_send_type);
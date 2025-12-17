#include "font5x7.h"
#include <linux/err.h>
#include <linux/slab.h>
int char_to_idx(char c)
{
    if(c < 32 || c > 127)
    {
        return -1; 
    }
    return ((c - 32) * 5);
}
int char_to_idx8x8(char c)
{
    if(c < 32 || c > 127)
    {
        return -1; 
    }
    return ((c - 32) * 8);
}
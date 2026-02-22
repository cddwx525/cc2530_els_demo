#ifndef __FONTS_H
#define __FONTS_H

#include <stdint.h>

#define MAX_HEIGHT_FONT         41
#define MAX_WIDTH_FONT          32

enum font_type {
        FONT_TYPE_EN,
        FONT_TYPE_ZH,
        };

struct font {
        enum font_type font_type;
        uint8_t width;
        uint8_t height;
        uint8_t bytes;
        const uint8_t * (* find_char)(
                struct font * font,
                uint8_t char_size,
                const uint8_t * string,
                uint8_t * char_width,
                uint8_t * char_bytes
                ) __reentrant;
        };

//
// ASCII
//
struct font_en {
        struct font base;
        const uint8_t * table;
        };


//
// GB2312/GBK
//
struct char_zh {
        uint8_t key[3];
        const uint8_t data[MAX_HEIGHT_FONT * MAX_WIDTH_FONT / 8];
        };

struct font_zh {
        struct font base;
        const struct char_zh * table;
        uint16_t table_size;
        uint8_t width_ascii;
        };

extern struct font_en font24;
extern struct font_en font20;
extern struct font_en font16;
extern struct font_en font12;
extern struct font_en font8;

extern struct font_zh font12zh;
extern struct font_zh font24zh;

#endif /* __FONTS_H */

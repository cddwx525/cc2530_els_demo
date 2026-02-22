//
// Font data for yahei
//
// size     : 16x21, 11x21
// Polarity : normal
// File encoding: GBK
//
#include "font_zh.h"

#include <stdint.h>


//
//  Font data for Courier New 12pt
//

__code static const struct char_zh table[] = {
        /*--  文字:  你  --*/
        /*--  微软雅黑12;  此字体下对应的点阵为：宽x高=16x21   --*/
        {
                "你",
                {
                        0x00,0x00,
                        0x00,0x00,
                        0x00,0x00,
                        0x00,0x00,
                        0x1D,0xC0,
                        0x1D,0x80,
                        0x3B,0xFF,
                        0x3B,0x07,
                        0x3F,0x77,
                        0x7E,0x76,
                        0xF8,0x70,
                        0xFB,0xFE,
                        0xFB,0xFE,
                        0x3F,0x77,
                        0x3F,0x77,
                        0x3E,0x73,
                        0x38,0x70,
                        0x38,0x70,
                        0x3B,0xE0,
                        0x00,0x00,
                        0x00,0x00
                        }
                },

        /*--  文字:  好  --*/
        /*--  微软雅黑12;  此字体下对应的点阵为：宽x高=16x21   --*/
        {
                "好",
                {
                        0x00,0x00,
                        0x00,0x00,
                        0x00,0x00,
                        0x00,0x00,
                        0x30,0x00,
                        0x73,0xFF,
                        0x70,0x0F,
                        0xFE,0x1E,
                        0x7E,0x3C,
                        0x6E,0x38,
                        0xEE,0x30,
                        0xEF,0xFF,
                        0xFC,0x30,
                        0x7C,0x30,
                        0x38,0x30,
                        0x3E,0x30,
                        0x7E,0x30,
                        0xE0,0x30,
                        0xC1,0xF0,
                        0x00,0x00,
                        0x00,0x00
                        }
                },

        /*--  文字:  树  --*/
        /*--  微软雅黑12;  此字体下对应的点阵为：宽x高=16x21   --*/
        {
                "树",
                {
                        0x00,0x00,
                        0x00,0x00,
                        0x00,0x00,
                        0x00,0x00,
                        0x30,0x0E,
                        0x30,0x0E,
                        0x3F,0xEE,
                        0x30,0xEE,
                        0xFC,0xFF,
                        0x76,0xCE,
                        0x77,0xFE,
                        0x7B,0xFE,
                        0xFF,0xFE,
                        0xF3,0xDE,
                        0xF3,0xCE,
                        0x37,0xEE,
                        0x3E,0x6E,
                        0x3C,0x0E,
                        0x30,0x3E,
                        0x00,0x00,
                        0x00,0x00
                        }
                },

        /*--  文字:  莓  --*/
        /*--  微软雅黑12;  此字体下对应的点阵为：宽x高=16x21   --*/
        {
                "莓",
                {
                        0x00,0x00,
                        0x00,0x00,
                        0x00,0x00,
                        0x00,0x00,
                        0x06,0x70,
                        0xFF,0xFF,
                        0x3E,0x70,
                        0x38,0x00,
                        0x7F,0xFF,
                        0xE0,0x00,
                        0xFF,0xFC,
                        0x3B,0x8C,
                        0x39,0xCC,
                        0xFF,0xFF,
                        0x73,0x9C,
                        0x71,0xDC,
                        0x7F,0xFF,
                        0x00,0x1C,
                        0x01,0xF8,
                        0x00,0x00,
                        0x00,0x00
                        }
                },

        /*--  文字:  派  --*/
        /*--  微软雅黑12;  此字体下对应的点阵为：宽x高=16x21   --*/
        {
                "派",
                {
                        0x00,0x00,
                        0x00,0x00,
                        0x00,0x00,
                        0x00,0x00,
                        0xE0,0x1F,
                        0xFF,0xF0,
                        0x3E,0x00,
                        0x0E,0x1F,
                        0xCF,0xFB,
                        0xFF,0xF8,
                        0x3F,0xFF,
                        0x0F,0xFF,
                        0x7F,0xD8,
                        0x7F,0xDC,
                        0x6F,0xCE,
                        0xED,0xFF,
                        0xFD,0xF7,
                        0xF9,0xC0,
                        0x00,0x00,
                        0x00,0x00,
                        0x00,0x00
                        }
                }
        };

struct font_zh font12zh = {
        .base = {
                .font_type = FONT_TYPE_ZH,
                .width = 16,
                .height = 21,
                .bytes = 2,
                .find_char = find_char_zh,
                },
        .table = table,
        .table_size = sizeof(table) / sizeof(struct char_zh),
        .width_ascii = 11
        };

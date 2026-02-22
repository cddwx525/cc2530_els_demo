#include "font_zh.h"

#include <stdint.h>

const uint8_t * find_char_zh(
        struct font * font,
        uint8_t char_size,
        const uint8_t * string,
        uint8_t * char_width,
        uint8_t * char_bytes
        ) __reentrant
{
    uint32_t offset = 0;
    struct font_zh * font_zh = (struct font_zh *) font;

    (void) char_size;

    if (* string <= 0x7F)
    {
        * char_width = font_zh->width_ascii;
        * char_bytes = 1;

        for (offset = 0; offset < font_zh->table_size; offset += 1)
        {
            if (* string == font_zh->table[offset].key[0])
            {
                return &font_zh->table[offset].data[0];

                // Found zh_char, exit loop.
                break;
            }
            else
            {
                // Continue loop.
            }
        }

    }
    else
    {
        * char_width = font->width;
        * char_bytes = font->bytes;

        for (offset = 0; offset < font_zh->table_size; offset += 1)
        {
            //
            // NOTE: GBK/GB2312 2 bytes.
            //
            if (
                    (* string == font_zh->table[offset].key[0])
                    &&
                    (* (string + 1) == font_zh->table[offset].key[1])
            )
            {
                return &font_zh->table[offset].data[0];

                // Found zh_char, exit loop.
                break;
            }
            else
            {
                // Continue loop.
            }
        }
    }

    return 0;
}

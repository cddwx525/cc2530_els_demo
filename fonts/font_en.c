#include "font_en.h"

#include <stdint.h>

const uint8_t * find_char_en(
        struct font * font,
        uint8_t char_size,
        const uint8_t * string,
        uint8_t * char_width,
        uint8_t * char_bytes
        ) __reentrant
{
    struct font_en * font_en = (struct font_en *) font;
    uint32_t offset = (* string - ' ') * char_size;

    * char_width = font->width;
    * char_bytes = font->bytes;


    return &font_en->table[offset];
}

#ifndef __FONT_ZH_H
#define __FONT_ZH_H

#include "fonts.h"

#include <stdint.h>

const uint8_t * find_char_zh(
        struct font * font,
        uint8_t char_size,
        const uint8_t * string,
        uint8_t * char_width,
        uint8_t * char_bytes
        ) __reentrant;

#endif /* __FONT_ZH_H */

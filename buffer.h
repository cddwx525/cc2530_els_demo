#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include "fonts/fonts.h"

enum buffer_mode {
        BUFFER_MODE_MONO,
        BUFFER_MODE_MULTI,
        BUFFER_MODE_MIX,
        BUFFER_MODE_MIX_TERNARY,
        };

enum color_mode {
        COLOR_MODE_BW,
        COLOR_MODE_COLOR,
        };

enum dtm_stage {
        DTM_STAGE_1 = 0,
        DTM_STAGE_2 = 1,
        };


//
// NOTE: The final bit value of this pixel can be decided through below
//         table, is bitwise exclusive OR of color and polarity.
//
// KWR mode:
//     data[1:0] = color[1:0] ^ polarity[1:0]
// +----------------+-------------+-------------+-------------+-------------+
// |                | COLOR_WHITE | COLOR_BLACK | COLOR_RED   | COLOR_RED   |
// |                |      00     |      01     |      10     |      11     |
// |                |      ^^     |             |             |             |
// +                +-------------+-------------+-------------+-------------+
// |                |     LUTW    |     LUTB    |     LUTR    |     LUTR    |
// +----------------+-------------+-------------+-------------+-------------+
// | POL_RB  DDX 00 |     00      |     01      |     10      |     11      |
// |             ^^ |     ^^      |             |             |             |
// |                |  Red  B/W   |             |             |             |
// +----------------+-------------+-------------+-------------+-------------+
// | POL_RW  DDX 01 |     01      |     00      |     11      |     10      |
// +----------------+-------------+-------------+-------------+-------------+
// | POL_KB  DDX 10 |     10      |     11      |     00      |     01      |
// +----------------+-------------+-------------+-------------+-------------+
// | POL_KW  DDX 11 |     11      |     10      |     01      |     00      |
// +----------------+-------------+-------------+-------------+-------------+
//
// KW mode:
//     data[1] = (color[1:0] ^ polarity[1:0]) && 0x01
//     data[0] depend on OLD/NEW mode and LUTXX/LUTXY mode.
// +----------------+-------------+-------------+-------------+-------------+
// |                |       COLOR_WHITE         |         COLOR_BLACK       |
// |                |             00            |             01            |
// |                |              ^            |                           |
// +                +-------------+-------------+-------------+-------------+
// |                |     LUTWW   |    LUTBW    |    LUTWB    |    LUTBB    |
// +----------------+-------------+-------------+-------------+-------------+
// | POL_B   DDX 00 |     00      |     01      |     10      |     11      |
// |              ^ |     ^       |             |             |             |
// +----------------+-------------+-------------+-------------+-------------+
// | POL_W   DDX 01 |     11      |     10      |     01      |     00      |
// +----------------+-------------+-------------+-------------+-------------+
// | POL_B   DDX 10 |     --      |     0-      |     1-      |     --      |
// +----------------+-------------+-------------+-------------+-------------+
// | POL_W   DDX 11 |     --      |     1-      |     0-      |     --      |
// +----------------+-------------+-------------+-------------+-------------+
//
enum polarity {
        POLARITY_KWR_RB         = 0x00, // 00: 1->red, 1->black
        POLARITY_KWR_RW         = 0x01, // 01: 1->red, 1->white
        POLARITY_KWR_KB         = 0x02, // 10: 1->keep, 1->black
        POLARITY_KWR_KW         = 0x03, // 11: 1->keep, 1->white

        POLARITY_KW_OLDNEW_B    = 0x00, // 00: 1->black, old depends
        POLARITY_KW_OLDNEW_W    = 0x01, // 01: 1->white, old depends
        POLARITY_KW_NEW_B       = 0x02, // 10: 1->black, no old
        POLARITY_KW_NEW_W       = 0x03, // 11: 1->white, no old
        };

enum color {
        COLOR_WHITE  = 0x00, // 0b00
        COLOR_BLACK  = 0x01, // 0b01
        COLOR_RED    = 0x02, // 0b10
        };

enum rotate {
        ROTATE_0,
        ROTATE_90,
        ROTATE_180,
        ROTATE_270
        };

enum mirror {
        MIRROR_NONE,
        MIRROR_HORIZONTAL,
        MIRROR_VERTICAL,
        MIRROR_ORIGIN,
        };

enum point_size {
        POINT_SIZE_1 = 1, // 1x1
        POINT_SIZE_3 = 3, // 3x3
        POINT_SIZE_5 = 5, // 5x5
        POINT_SIZE_7 = 7, // 7x7
        POINT_SIZE_9 = 9, // 9x9
        };

enum point_style {
        POINT_STYLE_AROUND,
        POINT_STYLE_RIGHTUP,
        };

enum line_style {
        LINE_STYLE_SOLID,
        LINE_STYLE_DOTTED,
        };

enum fill_style {
        FILL_STYLE_EMPTY,
        FILL_STYLE_FULL,
        };

struct area {
        uint16_t x;
        uint16_t y;
        uint16_t width;
        uint16_t height;
        };

struct lookuptable {
        const uint8_t (* decode)[5];
        const uint8_t * mul3;
        const uint8_t * mul9;
        const uint8_t * mul27;
        const uint8_t * mul81;
        };


struct buffer {
        uint8_t * data;
        uint16_t size;
        enum buffer_mode buffer_mode;
        enum color_mode color_mode;
        struct lookuptable lookuptable;
        uint16_t width_hardware;
        uint16_t height_hardware;
        uint16_t ram_size;
        enum rotate rotate; // The degree from hardware direction to current.
        enum mirror mirror; // The mirror from hardware direction to current.
        enum polarity polarity;
        uint16_t width;
        uint16_t height;
        enum color color;
        enum dtm_stage dtm_stage;
        uint16_t cursor_x;
        uint16_t cursor_y;


        void (* draw_string)(
                struct buffer * this,
                void * font,
                const uint8_t * string
                ) __reentrant;

        void (* draw_circle)(
                struct buffer * this,
                uint16_t x_center, uint16_t y_center, uint16_t radius,
                enum point_size point_size,
                enum fill_style fill_style
                ) __reentrant;
        void (* draw_rectangle)(
                struct buffer * this,
                uint16_t x_start, uint16_t y_start,
                uint16_t width, uint16_t height,
                enum point_size point_size,
                enum fill_style fill_style
                ) __reentrant;
        void (* draw_line)(
                struct buffer * this,
                uint16_t x_start, uint16_t y_start,
                uint16_t x_end, uint16_t y_end,
                enum point_size point_size,
                enum line_style line_style
                ) __reentrant;
        void (* draw_point)(
                struct buffer * this,
                uint16_t x, uint16_t y,
                enum point_size point_size,
                enum point_style point_style
                ) __reentrant;

        void (* fill)(
                struct buffer * this
                ) __reentrant;

        void (* set_cursor)(
                struct buffer * this,
                uint16_t cursor_x, uint16_t cursor_y
                ) __reentrant;
        void (* set_color)(
                struct buffer * this,
                enum color color
                ) __reentrant;
        void (* set_dtm_stage)(
                struct buffer * this,
                enum dtm_stage dtm_stage
                ) __reentrant;

        uint8_t (* get_byte)(
                struct buffer * this,
                enum dtm_stage dtm_stage,
                uint16_t ram_offset
                ) __reentrant;
        };


struct buffer * Buffer(
        uint8_t * data,
        uint16_t size,
        enum buffer_mode buffer_mode,
        enum color_mode color_mode,
        uint16_t width_hardware,
        uint16_t height_hardware,
        enum rotate rotate,
        enum mirror mirror,
        enum polarity polarity
        );

#else

#endif /* #ifndef BUFFER_H */

//
// Chip: UC8151D
//
#ifndef EPD_H
#define EPD_H

#include <stdint.h>
#include "buffer.h"

#define PIN_EPD_DC      P0_2
#define PORT_EPD_DC     0
#define PNUM_EPD_DC     2

#define PIN_EPD_SDA    P0_3
#define PORT_EPD_SDA   0
#define PNUM_EPD_SDA   3

#define PIN_EPD_CS      P0_4
#define PORT_EPD_CS     0
#define PNUM_EPD_CS     4

#define PIN_EPD_SCL     P0_5
#define PORT_EPD_SCL    0
#define PNUM_EPD_SCL    5

#define PIN_EPD_RST     P1_1
#define PORT_EPD_RST    1
#define PNUM_EPD_RST    1

#define PIN_EPD_BUSY    P1_2
#define PORT_EPD_BUSY   1
#define PNUM_EPD_BUSY   2

#define EPD_SDA_LOW    (PIN_EPD_SDA = 0)
#define EPD_SDA_HIGH   (PIN_EPD_SDA = 1)

#define EPD_SCL_LOW     (PIN_EPD_SCL = 0)
#define EPD_SCL_HIGH    (PIN_EPD_SCL = 1)

#define EPD_CS_LOW      (PIN_EPD_CS = 0)
#define EPD_CS_HIGH     (PIN_EPD_CS = 1)

#define EPD_DC_LOW      (PIN_EPD_DC = 0)
#define EPD_DC_HIGH     (PIN_EPD_DC = 1)

#define EPD_RST_LOW     (PIN_EPD_RST = 0)
#define EPD_RST_HIGH    (PIN_EPD_RST = 1)

#define EPD_BUSY_LEVEL  0

#define EPD_WIDTH       ((uint16_t) 128)
#define EPD_HEIGHT      ((uint16_t) 296)
#define EPD_RAM_SIZE    ((uint16_t) 4736) // EPD_WIDTH / 8 * EPD_HEIGHT

//
// Multi part: (EPD_WIDTH / 8 * EPD_HEIGHT) = 4736
// Mix 3-base: (EPD_WIDTH * EPD_HEIGHT + 4) / 5 = 7578
//
#define EPD_BUFFER_SIZE ((uint16_t) 7578)

enum mode {
        MODE_KWR = 0,
        MODE_KW  = 1,
        };

enum lut_mode {
        LUT_MODE_OTP = 0,
        LUT_MODE_REG = 1,
        };

// VBD for KWR mode:
// +----------------+-------------+-------------+-------------+-------------+
// |                | FLT         | LUTR        | LUTW        | LUTB        |
// |                |      00     |      01     |      10     |      11     |
// +----------------+-------------+-------------+-------------+-------------+
// | DDX         00 |      00     |      01     |      10     |      11     |
// +----------------+-------------+-------------+-------------+-------------+
// | DDX         01 |      11     |      10     |      01     |      00     |
// +----------------+-------------+-------------+-------------+-------------+
// | DDX         10 |      00     |      01     |      10     |      11     |
// +----------------+-------------+-------------+-------------+-------------+
// | DDX         11 |      11     |      10     |      01     |      00     |
// +----------------+-------------+-------------+-------------+-------------+
//
// VBD for KW mode:
// +----------------+-------------+-------------+-------------+-------------+
// |                | FLT         | LUTBW       | LUTWB       | FLT_        |
// |                |      00     |      01     |      10     |      11     |
// +----------------+-------------+-------------+-------------+-------------+
// | DDX         00 |      00     |      01     |      10     |      11     |
// +----------------+-------------+-------------+-------------+-------------+
// | DDX         01 |      11     |      10     |      01     |      00     |
// +----------------+-------------+-------------+-------------+-------------+
//
enum vbd_mode {
        VBD_MODE_KWR_FLT     = 0x00,
        VBD_MODE_KWR_LUTR    = 0x01,
        VBD_MODE_KWR_LUTW    = 0x02,
        VBD_MODE_KWR_LUTB    = 0x03,

        VBD_MODE_KW_FLT      = 0x00,
        VBD_MODE_KW_LUTBW    = 0x01,
        VBD_MODE_KW_LUTWB    = 0x02,
        };

struct window {
        uint16_t x;
        uint16_t y;
        uint16_t width;
        uint16_t height;
        };

struct epd {
        struct buffer * buffer;
        struct window window;
        enum mode mode;
        enum lut_mode lut_mode;
        enum vbd_mode vbd_mode;
        struct lut_pack * lut_kwr;
        struct lut_pack * lut_kw;

        void (* read_otp)(struct epd * this) __reentrant;

        void (* update)(
                struct epd * this,
                uint16_t x, uint16_t y, uint16_t width, uint16_t height
                ) __reentrant;
        void (* display)(struct epd * this) __reentrant;

        void (* update_with_data)(
                struct epd * this,
                uint8_t * dtm1_data, uint8_t * dtm2_data,
                uint16_t x, uint16_t y, uint16_t width, uint16_t height
                ) __reentrant;
        void (* display_with_data)(
                struct epd * this,
                uint8_t * dtm1_data, uint8_t * dtm2_data
                ) __reentrant;

        void (* output_partial)(struct epd * this) __reentrant;
        void (* write_dtm2_partial)(
                struct epd * this,
                uint8_t * data
                ) __reentrant;
        void (* write_dtm1_partial)(
                struct epd * this,
                uint8_t * data
                ) __reentrant;
        void (* set_window)(
                struct epd * this,
                uint16_t x, uint16_t y, uint16_t width, uint16_t height
                ) __reentrant;

        void (* clean)(struct epd * this) __reentrant;

        void (* output)(struct epd * this) __reentrant;
        void (* write_dtm2)(struct epd * this, uint8_t * data) __reentrant;
        void (* write_dtm1)(struct epd * this, uint8_t * data) __reentrant;

        void (* sleep)(struct epd * this) __reentrant;
        };


struct epd * Epd(
        enum rotate rotate,
        enum mirror mirror,
        enum mode mode,
        enum lut_mode lut_mode,
        enum vbd_mode vbd_mode,
        enum polarity polarity,
        enum buffer_mode buffer_mode
        );

#else

#endif /* #ifndef EPD_H */

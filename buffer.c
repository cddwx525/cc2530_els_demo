/**
 *
 * NOTE:
 *     1. x_*, y_* must NOT bigger than half of the data type.
 *     2. x_*, y_* count from 0.
 *
 */

#include "buffer.h"

#include <stdint.h>
#include "fonts/fonts.h"

// Access the complete struct.
#define THIS    ((struct This *)(this))

////////////////////////////////////////////////////////////////////////////////
//
// Complete struct.
//
////////////////////////////////////////////////////////////////////////////////
struct This {
        //
        // Public
        //
        struct buffer buffer;

        //
        // Private
        //
        void (* copy)(
                struct buffer * this,
                const uint8_t * ptr,
                uint16_t width, uint16_t height
                ) __reentrant;
        void (* set_pixel)(
                struct buffer * this,
                uint16_t x, uint16_t y
                ) __reentrant;
        void (* write_pixel)(
                struct buffer * this,
                uint16_t x_hardware, uint16_t y_hardware
                ) __reentrant;
        };

////////////////////////////////////////////////////////////////////////////////
//
// Private functions.
//
////////////////////////////////////////////////////////////////////////////////
static void write_pixel_mono(
        struct buffer * this,
        uint16_t x_hardware, uint16_t y_hardware
        ) __reentrant
{
    //
    // Mono color buffer mode.
    //
    // 8 pixels/byte.
    //
    // 7 6 5 4 3 2 1 0
    // --- --- --- ---
    // | | | | | | | |
    // | | | | | | | +-- pixel 7
    // | | | | | | +---- pixel 6
    // | | | | | +------ pixel 5
    // | | | | +-------- pixel 4
    // | | | +---------- pixel 3
    // | | +------------ pixel 2
    // | +-------------- pixel 1
    // + --------------- pixel 0
    //
    uint16_t addr = (y_hardware * this->width_hardware + x_hardware) >> 3;
    uint8_t byte = this->data[addr];
    uint8_t mask = 0x80 >> (x_hardware & 0x07);

    if ((((this->color ^ this->polarity) >> this->dtm_stage) & 0x01) == 0)
    {
        byte &= ~mask;
    }
    else
    {
        byte |= mask;
    }
    this->data[addr] = byte;
}

static void write_pixel_multi(
        struct buffer * this,
        uint16_t x_hardware,
        uint16_t y_hardware
        ) __reentrant
{
    //
    // Multi part color buffer mode.
    //
    // 2 bits/pixel, save in two bytes.
    //
    // 7 6 5 4 3 2 1 0 (Byte n)
    // --- --- --- ---
    // | | | | | | | |
    // | | | | | | | +-- pixel 7
    // | | | | | | +---- pixel 6
    // | | | | | +------ pixel 5
    // | | | | +-------- pixel 4
    // | | | +---------- pixel 3
    // | | +------------ pixel 2
    // | +-------------- pixel 1
    // + --------------- pixel 0
    //
    // 7 6 5 4 3 2 1 0 (Byte (n + ram_size))
    // --- --- --- ---
    // | | | | | | | |
    // | | | | | | | +-- pixel 7
    // | | | | | | +---- pixel 6
    // | | | | | +------ pixel 5
    // | | | | +-------- pixel 4
    // | | | +---------- pixel 3
    // | | +------------ pixel 2
    // | +-------------- pixel 1
    // + --------------- pixel 0 
    //
    uint16_t addr = (y_hardware * this->width_hardware + x_hardware) >> 3;
    uint8_t byte = this->data[addr];
    uint8_t mask = 0x80 >> (x_hardware & 0x07);
    uint8_t color_value = this->color ^ this->polarity;

    if ((color_value & 0x01) == 0)
    {
        byte &= ~mask;
    }
    else
    {
        byte |= mask;
    }
    this->data[addr] = byte;

    addr += this->ram_size;
    byte = this->data[addr];
    if ((color_value & 0x02) == 0)
    {
        byte &= ~mask;
    }
    else
    {
        byte |= mask;
    }
    this->data[addr] = byte;
}

static void write_pixel_mix(
        struct buffer * this,
        uint16_t x_hardware,
        uint16_t y_hardware
        ) __reentrant
{
    //
    // Mix color buffer mode.
    //
    // 2 bits/pixel, 4 pixels/byte.
    //
    // 7 6 5 4 3 2 1 0
    // --- --- --- ---
    //  |   |   |   |
    //  |   |   |   +--- pixel 3
    //  |   |   +------- pixel 2
    //  |   +----------- pixel 1
    //  +--------------- pixel 0
    //
    uint16_t addr = (y_hardware * this->width_hardware + x_hardware) >> 2;
    uint8_t byte = this->data[addr];
    uint8_t pixel_position = x_hardware & 0x03;
    uint8_t color_value = this->color ^ this->polarity;
    uint8_t mask = 0x40 >> (pixel_position << 1);

    if ((color_value & 0x01) == 0)
    {
        byte &= ~mask;
    }
    else
    {
        byte |= mask;
    }

    mask = 0x80 >> (pixel_position << 1);
    if ((color_value & 0x02) == 0)
    {
        byte &= ~mask;
    }
    else
    {
        byte |= mask;
    }

    this->data[addr] = byte;
}

static void write_pixel_mix_ternary(
        struct buffer * this,
        uint16_t x_hardware,
        uint16_t y_hardware
        ) __reentrant
{
    //
    // Mix color buffer mode, 3-base method.
    //
    // Byte range: 0 -- 242(3^5 - 1).
    // Bits      : 5.
    // 1 bits/pixel, 5 pixels/byte.
    //
    //  4   3   2   1   0
    // --- --- --- --- ---
    //  |   |   |   |   |
    //  |   |   |   |   +--- pixel 4
    //  |   |   |   +------- pixel 3
    //  |   |   +----------- pixel 2
    //  |   +--------------- pixel 1
    //  +------------------- pixel 0
    //
    uint16_t pixel_offset = y_hardware * this->width_hardware + x_hardware;
    uint16_t addr = pixel_offset / 5;
    uint8_t byte = this->data[addr];

    //
    // Old method.
    //
    //uint8_t pixels[5] = {0};
    //uint8_t i = 0;
    //for (i = 5; i > 0; i -= 1)
    //{
    //    pixels[i - 1] = byte % 3;
    //    byte /= 3;
    //}
    //pixels[pixel_offset % 5] = this->color;
    //this->data[addr] = (
    //        ((pixels[0] * 3 + pixels[1]) * 3 + pixels[2]) * 3 + pixels[3]
    //        ) * 3 + pixels[4];

    //
    // Fast method.
    //
    const uint8_t * base = this->lookuptable.decode[byte];
    uint8_t pixels[5] = {0};
    uint8_t i = 0;
    for (i = 0; i < 5; i += 1)
    {
        pixels[i] = base[i];
    }
    pixels[pixel_offset % 5] = this->color;

    this->data[addr] = this->lookuptable.mul81[pixels[0]]
            + this->lookuptable.mul27[pixels[1]]
            + this->lookuptable.mul9[pixels[2]]
            + this->lookuptable.mul3[pixels[3]]
            + pixels[4];
}



static void set_pixel(
        struct buffer * this,
        uint16_t x, uint16_t y
        ) __reentrant
{
    if (
            x > (this->width - 1)
            || y > (this->height - 1)
            )
    {
        return;
    }
    else
    {
    }

    uint16_t x_hardware = x;
    uint16_t y_hardware = y;

    switch(this->rotate)
    {
        case ROTATE_90:
            x_hardware = this->width_hardware - 1 - y;
            y_hardware = x;
            break;

        case ROTATE_180:
            x_hardware = this->width_hardware - 1 - x;
            y_hardware = this->height_hardware - 1 - y;
            break;

        case ROTATE_270:
            x_hardware = y;
            y_hardware = this->height_hardware - 1 - x;
            break;

        case ROTATE_0:
            break;

        default:
            return;
    }

    switch(this->mirror)
    {
        case MIRROR_HORIZONTAL:
            x_hardware = this->width_hardware - 1 - x_hardware;
            break;

        case MIRROR_VERTICAL:
            y_hardware = this->height_hardware - 1 - y_hardware;
            break;

        case MIRROR_ORIGIN:
            x_hardware = this->width_hardware - 1 - x_hardware;
            y_hardware = this->height_hardware - 1 - y_hardware;
            break;

        case MIRROR_NONE:
            break;

        default:
            return;
    }

    THIS->write_pixel(this, x_hardware, y_hardware);
}

static void copy(
        struct buffer * this,
        const uint8_t * ptr,
        uint16_t width, uint16_t height
        ) __reentrant
{
    uint8_t row_bytes = (width + 7) >> 3;
    uint8_t row = 0;
    uint8_t column = 0;
    for (row = 0; row < height; row += 1)
    {
        for (column = 0; column < width; column += 1)
        {
            if (* (ptr + (column >> 3)) & (0x80 >> (column & 0x07)))
            {
                // Bit is 1.
                THIS->set_pixel(
                        this,
                        this->cursor_x + column,
                        this->cursor_y + row
                        );
            }
            else
            {
            }
        }

        ptr += row_bytes;
    }
}



////////////////////////////////////////////////////////////////////////////////
//
// Public functions.
//
////////////////////////////////////////////////////////////////////////////////
static uint8_t get_byte_mono(
        struct buffer * this,
        enum dtm_stage dtm_stage,
        uint16_t ram_offset
        ) __reentrant
{
    (void) dtm_stage;
    return  this->data[ram_offset];
}

static uint8_t get_byte_multi(
        struct buffer * this,
        enum dtm_stage dtm_stage,
        uint16_t ram_offset
        ) __reentrant
{
    uint16_t addr = ram_offset;
    uint8_t i = 0;

    for (i = 0; i < dtm_stage; i += 1)
    {
        addr += this->ram_size;
    }

    return this->data[addr];
}


static uint8_t get_byte_mix(
        struct buffer * this,
        enum dtm_stage dtm_stage,
        uint16_t ram_offset
        ) __reentrant
{
    uint16_t addr = ram_offset << 1;

    //
    // Read 2 bytes from buffer, 8 pixels.
    //
    uint8_t byte1 = this->data[addr];
    uint8_t byte2 = this->data[addr + 1];

    uint8_t ram_byte = 0x00;
    uint8_t i = 0;
    for (i = 0; i < 4; i += 1)
    {
        ram_byte |= (((byte1 >> (i << 1)) >> dtm_stage) & 0x01) << (4 + i);
        ram_byte |= (((byte2 >> (i << 1)) >> dtm_stage) & 0x01) << i;
    }

    return ram_byte;
}

static uint8_t get_byte_mix_ternary(
        struct buffer * this,
        enum dtm_stage dtm_stage,
        uint16_t ram_offset
        ) __reentrant
{
    //
    // Byte location in buffer.
    //
    uint16_t pixel_offset = ram_offset << 3;
    uint16_t addr = pixel_offset / 5;

    //
    // Read 3 bytes from buffer, get enough data for 8 pixels at current
    // RAM byte.
    //
    uint8_t byte1 = this->data[addr];
    uint8_t byte2 = this->data[addr + 1];
    uint8_t byte3 = 0x00;

    if ((addr + 2) > (this->size - 1))
    {
        // Stop, set to 0, no problem, because this only happen at
        // last one, so we will not read it.
        byte3 = 0x00;
    }
    else
    {
        byte3 = this->data[addr + 2];
    }

    //
    // Old method.
    //
    //for (i = 5; i > 0; i -= 1)
    //{
    //    pixels[i - 1] = byte1 % 3;
    //    pixels[i + 5 - 1] = byte2 % 3;
    //    pixels[i + 10 - 1] = byte3 % 3;
    //    byte1 /= 3;
    //    byte2 /= 3;
    //    byte3 /= 3;
    //}

    //
    // Fast method,
    //
    const uint8_t * base1 = this->lookuptable.decode[byte1];
    const uint8_t * base2 = this->lookuptable.decode[byte2];
    const uint8_t * base3 = this->lookuptable.decode[byte3];
    uint8_t pixels[15] = {0};
    uint8_t i = 0;

    for (i = 0; i < 5; i += 1)
    {
        pixels[i] = base1[i];
        pixels[i + 5] = base2[i];
        pixels[i + 10] = base3[i];
    }

    //
    // Get pixels value for 8 B/W pixels, form one byte.
    //
    uint8_t ram_byte = 0x00;
    // The position of first pixel in buffer byte.
    uint8_t start = pixel_offset % 5;

    for (i = start; i < start + 8; i += 1)
    {
        // Get bit, shift to position.
        ram_byte |= (
                ((pixels[i] ^ this->polarity) >> dtm_stage) & 0x01
                )
                << (7 - (i - start));
    }

    return ram_byte;
}

static void set_dtm_stage(
        struct buffer * this,
        enum dtm_stage dtm_stage
        ) __reentrant
{
    this->dtm_stage = dtm_stage;
}

static void set_color(
        struct buffer * this,
        enum color color
        ) __reentrant
{
    if (this->color_mode == COLOR_MODE_COLOR)
    {
        this->color = color;
    }
    else
    {
        this->color = color > COLOR_BLACK ? COLOR_BLACK : color;
    }
}

static void set_cursor(
        struct buffer * this,
        uint16_t cursor_x, uint16_t cursor_y
        ) __reentrant
{
    if (
            cursor_x > (this->width - 1)
            || cursor_y > (this->height - 1)
            )
    {
        return;
    }
    else
    {
    }

    this->cursor_x = cursor_x;
    this->cursor_y = cursor_y;
}

static void fill(struct buffer * this)
{
    uint8_t byte = 0x00;
    uint8_t byte1 = 0x00;
    uint8_t byte2 = 0x00;
    uint16_t addr_offset = 0;
    uint8_t i = 0;
    uint8_t color_value = this->color ^ this->polarity;


    switch (this->buffer_mode)
    {
        case BUFFER_MODE_MONO:
            if (((color_value >> this->dtm_stage) & 0x01) == 0)
            {
                byte = 0x00;
            }
            else
            {
                byte = 0xFF;
            }
            break;

        case BUFFER_MODE_MULTI:
            if ((color_value & 0x01) == 0)
            {
                byte1 = 0x00;
            }
            else
            {
                byte1 = 0xFF;
            }

            if ((color_value & 0x02) ==00)
            {
                byte2 = 0x00;
            }
            else
            {
                byte2 = 0xFF;
            }
            break;

        case BUFFER_MODE_MIX:
            byte = color_value;
            for (i = 2; i < 8; i += 2)
            {
                byte |= byte << i;
            }
            break;

        case BUFFER_MODE_MIX_TERNARY:
            //
            // 3-base method.
            //
            //
            // Old method.
            //
            //uint8_t byte;
            //byte = (
            //        (
            //                (this->color * 3 + this->color) * 3 + this->color
            //                ) * 3 + this->color
            //        ) * 3 + this->color;

            //
            // Fast method.
            //
            byte = this->lookuptable.mul81[this->color]
                    + this->lookuptable.mul27[this->color]
                    + this->lookuptable.mul9[this->color]
                    + this->lookuptable.mul3[this->color]
                    + this->color;
            break;

        default:
            return;
    }

    if (this->buffer_mode == BUFFER_MODE_MULTI)
    {
        for (addr_offset = 0; addr_offset < this->ram_size; addr_offset += 1)
        {
            this->data[addr_offset] = byte1;
            this->data[addr_offset + this->ram_size] = byte2;
        }
    }
    else
    {
        for (addr_offset = 0; addr_offset < this->size; addr_offset += 1)
        {
            this->data[addr_offset] = byte;
        }
    }
}

static void draw_point(
        struct buffer * this,
        uint16_t x, uint16_t y,
        enum point_size point_size,
        enum point_style point_style
        ) __reentrant
{
    if (
            x > (this->width - 1)
            || y > (this->height - 1)
            )
    {
        return;
    }
    else
    {
    }

    uint16_t x_offset = 0;
    uint16_t y_offset = 0;

    if (point_style == POINT_STYLE_AROUND)
    {
        for (x_offset = 0; x_offset < point_size; x_offset += 1)
        {
            for (y_offset = 0; y_offset < point_size; y_offset += 1)
            {
                if (
                        (int16_t) x - (((int16_t) point_size - 1) >> 1)
                                + (int16_t) x_offset < 0
                        ||
                        (int16_t) y - (((int16_t) point_size - 1) >> 1)
                                + (int16_t) y_offset < 0
                        )
                {
                    break;
                }
                else
                {
                }

                THIS->set_pixel(
                        this,
                        x - ((point_size - 1) >> 1) + x_offset,
                        y - ((point_size - 1) >> 1) + y_offset
                        );
            }
        }
    }
    else
    {
        for (x_offset = 0; x_offset < point_size; x_offset += 1)
        {
            for (y_offset = 0; y_offset < point_size; y_offset += 1)
            {
                THIS->set_pixel(
                        this,
                        x + x_offset, y + y_offset
                        );
            }
        }
    }
}


static void draw_line(
        struct buffer * this,
        uint16_t x_start, uint16_t y_start,
        uint16_t x_end, uint16_t y_end,
        enum point_size point_size,
        enum line_style line_style
        ) __reentrant
{
    if (
            x_start > (this->width - 1)
            || y_start > (this->height - 1)
            || x_end > (this->width - 1)
            || y_end > (this->height - 1)
            )
    {
        return;
    }
    else
    {
    }

    ///////////////////////////////////
    //
    // Bresenham's line algorithm.
    //
    ///////////////////////////////////
    int16_t x_position = (int16_t) x_start;
    int16_t y_position = (int16_t) y_start;
    uint16_t dx = x_start < x_end ? x_end - x_start : x_start - x_end;
    uint16_t dy = y_start < y_end ? y_end - y_start : y_start - y_end;
    uint16_t dx_mul2 = dx << 1;
    uint16_t dy_mul2 = dy << 1;
    int8_t x_step = x_start < x_end ? point_size : 0 - point_size;
    int8_t y_step = y_start < y_end ? point_size : 0 - point_size;
    uint16_t real = 0;
    uint16_t target = 0;

    uint8_t dotted_len = 0;

    if (dx >= dy)
    {
        real = dy_mul2;
        target = dx;
        while (1)
        {
            //
            // Painted dotted line, 2 point + 1 blank.
            //
            if (line_style == LINE_STYLE_DOTTED && dotted_len == 2)
            {
                dotted_len = 0;
            }
            else
            {
                this->draw_point(
                        this,
                        x_position, y_position,
                        point_size,
                        POINT_STYLE_AROUND
                        );
                dotted_len += 1;
            }

            x_position += x_step;

            if (
                    (x_step > 0 && x_position > (int16_t) x_end)
                    ||
                    (x_step < 0 && x_position < (int16_t) x_end)
                    )
            {
                break;
            }
            else
            {
                if (real >= target)
                {
                    y_position += y_step;
                    target += dx_mul2;
                }
                else
                {
                }
                real += dy_mul2;
            }
        }
    }
    else
    {
        real = dx_mul2;
        target = dy;
        while (1)
        {
            //
            // Painted dotted line, 2 point + 1 blank.
            //
            if (line_style == LINE_STYLE_DOTTED && dotted_len == 2)
            {
                dotted_len = 0;
            }
            else
            {
                this->draw_point(
                        this,
                        x_position, y_position,
                        point_size,
                        POINT_STYLE_AROUND
                        );
                dotted_len += 1;
            }

            y_position += y_step;

            if (
                    (y_step > 0 && y_position > (int16_t) y_end)
                    ||
                    (y_step < 0 && y_position < (int16_t) y_end)
                    )
            {
                break;
            }
            else
            {
                if (real >= target)
                {
                    x_position += x_step;
                    target += dy_mul2;
                }
                else
                {
                }
                real += dx_mul2;
            }
        }
    }

    //
    // Last point.
    //
    // Last point maybe a blank or point not at point_size border, we just draw
    // it instead check what is.
    //
    this->draw_point(
            this,
            x_end, y_end,
            point_size,
            POINT_STYLE_AROUND
            );
}

static void draw_rectangle(
        struct buffer * this,
        uint16_t x_start, uint16_t y_start,
        uint16_t width, uint16_t height,
        enum point_size point_size,
        enum fill_style fill_style
        ) __reentrant
{
    if (
            x_start > (this->width - 1)
            || y_start > (this->height - 1)
            || (width > this->width)
            || (width == 0)
            || (height > this->height)
            || (height == 0)
            )
    {
        return;
    }
    else
    {
    }

    uint16_t x_end = x_start + width <= this->width
            ? x_start + width - 1 : this->width - 1;
    uint16_t y_end = y_start + height <= this->height
            ? y_start + height - 1 : this->height - 1;
    uint16_t y_position = y_start;

    if (fill_style == FILL_STYLE_FULL)
    {
        for (y_position = y_start; y_position <= y_end; y_position += 1)
        {
            this->draw_line(
                    this,
                    x_start, y_position, x_end, y_position,
                    point_size,
                    LINE_STYLE_SOLID
                    );
        }
    }
    else
    {
        this->draw_line(
                this,
                x_start, y_start, x_end, y_start,
                point_size,
                LINE_STYLE_SOLID
                );
        this->draw_line(
                this,
                x_start, y_start, x_start, y_end,
                point_size,
                LINE_STYLE_SOLID
                );
        this->draw_line(
                this,
                x_end, y_end, x_end, y_start,
                point_size,
                LINE_STYLE_SOLID
                );
        this->draw_line(
                this,
                x_end, y_end, x_start, y_end,
                point_size,
                LINE_STYLE_SOLID
                );
    }
}

static void draw_circle(
        struct buffer * this,
        uint16_t x_center, uint16_t y_center, uint16_t radius,
        enum point_size point_size,
        enum fill_style fill_style
        ) __reentrant
{
    if (
            x_center > (this->width - 1)
            || y_center > (this->height - 1)
            )
    {
        return;
    }
    else
    {
    }

    //
    // Draw a circle from(0, R) as a starting point.
    //
    uint16_t x_current = 0;
    uint16_t y_current = radius;

    //
    // Cumulative error,judge the next point of the logo.
    //
    int16_t esp = 3 - (radius << 1);

    uint16_t s_count_y = 0;

    if (fill_style == FILL_STYLE_FULL)
    {
        while (x_current <= y_current)
        {
            //
            // Realistic circles.
            //
            for (s_count_y = x_current; s_count_y <= y_current; s_count_y += 1)
            {
                this->draw_point(
                        this,
                        x_center + x_current, y_center + s_count_y,
                        point_size, POINT_STYLE_AROUND
                        );//1
                this->draw_point(
                        this,
                        x_center - x_current, y_center + s_count_y,
                        point_size, POINT_STYLE_AROUND
                        );//2
                this->draw_point(
                        this,
                        x_center - s_count_y, y_center + x_current,
                        point_size, POINT_STYLE_AROUND
                        );//3
                this->draw_point(
                        this,
                        x_center - s_count_y, y_center - x_current,
                        point_size, POINT_STYLE_AROUND
                        );//4
                this->draw_point(
                        this,
                        x_center - x_current, y_center - s_count_y,
                        point_size, POINT_STYLE_AROUND
                        );//5
                this->draw_point(
                        this,
                        x_center + x_current, y_center - s_count_y,
                        point_size, POINT_STYLE_AROUND
                        );//6
                this->draw_point(
                        this,
                        x_center + s_count_y, y_center - x_current,
                        point_size, POINT_STYLE_AROUND
                        );//7
                this->draw_point(
                        this,
                        x_center + s_count_y, y_center + x_current,
                        point_size, POINT_STYLE_AROUND
                        );//8
            }

            if (esp < 0 )
            {
                esp += (x_current << 2) + 6;
            }
            else
            {
                esp += ((x_current - y_current) << 2) + 10;
                y_current -= 1;
            }

            x_current += 1;
        }
    }
    else
    {
        //
        // Draw a hollow circle.
        //
        while (x_current <= y_current)
        {
            this->draw_point(
                    this,
                    x_center + x_current, y_center + y_current,
                    point_size, POINT_STYLE_AROUND
                    );//1
            this->draw_point(
                    this,
                    x_center - x_current, y_center + y_current,
                    point_size, POINT_STYLE_AROUND
                    );//2
            this->draw_point(
                    this,
                    x_center - y_current,
                    y_center + x_current,
                    point_size, POINT_STYLE_AROUND
                    );//3
            this->draw_point(
                    this,
                    x_center - y_current, y_center - x_current,
                    point_size, POINT_STYLE_AROUND
                    );//4
            this->draw_point(
                    this,
                    x_center - x_current, y_center - y_current,
                    point_size, POINT_STYLE_AROUND
                    );//5
            this->draw_point(
                    this,
                    x_center + x_current, y_center - y_current,
                    point_size, POINT_STYLE_AROUND
                    );//6
            this->draw_point(
                    this,
                    x_center + y_current, y_center - x_current,
                    point_size, POINT_STYLE_AROUND
                    );//7
            this->draw_point(
                    this,
                    x_center + y_current, y_center + x_current,
                    point_size, POINT_STYLE_AROUND
                    );//0

            if (esp < 0 )
            {
                esp += (x_current << 2) + 6;
            }
            else
            {
                esp += ((x_current - y_current) << 2) + 10;
                y_current -= 1;
            }

            x_current += 1;
        }
    }
}


static void draw_string(
        struct buffer * this,
        void * font,
        const uint8_t * string
        ) __reentrant
{
    uint16_t x_start = this->cursor_x;
    uint16_t y_start = this->cursor_y;

    struct font * font_base = (struct font *) font;
    const uint8_t * ptr = 0;
    uint8_t char_width = 0;
    uint8_t char_bytes = 0;
    uint8_t char_size = font_base->height * ((font_base->width + 7) >> 3);

    while (* string != '\0')
    {
        ptr = font_base->find_char(
                font_base,
                char_size,
                string,
                &char_width, &char_bytes
                );

        //
        // If X direction full, goto next line.
        //
        if (
                (this->cursor_x + char_width)
                >
                (this->width - 1)
                )
        {
            this->cursor_x = x_start;
            this->cursor_y += font_base->height;
        }
        else
        {
        }

        //
        // If Y direction full, goto start point.
        //
        if (
                (this->cursor_y + font_base->height)
                >
                (this->height - 1)
                )
        {
            this->cursor_x = x_start;
            this->cursor_y = y_start;
        }
        else
        {
        }

        if (ptr != 0)
        {
            THIS->copy(this, ptr, char_width, font_base->height);
        }
        else
        {
            // Not found char.
        }

        this->cursor_x += char_width;

        string += char_bytes;
    }
}



////////////////////////////////////////////////////////////////////////////////
//
// External functions.
//
////////////////////////////////////////////////////////////////////////////////

static struct This This;

__code static const uint8_t decode_table[243][5] = {
        {0, 0, 0, 0, 0, },
        {0, 0, 0, 0, 1, },
        {0, 0, 0, 0, 2, },
        {0, 0, 0, 1, 0, },
        {0, 0, 0, 1, 1, },
        {0, 0, 0, 1, 2, },
        {0, 0, 0, 2, 0, },
        {0, 0, 0, 2, 1, },
        {0, 0, 0, 2, 2, },
        {0, 0, 1, 0, 0, },
        {0, 0, 1, 0, 1, },
        {0, 0, 1, 0, 2, },
        {0, 0, 1, 1, 0, },
        {0, 0, 1, 1, 1, },
        {0, 0, 1, 1, 2, },
        {0, 0, 1, 2, 0, },
        {0, 0, 1, 2, 1, },
        {0, 0, 1, 2, 2, },
        {0, 0, 2, 0, 0, },
        {0, 0, 2, 0, 1, },
        {0, 0, 2, 0, 2, },
        {0, 0, 2, 1, 0, },
        {0, 0, 2, 1, 1, },
        {0, 0, 2, 1, 2, },
        {0, 0, 2, 2, 0, },
        {0, 0, 2, 2, 1, },
        {0, 0, 2, 2, 2, },
        {0, 1, 0, 0, 0, },
        {0, 1, 0, 0, 1, },
        {0, 1, 0, 0, 2, },
        {0, 1, 0, 1, 0, },
        {0, 1, 0, 1, 1, },
        {0, 1, 0, 1, 2, },
        {0, 1, 0, 2, 0, },
        {0, 1, 0, 2, 1, },
        {0, 1, 0, 2, 2, },
        {0, 1, 1, 0, 0, },
        {0, 1, 1, 0, 1, },
        {0, 1, 1, 0, 2, },
        {0, 1, 1, 1, 0, },
        {0, 1, 1, 1, 1, },
        {0, 1, 1, 1, 2, },
        {0, 1, 1, 2, 0, },
        {0, 1, 1, 2, 1, },
        {0, 1, 1, 2, 2, },
        {0, 1, 2, 0, 0, },
        {0, 1, 2, 0, 1, },
        {0, 1, 2, 0, 2, },
        {0, 1, 2, 1, 0, },
        {0, 1, 2, 1, 1, },
        {0, 1, 2, 1, 2, },
        {0, 1, 2, 2, 0, },
        {0, 1, 2, 2, 1, },
        {0, 1, 2, 2, 2, },
        {0, 2, 0, 0, 0, },
        {0, 2, 0, 0, 1, },
        {0, 2, 0, 0, 2, },
        {0, 2, 0, 1, 0, },
        {0, 2, 0, 1, 1, },
        {0, 2, 0, 1, 2, },
        {0, 2, 0, 2, 0, },
        {0, 2, 0, 2, 1, },
        {0, 2, 0, 2, 2, },
        {0, 2, 1, 0, 0, },
        {0, 2, 1, 0, 1, },
        {0, 2, 1, 0, 2, },
        {0, 2, 1, 1, 0, },
        {0, 2, 1, 1, 1, },
        {0, 2, 1, 1, 2, },
        {0, 2, 1, 2, 0, },
        {0, 2, 1, 2, 1, },
        {0, 2, 1, 2, 2, },
        {0, 2, 2, 0, 0, },
        {0, 2, 2, 0, 1, },
        {0, 2, 2, 0, 2, },
        {0, 2, 2, 1, 0, },
        {0, 2, 2, 1, 1, },
        {0, 2, 2, 1, 2, },
        {0, 2, 2, 2, 0, },
        {0, 2, 2, 2, 1, },
        {0, 2, 2, 2, 2, },
        {1, 0, 0, 0, 0, },
        {1, 0, 0, 0, 1, },
        {1, 0, 0, 0, 2, },
        {1, 0, 0, 1, 0, },
        {1, 0, 0, 1, 1, },
        {1, 0, 0, 1, 2, },
        {1, 0, 0, 2, 0, },
        {1, 0, 0, 2, 1, },
        {1, 0, 0, 2, 2, },
        {1, 0, 1, 0, 0, },
        {1, 0, 1, 0, 1, },
        {1, 0, 1, 0, 2, },
        {1, 0, 1, 1, 0, },
        {1, 0, 1, 1, 1, },
        {1, 0, 1, 1, 2, },
        {1, 0, 1, 2, 0, },
        {1, 0, 1, 2, 1, },
        {1, 0, 1, 2, 2, },
        {1, 0, 2, 0, 0, },
        {1, 0, 2, 0, 1, },
        {1, 0, 2, 0, 2, },
        {1, 0, 2, 1, 0, },
        {1, 0, 2, 1, 1, },
        {1, 0, 2, 1, 2, },
        {1, 0, 2, 2, 0, },
        {1, 0, 2, 2, 1, },
        {1, 0, 2, 2, 2, },
        {1, 1, 0, 0, 0, },
        {1, 1, 0, 0, 1, },
        {1, 1, 0, 0, 2, },
        {1, 1, 0, 1, 0, },
        {1, 1, 0, 1, 1, },
        {1, 1, 0, 1, 2, },
        {1, 1, 0, 2, 0, },
        {1, 1, 0, 2, 1, },
        {1, 1, 0, 2, 2, },
        {1, 1, 1, 0, 0, },
        {1, 1, 1, 0, 1, },
        {1, 1, 1, 0, 2, },
        {1, 1, 1, 1, 0, },
        {1, 1, 1, 1, 1, },
        {1, 1, 1, 1, 2, },
        {1, 1, 1, 2, 0, },
        {1, 1, 1, 2, 1, },
        {1, 1, 1, 2, 2, },
        {1, 1, 2, 0, 0, },
        {1, 1, 2, 0, 1, },
        {1, 1, 2, 0, 2, },
        {1, 1, 2, 1, 0, },
        {1, 1, 2, 1, 1, },
        {1, 1, 2, 1, 2, },
        {1, 1, 2, 2, 0, },
        {1, 1, 2, 2, 1, },
        {1, 1, 2, 2, 2, },
        {1, 2, 0, 0, 0, },
        {1, 2, 0, 0, 1, },
        {1, 2, 0, 0, 2, },
        {1, 2, 0, 1, 0, },
        {1, 2, 0, 1, 1, },
        {1, 2, 0, 1, 2, },
        {1, 2, 0, 2, 0, },
        {1, 2, 0, 2, 1, },
        {1, 2, 0, 2, 2, },
        {1, 2, 1, 0, 0, },
        {1, 2, 1, 0, 1, },
        {1, 2, 1, 0, 2, },
        {1, 2, 1, 1, 0, },
        {1, 2, 1, 1, 1, },
        {1, 2, 1, 1, 2, },
        {1, 2, 1, 2, 0, },
        {1, 2, 1, 2, 1, },
        {1, 2, 1, 2, 2, },
        {1, 2, 2, 0, 0, },
        {1, 2, 2, 0, 1, },
        {1, 2, 2, 0, 2, },
        {1, 2, 2, 1, 0, },
        {1, 2, 2, 1, 1, },
        {1, 2, 2, 1, 2, },
        {1, 2, 2, 2, 0, },
        {1, 2, 2, 2, 1, },
        {1, 2, 2, 2, 2, },
        {2, 0, 0, 0, 0, },
        {2, 0, 0, 0, 1, },
        {2, 0, 0, 0, 2, },
        {2, 0, 0, 1, 0, },
        {2, 0, 0, 1, 1, },
        {2, 0, 0, 1, 2, },
        {2, 0, 0, 2, 0, },
        {2, 0, 0, 2, 1, },
        {2, 0, 0, 2, 2, },
        {2, 0, 1, 0, 0, },
        {2, 0, 1, 0, 1, },
        {2, 0, 1, 0, 2, },
        {2, 0, 1, 1, 0, },
        {2, 0, 1, 1, 1, },
        {2, 0, 1, 1, 2, },
        {2, 0, 1, 2, 0, },
        {2, 0, 1, 2, 1, },
        {2, 0, 1, 2, 2, },
        {2, 0, 2, 0, 0, },
        {2, 0, 2, 0, 1, },
        {2, 0, 2, 0, 2, },
        {2, 0, 2, 1, 0, },
        {2, 0, 2, 1, 1, },
        {2, 0, 2, 1, 2, },
        {2, 0, 2, 2, 0, },
        {2, 0, 2, 2, 1, },
        {2, 0, 2, 2, 2, },
        {2, 1, 0, 0, 0, },
        {2, 1, 0, 0, 1, },
        {2, 1, 0, 0, 2, },
        {2, 1, 0, 1, 0, },
        {2, 1, 0, 1, 1, },
        {2, 1, 0, 1, 2, },
        {2, 1, 0, 2, 0, },
        {2, 1, 0, 2, 1, },
        {2, 1, 0, 2, 2, },
        {2, 1, 1, 0, 0, },
        {2, 1, 1, 0, 1, },
        {2, 1, 1, 0, 2, },
        {2, 1, 1, 1, 0, },
        {2, 1, 1, 1, 1, },
        {2, 1, 1, 1, 2, },
        {2, 1, 1, 2, 0, },
        {2, 1, 1, 2, 1, },
        {2, 1, 1, 2, 2, },
        {2, 1, 2, 0, 0, },
        {2, 1, 2, 0, 1, },
        {2, 1, 2, 0, 2, },
        {2, 1, 2, 1, 0, },
        {2, 1, 2, 1, 1, },
        {2, 1, 2, 1, 2, },
        {2, 1, 2, 2, 0, },
        {2, 1, 2, 2, 1, },
        {2, 1, 2, 2, 2, },
        {2, 2, 0, 0, 0, },
        {2, 2, 0, 0, 1, },
        {2, 2, 0, 0, 2, },
        {2, 2, 0, 1, 0, },
        {2, 2, 0, 1, 1, },
        {2, 2, 0, 1, 2, },
        {2, 2, 0, 2, 0, },
        {2, 2, 0, 2, 1, },
        {2, 2, 0, 2, 2, },
        {2, 2, 1, 0, 0, },
        {2, 2, 1, 0, 1, },
        {2, 2, 1, 0, 2, },
        {2, 2, 1, 1, 0, },
        {2, 2, 1, 1, 1, },
        {2, 2, 1, 1, 2, },
        {2, 2, 1, 2, 0, },
        {2, 2, 1, 2, 1, },
        {2, 2, 1, 2, 2, },
        {2, 2, 2, 0, 0, },
        {2, 2, 2, 0, 1, },
        {2, 2, 2, 0, 2, },
        {2, 2, 2, 1, 0, },
        {2, 2, 2, 1, 1, },
        {2, 2, 2, 1, 2, },
        {2, 2, 2, 2, 0, },
        {2, 2, 2, 2, 1, },
        {2, 2, 2, 2, 2, },
        };
__code static const uint8_t mul3_table[] = {0, 3, 6};
__code static const uint8_t mul9_table[] = {0, 9, 18};
__code static const uint8_t mul27_table[] = {0, 27, 54};
__code static const uint8_t mul81_table[] = {0, 81, 162};

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
        )
{
    ////////////////////////////////////////////////////////////////////////////
    // Bind functions.
    ////////////////////////////////////////////////////////////////////////////
    //
    // Public.
    //
    This.buffer.draw_string = draw_string;

    This.buffer.draw_rectangle = draw_rectangle;
    This.buffer.draw_circle = draw_circle;
    This.buffer.draw_line = draw_line;
    This.buffer.draw_point = draw_point;

    This.buffer.fill = fill;

    This.buffer.set_cursor = set_cursor;
    This.buffer.set_color = set_color;
    This.buffer.set_dtm_stage = set_dtm_stage;
    //This.buffer.get_byte = ;

    //
    // Private.
    //
    This.copy = copy;
    This.set_pixel = set_pixel;
    //This.write_pixel = ;


    switch (buffer_mode)
    {
        case BUFFER_MODE_MONO:
            This.buffer.get_byte = get_byte_mono;

            This.write_pixel = write_pixel_mono;
            break;

        case BUFFER_MODE_MULTI:
            This.buffer.get_byte = get_byte_multi;

            This.write_pixel = write_pixel_multi;
            break;

        case BUFFER_MODE_MIX:
            This.buffer.get_byte = get_byte_mix;

            This.write_pixel = write_pixel_mix;
            break;

        case BUFFER_MODE_MIX_TERNARY:
            This.buffer.get_byte = get_byte_mix_ternary;

            This.write_pixel = write_pixel_mix_ternary;
            break;

        default:
            return 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Init.
    ////////////////////////////////////////////////////////////////////////////
    //
    // Not use malloc, use pre defined "This".
    //
    This.buffer.data = data;
    This.buffer.size = size;
    This.buffer.buffer_mode = buffer_mode;
    This.buffer.color_mode = color_mode;

    struct lookuptable lookuptable = {
            .decode = decode_table,
            .mul3 = mul3_table,
            .mul9 = mul9_table,
            .mul27 = mul27_table,
            .mul81 = mul81_table,
            };
    This.buffer.lookuptable = lookuptable;

    This.buffer.width_hardware = width_hardware;
    This.buffer.height_hardware = height_hardware;
    This.buffer.ram_size = (width_hardware >> 3) * height_hardware;

    This.buffer.rotate = rotate;
    This.buffer.mirror = mirror;
    This.buffer.polarity = polarity;

    This.buffer.color = COLOR_WHITE;
    This.buffer.dtm_stage = DTM_STAGE_1;
    This.buffer.cursor_x = 0;
    This.buffer.cursor_y = 0;

    if (rotate == ROTATE_0 || rotate == ROTATE_180)
    {
        This.buffer.width = width_hardware;
        This.buffer.height = height_hardware;
    }
    else
    {
        This.buffer.width = height_hardware;
        This.buffer.height = width_hardware;
    }

    return &This.buffer;
}


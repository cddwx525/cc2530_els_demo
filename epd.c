#include "epd.h"

#include <stdint.h>
#include "cc2530_ioctl.h"
#include "delay.h"
#include "lut.h"
#include "uart.h"
#include "string.h"

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
        struct epd epd;

        //
        // Private
        //
        void (* write_with_buffer_partial)(
                struct epd * this,
                enum dtm_stage dtm_stage
                ) __reentrant;
        void (* write_with_buffer_full)(
                struct epd * this,
                enum dtm_stage dtm_stage
                ) __reentrant;

        void (* write_with_data_partial)(
                struct epd * this,
                uint8_t * data
                ) __reentrant;
        void (* write_with_data_full)(
                struct epd * this,
                uint8_t * data
                ) __reentrant;

        void (* write_lut)(
                struct epd * this,
                struct lut * lut
                ) __reentrant;
        void (* config)(struct epd * this) __reentrant;

        void (* partial_out)(struct epd * this) __reentrant;
        void (* partial_in)(struct epd * this) __reentrant;
        void (* voltage_off)(struct epd * this) __reentrant;
        void (* voltage_on)(struct epd * this) __reentrant;
        void (* channel_refresh)(struct epd * this) __reentrant;
        void (* reset)(void) __reentrant;
        void (* waitbusy)(void) __reentrant;

        uint8_t (* cmd_receive)(struct epd * this) __reentrant;
        void (* cmd_param)(struct epd * this, uint8_t data) __reentrant;
        void (* cmd_reg)(struct epd * this, uint8_t command) __reentrant;
        uint8_t (* spi_read)(void) __reentrant;
        void (* spi_write)(uint8_t value) __reentrant;

        void (* gpio_init)(void) __reentrant;
        };

////////////////////////////////////////////////////////////////////////////////
//
// Private functions.
//
////////////////////////////////////////////////////////////////////////////////
static void gpio_init(void) __reentrant
{
    cc2530_ioctl(PORT_EPD_DC, PNUM_EPD_DC, CC2530_OUTPUT);
    EPD_DC_LOW;
    cc2530_ioctl(PORT_EPD_SDA, PNUM_EPD_SDA, CC2530_OUTPUT);
    EPD_SDA_LOW;
    cc2530_ioctl(PORT_EPD_CS, PNUM_EPD_CS, CC2530_OUTPUT);
    EPD_CS_HIGH;
    cc2530_ioctl(PORT_EPD_SCL, PNUM_EPD_SCL, CC2530_OUTPUT);
    EPD_SCL_LOW;
    cc2530_ioctl(PORT_EPD_RST, PNUM_EPD_RST, CC2530_OUTPUT);
    EPD_RST_HIGH;
    cc2530_ioctl(PORT_EPD_BUSY, PNUM_EPD_BUSY, CC2530_INPUT_PULLUP);
}

static void spi_write(uint8_t value) __reentrant
{
    uint8_t i = 0;

    for (i = 0; i < 8; i += 1)
    {
        if (value & 0x80)
        {
            EPD_SDA_HIGH;  // SDA >= 30 ns
        }
        else
        {
            EPD_SDA_LOW;   // SDA >= 30 ns
        }
        delay_31ns();

        EPD_SCL_HIGH;   // SCL >= 35 ns, SDA >= 30 ns
                        // last clock: CSB >= 65 ns, DC >= 30 ns
        delay_31ns();
        EPD_SCL_LOW;    // SCL >= 35 ns, SCL cycle >= 100 ns
                        // last clock: CSB >= 20 ns
        delay_31ns();
        delay_31ns();

        value <<= 1;
    }
}

uint8_t spi_read(void) __reentrant
{
    uint8_t byte = 0x00;
    uint8_t i = 0;

    for (i = 0; i < 8; i += 1)
    {
        byte <<= 1;

        if (PIN_EPD_SDA)
        {
            byte |= 1;
        }
        else
        {
        }

        EPD_SCL_HIGH;   // SCL >= 60 ns
                        // last clock: CSB >= 65 ns, DC >= 30 ns
        delay_31ns();
        delay_31ns();

        EPD_SCL_LOW;    // SCL >= 60 ns, SDA >= 15 ns, SCL cycle >= 150 ns
                        // last clock: CSB >= 20 ns
        delay_31ns();
        delay_31ns();
        delay_31ns();
    }

    return byte;
}

static void cmd_reg(struct epd * this, uint8_t command) __reentrant
{
    EPD_DC_LOW;     // DC >= 30 ns
    EPD_CS_LOW;     // CSB >= 60 ns
    delay_31ns();
    delay_31ns();

    THIS->spi_write(command);

    EPD_CS_HIGH;    // CSB >= 40 ns
    delay_31ns();
}

static void cmd_param(struct epd * this, uint8_t data) __reentrant
{
    EPD_DC_HIGH;    // DC >= 30 ns
    EPD_CS_LOW;     // CSB >= 60 ns
    delay_31ns();
    delay_31ns();

    THIS->spi_write(data);

    EPD_CS_HIGH;    // CSB >= 40 ns
    delay_31ns();
}

static uint8_t cmd_receive(struct epd * this) __reentrant
{
    uint8_t byte = 0x00;

    cc2530_ioctl(PORT_EPD_SDA, PNUM_EPD_SDA, CC2530_INPUT_PULLUP);

    EPD_DC_HIGH;    // DC >= 30 ns
    EPD_CS_LOW;     // CSB >= 60 ns, SDA out <= 50 ns
    delay_31ns();
    delay_31ns();

    byte = THIS->spi_read();

    EPD_CS_HIGH;    // CSB >= 40 ns
    delay_31ns();

    cc2530_ioctl(PORT_EPD_SDA, PNUM_EPD_SDA, CC2530_OUTPUT);
    EPD_SDA_LOW;

    return byte;
}

static void waitbusy(void) __reentrant
{
    while (PIN_EPD_BUSY == EPD_BUSY_LEVEL)
    {
    }
}

static void reset(void) __reentrant
{
    //
    // Hardware reset.
    //
    EPD_RST_LOW;
    delay_ms(10);  // At least 10ms
    EPD_RST_HIGH;

    delay_ms(1);
}


static void channel_refresh(struct epd * this) __reentrant
{
    THIS->cmd_reg(this, 0x12);
    //delay_us(200);  // 200uS at least before FLG command
    THIS->waitbusy();
}

static void voltage_on(struct epd * this) __reentrant
{
    THIS->cmd_reg(this, 0x04);
    THIS->waitbusy();
}

static void voltage_off(struct epd * this) __reentrant
{
    THIS->cmd_reg(this, 0x02);
    THIS->waitbusy();
}

static void partial_in(struct epd * this) __reentrant
{
    //
    // 0x91
    // Partial in.
    //
    THIS->cmd_reg(this, 0x91);
}

static void partial_out(struct epd * this) __reentrant
{
    //
    // 0x92
    // Partial out.
    //
    THIS->cmd_reg(this, 0x92);
}

static void write_lut(
        struct epd * this,
        struct lut * lut
        ) __reentrant
{
    uint8_t i = 0;

    for (i = 0; i < lut->size; i += 1)
    {
        THIS->cmd_param(this, lut->data[i]);
    }
}

static void config(struct epd * this) __reentrant
{
    //
    // 0x00
    // Panel setting.
    //
    // NOTE: Keep default resolution, we set in resolution setting later.
    //
    //     00:  96x230
    //     0 :  LUT from OTP
    //     0 :  KWR mode
    //     1 :  Gate scan up
    //     1 :  Source shift right
    //     1 :  Booster on
    //     1 :  Software reset no effect
    //
    uint8_t psr = 0x0F;

    if (this->mode == MODE_KWR)
    {
        psr &= ~(0x01 << 4);
    }
    else
    {
        psr |= (0x01 << 4);
    }

    if (this->lut_mode == LUT_MODE_OTP)
    {
        psr &= ~(0x01 << 5);
    }
    else
    {
        psr |= (0x01 << 5);
    }

    THIS->cmd_reg(this, 0x00);
    THIS->cmd_param(this, psr);

    //
    // 0x30
    // PLL control.
    //
    //    --###### VD,VG {0x3C: 50 Hz, 0x3A: 100 Hz}
    //
    THIS->cmd_reg(this, 0x30);
    THIS->cmd_param(this, 0x3C);

    //
    // 0x01
    // Power setting.
    //
    //    ------## VD,VG {0: external, 1: internal}
    //    -----### VCOM {0: VD, 1: VG}, VG {00: +16V/-16V, ...}
    //    ######## VDH {0x26: +10V, 0x2B: +11V}
    //    ######## VDL {0x26: -10V, 0x2B: -11V}
    //    ######## VDHR {0x03: +3V, 0x1A: +7.6V, 0x12: +6V, 0x0D: +5V}
    //
    THIS->cmd_reg(this, 0x01);
    THIS->cmd_param(this, 0x03);
    THIS->cmd_param(this, 0x06);
    THIS->cmd_param(this, 0x26);
    THIS->cmd_param(this, 0x26);
    THIS->cmd_param(this, 0x03);

    //
    // 0x03
    // Power off sequece setting.
    //
    //     --##---- source to gate power off interval time
    //             {00: 1 frame, 01: 2 frame, ,,, , 11: 4 frames}
    //
    THIS->cmd_reg(this, 0x03);
    THIS->cmd_param(this, 0x00);

    //
    // 0x06
    // Booster soft start.
    //
    //    ########
    //        **###### Phase A soft start period {00: 10ms, ...}
    //        ##***### Phase A driving strength {010: level 3, ...}
    //        #####*** Phase A min GDR off time {110: 3.34 us, 111: 6.58 us,...}
    //
    //    ########
    //        **###### Phase B soft start period {00: 10ms, ...}
    //        ##***### Phase B driving strength {010: level 3, ...}
    //        #####*** Phase B min GDR off time {110: 3.34 us, 111: 6.58 us,...}
    //
    //    --######
    //        --***### Phase C driving strength {010: level 3, ...}
    //        --###*** Phase C min GDR off time {110: 3.34 us, 111: 6.58 us,...}
    //
    THIS->cmd_reg(this, 0x06);
    THIS->cmd_param(this, 0x17);
    THIS->cmd_param(this, 0x17);
    THIS->cmd_param(this, 0x17);

    //
    // 0x50
    // VCOM and data interval setting.
    //
    //     ########
    //         KWR mode:
    //             **######  VBD[1:0]
    //                     DDX[0]=0: {00:Floating, 01:LUTR, 10:LUTW, 11:LUTB}
    //                     DDX[0]=1: {00:LUTB, 01:LUTW, 10:LUTR, 11:Floating}
    //             ##*##### DDX[1] Red polarity inverted(0:on, 1:off)
    //             ###*#### DDX[0] B/W polarity inverted(0: B, 1: W)
    //
    //         KW mode:
    //             **######  VBD[1:0]
    //                     DDX[0]=0: {00:Floating, 01:LUTBW, 10:LUTWB, 11:Floating}
    //                     DDX[0]=1: {00:Floating, 01:LUTWB, 10:LUTBW, 11:Floating}
    //             ##*##### DDX[1] OLD/NEW data mode {0: old/new, 1: new}
    //             ###*#### DDX[0] B/W polarity inverted(0: B, 1: W)
    //
    //         ####**** VCOM Date interval(0111: 10 Hsync, 1110: 3 Hsync, ...}
    //
    uint8_t cdi = 0x07;
    cdi |= this->buffer->polarity << 4;
    cdi |= (this->vbd_mode ^ this->buffer->polarity) << 6;

    THIS->cmd_reg(this, 0x50);
    THIS->cmd_param(this, cdi);

    //
    // 0x61
    // Resolution setting.
    //
    // NOTE: This has high priority than Panel setting, and can use value
    //     , recommanded.
    //
    //     *****000         : Horizontal resolution(Must divided by 8)
    //     -------* ********: Vertical resolution
    THIS->cmd_reg(this, 0x61);
    THIS->cmd_param(this, EPD_WIDTH);
    THIS->cmd_param(this, EPD_HEIGHT >> 8);
    THIS->cmd_param(this, EPD_HEIGHT & 0xFF);

    //
    // 0x20, 0x21, 0x22, 0x23, 0x24
    // LUTC, LUTW, LUTBW, LUTWB, LUTBB.
    //
    if (this->lut_mode == LUT_MODE_REG)
    {
        if (this->mode == MODE_KWR)
        {
            THIS->cmd_reg(this, 0x20);
            THIS->write_lut(this, &this->lut_kwr->vcom);

            THIS->cmd_reg(this, 0x21);
            THIS->write_lut(this, &this->lut_kwr->ww);

            THIS->cmd_reg(this, 0x22);
            THIS->write_lut(this, &this->lut_kwr->bw);

            THIS->cmd_reg(this, 0x23);
            THIS->write_lut(this, &this->lut_kwr->wb);

            THIS->cmd_reg(this, 0x24);
            THIS->write_lut(this, &this->lut_kwr->bb);
        }
        else
        {
            THIS->cmd_reg(this, 0x20);
            THIS->write_lut(this, &this->lut_kw->vcom);

            THIS->cmd_reg(this, 0x21);
            THIS->write_lut(this, &this->lut_kw->ww);

            THIS->cmd_reg(this, 0x22);
            THIS->write_lut(this, &this->lut_kw->bw);

            THIS->cmd_reg(this, 0x23);
            THIS->write_lut(this, &this->lut_kw->wb);

            THIS->cmd_reg(this, 0x24);
            THIS->write_lut(this, &this->lut_kw->bb);
        }
    }
    else
    {
    }
}


static void write_with_data_full(
        struct epd * this,
        uint8_t * data
        ) __reentrant
{
    uint16_t ram_offset = 0;
    for (ram_offset = 0; ram_offset < EPD_RAM_SIZE; ram_offset += 1)
    {
        THIS->cmd_param(this, data[ram_offset]);
    }
}

static void write_with_data_partial(
        struct epd * this,
        uint8_t * data
        ) __reentrant
{
    uint16_t row = 0;
    uint16_t column = 0;
    uint16_t ram_offset = 0;
    for (row = 0; row < this->window.height; row += 1)
    {
        for (column = 0; column < (this->window.width >> 3); column += 1 )
        {
            ram_offset = (this->window.y + row) * (EPD_WIDTH >> 3)
                    + (this->window.x >> 3) + column;
            THIS->cmd_param(this, data[ram_offset]);
        }
    }
}

static void write_with_buffer_full(
        struct epd * this,
        enum dtm_stage dtm_stage
        ) __reentrant
{
    uint16_t ram_offset = 0;
    for (ram_offset = 0; ram_offset < EPD_RAM_SIZE; ram_offset += 1)
    {
        THIS->cmd_param(
                this,
                this->buffer->get_byte(this->buffer, dtm_stage, ram_offset)
                );
    }
}

static void write_with_buffer_partial(
        struct epd * this,
        enum dtm_stage dtm_stage
        ) __reentrant
{
    uint16_t row = 0;
    uint16_t column = 0;
    uint16_t ram_offset = 0;
    for (row = 0; row < this->window.height; row += 1)
    {
        for (column = 0; column < (this->window.width >> 3); column += 1 )
        {
            ram_offset = (this->window.y + row) * (EPD_WIDTH >> 3)
                    + (this->window.x >> 3) + column;
            THIS->cmd_param(
                    this,
                    this->buffer->get_byte(this->buffer, dtm_stage, ram_offset)
                    );
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// Public functions.
//
////////////////////////////////////////////////////////////////////////////////
static void sleep(struct epd * this) __reentrant
{
    THIS->voltage_off(this);

    THIS->cmd_reg(this, 0x07);
    THIS->cmd_param(this, 0xA5);
}

static void write_dtm1(struct epd * this, uint8_t * data) __reentrant
{
    THIS->cmd_reg(this, 0x10);

    THIS->write_with_data_full(this, data);
}

static void write_dtm2(struct epd * this, uint8_t * data) __reentrant
{
    THIS->cmd_reg(this, 0x13);

    THIS->write_with_data_full(this, data);
}


static void output(struct epd * this) __reentrant
{
    THIS->voltage_on(this);
    THIS->channel_refresh(this);
    THIS->voltage_off(this);
}

static void clean(struct epd * this) __reentrant
{
    uint8_t dtm1_byte = 0x00;
    uint8_t dtm2_byte = 0x00;
    uint16_t ram_offset = 0;
    uint8_t color_value = this->buffer->color ^ this->buffer->polarity;

    if (this->mode == MODE_KWR)
    {
        if ((color_value & 0x01) == 0)
        {
            dtm1_byte = 0x00;
        }
        else
        {
            dtm1_byte = 0xFF;
        }

        if ((color_value & 0x02) == 0)
        {
            dtm2_byte = 0x00;
        }
        else
        {
            dtm2_byte = 0xFF;
        }
    }
    else
    {
        //
        // MODE_KW.
        //
        if ((this->buffer->polarity & 0x02) != 0)
        {
            //
            // NEW only.
            //
            if ((color_value & 0x01) == 0)
            {
                dtm2_byte = 0x00;
            }
            else
            {
                dtm2_byte = 0xFF;
            }
        }
        else
        {
            //
            // OLD/NEW.
            //
            if ((color_value & 0x01) == 0)
            {
                dtm1_byte = 0x00;
                dtm2_byte = 0x00;
            }
            else
            {
                dtm1_byte = 0xFF;
                dtm2_byte = 0xFF;
            }
        }
    }

    if ((this->mode == MODE_KW) && ((this->buffer->polarity & 0x02) != 0))
    {
        // KW NEW only mode.
        THIS->cmd_reg(this, 0x13);
        for (ram_offset = 0; ram_offset < EPD_RAM_SIZE; ram_offset += 1)
        {
            THIS->cmd_param(this, dtm2_byte);
        }
    }
    else
    {
        THIS->cmd_reg(this, 0x10);
        for (ram_offset = 0; ram_offset < EPD_RAM_SIZE; ram_offset += 1)
        {
            THIS->cmd_param(this, dtm1_byte);
        }

        THIS->cmd_reg(this, 0x13);
        for (ram_offset = 0; ram_offset < EPD_RAM_SIZE; ram_offset += 1)
        {
            THIS->cmd_param(this, dtm2_byte);
        }
    }

    this->output(this);
}


static void set_window(
        struct epd * this,
        uint16_t x, uint16_t y, uint16_t width, uint16_t height
        ) __reentrant
{
    if (
            (x > (this->buffer->width - 1))
            || (y > (this->buffer->height - 1))
            || (width > this->buffer->width)
            || (width == 0)
            || (height > this->buffer->height)
            || (height == 0)
            )
    {
        return;
    }
    else
    {
    }

    width = x + width <= this->buffer->width
            ? width : this->buffer->width - x;
    height = y + height <= this->buffer->height
            ? height : this->buffer->height - y;

    uint16_t x_hardware = x;
    uint16_t y_hardware = y;
    uint16_t width_hardware = width;
    uint16_t height_hardware = height;

    switch(this->buffer->rotate)
    {
        case ROTATE_90:
            x_hardware = (EPD_WIDTH - 1) - (y + height - 1);
            y_hardware = x;
            width_hardware = height;
            height_hardware = width;
            break;

        case ROTATE_180:
            x_hardware = (EPD_WIDTH - 1) - (x + width - 1);
            y_hardware = (EPD_HEIGHT - 1) - (y + height - 1);
            width_hardware = width;
            height_hardware = height;
            break;

        case ROTATE_270:
            x_hardware = y;
            y_hardware = (EPD_HEIGHT - 1) - (x + width - 1);
            width_hardware = height;
            height_hardware = width;
            break;

        case ROTATE_0:
            break;

        default:
            return;
    }

    switch(this->buffer->mirror)
    {
        case MIRROR_HORIZONTAL:
            x_hardware = (EPD_WIDTH - 1)
                    - (x_hardware + width_hardware - 1);
            break;

        case MIRROR_VERTICAL:
            y_hardware = (EPD_HEIGHT - 1)
                    - (y_hardware + height_hardware - 1);
            break;

        case MIRROR_ORIGIN:
            x_hardware = (EPD_WIDTH - 1)
                    - (x_hardware + width_hardware - 1);
            y_hardware = (EPD_HEIGHT - 1)
                    - (y_hardware + height_hardware - 1);
            break;

        case MIRROR_NONE:
            break;

        default:
            return;
    }

    // X start rounddown to 8 bits start.
    x_hardware &= ~0x07;

    // X end roundup to 8 bits end.
    uint16_t x_hardware_end = (x_hardware + width_hardware - 1) | 0x07;

    // W roundup to contain whole 8 bits.
    width_hardware = (width_hardware + 7) & ~0x07;

    uint16_t y_hardware_end = y_hardware + height_hardware - 1;

    this->window.x = x_hardware;
    this->window.y = y_hardware;
    this->window.width = width_hardware;
    this->window.height = height_hardware;


    //
    // 0x90
    // Partial window.
    //
    // *****000         : Window horizontal start.
    // *****111         : Window horizontal end.
    // -------* ********: Window vertical start.
    // -------* ********: Window vertical end.
    // -------1         : Gates scan both inside and outside partial window.
    //
    THIS->cmd_reg(this, 0x90);
    THIS->cmd_param(this, x_hardware & 0xFF);       // Low 8 bits.
    THIS->cmd_param(this, x_hardware_end & 0xFF);   // Low 8 bits.
    THIS->cmd_param(this, y_hardware >> 8);         // High 8 bits.
    THIS->cmd_param(this, y_hardware & 0xFF);       // Low 8 bits.
    THIS->cmd_param(this, y_hardware_end >> 8);     // High 8 bits.
    THIS->cmd_param(this, y_hardware_end & 0xFF);   // Low 8 bits.
    THIS->cmd_param(this, 0x01);
}

static void write_dtm1_partial(
        struct epd * this,
        uint8_t * data
        ) __reentrant
{
    THIS->partial_in(this);
    THIS->cmd_reg(this, 0x10);
    THIS->write_with_data_partial(this, data);
    THIS->partial_out(this);
}

static void write_dtm2_partial(
        struct epd * this,
        uint8_t * data
        ) __reentrant
{
    THIS->partial_in(this);
    THIS->cmd_reg(this, 0x13);
    THIS->write_with_data_partial(this, data);
    THIS->partial_out(this);
}

static void output_partial(struct epd * this) __reentrant
{
    THIS->partial_in(this);
    this->output(this);
    THIS->partial_out(this);
}


static void display_with_data(
        struct epd * this,
        uint8_t * dtm1_data, uint8_t * dtm2_data
        ) __reentrant
{
    this->write_dtm1(this, dtm1_data);
    this->write_dtm2(this, dtm2_data);
    this->output(this);
}

static void update_with_data(
        struct epd * this,
        uint8_t * dtm1_data, uint8_t * dtm2_data,
        uint16_t x, uint16_t y, uint16_t width, uint16_t height
        ) __reentrant
{
    this->set_window(this, x, y, width, height);
    this->write_dtm1_partial(this, dtm1_data);
    this->write_dtm2_partial(this, dtm2_data);
    this->output_partial(this);
}

static void display(struct epd * this) __reentrant
{
    if (this->mode == MODE_KW)
    {
        //
        // Only send NEW data, in KW mode.
        //
        THIS->cmd_reg(this, 0x13);
        THIS->write_with_buffer_full(this, DTM_STAGE_2);
    }
    else
    {
        THIS->cmd_reg(this, 0x10);
        THIS->write_with_buffer_full(this, DTM_STAGE_1);
        THIS->cmd_reg(this, 0x13);
        THIS->write_with_buffer_full(this, DTM_STAGE_2);
    }

    this->output(this);

    //
    // When KW OLD/NEW mode, send current NEW data as OLD data of next display.
    //
    if ((this->mode == MODE_KW) && ((this->buffer->polarity & 0x02) == 0))
    {
        THIS->cmd_reg(this, 0x10);
        THIS->write_with_buffer_full(this, DTM_STAGE_1);
    }
    else
    {
    }
}

static void update(
        struct epd * this,
        uint16_t x, uint16_t y, uint16_t width, uint16_t height
        ) __reentrant
{
    this->set_window(this, x, y, width, height);

    THIS->partial_in(this);
    if (this->mode == MODE_KW)
    {
        //
        // Only send NEW data, in KW mode.
        //
        THIS->cmd_reg(this, 0x13);
        THIS->write_with_buffer_partial(this, DTM_STAGE_2);
    }
    else
    {
        THIS->cmd_reg(this, 0x10);
        THIS->write_with_buffer_partial(this, DTM_STAGE_1);
        THIS->cmd_reg(this, 0x13);
        THIS->write_with_buffer_partial(this, DTM_STAGE_2);
    }
    THIS->partial_out(this);

    this->output_partial(this);


    //
    // When KW OLD/NEW mode, send current NEW data as OLD data of next display.
    //
    if ((this->mode == MODE_KW) && ((this->buffer->polarity & 0x02) == 0))
    {
        THIS->partial_in(this);
        THIS->cmd_reg(this, 0x10);
        THIS->write_with_buffer_partial(this, DTM_STAGE_1);
        THIS->partial_out(this);
    }
    else
    {
    }
}

static void read_otp(
        struct epd * this
        ) __reentrant
{
    uint8_t byte = 0xAA;
    char string[3];

    THIS->cmd_reg(this, 0xA0);
    THIS->cmd_reg(this, 0xA2);

    // Dummy byte.
    byte = THIS->cmd_receive(this);

    uint16_t offset = 0;
    for (offset = 0; offset < 4100; offset += 1)
    {
        //this->buffer->data[offset] = THIS->cmd_receive(this);

        if ((offset & 0x0F) == 0x00)
        {
            byte = offset >> 8;
            byte_to_hex(byte, string);
            uart_send_string(string);

            byte = offset & 0xFF;
            byte_to_hex(byte, string);
            uart_send_string(string);

            uart_send_string(":  ");
        }
        else
        {
        }

        byte = THIS->cmd_receive(this);

        byte_to_hex(byte, string);
        uart_send_string("0x");
        uart_send_string(string);
        if ((offset & 0x0F) == 0x0F)
        {
            uart_send_string(",\r\n");
        }
        else
        {
            uart_send_string(", ");
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// External function.
//
////////////////////////////////////////////////////////////////////////////////

static uint8_t display_buffer[EPD_BUFFER_SIZE] = {0};
static struct This This;

struct epd * Epd(
        enum rotate rotate,
        enum mirror mirror,
        enum mode mode,
        enum lut_mode lut_mode,
        enum vbd_mode vbd_mode,
        enum polarity polarity,
        enum buffer_mode buffer_mode
        )
{
    ////////////////////////////////////////////////////////////////////////////
    // Bind functions.
    ////////////////////////////////////////////////////////////////////////////
    //
    // Public.
    //
    This.epd.read_otp = read_otp;

    This.epd.update = update;
    This.epd.display = display;

    This.epd.update_with_data = update_with_data;
    This.epd.display_with_data = display_with_data;

    This.epd.output_partial = output_partial;
    This.epd.write_dtm2_partial = write_dtm2_partial;
    This.epd.write_dtm1_partial = write_dtm1_partial;
    This.epd.set_window = set_window;

    This.epd.clean = clean;

    This.epd.output = output;
    This.epd.write_dtm2 = write_dtm2;
    This.epd.write_dtm1 = write_dtm1;

    This.epd.sleep = sleep;


    //
    // Private functions
    //
    This.write_with_buffer_partial = write_with_buffer_partial;
    This.write_with_buffer_full = write_with_buffer_full;

    This.write_with_data_partial = write_with_data_partial;
    This.write_with_data_full = write_with_data_full;

    This.config = config;
    This.write_lut = write_lut;

    This.partial_out = partial_out;
    This.partial_in = partial_in;
    This.voltage_off = voltage_off;
    This.voltage_on = voltage_on;
    This.channel_refresh = channel_refresh;
    This.reset = reset;
    This.waitbusy = waitbusy;

    This.cmd_receive = cmd_receive;
    This.cmd_param = cmd_param;
    This.cmd_reg = cmd_reg;
    This.spi_read = spi_read;
    This.spi_write = spi_write;

    This.gpio_init = gpio_init;

    ///////////////////////////////////////////////////////////////////////////
    // Init.
    ///////////////////////////////////////////////////////////////////////////
    //
    // Not use malloc, use pre defined "This" and "display_buffer".
    //
    enum color_mode color_mode =
            mode == MODE_KWR ? COLOR_MODE_COLOR : COLOR_MODE_BW;
    buffer_mode = mode == MODE_KWR ? buffer_mode : BUFFER_MODE_MONO;

    This.epd.buffer = Buffer(
            display_buffer, EPD_BUFFER_SIZE, buffer_mode, color_mode,
            EPD_WIDTH, EPD_HEIGHT, rotate, mirror, polarity
            );

    struct window window = {
            .x = 0,
            .y = 0,
            .width = EPD_WIDTH,
            .height = EPD_HEIGHT
            };
    This.epd.window = window;
    This.epd.mode = mode;
    This.epd.lut_mode = lut_mode;
    This.epd.vbd_mode = vbd_mode;
    This.epd.lut_kwr = &lut_kwr;
    This.epd.lut_kw = &lut_kw;

    This.gpio_init();
    This.reset();
    This.config(&This.epd);

    return &This.epd;
}


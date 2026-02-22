#include <cc2530.h>
#include "cc2530_ioctl.h"
#include "uart.h"
#include "delay.h"
#include "string.h"
#include "epd.h"
//#include "imagedata.h"

#define PIN_EPDPOWER      P0_7
#define PORT_EPDPOWER     0
#define PNUM_EPDPOWER     7

void epd_message(char * message)
{
    struct epd * epd  = Epd(
            ROTATE_270, MIRROR_NONE,
            MODE_KWR, LUT_MODE_OTP, VBD_MODE_KWR_FLT,
            POLARITY_KWR_RB, BUFFER_MODE_MONO
            );
    epd->buffer->set_dtm_stage(epd->buffer, DTM_STAGE_1);

    epd->buffer->set_color(epd->buffer, COLOR_WHITE);
    epd->buffer->fill(epd->buffer);
    epd->buffer->set_color(epd->buffer, COLOR_BLACK);
    epd->buffer->set_cursor(epd->buffer, 30, 50);
    epd->buffer->draw_string(epd->buffer, &font12, message);

    epd->write_dtm1(epd, epd->buffer->data);

    epd->buffer->set_dtm_stage(epd->buffer, DTM_STAGE_2);

    epd->buffer->set_color(epd->buffer, COLOR_WHITE);
    epd->buffer->fill(epd->buffer);
    epd->buffer->set_color(epd->buffer, COLOR_BLACK);
    epd->buffer->set_cursor(epd->buffer, 30, 50);
    epd->buffer->draw_string(epd->buffer, &font12, message);

    epd->write_dtm2(epd, epd->buffer->data);

    epd->output(epd);
}


void epd_test_buffer_mix_3(void)
{
    epd_message("KWR, buffer mix-3 mode display test ...");

    struct epd * epd = Epd(
            ROTATE_270, MIRROR_NONE,
            MODE_KWR, LUT_MODE_OTP, VBD_MODE_KWR_FLT,
            POLARITY_KWR_RB, BUFFER_MODE_MIX_TERNARY
            );

    //
    // Table border.
    //
    epd->buffer->set_color(epd->buffer, COLOR_WHITE);
    epd->buffer->fill(epd->buffer);
    epd->buffer->set_color(epd->buffer, COLOR_BLACK);
    epd->buffer->draw_rectangle(epd->buffer, 1, 1, 294, 126,
            POINT_SIZE_3, FILL_STYLE_EMPTY
            );
    epd->buffer->draw_line(epd->buffer, 0, 18, 295, 18,
            POINT_SIZE_3, LINE_STYLE_SOLID
            );
    epd->buffer->draw_rectangle(epd->buffer, 147, 0, 2, 128,
            POINT_SIZE_1, FILL_STYLE_FULL
            );
    //
    // Table header.
    //
    epd->buffer->set_cursor(epd->buffer, 5, 4);
    epd->buffer->draw_string(epd->buffer, &font12, "== TEXT ==");
    epd->buffer->set_cursor(epd->buffer, 151, 4);
    epd->buffer->draw_string(epd->buffer, &font12, "== GRAPHIC ==");

    //
    // Text.
    //
    epd->buffer->set_cursor(epd->buffer, 5, 20);
    epd->buffer->draw_string(epd->buffer, &font12, "black on white");
    epd->buffer->set_cursor(epd->buffer, 5, 35);
    epd->buffer->set_color(epd->buffer, COLOR_RED);
    epd->buffer->draw_string(epd->buffer, &font12, "red on white");
    epd->buffer->set_cursor(epd->buffer, 5, 50);
    epd->buffer->set_color(epd->buffer, COLOR_BLACK);
    epd->buffer->draw_rectangle(epd->buffer, 5, 50, 135, 12,
            POINT_SIZE_1, FILL_STYLE_FULL
            );
    epd->buffer->set_color(epd->buffer, COLOR_WHITE);
    epd->buffer->draw_string(epd->buffer, &font12, "white on black");
    epd->buffer->set_cursor(epd->buffer, 5, 65);
    epd->buffer->set_color(epd->buffer, COLOR_BLACK);
    epd->buffer->draw_rectangle(epd->buffer, 5, 65, 135, 12,
            POINT_SIZE_1, FILL_STYLE_FULL
            );
    epd->buffer->set_color(epd->buffer, COLOR_RED);
    epd->buffer->draw_string(epd->buffer, &font12, "red on black");
    epd->buffer->set_cursor(epd->buffer, 5, 80);
    epd->buffer->set_color(epd->buffer, COLOR_RED);
    epd->buffer->draw_rectangle(epd->buffer, 5, 80, 135, 12,
            POINT_SIZE_1, FILL_STYLE_FULL
            );
    epd->buffer->set_color(epd->buffer, COLOR_WHITE);
    epd->buffer->draw_string(epd->buffer, &font12, "white on red");
    epd->buffer->set_cursor(epd->buffer, 5, 95);
    epd->buffer->set_color(epd->buffer, COLOR_RED);
    epd->buffer->draw_rectangle(epd->buffer, 5, 95, 135, 12,
            POINT_SIZE_1, FILL_STYLE_FULL
            );
    epd->buffer->set_color(epd->buffer, COLOR_BLACK);
    epd->buffer->draw_string(epd->buffer, &font12, "black on red");
    epd->buffer->set_cursor(epd->buffer, 5, 105);
    epd->buffer->draw_string(epd->buffer, &font12zh, "ÄãºÃÊ÷Ý®ÅÉ");

    //
    // Graphic.
    //
    epd->buffer->set_color(epd->buffer, COLOR_RED);
    epd->buffer->draw_rectangle(epd->buffer, 150, 25, 130, 75,
            POINT_SIZE_1, FILL_STYLE_FULL
            );
    epd->buffer->set_color(epd->buffer, COLOR_WHITE);
    epd->buffer->draw_circle(epd->buffer, 184, 60, 30,
            POINT_SIZE_1, FILL_STYLE_EMPTY
            );
    epd->buffer->draw_rectangle(epd->buffer, 184, 54, 76, 40,
            POINT_SIZE_1, FILL_STYLE_EMPTY
            );
    epd->buffer->draw_line(epd->buffer, 160, 89, 269, 30,
            POINT_SIZE_1, LINE_STYLE_SOLID
            );
    epd->buffer->set_color(epd->buffer, COLOR_BLACK);
    epd->buffer->draw_line(epd->buffer, 155, 105, 259, 105,
            POINT_SIZE_1, LINE_STYLE_DOTTED
            );
    epd->buffer->draw_line(epd->buffer, 155, 110, 259, 110,
            POINT_SIZE_3, LINE_STYLE_DOTTED
            );
    epd->buffer->set_color(epd->buffer, COLOR_RED);
    epd->buffer->draw_line(epd->buffer, 155, 120, 259, 120,
            POINT_SIZE_5, LINE_STYLE_DOTTED
            );
    epd->buffer->set_color(epd->buffer, COLOR_BLACK);
    epd->buffer->draw_line(epd->buffer, 279, 120, 180, 30,
            POINT_SIZE_3, LINE_STYLE_DOTTED
            );
    epd->display(epd);

    epd->buffer->set_color(epd->buffer, COLOR_BLACK);
    epd->buffer->draw_rectangle(epd->buffer, 100, 50, 100, 25,
            POINT_SIZE_1, FILL_STYLE_FULL
            );
    epd->buffer->set_color(epd->buffer, COLOR_WHITE);
    epd->buffer->set_cursor(epd->buffer, 110, 52);
    epd->buffer->draw_string(epd->buffer, &font12zh, "Ê÷Ý®ÅÉÄãºÃ");
    epd->update(epd, 100, 52, 100, 25);
}


void main(void)
{
    cc2530_set_sysclk_32mhz();

    //
    // NFC PINs.
    //
    //cc2530_ioctl(0, 0, CC2530_INPUT_TRISTATE);
    //cc2530_ioctl(0, 1, CC2530_INPUT_TRISTATE);
    //cc2530_ioctl(1, 0, CC2530_INPUT_TRISTATE);
    //cc2530_ioctl(2, 0, CC2530_INPUT_TRISTATE);
    //cc2530_ioctl(2, 3, CC2530_OUTPUT);
    //P2_3 = 0;
    //cc2530_ioctl(2, 4, CC2530_INPUT_TRISTATE);
    //cc2530_ioctl(1, 3, CC2530_INPUT_TRISTATE);
    //cc2530_ioctl(1, 4, CC2530_INPUT_TRISTATE);
    //cc2530_ioctl(1, 5, CC2530_INPUT_TRISTATE);

    uart_init();

    cc2530_ioctl(PORT_EPDPOWER, PNUM_EPDPOWER, CC2530_OUTPUT);
    PIN_EPDPOWER = 1;

    struct epd * epd = Epd(
            ROTATE_270, MIRROR_NONE,
            MODE_KW, LUT_MODE_OTP, VBD_MODE_KW_FLT,
            POLARITY_KW_OLDNEW_B, BUFFER_MODE_MONO
            );

    while (1)
    {
        //uart_send_string("Hello world!\n");

        epd_test_buffer_mix_3();

        delay_ms(3000);
    }
}


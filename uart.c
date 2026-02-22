#include "uart.h"

#include <stdint.h>
#include <cc2530.h>

void uart_init(void)
{
    //
    // Function selection.
    //
    // PERCFG:
    //     **** **1* USART1 pin location alt2 -- P1_6 TX，P1_7 RX
    // P2SEL:
    //     -10* **** USART1 > USART0, TIMER3
    // P1SEL:
    //     1100 0000 P1_6, P1_7 peripheral function
    //
    PERCFG |= 0x01 << 1;
    P2SEL |= 0x01 << 6;
    P2SEL &= ~(0x01 << 5);
    P1SEL |= 0x01 << 7;
    P1SEL |= 0x01 << 6;

    //
    // UART setting.
    //
    // U1CSR:
    //     1*** **** UART mode
    // U1UCR:
    //     0000 0010 8N1
    //
    U1CSR |= 0x01 << 7;
    U1UCR = 0x02;

    //
    // Frequency setting.
    //
    // 32MHz
    //
    //     ----------------------------
    //     Rate         BAUD_E  BAUD_M
    //     ----------------------------
    //     9600bps      8       59
    //     19200bps     9       59
    //     115200bps    11      216
    //     ----------------------------
    //
    // U1GCR:
    //     ***# #### BAUD_E
    // U1BAUD:
    //     #### #### BAUD_M
    //
    U1GCR  &= ~0x1F;
    U1GCR  |= 8;
    U1BAUD = 59;

    //
    // Interrupt setting.
    //
    // TCON:
    //     0*** **** clear UART 1 RX interrupt flag
    // IRCON2:
    //     **** *0** clear UART 1 TX interrupt flag
    // IEN0:
    //     **** 1*** enable UART 1 RX interrupt
    // IEN2:
    //     **** 1*** enable UART 1 TX interrupt
    // IEN0:
    //     1*** **** enable global interrupt
    //
    TCON &= ~(0x01 << 7);   //URX1IF = 0;
    IRCON2 &= ~(0x01 << 2); //UTX1IF = 0;
    //IEN0 |= 0x01 << 3;      //URX1IE = 1;
    //IEN2 |= 0x01 << 3;      //UTX1IE = 1;
    //IEN0 |= 0x01 << 7;      //EA = 1;

    //
    // U1CSR:
    //     *1** **** receiver enabled
    //
    U1CSR |= 0x01 << 6;
}

void uart_send_byte(uint8_t byte)
{
    U1DBUF = byte;
    while ((IRCON2 & (0x01 << 2)) == 0)
    {
    }
    IRCON2 &= ~(0x01 << 2);
}

void uart_send_string(char * string)
{
    while (* string != '\0')
    {
        uart_send_byte(* string);
        string += 1;
    }
}


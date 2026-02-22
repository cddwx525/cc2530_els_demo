#ifndef CC2530_IOCTL_H
#define CC2530_IOCTL_H

#include <cc2530.h>
#include <stdint.h>

#define CC2530_IOCTL_REVISION   1

#define CC2530_OUTPUT           0
#define CC2530_INPUT_PULLUP     1
#define CC2530_INPUT_PULLDOWN   2
#define CC2530_INPUT_TRISTATE   3

#define CC2530_IOCTL_BV(x)      (1 << (x))


//
// TODO: SDCC "warning 126: unreachable code"
//
/*
////////////////////////////////////////////////////////////////////////////////
#define CC2530_REGCFG_PxSEL(port, pin, val)     do \
        { \
            if (val == 0) \
            { \
                P##port##SEL &= ~CC2530_IOCTL_BV(pin); \
            } \
            else \
            { \
                P##port##SEL |= CC2530_IOCTL_BV(pin); \
            } \
        } \
        while (0)

#define CC2530_REGCFG_PxDIR(port, pin, val)     do \
        { \
            if (val == 0) \
            { \
                P##port##DIR &= ~CC2530_IOCTL_BV(pin); \
            } \
            else \
            { \
                P##port##DIR |= CC2530_IOCTL_BV(pin); \
            } \
        } \
        while (0)

#define CC2530_REGCFG_PxINP(port, pin, val)     do \
        { \
            if (val == 0) \
            { \
                P##port##INP &= ~CC2530_IOCTL_BV(pin); \
            } \
            else \
            { \
                P##port##INP |= CC2530_IOCTL_BV(pin); \
            } \
        } \
        while (0)

#define CC2530_IO_OUTPUT(port, pin)     do \
        { \
            CC2530_REGCFG_PxDIR(port, pin, 1); \
            CC2530_REGCFG_PxSEL(port, pin, 0); \
        } \
        while (0)

#define CC2530_IO_INPUT(port, pin, mode)    do \
        {                                  \
            if ((port) == 1 && ((pin) == 0 || (pin) == 1)) \
            { \
                break; \
            } \
            \
            CC2530_REGCFG_PxDIR(port, pin, 0); \
            CC2530_REGCFG_PxSEL(port, pin, 0); \
            \
            if (mode == CC2530_INPUT_TRISTATE) \
            { \
                CC2530_REGCFG_PxINP(port, pin, 1); \
            } \
            else \
            { \
                CC2530_REGCFG_PxINP(port, pin, 0); \
            } \
            \
            if (mode == CC2530_INPUT_PULLUP) \
            { \
                CC2530_REGCFG_PxINP(2 , (5 + port), 0); \
            } \
            else \
            { \
                CC2530_REGCFG_PxINP(2 , (5 + port), 1); \
            } \
        } \
        while (0)


//
// WARNING:  P1_0, P1_1 can't configed as input mode.
//
#define CC2530_IOCTL(port, pin, mode)   do \
        { \
            if (port > 2 || pin > 7) \
            { \
                break; \
            } \
            \
            if (mode == CC2530_OUTPUT) \
            { \
                CC2530_IO_OUTPUT(port, pin); \
            } \
            else \
            { \
                CC2530_IO_INPUT(port, pin, mode); \
            } \
        } \
        while (0)

//
// WARNING:  GPIO must in output mode.
//
#define CC2530_GPIO_SET(port, pin)      P##port##_##pin = 1
#define CC2530_GPIO_CLEAR(port, pin)    P##port##_##pin = 0

#define CC2530_GPIO_GET(port, pin)      P##port##_##pin

#define setSystemClk32MHZ()     do \
        { \
            CLKCONCMD &= ~0x40; \
            while (CLKCONSTA & 0x40) \
            { \
            } \
            CLKCONCMD &= ~0x47; \
        } \
        while (0)
////////////////////////////////////////////////////////////////////////////////
*/

//
// WARNING:  GPIO must in output mode.
//
#define CC2530_GPIO_SET(port, pin)      P##port##_##pin = 1
#define CC2530_GPIO_CLEAR(port, pin)    P##port##_##pin = 0

#define CC2530_GPIO_GET(port, pin)      P##port##_##pin


/*
*/
void cc2530_regcfg_pxsel(uint8_t port, uint8_t pin, uint8_t val);
void cc2530_regcfg_pxdir(uint8_t port, uint8_t pin, uint8_t val);
void cc2530_regcfg_pxinp(uint8_t port, uint8_t pin, uint8_t val);
void cc2530_io_output(uint8_t port, uint8_t pin);
void cc2530_io_input(uint8_t port, uint8_t pin, uint8_t mode);
void cc2530_ioctl(uint8_t port, uint8_t pin, uint8_t mode);
void cc2530_set_sysclk_32mhz(void);

#endif /* #ifndef CC2530_IOCTL_H */

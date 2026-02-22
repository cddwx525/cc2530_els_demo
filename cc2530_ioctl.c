#include "cc2530_ioctl.h"

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
//
// Public
//
////////////////////////////////////////////////////////////////////////////////

void cc2530_regcfg_pxsel(uint8_t port, uint8_t pin, uint8_t val)
{
    if (val == 0)
    {
        if (port == 0)
        {
            P0SEL &= ~CC2530_IOCTL_BV(pin);
        }
        else if (port == 1)
        {
            P1SEL &= ~CC2530_IOCTL_BV(pin);
        }
        else if (port == 2)
        {
            P2SEL &= ~CC2530_IOCTL_BV(pin);
        }
        else
        {
        }
    }
    else
    {
        if (port == 0)
        {
            P0SEL |= CC2530_IOCTL_BV(pin);
        }
        else if (port == 1)
        {
            P1SEL |= CC2530_IOCTL_BV(pin);
        }
        else if (port == 2)
        {
            P2SEL |= CC2530_IOCTL_BV(pin);
        }
        else
        {
        }
    }
}

void cc2530_regcfg_pxdir(uint8_t port, uint8_t pin, uint8_t val)
{
    if (val == 0)
    {
        if (port == 0)
        {
            P0DIR &= ~CC2530_IOCTL_BV(pin);
        }
        else if (port == 1)
        {
            P1DIR &= ~CC2530_IOCTL_BV(pin);
        }
        else if (port == 2)
        {
            P2DIR &= ~CC2530_IOCTL_BV(pin);
        }
        else
        {
        }
    }
    else
    {
        if (port == 0)
        {
            P0DIR |= CC2530_IOCTL_BV(pin);
        }
        else if (port == 1)
        {
            P1DIR |= CC2530_IOCTL_BV(pin);
        }
        else if (port == 2)
        {
            P2DIR |= CC2530_IOCTL_BV(pin);
        }
        else
        {
        }
    }
}

void cc2530_regcfg_pxinp(uint8_t port, uint8_t pin, uint8_t val)
{
    if (val == 0)
    {
        if (port == 0)
        {
            P0INP &= ~CC2530_IOCTL_BV(pin);
        }
        else if (port == 1)
        {
            P1INP &= ~CC2530_IOCTL_BV(pin);
        }
        else if (port == 2)
        {
            P2INP &= ~CC2530_IOCTL_BV(pin);
        }
        else
        {
        }
    }
    else
    {
        if (port == 0)
        {
            P0INP |= CC2530_IOCTL_BV(pin);
        }
        else if (port == 1)
        {
            P1INP |= CC2530_IOCTL_BV(pin);
        }
        else if (port == 2)
        {
            P2INP |= CC2530_IOCTL_BV(pin);
        }
        else
        {
        }
    }
}

void cc2530_io_output(uint8_t port, uint8_t pin)
{
    cc2530_regcfg_pxdir(port, pin, 1);
    cc2530_regcfg_pxsel(port, pin, 0);
}

void cc2530_io_input(uint8_t port, uint8_t pin, uint8_t mode)
{
    if (port == 1 && (pin == 0 || pin == 1))
    {
        return;
    }

    cc2530_regcfg_pxdir(port, pin, 0);
    cc2530_regcfg_pxsel(port, pin, 0);

    if (mode == CC2530_INPUT_TRISTATE)
    {
        cc2530_regcfg_pxinp(port, pin, 1);
    }
    else
    {
        cc2530_regcfg_pxinp(port, pin, 0);
    }

    if (mode == CC2530_INPUT_PULLUP)
    {
        cc2530_regcfg_pxinp(2, (5 + port), 0);
    }
    else
    {
        cc2530_regcfg_pxinp(2, (5 + port), 1);
    }
}


//
// WARNING:  P1_0, P1_1 can't configed as input mode.
//
void cc2530_ioctl(uint8_t port, uint8_t pin, uint8_t mode)
{
    if (port > 2 || pin > 7)
    {
        return;
    }

    if (mode == CC2530_OUTPUT)
    {
        cc2530_io_output(port, pin);
    }
    else
    {
        cc2530_io_input(port, pin, mode);
    }
}

void cc2530_set_sysclk_32mhz(void)
{
    //
    // OSC: XOSC 32MHz
    // System clock: 32MHz
    //
    CLKCONCMD &= ~0x40;
    while (CLKCONSTA & 0x40)
    {
    }
    CLKCONCMD &= ~0x47;
}

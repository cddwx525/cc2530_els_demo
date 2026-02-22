//
// MCU  : CC2530
// Clock: 32MHz
//

#include "delay.h"

#include <stdint.h>
#include <compiler.h>

//
// Parameter(us)    Measured(us)
// -------------    -----------
// 1                2.9
// 10               12
// 100              102
// 500              500
// 1000             1020
//
void delay_us(__data uint16_t micro_secs)
{
    while (micro_secs -= 1)
    {
        NOP();
        NOP();
    }
}

void delay_ms(__data uint16_t milli_secs)
{
    while (milli_secs -= 1)
    {
        delay_us(1000);
    }
}

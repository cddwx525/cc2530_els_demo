#ifndef _DELAY_H_
#define _DELAY_H_

#include <stdint.h>

#define delay_31ns()    NOP()

void delay_us(__data uint16_t micro_secs);
void delay_ms(__data uint16_t milli_secs);

#endif /* #ifndef _DELAY_H_ */

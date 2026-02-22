#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>

void uart_send_string(char * string);
void uart_send_byte(uint8_t byte);
void uart_init(void);

#endif /* #ifndef _UART_H_ */

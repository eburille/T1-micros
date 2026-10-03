#ifndef UART_LIB__H
#define UART_LIB__H

void UART_init(void);
void UART_transmit(char data);
void UART_envia_string(char string[16]);

#endif

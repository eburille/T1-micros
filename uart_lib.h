#ifndef UART_LIB__H
#define UART_LIB__H

#include "buffer_circular.h"

void UART_init();
void UART_transmit(char data);
void UART_envia_string(char string[16]);
char novo_caracter_recebido();

#endif

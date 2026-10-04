#include <avr/io.h>
#include "uart_lib.h"

#define BAUD 9600
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

#define ESPERA              0
#define RECEBENDO_MENSAGEM  1
#define ENVIANDO_MENSAGEM   2

void UART_init(){
    UBRR0H = (unsigned char)(UBRR_VALUE >> 8);
    UBRR0L = (unsigned char)UBRR_VALUE;

    // Habilita transmissão
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    // 8 bits, 1 stop bit, sem paridade
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_transmit(char data)
{
    // Espera o registrador de transmissão ficar disponível
    while (!(UCSR0A & (1 << UDRE0)));

    UDR0 = data;
}

void recebe_letra(){
    char c = UDR0;

    adiciona_char(c);
}

char novo_caracter_recebido_UART(){
	if ((UCSR0A >> 7) == 1) {
        recebe_letra();
		return 1;
	} 
	
	return 0;
}


void UART_envia_string(char string[16]){
    int i;
	for (i = 0; i < 32; i++){
		if (string[i] == '\0'){
			return;
		}
		UART_transmit(string[i]);
	}
}
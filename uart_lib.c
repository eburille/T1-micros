#include <avr/io.h>
#include "uart_lib.h"
#include <avr/interrupt.h>
#include "codificacao.h"

#define BAUD 19200
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

#define ESPERA              0
#define RECEBENDO_MENSAGEM  1
#define ENVIANDO_MENSAGEM   2

/// @brief Inicializa o UART
void UART_init(){
    UBRR0H = (unsigned char)(UBRR_VALUE >> 8);
    UBRR0L = (unsigned char)UBRR_VALUE;

    // Habilita transmissão, receptor e Interupção
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);

    // 8 bits, 1 stop bit, paridade impar
    UCSR0C = (1 << UPM01) | (1 << UPM00) | (1 << UCSZ01) | (1 << UCSZ00);

    // 8 bits, 1 stop bit, sem paridade
    //UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

/// @brief Envia um caracter para o UAR
/// @param data 
void UART_transmit(char data)
{
    // Espera o registrador de transmissão ficar disponível
    while (!(UCSR0A & (1 << UDRE0)));

    UDR0 = data;
}

/// @brief Interrupção para receber um carcater via UART. O caracter é adicionado no buffer de caracteres para ser processado quando for mais conveniente
/// @param  
ISR(USART0_RX_vect) {
    // Lê o dado recebido do registrador UDR0
    char c = UDR0;

    adiciona_char(c);
}

/// @brief Fução usada para receber char pelo UART. FOI SUBSITUIDO POR INTERRUPÇÃO
void recebe_letra(){
    char c = UDR0;

    adiciona_char(c);
}

/// @brief Fução usada para receber char pelo UART. FOI SUBSITUIDO POR INTERRUPÇÃO
/// @return 
char novo_caracter_recebido_UART(){
	if ((UCSR0A >> 7) == 1) {
        recebe_letra();
		return 1;
	} 
	
	return 0;
}

/// @brief Envia string codificada com o desloca soma pela UART
/// @param string 
void UART_envia_string(char *string){
    int i = 0;
    

    codifica(string, chave);

    while (string[i] != '\0'){
        UART_transmit(string[i]);
        i++;
    }
}

/// @brief Envia string pela UART
/// @param string 
void UART_envia_comando(char *string){
    int i = 0;
    
    while (string[i] != '\0'){
        UART_transmit(string[i]);
        i++;
    }
}
#include <avr/io.h>
#include <avr/interrupt.h>

#include "teclado.h"
#include "lcd_lib.h"
#include "delay.h"
#include "uart_lib.h"

int indice_placa = 0; 
char placa_digitada[8];

void registra_caractere_confirmado(char letra){
    if (indice_placa<7){ 
        //envia_char(letra);
        lcd_data(letra);
        placa_digitada[indice_placa] = letra;
        indice_placa++;
    }
}

ISR (TIMER1_COMPA_vect){
    confirmar_tecla = 1;
    TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); //desliga o timer
}

int main(void) {
    
    PINH = 0xFF;
    
    UART_init();

    /////////  TIMERS //////////
    TCCR0A = 0;
    TCCR0B = 3; // prescaler de 64
    
    TIMSK1 = (1 << 1); // interrupcao timer 1
    sei(); // Ativa interrupcao global
    
    TCCR1A = 0;
    TCCR1B = (1 << WGM12); //modo ctc
     
    OCR1A = 31249; //31250 - 1 (500ms)
    TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); // começa desligado

    TCCR2A = 0; 
	TCCR2B = 2; //Timer 2 com prescaler de 8;
    
    lcd_init(); //inicia lcd

    lcd_cmd(0x80); //posiciona cursor na primeira linha
    envia_string("DIGITE PLACA");
    lcd_cmd(0xC0); //cursor na segunda linha

    init_teclado();

    while(1) {
        
        roda_teclado();
        if (nova_tecla == TECLADO_PRESSIONADO){
            registra_caractere_confirmado(saida_teclado);
            UART_transmit(saida_teclado);
            nova_tecla = TECLADO_LIVRE;
        }


    }

    return 0;
}
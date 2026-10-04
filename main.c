#include <avr/io.h>
#include <avr/interrupt.h>

#include "teclado.h"
#include "lcd_lib.h"
#include "delay.h"
#include "uart_lib.h"
#include "gerenciador_msgs.h"
#include "mensagem.h"
#include "buffer_circular.h"

int indice_placa = 0; 
char placa_digitada[8];
int estado_sistema = 0; 
// 0 = digitando placa
// 1 = validando
//2 = placa invalida

int resetar_tela = 0;
int tempo_erro = 0;

void registra_caractere_confirmado(char letra){
    if (indice_placa<7){ 
        //envia_char(letra);
        lcd_data(letra);
        placa_digitada[indice_placa] = letra;
        indice_placa++;
    }
}

void verifica_placa(){
    
    reset_memoria_teclado();
    //placa inválida
    if ((placa_digitada[0]<'A'|| placa_digitada[0]>'Z')|| (placa_digitada[1]<'A'|| placa_digitada[1]>'Z')|| (placa_digitada[2]<'A'|| placa_digitada[2]>'Z')
        || (placa_digitada[3]<'0'|| placa_digitada[3]>'9')|| (placa_digitada[5]<'0'|| placa_digitada[5]>'9')|| (placa_digitada[6]<'0'|| placa_digitada[6]>'9') 
        ||  ((placa_digitada[4]<'0'|| placa_digitada[4]>'9')&&(placa_digitada[4]<'A'|| placa_digitada[4]>'Z')) ){
        limpa_lcd();    
        envia_string("PLACA INVALIDA");
        estado_sistema = 2;
        TCNT1 = 0; 
        TCCR1B |= (1 << CS12);
    }else{
        limpa_lcd();
        
        lcd_cmd(0x80);
        envia_string("TEMPO: 1)30min");
        lcd_cmd(0xC0);
        envia_string("2)1h 3)1h30 4)2h");
        estado_sistema = 1;
    }
      
}


ISR (TIMER1_COMPA_vect){
    if (estado_sistema == 0 || estado_sistema == 1){ 
        confirmar_tecla = 1;
        TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); //desliga o timer
    }
    else if (estado_sistema == 2){
        tempo_erro++;
        if(tempo_erro >= 4){ //So entra aqui quando contar 4 vezes
            resetar_tela = 1; //ativa a flag para voltar para a tela inicial
            tempo_erro = 0;
            TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); //desliga o timer
        }
        
    }

}

int main(void) {
    Mensagem msg;

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
    envia_string("DIGITE PLACA d");
    lcd_cmd(0xC0); //cursor na segunda linha

    init_teclado();

    init_gerenciador_msgs();

    while(1) {
        
        if (estado_sistema == 0){ 
            roda_teclado();
            
            if (nova_tecla == TECLADO_PRESSIONADO){
                registra_caractere_confirmado(saida_teclado);
                UART_transmit(saida_teclado);
                nova_tecla = TECLADO_LIVRE;
            }
                    
            if(indice_placa == 7){
                placa_digitada [7] = '\0';
                verifica_placa();
            }
        }
        else if (estado_sistema == 2 && resetar_tela == 1){ //Se a placa é invalida e ja pode resetar a tela
            resetar_tela = 0;
            limpa_lcd();
            lcd_cmd(0x80); //posiciona cursor na primeira linha
            envia_string("DIGITE PLACA");
            lcd_cmd(0xC0); //cursor na segunda linha
            indice_placa = 0;
            estado_sistema = 0; //Volta para o estado inicial de dgitar a placa
        }
        else if (estado_sistema == 1) {
            
            roda_teclado();
            if (nova_tecla == TECLADO_PRESSIONADO){
                int tempo_escolhido = 0;
                if (saida_teclado == '1'){
                    tempo_escolhido = 30; //30min
                }
                else if (saida_teclado == 'A'){
                    tempo_escolhido = 60;
                }
                else if (saida_teclado == 'D'){
                    tempo_escolhido = 90;
                }
                else if (saida_teclado =='G'){
                    tempo_escolhido = 120;
                }
            
                if (tempo_escolhido>0){
                    limpa_lcd ();
                    envia_string("PAGAMENTO");
                    estado_sistema = 3;
                }
                nova_tecla = TECLADO_LIVRE;
            }
        }

        novo_caracter_recebido_UART();
        gerenciar_msgs();

        if (novas_mensagens() > 0){
            msg = le_mensagem();
            limpa_lcd();
            lcd_cmd(0x80); //posiciona cursor na primeira linha
            envia_string(msg.tipo);
            lcd_cmd(0xC0); //cursor na segunda linha
            envia_string(msg.string);
        }
    }
    return 0;
}
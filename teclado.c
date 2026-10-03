#include "teclado.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include "delay.h"

#define BOUNCE 7

#define C0 PH3
#define C1 PH4
#define C2 PH5

#define L0 PB1
#define L1 PB2
#define L2 PB3
#define L3 PB4

char tecla_atual = '3';
int confirmar_tecla;
int toques_em_sequencia = 0;

char nova_tecla = TECLADO_LIVRE;
char saida_teclado = '\0';

char tecla_pressionada = '\0';
char tecla_pendente    = '\0';
char tecla_anterior    = '\0';

void init_teclado(){
    DDRH &= ~((1 << C0) | (1 << C1) | (1 << C2)); //entradas
    DDRB |= (1 << 7) | (1<<L0) | (1<<L1)| (1<<L2)| (1<<L3); //saidas
}

char debouncer(char pin_index){
    char count = 0;
    char key_last = (PINH >> pin_index) & 0x01;
    char key_now;
    
    while (1){
        delay_1ms();
        key_now = (PINH >> pin_index) & 0x01;
        
        if (key_now == key_last) {
            count = count + 1;
        } else {
            count = 0;
        }

        if (count == BOUNCE) {
            return key_now;
        }

        key_last = key_now;
    }
}

char ler_teclado(void) {
    
    
    PORTH |= (1<<C0) | (1<<C1) | (1<<C2);
    
    PORTB &= ~(1<<L0); //Ativa linha 0 
    
    PORTB |= (1<<L1)| (1<<L2)| (1<<L3); //desativas as outras 

    if (debouncer(3) == 0) {
         return '1';
    }
    if (debouncer(4) == 0) {
         return '2';
    }
    if (debouncer(5) == 0) {
         return '3';
    }
    
    PORTB &= ~(1<<L1); //Ativa linha 1 
    
    PORTB |= (1<<L0)| (1<<L2)| (1<<L3); //desativas as outras 

    if (debouncer(3) == 0) {
         return '4';
    }
    if (debouncer(4) == 0) {
         return '5';
    }
    if (debouncer(5) == 0) {
         return '6';
    }


    PORTB &= ~(1<<L2); //Ativa linha 2 
    
    PORTB |= (1<<L0)| (1<<L1)| (1<<L3); //desativas as outras 

    if (debouncer(3) == 0) {
         return '7';
    }
    if (debouncer(4) == 0) {
         return '8';
    }
    if (debouncer(5) == 0) {
         return '9';
    }


    PORTB &= ~(1<<L3); //Ativa linha 3 
    
    PORTB |= (1<<L0)| (1<<L2)| (1<<L1); //desativas as outras 

    if (debouncer(3) == 0) {
         return '*';
    }
    if (debouncer(4) == 0) {
         return '0';
    }
    if (debouncer(5) == 0) {
         return '#';
    }

    return '\0';
}

char decodifica_tecla (char tecla){
    if (tecla == '2'){
        if (toques_em_sequencia == 1)return 'A';
        if (toques_em_sequencia == 2)return 'B';
        if (toques_em_sequencia == 3)return 'C';
        if (toques_em_sequencia == 4)return '2';
        toques_em_sequencia = 1; return 'A';
    }
    if (tecla == '3'){
        if (toques_em_sequencia == 1)return 'D';
        if (toques_em_sequencia == 2)return 'E';
        if (toques_em_sequencia == 3)return 'F';
        if (toques_em_sequencia == 4)return '3';
        toques_em_sequencia = 1; return 'D';
    }
    if (tecla == '4'){
        if (toques_em_sequencia == 1)return 'G';
        if (toques_em_sequencia == 2)return 'H';
        if (toques_em_sequencia == 3)return 'I';
        if (toques_em_sequencia == 4)return '4';
        toques_em_sequencia = 1; return 'G';
    }
    if (tecla == '5'){
        if (toques_em_sequencia == 1)return 'J';
        if (toques_em_sequencia == 2)return 'K';
        if (toques_em_sequencia == 3)return 'L';
        if (toques_em_sequencia == 4)return '5';
        toques_em_sequencia = 1; return 'J';
    }
    if (tecla == '6'){
        if (toques_em_sequencia == 1)return 'M';
        if (toques_em_sequencia == 2)return 'N';
        if (toques_em_sequencia == 3)return 'O';
        if (toques_em_sequencia == 4)return '6';
        toques_em_sequencia = 1; return 'M';
    }
    if (tecla == '7'){
        if (toques_em_sequencia == 1)return 'P';
        if (toques_em_sequencia == 2)return 'Q';
        if (toques_em_sequencia == 3)return 'R';
        if (toques_em_sequencia == 4)return 'S';
        if (toques_em_sequencia == 5)return '7';
        toques_em_sequencia = 1; return 'P';
    }
    if (tecla == '8'){
        if (toques_em_sequencia == 1)return 'T';
        if (toques_em_sequencia == 2)return 'U';
        if (toques_em_sequencia == 3)return 'V';
        if (toques_em_sequencia == 4)return '8';
        toques_em_sequencia = 1; return 'T';
    }
    if (tecla == '9'){
        if (toques_em_sequencia == 1)return 'W';
        if (toques_em_sequencia == 2)return 'X';
        if (toques_em_sequencia == 3)return 'Y';
        if (toques_em_sequencia == 4)return 'Z';
        if (toques_em_sequencia == 5)return '9';
        toques_em_sequencia = 1; return 'W';
    }
    toques_em_sequencia = 1;
    return tecla;
}


void roda_teclado() {
    tecla_pressionada = ler_teclado();
        
    if(tecla_pressionada != '\0' && (tecla_pressionada != tecla_anterior)){      
        tecla_atual = tecla_pressionada;
            
        if (tecla_atual == tecla_pendente){ //se a tecla pressionada for igual a anterior
            toques_em_sequencia++;         
        } else { //se foi apertada uma tecla diferente        
            if (tecla_pendente != '\0'){
                char caractere_confirmado = decodifica_tecla(tecla_pendente); //confirma a tecla anterior
                nova_tecla = TECLADO_PRESSIONADO;
                saida_teclado = caractere_confirmado;
                // registra_caractere_confirmado(caractere_confirmado);
            }
                
            toques_em_sequencia = 1;
            tecla_pendente = tecla_atual;
        }
            
        confirmar_tecla = 0;
        TCNT1 = 0; //Zera o contador
        TCCR1B |= (1 << CS12); //liga timer com prescaler 256
    }
            
    if (confirmar_tecla == 1){ //se passou o tempo
        if(tecla_pendente !='\0'){ 
            char caractere_confirmado = decodifica_tecla(tecla_pendente); //confirma a tecla
            // registra_caractere_confirmado(caractere_confirmado);
            nova_tecla = TECLADO_PRESSIONADO;
            saida_teclado = caractere_confirmado;
            toques_em_sequencia = 0;
            tecla_pendente = '\0';        
        }
    }
    confirmar_tecla = 0;
    tecla_anterior = tecla_pressionada;
}
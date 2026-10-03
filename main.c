#include <avr/io.h>
#include <avr/interrupt.h>

#define BOUNCE 7
#define MYUBRR 51 //19200 baud rate

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

// Inicializa a Serial 
void USART_init(int ubrr) {
    UBRR0H = ubrr>>8;
    UBRR0L = ubrr; 
	UCSR0B = (1<<4) | (1<<3); // //receptor e transmissor ativos
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00) | (1<<UPM01) | (1<<UPM00); // 8 bits de dados, 1 stop bit, paridade impar
}

// Envia uma string pela serial
void envia_mensagem(char *texto){
	int i = 0;
	while (texto[i]!='\0'){
		while ((UCSR0A & (1<<5)) == 0);
		UDR0 = texto[i];
		i++;
	}
	
}

// Envia apenas um caractere isolado
void envia_char(char c) {
    while ((UCSR0A & (1<<5)) == 0);
    UDR0 = c;
}


void delay_1ms(){
    TCNT0 = 6;  
    TIFR0 = 1;
    while ((TIFR0 & (1 << 0) )==0);
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

ISR (TIMER1_COMPA_vect){
    confirmar_tecla = 1;
    TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); //desliga o timer
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

int main(void) {
    char tecla_pressionada;
    char tecla_anterior = '\0';
    char tecla_pendente = '\0';
    
    PINH = 0xFF;
    
    // Inicializa a Serial a 9600 bauds
    USART_init(MYUBRR);
    
    TCCR0A = 0;
    TCCR0B = 3; // prescaler de 64
    
    TIMSK1 = (1 << 1); // interrupcao timer 1
    sei(); // Ativa interrupcao global
    
    TCCR1A = 0;
    TCCR1B = (1 << WGM12); //modo ctc
     
    OCR1A = 31249; //31250 - 1 (500ms)
    TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); // começa desligado
    
    DDRH &= ~((1 << C0) | (1 << C1) | (1 << C2)); //entradas
    DDRB |= (1 << 7) | (1<<L0) | (1<<L1)| (1<<L2)| (1<<L3); //saidas

    while(1) {
        tecla_pressionada = ler_teclado();
        
        if(tecla_pressionada != '\0' && (tecla_pressionada != tecla_anterior)){      
            tecla_atual = tecla_pressionada;
            
            if (tecla_atual == tecla_pendente){ //se a tecla pressionada for igual a anterior
                toques_em_sequencia++;         
            }
            
            else{ //se foi apertada uma tecla diferente
                
                if (tecla_pendente != '\0'){
                    envia_char (decodifica_tecla(tecla_pendente));
                }
                
                toques_em_sequencia = 1;
                tecla_pendente = tecla_atual;
            }
            
            confirmar_tecla = 0;
            TCNT1 = 0; //Zera o contador
            TCCR1B |= (1 << CS12); //liga timer com prescaler 256
        }
            
        if (confirmar_tecla == 1){
            if(tecla_pendente !='\0'){ 
                envia_char (decodifica_tecla(tecla_pendente));
                toques_em_sequencia = 0;
                tecla_pendente = '\0';
                confirmar_tecla = 0;
            }
        }
        
    tecla_anterior = tecla_pressionada;
    }
    
    return 0;
}
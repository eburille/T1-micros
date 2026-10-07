#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include "hora.h"

#include "teclado.h"
#include "lcd_lib.h"
#include "delay.h"
#include "uart_lib.h"
#include "gerenciador_msgs.h"
#include "mensagem.h"
#include "buffer_circular.h"

#define DIGITANDO_PLACA    0
#define PLACA_VALIDA       1
#define PLACA_INVALIDA     2
#define MENU_PAGAMENTO     3
#define MOEDA              4
#define CARTAO             5 
#define CARTAO_VIRT        6 
#define CARTAO_SENHA       7 
#define CARTAO_INVALIDO    8
#define SENHA_INVALIDA     9
#define REQ_DADOS_ESTAC    10
#define AGUARDA_E          11
#define REQ_PAGAMENTO      12
#define AGUARDA_P          13



int indice_placa = 0; 
int indice_cartao = 0;
int indice_cartao_senha = 0;
char placa_digitada[8];
char cartao_digitado [7];
char senha_cartao_digitado [6];
int estado_sistema = 0; 

int flag_vaga_especial = 0;
int resetar_tela = 0;
int tempo_erro = 0;

char lista_placas_especiais[5][8] = {
	"IOS0098",
	"PEI1Z92",
	"IDO0192",
	"IDS0090",
	"PDE3400" };


void registra_caractere_confirmado(char letra){
    
    if (estado_sistema == DIGITANDO_PLACA){ 
        if (indice_placa<7){ 
            //envia_char(letra);
            lcd_data(letra);
            placa_digitada[indice_placa] = letra;
            indice_placa++;
        }
    }
    else if (estado_sistema == CARTAO){
        if (indice_cartao < 6){
            lcd_data(letra);
            cartao_digitado[indice_cartao] = letra;
            indice_cartao++;
        }
    }
    else if (estado_sistema == CARTAO_SENHA){
        if (indice_cartao_senha < 5){
            lcd_data ('*');
            senha_cartao_digitado[indice_cartao_senha] = letra;
            indice_cartao_senha++;
        }
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
        estado_sistema = PLACA_INVALIDA;
        TCNT1 = 0; 
        TCCR1B |= (1 << CS12);
    }else{
        flag_vaga_especial = 0;
        for (int i = 0; i < 5; i++)
        {
            if (strcmp(placa_digitada, lista_placas_especiais[i]) == 0){
                flag_vaga_especial = 1;
                break;
            }
        }

        limpa_lcd();
        lcd_cmd(0x80);
        if (flag_vaga_especial){
            envia_string("1)30min(R$0)");
            lcd_cmd(0xC0);
            envia_string("2)1h 3)2h 4)>2h");
        }else{
            envia_string("TEMPO: 1)30min");
            lcd_cmd(0xC0);
            envia_string("2)1h 3)1h30 4)2h");
        }
        estado_sistema = PLACA_VALIDA;
    }
}

//Por enquanto verifica só se é numero, depois tem que comparar com as contas reais
void verifica_cartao(){
    reset_memoria_teclado();
    int i;
    int flag_cartao_invalido = 0;
    for (i = 0; i < 6; i++){
        if (cartao_digitado[i]<'0'|| cartao_digitado [i]>'9'){ 
            flag_cartao_invalido = 1;
            break;
        }
    }
    if (flag_cartao_invalido){
        limpa_lcd ();
        lcd_cmd (0x80);
        envia_string ("CARTAO INVALIDO");
        lcd_cmd(0xC0);
        envia_string ("APENAS NUMEROS");
        estado_sistema = CARTAO_INVALIDO;
        TCNT1 = 0; 
        TCCR1B |= (1 << CS12);
    }else{
        limpa_lcd ();
        lcd_cmd (0x80);
        envia_string ("SENHA:");
        lcd_cmd (0XC0);
        estado_sistema = CARTAO_SENHA;
    }
      
}


//Por enquanto verifica só se é numero, depois tem que comparar com as contas reais
void verifica_senha_cartao (){
    reset_memoria_teclado();
    int i;
    int flag_senha_invalida = 0;
    for (i = 0; i < 5; i++){
        if (senha_cartao_digitado[i]<'0'|| senha_cartao_digitado [i]>'9'){ //verificar se o cartao contem apenas numeros
            flag_senha_invalida = 1;
            break;
        }
    }
    if (flag_senha_invalida){
        limpa_lcd ();
        lcd_cmd (0x80);
        envia_string ("SENHA INVALIDA");
        estado_sistema = SENHA_INVALIDA;
        TCNT1 = 0; 
        TCCR1B |= (1 << CS12);
    }else{
        limpa_lcd ();
        lcd_cmd (0x80);
        envia_string ("OK");
        estado_sistema = REQ_DADOS_ESTAC;
    }
}




void apaga_caractere_lcd(){
    
    if(estado_sistema == DIGITANDO_PLACA){
        indice_placa--;
        lcd_cmd (0xC0 + indice_placa);
        lcd_data (' ');
        lcd_cmd (0XC0 + indice_placa);
        placa_digitada [indice_placa] = '\0';
    }
    else if (estado_sistema == CARTAO){
        indice_cartao--;
        lcd_cmd (0xC0 + indice_cartao);
        lcd_data (' ');
        lcd_cmd (0XC0 + indice_cartao);
        cartao_digitado [indice_cartao] = '\0';
    }
    else if (estado_sistema == CARTAO_SENHA){
        indice_cartao_senha--;
        lcd_cmd (0xC0 + indice_cartao_senha);
        lcd_data (' ');
        lcd_cmd (0XC0 + indice_cartao_senha);
        senha_cartao_digitado[indice_cartao_senha] = '\0';
    }
    
}

void envia_dados_estacionamento(char tempo, int preco){
    char pacote_estacionamento[11];
    char pacote_pagamento [17];
    
    if (estado_sistema == REQ_DADOS_ESTAC){
        sprintf(pacote_estacionamento,"PE%s%d", placa_digitada,tempo); //Isso tudo ai vai ter que estar codificado depois

        UART_envia_string (pacote_estacionamento);
        limpa_lcd ();
        lcd_cmd (0x80);
        envia_string ("REGISTRANDO");
        
        estado_sistema = AGUARDA_E;
    }
    else if (estado_sistema == REQ_PAGAMENTO){
         limpa_lcd ();
        lcd_cmd (0x80);
        envia_string ("CONFIRMANDO");
        sprintf(pacote_pagamento,"PP%s%s%d", cartao_digitado,senha_cartao_digitado,preco); //Isso tudo ai vai ter que estar codificado depois
        UART_envia_string (pacote_pagamento);
        estado_sistema = AGUARDA_P;
    }
    
}



ISR (TIMER1_COMPA_vect){
    if (estado_sistema == DIGITANDO_PLACA || estado_sistema == PLACA_VALIDA || estado_sistema == MENU_PAGAMENTO|| 
        estado_sistema == CARTAO || estado_sistema == CARTAO_SENHA){ 
        
        confirmar_tecla = 1;
        TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); //desliga o timer
    }
    else if (estado_sistema == PLACA_INVALIDA || estado_sistema == CARTAO_INVALIDO || estado_sistema == SENHA_INVALIDA){
        tempo_erro++;
        if(tempo_erro >= 4){ //So entra aqui quando contar 4 vezes
            resetar_tela = 1; //ativa a flag para voltar para a tela inicial
            tempo_erro = 0;
            TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); //desliga o timer
        }
    }
}

ISR(TIMER3_COMPA_vect) {
    static char contador_segundos = 0;

    contador_segundos++;
    if (contador_segundos >= 60) {
        contador_segundos = 0;
        prox_minuto();
    }
}



int main(void) {
    
    signed char tempo_escolhido = -1;
    int preco_a_pagar = 0;
    char eh_cartao = 0;
    
    Mensagem msg;

    PINH = 0xFF;
    
    UART_init();

     /////////  TIMERS //////////

      ///// TIMER 0 /////
    TCCR0A = 0;
    TCCR0B = 3; // prescaler de 64

      ///// TIMER 1 /////
    TIMSK1 = (1 << 1); // interrupcao timer 1
    sei(); // Ativa interrupcao global

    TCCR1A = 0;
    TCCR1B = (1 << WGM12); //modo ctc
     
    
    OCR1A = 31249; //31250 - 1 (500ms)
    TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); // começa desligado

    
      ///// TIMER 2 /////
    TCCR2A = 0; 
	TCCR2B = 2; //Timer 2 com prescaler de 8;

      ///// TIMER 3 /////

    // 1 SEGUNDO
    OCR3A = 15625; 

    // Habilita o modo CTC + Prescaler para 1024
    TCCR3B |= (1 << WGM32) | (1 << CS32) | (1 << CS30); 
    
    // Habilita a interrupção de comparação A do Timer3 (OCIE3A)
    TIMSK3 |= (1 << OCIE3A);

    sei(); // Ativa interrupcao global
    
    lcd_init(); //inicia lcd

    lcd_cmd(0x80); //posiciona cursor na primeira linha
    envia_string("DIGITE PLACA");
    lcd_cmd(0xC0); //cursor na segunda linha

    init_teclado();

    init_gerenciador_msgs();


    while(1) {
        
        if (estado_sistema == DIGITANDO_PLACA){ 
            roda_teclado();
            
            if (nova_tecla == TECLADO_PRESSIONADO){
                if (saida_teclado == '*'){
                    if (indice_placa > 0){
                    UART_transmit (saida_teclado);
                    apaga_caractere_lcd();
                    }
                }
                else if (saida_teclado == '#'){ // precisa apertar '#' para confirmar
                    if(indice_placa == 7){
                    placa_digitada [7] = '\0';
                    verifica_placa();
                    UART_transmit (saida_teclado);
                    }
                }
                else {
                    registra_caractere_confirmado(saida_teclado);
                    UART_transmit(saida_teclado);
                    
                }
                nova_tecla = TECLADO_LIVRE;
            }

            
           
        }
        else if (estado_sistema == PLACA_INVALIDA && resetar_tela == 1){ //Se a placa é invalida e ja pode resetar a tela
            resetar_tela = 0;
            limpa_lcd();
            lcd_cmd(0x80); //posiciona cursor na primeira linha
            envia_string("DIGITE PLACA");
            lcd_cmd(0xC0); //cursor na segunda linha
            indice_placa = 0;
            reset_memoria_teclado();
            estado_sistema = DIGITANDO_PLACA; //Volta para o estado inicial de dgitar a placa
        }
        else if (estado_sistema == PLACA_VALIDA) {
            
            roda_teclado();
            if (nova_tecla == TECLADO_PRESSIONADO){
                UART_transmit(saida_teclado);
                tempo_escolhido = -1;
                preco_a_pagar = 0;

                if (saida_teclado == '1'){
                    if (flag_vaga_especial)preco_a_pagar = 0;
                    else preco_a_pagar = 200;
                    tempo_escolhido = 0; //30min
                }
                else if (saida_teclado == 'A'){
                    if (flag_vaga_especial)preco_a_pagar = 200;
                    else preco_a_pagar = 350;
                    tempo_escolhido = 1; //30min a 1h
                }
                else if (saida_teclado == 'D'){
                    if (flag_vaga_especial){
                        preco_a_pagar = 350;
                        tempo_escolhido = 3; //1h e 30min a 2h

                    }else {
                        preco_a_pagar = 450;
                        tempo_escolhido = 2; //1h a 1h e 30min
                    }
                }
                else if (saida_teclado =='G'){
                    if (flag_vaga_especial){
                        preco_a_pagar = 450;
                        tempo_escolhido = 4; //ilimitado
                    }else {
                        preco_a_pagar = 600;
                        tempo_escolhido = 3; //1h e 30min a 2h
                    }
                }
            
                if (tempo_escolhido>=0){
                    limpa_lcd ();
                    lcd_cmd(0x80);
                    char msg_valor_pagamento[20];
                    
                    if (preco_a_pagar != 0){
                        sprintf(msg_valor_pagamento, "VALOR: R$%d,%02d", preco_a_pagar / 100, preco_a_pagar % 100);
                        envia_string (msg_valor_pagamento);
                        lcd_cmd(0XC0);
                        envia_string ("1)MOD 2)CRT 3)VR");
                        reset_memoria_teclado();
                        estado_sistema = MENU_PAGAMENTO;
                    }else{
                        estado_sistema = REQ_DADOS_ESTAC;
                    }
                    
                }
                nova_tecla = TECLADO_LIVRE;
            }
        }
        else if (estado_sistema == MENU_PAGAMENTO){
            roda_teclado();
            
            if (nova_tecla == TECLADO_PRESSIONADO){
                eh_cartao = 0;
                UART_transmit(saida_teclado);
                if (saida_teclado == '1'){
                    limpa_lcd();
                    //Lógica para as moedas
                }
                else if (saida_teclado == 'A'){
                    eh_cartao = 1;
                    limpa_lcd();
                    lcd_cmd(0X80);
                    envia_string ("NUMERO CARTAO:");
                    lcd_cmd(0XC0);
                    reset_memoria_teclado();
                    estado_sistema = CARTAO;
                }
                else if (saida_teclado == 'D'){
                    limpa_lcd();
                    //Lógica para cartao virtual
                }
                nova_tecla = TECLADO_LIVRE;
            }
        }
        else if (estado_sistema == CARTAO){
            roda_teclado();
            if(nova_tecla == TECLADO_PRESSIONADO){
                
                if (saida_teclado == '*'){
                    if (indice_cartao > 0){
                        apaga_caractere_lcd();
                    }
                }

                else if (saida_teclado == '#'){
                    if (indice_cartao == 6){
                        cartao_digitado [6] = '\0';
                        verifica_cartao();
                    }
                }
                else {
                    UART_transmit(saida_teclado);
                    registra_caractere_confirmado(saida_teclado);
                }
                nova_tecla = TECLADO_LIVRE;
            }
            
            
        }
        else if (estado_sistema == CARTAO_INVALIDO && resetar_tela == 1){ //Se o cartao é invalido e ja pode resetar a tela
            resetar_tela = 0;
            limpa_lcd();
            lcd_cmd(0x80); //posiciona cursor na primeira linha
            envia_string("NUMERO CARTAO:");
            lcd_cmd(0xC0); //cursor na segunda linha
            indice_cartao = 0;
            reset_memoria_teclado();
            estado_sistema = CARTAO; //Volta para o estado inicial de dgitar o numero do cartao
        }
        else if (estado_sistema == CARTAO_SENHA){
            roda_teclado ();
            if (nova_tecla == TECLADO_PRESSIONADO){
                
                if (saida_teclado == '*'){
                    if (indice_cartao_senha > 0){
                        apaga_caractere_lcd();
                    }
                }
                else if (saida_teclado == '#'){
                    if (indice_cartao_senha == 5){
                        senha_cartao_digitado[5] = '\0';
                        verifica_senha_cartao();
                    }
                }
                else {
                    UART_transmit(saida_teclado);
                    registra_caractere_confirmado(saida_teclado);
                }
                nova_tecla = TECLADO_LIVRE;
            }
        
            
        }
        else if (estado_sistema == SENHA_INVALIDA && resetar_tela == 1){
            resetar_tela = 0;
            limpa_lcd();
            lcd_cmd(0x80); //posiciona cursor na primeira linha
            envia_string("SENHA:");
            lcd_cmd(0xC0); //cursor na segunda linha
            indice_cartao_senha = 0;
            reset_memoria_teclado();
            estado_sistema = CARTAO_SENHA; //Volta para o estado inicial de dgitar a senha do cartao
        }

        else if (estado_sistema == REQ_DADOS_ESTAC){
        
            envia_dados_estacionamento(tempo_escolhido, preco_a_pagar);    
            
        }
        else if (estado_sistema == REQ_PAGAMENTO){
            envia_dados_estacionamento(tempo_escolhido, preco_a_pagar);
        }
        

        // novo_caracter_recebido_UART();
        gerenciar_msgs();

    
        if (novas_mensagens_servidor() > 0){
            msg = le_mensagem();
            UART_envia_string("MENSAGEMEH:");
            UART_transmit(msg.tipo);
            UART_envia_string(msg.string);

            if(estado_sistema == AGUARDA_E && msg.tipo == 'E'){
                if (eh_cartao){
                    estado_sistema = REQ_PAGAMENTO;
                    
                    
                }else{
                    estado_sistema = 15;
                }
            }
            else if (estado_sistema == AGUARDA_P && msg.tipo == 'P'){
                
                if (msg.string[0] == 'S'){
                    limpa_lcd();
                    lcd_cmd (0x80);
                    envia_string ("PAGAMENTO OK");
                    estado_sistema = 15;//So pra ir pra outro estado por enquanto
                }
                else if (msg.string[0] == 'F'){
                    limpa_lcd();
                    lcd_cmd (0x80);
                    envia_string ("DADOS ERRADOS");
                    estado_sistema = 16;//So pra ir pra outro estado por enquanto
                }
                else if (msg.string[0] == 'I'){
                    limpa_lcd();
                    lcd_cmd (0x80);
                    envia_string ("SEM SALDO");
                   estado_sistema = 17;//So pra ir pra outro estado por enquanto
                }
            }
            
            
        }
    }
    return 0;
}




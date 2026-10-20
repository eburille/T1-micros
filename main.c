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
#include "parkimetro.h"
#include "led.h"

#define DIGITANDO_PLACA     0
#define SELECAO_TEMPO        1
#define PLACA_INVALIDA      2
#define MENU_PAGAMENTO      3
#define MOEDA               4
#define CARTAO              5 
#define CARTAO_VIRT         6 
#define CARTAO_SENHA        7 
#define CARTAO_INVALIDO     8
#define SENHA_INVALIDA      9
#define REQ_DADOS_ESTAC    10
#define AGUARDA_E          11
#define REQ_PAGAMENTO      12
#define AGUARDA_P          13
#define ERRO_DADOS_CARTAO  14
#define ERRO_SALDO_CARTAO  15
#define QUER_COMPROVANTE   16
#define CARTAO_VIRTUAL     17
#define SUCESSO_CART_VIRT  18
#define SELECAO_RUA        20


int estado_sistema = 0; 
int indice_placa = 0; 
int indice_cartao = 0;
int indice_cartao_senha = 0;
char placa_digitada[8];
char cartao_digitado [7];
char senha_cartao_digitado [6];

int contador_inatividade = 0;
char flag_atualiza_relogio = 0;
char sistema_inativo = 1;
char rua_escolhida = -1;


int flag_vaga_especial = 0;
int resetar_tela = 0;
int tempo_erro = 0;

char lista_cartao_virtual[5][8] = {
	"WCC9850",
	"PIN0C20",
	"CAR0230",
	"VIR7960",
	"PAR8320" };


int saldo_cartao_virtual[5] = {
	5000, 
	2000, 
	3000, 
	6000, 
	2000 };


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

        flag_vaga_especial = N_ESPECIAL;
        for (int i = 0; i < 5; i++)
        {
            if (strcmp(placa_digitada, lista_placas_especiais[i]) == 0){
                flag_vaga_especial = ESPECIAL;
                break;
            }
        }

        limpa_lcd();
        lcd_cmd(PRIMEIRA_LINHA);
        envia_string("0)OSV  1)SARM");
        lcd_cmd(SEGUNDA_LINHA);
        envia_string("2)JaoP.");
        estado_sistema = SELECAO_RUA;
    }
}

void verifica_cartao_virtual(int preco){
    int i;
    char placa_encontrada = 0;
    int indice_virtual = -1;
    for (i=0;i<5;i++){
        if(strcmp(placa_digitada, lista_cartao_virtual[i])==0){
            placa_encontrada = 1;
            indice_virtual = i;
            break;
        }
    }

    limpa_lcd();
    if (placa_encontrada){
        if(saldo_cartao_virtual [indice_virtual]>= preco){
            saldo_cartao_virtual [indice_virtual] -= preco;
            lcd_cmd (PRIMEIRA_LINHA);
            envia_string("PAGAMENTO OK");
            lcd_cmd (SEGUNDA_LINHA);
            char msg_saldo[16];
            sprintf(msg_saldo, "SALDO:R$%d,%02d", saldo_cartao_virtual[indice_virtual]/100, saldo_cartao_virtual[indice_virtual]%100);
            envia_string(msg_saldo);
            estado_sistema = SUCESSO_CART_VIRT;
            TCNT1 = 0; 
            TCCR1B |= (1 << CS12);
            
        }else{
            lcd_cmd (PRIMEIRA_LINHA);
            envia_string("SEM CREDITO");
            estado_sistema = ERRO_SALDO_CARTAO;
            TCNT1 = 0; 
            TCCR1B |= (1 << CS12);

        }
        
    }else {
        lcd_cmd (PRIMEIRA_LINHA);
        envia_string("NAO REGISTRADO");
        estado_sistema = ERRO_DADOS_CARTAO;
        TCNT1 = 0; 
        TCCR1B |= (1 << CS12);

    }
    
}

//Verifica só se é numero
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
        lcd_cmd (PRIMEIRA_LINHA);
        envia_string ("CARTAO INVALIDO");
        lcd_cmd(SEGUNDA_LINHA);
        envia_string ("APENAS NUMEROS");
        estado_sistema = CARTAO_INVALIDO;
        TCNT1 = 0; 
        TCCR1B |= (1 << CS12);
    }else{
        limpa_lcd ();
        lcd_cmd (PRIMEIRA_LINHA);
        envia_string ("SENHA:");
        lcd_cmd (SEGUNDA_LINHA);
        estado_sistema = CARTAO_SENHA;
    }
      
}


//Verifica só se é numero
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
        lcd_cmd (PRIMEIRA_LINHA);
        envia_string ("SENHA INVALIDA");
        estado_sistema = SENHA_INVALIDA;
        TCNT1 = 0; 
        TCCR1B |= (1 << CS12);
    }else{
        limpa_lcd ();
        lcd_cmd (PRIMEIRA_LINHA);
        envia_string ("OK");
        // estado_sistema = REQ_DADOS_ESTAC;
        estado_sistema = REQ_PAGAMENTO;
    }
}




void apaga_caractere_lcd(){
    
    if(estado_sistema == DIGITANDO_PLACA){
        indice_placa--;
        lcd_cmd (SEGUNDA_LINHA + indice_placa);
        lcd_data (' ');
        lcd_cmd (SEGUNDA_LINHA + indice_placa);
        placa_digitada [indice_placa] = '\0';
    }
    else if (estado_sistema == CARTAO){
        indice_cartao--;
        lcd_cmd (SEGUNDA_LINHA + indice_cartao);
        lcd_data (' ');
        lcd_cmd (SEGUNDA_LINHA + indice_cartao);
        cartao_digitado [indice_cartao] = '\0';
    }
    else if (estado_sistema == CARTAO_SENHA){
        indice_cartao_senha--;
        lcd_cmd (SEGUNDA_LINHA + indice_cartao_senha);
        lcd_data (' ');
        lcd_cmd (SEGUNDA_LINHA + indice_cartao_senha);
        senha_cartao_digitado[indice_cartao_senha] = '\0';
    }
    
}

void envia_dados_estacionamento(char tempo){
    char pacote_estacionamento[11];
    
    //Formato é 'P''E''PLACA_DO_CARRO''MODALIDADE'
    sprintf(pacote_estacionamento,"PE%s%d", placa_digitada,tempo); //Isso tudo ai vai ter que estar codificado depois

    UART_envia_string(pacote_estacionamento);
    UART_envia_string("\n\n");
    limpa_lcd ();
    lcd_cmd (PRIMEIRA_LINHA);
    envia_string ("REGISTRANDO...");
        
    estado_sistema = AGUARDA_E; //Estado onde está aguardando o 'S''E' do servidor externo
        
}

void envia_dados_pagamento(int preco){
    char pacote_dados [15];

    limpa_lcd ();
    lcd_cmd (PRIMEIRA_LINHA);
    envia_string ("CONFIRMANDO...");
    int tamanho_payload = sprintf(pacote_dados, "%s%s%d", cartao_digitado, senha_cartao_digitado, preco);
    //Formato é 'P''P''N_BYTES''NUMERO_CARTAO''SENHA_CARTAO''VALOR'
    UART_envia_string("\n\n");
    UART_transmit ('P');
    UART_transmit ('P');
    UART_transmit((char)tamanho_payload);
    UART_envia_string(pacote_dados);
    estado_sistema = AGUARDA_P; //Estado onde está aguardando o 'S''P' do servidor externo
}

ISR (TIMER1_COMPA_vect){
    if (estado_sistema == DIGITANDO_PLACA || estado_sistema == SELECAO_TEMPO || estado_sistema == MENU_PAGAMENTO|| //depois penso em um jeito mais inteligente de fazer isso kkkkkkkkkkkkkkkkkk bagulho feio
        estado_sistema == CARTAO || estado_sistema == CARTAO_SENHA || estado_sistema == QUER_COMPROVANTE){ 
        
        confirmar_tecla = 1;
        TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); //desliga o timer
    }
    else if (estado_sistema == PLACA_INVALIDA || estado_sistema == CARTAO_INVALIDO || estado_sistema == SENHA_INVALIDA||estado_sistema == ERRO_SALDO_CARTAO
            || estado_sistema == ERRO_DADOS_CARTAO || estado_sistema == SUCESSO_CART_VIRT){
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

    toggle_led();
    contador_segundos++;
    contador_inatividade++; //conta o tempo que o display ta inativo
    flag_atualiza_relogio = 1; //avisa que passou um segundo

    if (contador_segundos >= 60) {
        contador_segundos = 0;
        prox_minuto();
    }
}



int main(void) {
    
    signed char tempo_escolhido = -1;
    int preco_a_pagar = 0;
    char eh_cartao = 0;
    char msg_valor_pagamento[20];
    
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
    
    init_led();    

    lcd_init(); //inicia lcd

    lcd_cmd(PRIMEIRA_LINHA); //posiciona cursor na primeira linha
    envia_string("DIGITE PLACA");
    lcd_cmd(SEGUNDA_LINHA); //cursor na segunda linha

    init_teclado();

    init_gerenciador_msgs();

    while(1) {
        
        if (contador_inatividade >=30 && sistema_inativo == 0){ //30 segundos sem ninguem tocar no teclado
            limpa_lcd();
            estado_sistema = DIGITANDO_PLACA; //Volta para o estado inicial de digitar a placa
            indice_placa = 0;
            reset_memoria_teclado ();
            sistema_inativo = 1;
            flag_atualiza_relogio = 1;
        }


        if (estado_sistema == DIGITANDO_PLACA){ 
            
            if (sistema_inativo == 1 && flag_atualiza_relogio == 1){ //Se estiver inativa, exibe o relogio
                flag_atualiza_relogio = 0;
                char msg_hora[17];
                sprintf(msg_hora, " %02d/%02d  %02d:%02d ", dia, mes, hora, min);
                lcd_cmd(0x80);
                envia_string(msg_hora);
                lcd_cmd(0xC0);
                envia_string("PRESSIONE TECLA");
            }

            
            roda_teclado();
            
            if (nova_tecla == TECLADO_PRESSIONADO){
                contador_inatividade = 0; //qnd alguma tecla for pressionada, o contador de inatividade
                if (sistema_inativo == 1){
                    sistema_inativo = 0;
                    nova_tecla = TECLADO_LIVRE; //descarta a primeira tecla pressionada (ela é usada só para tirar o sistema da inatividade)
                    limpa_lcd();
                    lcd_cmd(0x80);
                    envia_string("DIGITE PLACA");
                    lcd_cmd(0xC0);
                }
            
                else if (saida_teclado == '*'){ // '*' apaga
                    if (indice_placa > 0){
                    //UART_transmit (saida_teclado);
                    apaga_caractere_lcd();
                    }
                }
                else if (saida_teclado == '#'){ // precisa apertar '#' para confirmar
                    if(indice_placa == 7){
                    placa_digitada [7] = '\0';
                    verifica_placa();
                    //UART_transmit (saida_teclado);
                    }
                }
                else {
                    registra_caractere_confirmado(saida_teclado);
                    //UART_transmit(saida_teclado);
                    
                }
                nova_tecla = TECLADO_LIVRE;
            }
        }

        else if (estado_sistema == PLACA_INVALIDA && resetar_tela == 1){ //Se a placa é invalida e ja pode resetar a tela
            resetar_tela = 0;
            limpa_lcd();
            lcd_cmd(PRIMEIRA_LINHA); //posiciona cursor na primeira linha
            envia_string("DIGITE PLACA");
            lcd_cmd(SEGUNDA_LINHA); //cursor na segunda linha
            indice_placa = 0;
            reset_memoria_teclado();
            estado_sistema = DIGITANDO_PLACA; //Volta para o estado inicial de dgitar a placa
        }

        else if (estado_sistema == SELECAO_RUA) {
            roda_teclado_numerico();
            if (nova_tecla == TECLADO_PRESSIONADO){
                if (saida_teclado == '*'){ // '*' volta para o inicio
                    resetar_tela = 0;
                    limpa_lcd();
                    lcd_cmd(PRIMEIRA_LINHA); //posiciona cursor na primeira linha
                    envia_string("DIGITE PLACA");
                    lcd_cmd(SEGUNDA_LINHA); //cursor na segunda linha
                    indice_placa = 0;
                    reset_memoria_teclado();
                    estado_sistema = DIGITANDO_PLACA; //Volta para o estado inicial de dgitar a placa
                } else if ( (saida_teclado >= '0') && (saida_teclado <= '2')) {
                    rua_escolhida = saida_teclado;
                    limpa_lcd();
                    lcd_cmd(PRIMEIRA_LINHA); 
                    if (flag_vaga_especial){
                        envia_string("1)30min(R$0)");
                        lcd_cmd(SEGUNDA_LINHA);
                        envia_string("2)1h 3)2h 4)>2h");
                    }else{
                        envia_string("TEMPO: 1)30min");
                        lcd_cmd(SEGUNDA_LINHA);
                        envia_string("2)1h 3)1h30 4)2h");
                    }
                    estado_sistema = SELECAO_TEMPO;
                    UART_transmit(saida_teclado);
                    reset_memoria_teclado();
                }
                nova_tecla = TECLADO_LIVRE;
            }
        }

        else if (estado_sistema == SELECAO_TEMPO) {
            
            roda_teclado();
            if (nova_tecla == TECLADO_PRESSIONADO){
                //UART_transmit(saida_teclado);
                contador_inatividade = 0;
                tempo_escolhido = -1;
                preco_a_pagar = 0;

                if (saida_teclado == '1'){
                    if (flag_vaga_especial)preco_a_pagar = 0;
                    else preco_a_pagar = 200;
                    tempo_escolhido = t_30_min; //30min
                }
                else if (saida_teclado == 'A'){
                    if (flag_vaga_especial)preco_a_pagar = 200;
                    else preco_a_pagar = 350;
                    tempo_escolhido = t_1_hora; //30min a 1h
                }
                else if (saida_teclado == 'D'){
                    if (flag_vaga_especial){
                        preco_a_pagar = 350;
                        tempo_escolhido = t_2_hora; //1h e 30min a 2h

                    }else {
                        preco_a_pagar = 450;
                        tempo_escolhido = t_1h_30m; //1h a 1h e 30min
                    }
                }
                else if (saida_teclado =='G'){
                    if (flag_vaga_especial){
                        preco_a_pagar = 450;
                        tempo_escolhido = t_ilimitado; //ilimitado
                    }else {
                        preco_a_pagar = 600;
                        tempo_escolhido = t_2_hora; //1h e 30min a 2h
                    }
                }
                else if (saida_teclado == '*'){ //Se apertar para voltar
                    limpa_lcd();
                    lcd_cmd(PRIMEIRA_LINHA);
                    envia_string("0)OSV  1)SARM");
                    lcd_cmd(SEGUNDA_LINHA);
                    envia_string("2)JP");
                    reset_memoria_teclado();
                    estado_sistema = SELECAO_RUA;
                }
                

                if (tempo_escolhido>=0){
                    limpa_lcd ();
                    lcd_cmd(PRIMEIRA_LINHA);
                    
                    
                    if (preco_a_pagar != 0){
                        sprintf(msg_valor_pagamento, "VALOR: R$%d,%02d", preco_a_pagar / 100, preco_a_pagar % 100);
                        envia_string (msg_valor_pagamento);
                        lcd_cmd(SEGUNDA_LINHA);
                        envia_string ("1)MOD 2)CRT 3)VR");
                        reset_memoria_teclado();
                        estado_sistema = MENU_PAGAMENTO;
                    }else{
                        estado_sistema = REQ_DADOS_ESTAC; //Se está isento passa direto os dados do estacionamento para os serv externo
                    }
                    
                }
                nova_tecla = TECLADO_LIVRE;
            }
        }
        else if (estado_sistema == MENU_PAGAMENTO){
            roda_teclado();
            
            if (nova_tecla == TECLADO_PRESSIONADO){
                contador_inatividade = 0;
                eh_cartao = 0;
                //UART_transmit(saida_teclado);
                if (saida_teclado == '1'){
                    limpa_lcd();
                    //Lógica para as moedas
                }
                else if (saida_teclado == 'A'){//Cartao
                    eh_cartao = 1;
                    limpa_lcd();
                    lcd_cmd(PRIMEIRA_LINHA);
                    envia_string ("NUMERO CARTAO:");
                    lcd_cmd(SEGUNDA_LINHA);
                    indice_cartao = 0;
                    indice_cartao_senha=0;
                    reset_memoria_teclado();
                    estado_sistema = CARTAO;
                }
                else if (saida_teclado == 'D'){//Cartao virtual
                    verifica_cartao_virtual(preco_a_pagar);
                }
                else if (saida_teclado == '*'){//Volta para a tela anterior
                    limpa_lcd();
                    lcd_cmd(PRIMEIRA_LINHA);
                    if (flag_vaga_especial){
                        envia_string("1)30min(R$0)");
                        lcd_cmd(SEGUNDA_LINHA);
                        envia_string("2)1h 3)2h 4)>2h");
                    }else{
                        envia_string("TEMPO: 1)30min");
                        lcd_cmd(SEGUNDA_LINHA);
                        envia_string("2)1h 3)1h30 4)2h");
                    }
                    reset_memoria_teclado();
                    estado_sistema = SELECAO_TEMPO;
                }
                nova_tecla = TECLADO_LIVRE;
            }
        }
        else if (estado_sistema == CARTAO){
            roda_teclado_numerico();
            if(nova_tecla == TECLADO_PRESSIONADO){
                contador_inatividade = 0;
              
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
                    //UART_transmit(saida_teclado);
                    registra_caractere_confirmado(saida_teclado);
                }
                nova_tecla = TECLADO_LIVRE;
            }
            
            
        }
        else if (estado_sistema == CARTAO_INVALIDO && resetar_tela == 1){ //Se o cartao é invalido e ja pode resetar a tela
            resetar_tela = 0;
            limpa_lcd();
            lcd_cmd(PRIMEIRA_LINHA); //posiciona cursor na primeira linha
            envia_string("NUMERO CARTAO:");
            lcd_cmd(SEGUNDA_LINHA); //cursor na segunda linha
            indice_cartao = 0;
            reset_memoria_teclado();
            estado_sistema = CARTAO; //Volta para o estado inicial de dgitar o numero do cartao
        }
        else if (estado_sistema == CARTAO_SENHA){
            roda_teclado_numerico ();
            if (nova_tecla == TECLADO_PRESSIONADO){
                contador_inatividade = 0;
                
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
                    //UART_transmit(saida_teclado);
                    registra_caractere_confirmado(saida_teclado);
                }
                nova_tecla = TECLADO_LIVRE;
            }
        
        }
        else if (estado_sistema == SENHA_INVALIDA && resetar_tela == 1){
            resetar_tela = 0;
            limpa_lcd();
            lcd_cmd(PRIMEIRA_LINHA); //posiciona cursor na primeira linha
            envia_string("SENHA:");
            lcd_cmd(SEGUNDA_LINHA); //cursor na segunda linha
            indice_cartao_senha = 0;
            reset_memoria_teclado();
            estado_sistema = CARTAO_SENHA; //Volta para o estado inicial de dgitar a senha do cartao
        }

        else if (estado_sistema == REQ_DADOS_ESTAC){
            envia_dados_estacionamento(tempo_escolhido);    
        }
        else if (estado_sistema == REQ_PAGAMENTO){
            envia_dados_pagamento(preco_a_pagar);
        }


        //Se o sistema está em algum dos estados de erro do pagamento e ja passou o tempo da mensagem na tela:
        else if ((estado_sistema == ERRO_SALDO_CARTAO|| estado_sistema == ERRO_DADOS_CARTAO) && resetar_tela == 1){  
            resetar_tela = 0;
            limpa_lcd();
            lcd_cmd(PRIMEIRA_LINHA); 
            envia_string(msg_valor_pagamento);
            lcd_cmd(SEGUNDA_LINHA);
            envia_string("1)MOD 2)CRT 3)VR");//Volta para a tela da selecao do metodo de pagamento
            reset_memoria_teclado();
            estado_sistema = MENU_PAGAMENTO;
        }
        else if (estado_sistema == QUER_COMPROVANTE){
            roda_teclado();
            if (nova_tecla == TECLADO_PRESSIONADO){
                contador_inatividade = 0;
                if (saida_teclado == '*'){ //Caso nao queira comprovante, apenas volta para o estado inicial
                    limpa_lcd();
                    indice_placa = 0;
                    reset_memoria_teclado();
                    lcd_cmd(PRIMEIRA_LINHA); //posiciona cursor na primeira linha
                    envia_string("DIGITE PLACA");
                    lcd_cmd(SEGUNDA_LINHA); //cursor na segunda linha
                    estado_sistema = DIGITANDO_PLACA;
                }
                else if (saida_teclado == '#'){
                    //Dai fazer lógica para impressao do comprovante
                }
                

                nova_tecla = TECLADO_LIVRE;
            }
        }
        else if (estado_sistema == SUCESSO_CART_VIRT && resetar_tela == 1){
            resetar_tela = 0;
            estado_sistema = REQ_DADOS_ESTAC;
        }

        // novo_caracter_recebido_UART();
        gerenciar_msgs();

        if (novas_mensagens_servidor() > 0){
            msg = le_mensagem();

            if(estado_sistema == AGUARDA_E && msg.tipo == 'E'){ //Se o tipo da mensagem for 'E', quer dizer que recebeu os dados do estacionamento
                adiciona_carro(placa_digitada, flag_vaga_especial, tempo_escolhido, rua_escolhida);
                limpa_lcd();
                lcd_cmd (PRIMEIRA_LINHA);
                envia_string ("COMPROVANTE?");
                lcd_cmd (SEGUNDA_LINHA);
                envia_string ("*-NAO   #-SIM");
                estado_sistema = QUER_COMPROVANTE; 
            }
            else if (estado_sistema == AGUARDA_P && msg.tipo == 'P'){//Se o tipo da mensagem for P, servidor externo está enviando a resposta do pagamento
                
                if (msg.string[0] == 'S'){//Se a string for 'S', pagamento foi realizado com sucesso
                    limpa_lcd();
                    lcd_cmd (PRIMEIRA_LINHA);
                    envia_string ("COMPROVANTE?");
                    lcd_cmd (SEGUNDA_LINHA);
                    envia_string ("*-NAO   #-SIM");
                    estado_sistema = REQ_DADOS_ESTAC;
                }
                else if (msg.string[0] == 'F'){//Se a string for 'F', numero ou senha incorretos
                    limpa_lcd();
                    lcd_cmd (PRIMEIRA_LINHA);
                    envia_string ("DADOS ERRADOS");
                    estado_sistema = ERRO_DADOS_CARTAO;
                    TCNT1 = 0; //Inicia timer para ficar um tempo com a mensagem "Dados errados"
                    TCCR1B |= (1 << CS12);
                }
                else if (msg.string[0] == 'I'){//Se a string for 'I', saldo insuficiente
                    limpa_lcd();
                    lcd_cmd (PRIMEIRA_LINHA);
                    envia_string ("SEM SALDO");
                    estado_sistema = ERRO_SALDO_CARTAO;
                    TCNT1 = 0; 
                    TCCR1B |= (1 << CS12);
                }
            }
            else if (msg.tipo == 'H'){ // 
                ajustar_hora(msg.string); 
                UART_envia_string ("PH"); //mensagem de resposta para o servidor externo
            }
            UART_envia_string("\nMENSAGEM RECEBIDA");
        }


    }
    return 0;
}

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include "codificacao.h"

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


//Definição dos estados da maquina de estados
#define DIGITANDO_PLACA     0 // Estado onde aguarda a digitação da placa
#define SELECAO_TEMPO       1 // Tela da seleção da modalidade de tempo
#define PLACA_INVALIDA      2 // exibe erro de placa digitada invalida 
#define MENU_PAGAMENTO      3 // seleção de método de pagamento
#define MOEDA               4 // caso método de pagamento seja moedas
#define CARTAO              5 // caso método de pagamento seja cartao
#define CARTAO_SENHA        7 // valida senha do cartao
#define CARTAO_INVALIDO     8 // erro do formato do cartão (nao digitou apenas numeros)
#define SENHA_INVALIDA      9 // erro do formato da senha do cartao (nao digitou apenas numeros
#define REQ_DADOS_ESTAC    10 // envia os dados do estacionamento para o serv externo
#define AGUARDA_E          11 // aguarda 'E' do servidor externo
#define REQ_PAGAMENTO      12 // envia dados do pagamento via cartao para serv externo
#define AGUARDA_P          13 // aguarda mensagem tipo 'P' do servidor externo
#define ERRO_DADOS_CARTAO  14 // caso cartao tenha dados inválidos (senha ou numero)
#define ERRO_SALDO_CARTAO  15 // caso o cartao tenha saldo insuficiente
#define QUER_COMPROVANTE   16 // opcao de impressao de comprovante 
#define SUCESSO_CART_VIRT  17 // confirmação do pagamento por cartao virtual
#define ADM_SENHA          18 // valida senha do adm
#define ADM_AJUSTE_TEMPO   19 // estado alteracao do relogio pelo adm
#define ADM_MENU           20 // menu principal do adm
#define SELECAO_RUA        21 // selecao da rua estacionada
#define ADM_IRREG          22 // visualização dos carros irregulares pelo adm
#define FORA_SERVICO       23 // bloqueio do sistema fora do horario de funcionamento
#define RECARGA            24 // estado de extensão de tempo para carros ja estacionados

int estado_sistema = 0; 
int indice_placa = 0; 
int preco_pago = 0;
int indice_cartao = 0;
int indice_cartao_senha = 0;
int indice_senha_adm = 0;
int indice_ajuste_tempo = 0;
int id_carro = 0; //indice para operacoes de recarga

//entradas do usuario
char placa_digitada[8];
char cartao_digitado [7];
char senha_cartao_digitado [6];
char senha_adm_digitada [6];
char ajuste_tempo [13]; //12 digitos DDMMAAAAhhmm


int contador_inatividade = 0;
char flag_atualiza_relogio = 0;
char sistema_inativo = 1;
char rua_escolhida = -1;


int flag_vaga_especial = N_ESPECIAL;
int resetar_tela = 0; //flag para limpar do lcd mensagens temporarias
int tempo_erro = 0;


// dados cartoes virtuais cadastrados
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

// placas especiais
char lista_placas_especiais[5][8] = {
	"IOS0098",
	"PEI1Z92",
	"IDO0192",
	"IDS0090",
	"PDE3400" };

//exibe as opções de recarga baseado na modalidade contratada e impede que passe de 2 horas
void tempo_recarga(){
    if (carros[id_carro].tempo_contratado == t_30_min){
        limpa_lcd();
        lcd_cmd(PRIMEIRA_LINHA);
        if (flag_vaga_especial){
            envia_string("1)30min(R$0)");
            lcd_cmd(SEGUNDA_LINHA);
            envia_string("2)1h 3)2h 4)>2h");
        }else{
            envia_string("TEMPO: 1)30min");
            lcd_cmd(SEGUNDA_LINHA);
            envia_string("2)1h 3)1h30");
        }

    } else if (carros[id_carro].tempo_contratado == t_1_hora) {
        limpa_lcd();
        lcd_cmd(PRIMEIRA_LINHA);
        if (flag_vaga_especial){
            envia_string("1)30min(R$0)");
            lcd_cmd(SEGUNDA_LINHA);
            envia_string("2)1h 3)2h 4)>2h");
        }else{
            envia_string("TEMPO: 1)30min");
            lcd_cmd(SEGUNDA_LINHA);
            envia_string("2)1h");
        }

    } else if (carros[id_carro].tempo_contratado == t_1h_30m) {
        limpa_lcd();
        lcd_cmd(PRIMEIRA_LINHA);
        if (flag_vaga_especial){
            envia_string("1)30min(R$0)");
            lcd_cmd(SEGUNDA_LINHA);
            envia_string("2)1h 3)2h 4)>2h");
        }else{
            envia_string("TEMPO: 1)30min");
        }

    } else if (carros[id_carro].tempo_contratado == t_2_hora) {
        limpa_lcd();
        lcd_cmd(PRIMEIRA_LINHA);
        if (flag_vaga_especial){
            envia_string("1)30min(R$0)");
            lcd_cmd(SEGUNDA_LINHA);
            envia_string("2)1h 3)2h 4)>2h");
        }else{
            lcd_cmd(PRIMEIRA_LINHA);
            envia_string("LIMITE");
            lcd_cmd(SEGUNDA_LINHA);
            envia_string("ESGOTADO");
        }
    } else if (carros[id_carro].tempo_contratado == t_0){
        limpa_lcd();
        lcd_cmd(PRIMEIRA_LINHA); 
        if (flag_vaga_especial){ //caso seja placa especial
            envia_string("1)30min(R$0)");
            lcd_cmd(SEGUNDA_LINHA);
            envia_string("2)1h 3)2h 4)>2h");
        }else{
            envia_string("TEMPO: 1)30min");
            lcd_cmd(SEGUNDA_LINHA);
            envia_string("2)1h 3)1h30 4)2h");
        }
        estado_sistema = SELECAO_TEMPO;
    }
}

//armazena os caracteres digitados nos seus respectivos vetores, dependendo do estado do sistema. Exibe '*' no display para senhas digitadas
void registra_caractere_confirmado(char letra){
    
    if (estado_sistema == DIGITANDO_PLACA){ 
        if (indice_placa<7){ 
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
            lcd_data ('*'); //proteção da senha
            senha_cartao_digitado[indice_cartao_senha] = letra;
            indice_cartao_senha++;
        }
    }
    else if (estado_sistema == ADM_SENHA){
        if (indice_senha_adm < 5){
            lcd_data ('*');
            senha_adm_digitada [indice_senha_adm] = letra;
            indice_senha_adm++;
        }
    }
    else if (estado_sistema == ADM_AJUSTE_TEMPO){
        if (indice_ajuste_tempo < 12){
            lcd_data (letra);
            ajuste_tempo [indice_ajuste_tempo] = letra;
            indice_ajuste_tempo++;
        }
    }
}

//valida se a placa está no modelo mercosul ou antigo
// vai para o estado de adm caso seja digitado "ADMPARK" ao invés de uma placa
//analisa se a placa é de uma vaga especial
//encaminha para recarga (aumentar tempo contratado) caso o veiculo ja esteja estacionado
void verifica_placa(){
    
    reset_memoria_teclado();
    
    //Caso esteja tentando logar na conta de adm
    if (strcmp(placa_digitada, "ADMPARK")==0){
        limpa_lcd();
        lcd_cmd(PRIMEIRA_LINHA);
        envia_string("SENHA:");
        lcd_cmd (SEGUNDA_LINHA);
        estado_sistema = ADM_SENHA;
        indice_senha_adm = 0;
        return;
    }
    
    //placa inválida
    if ((placa_digitada[0]<'A'|| placa_digitada[0]>'Z')|| (placa_digitada[1]<'A'|| placa_digitada[1]>'Z')|| (placa_digitada[2]<'A'|| placa_digitada[2]>'Z')
        || (placa_digitada[3]<'0'|| placa_digitada[3]>'9')|| (placa_digitada[5]<'0'|| placa_digitada[5]>'9')|| (placa_digitada[6]<'0'|| placa_digitada[6]>'9') 
        ||  ((placa_digitada[4]<'0'|| placa_digitada[4]>'9')&&(placa_digitada[4]<'A'|| placa_digitada[4]>'Z')) ){
        limpa_lcd();    
        envia_string("PLACA INVALIDA");
        estado_sistema = PLACA_INVALIDA;
        
        //timer de exibicao de erro temporario
        TCNT1 = 0; 
        TCCR1B |= (1 << CS12);
    }else{

        flag_vaga_especial = N_ESPECIAL;
        for (int i = 0; i < 5; i++)
        {
            if (strcmp(placa_digitada, lista_placas_especiais[i]) == 0){ //caso a placa digitada seja uma das cadastradas como especial
                flag_vaga_especial = ESPECIAL; //ativa a flag de vaga especial
                break;
            }
        }
        id_carro = -1;
        for(int i =0; i < num_carros;i++){
            //caso a placa digitada ja esteja estacionada, vai para a recarga (aumento do tempo)
            if (strcmp(carros[i].placa, placa_digitada) == 0 ) {
                id_carro = i;
                tempo_recarga();
            
                estado_sistema = RECARGA;
    
                break;
            }

        } 
        //caso seja um novo veiculo
        if(estado_sistema != RECARGA ){
            limpa_lcd();
            lcd_cmd(PRIMEIRA_LINHA);
            envia_string("0)OSV  1)SARM");
            lcd_cmd(SEGUNDA_LINHA);
            envia_string("2)JaoP.");
            estado_sistema = SELECAO_RUA;
        }
           
    
    }
}

//Verifica se a placa digitada está cadastrada como cartao virtual e realiza pagamento
void verifica_cartao_virtual(int preco){
    int i;
    char placa_encontrada = 0;
    int indice_virtual = -1;
    
    //Verifica se a placa digitada está cadastrada como cartao virtual
    for (i=0;i<5;i++){
        if(strcmp(placa_digitada, lista_cartao_virtual[i])==0){
            placa_encontrada = 1;
            indice_virtual = i;
            break;
        }
    }

    limpa_lcd();
    //caso a placa esteja cadastrada como cartao virtual
    if (placa_encontrada){
        if(saldo_cartao_virtual [indice_virtual]>= preco){ //se o saldo disponível no cartao foir >= ao preço que está pagando
            saldo_cartao_virtual [indice_virtual] -= preco;
            lcd_cmd (PRIMEIRA_LINHA);
            envia_string("PAGAMENTO OK");
            lcd_cmd (SEGUNDA_LINHA);
            char msg_saldo[16];
            fflush(stdin);
            sprintf(msg_saldo, "SALDO:R$%d,%02d", saldo_cartao_virtual[indice_virtual]/100, saldo_cartao_virtual[indice_virtual]%100); //confirma pagamento e exibe saldo 
            envia_string(msg_saldo);
            estado_sistema = SUCESSO_CART_VIRT;
            //timer para exibir a mensagem temporariamente no display
            TCNT1 = 0; 
            TCCR1B |= (1 << CS12);
            
        }else{ //se o saldo disponível no cartao foir < ao preço que está pagando 
            lcd_cmd (PRIMEIRA_LINHA);
            envia_string("SEM CREDITO");
            estado_sistema = ERRO_SALDO_CARTAO;
            //timer para exibir a mensagem temporariamente
            TCNT1 = 0; 
            TCCR1B |= (1 << CS12);

        }
        
    }else { //Caso a placa nao esteja cadastrada como cartão virtual
        lcd_cmd (PRIMEIRA_LINHA);
        envia_string("NAO REGISTRADO");
        estado_sistema = ERRO_DADOS_CARTAO;
        //timer para exibir a mensagem temporariamente
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
    }else{ //caso cartao seja apenas numeros, vai para estado de digitar a senha
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
    if (flag_senha_invalida){ //senha inválida no caso nao digitou apenas numeros
        limpa_lcd ();
        lcd_cmd (PRIMEIRA_LINHA);
        envia_string ("SENHA INVALIDA");
        estado_sistema = SENHA_INVALIDA;
        TCNT1 = 0; 
        TCCR1B |= (1 << CS12);
    }else{ // caso a senha seja apenas numeros vai para o estado de enviar os dados de pagamento
        limpa_lcd ();
        lcd_cmd (PRIMEIRA_LINHA);
        envia_string ("OK");
        // estado_sistema = REQ_DADOS_ESTAC;
        estado_sistema = REQ_PAGAMENTO;
    }
}
// autenticação da senha do adm
void verifica_senha_adm (){
    if (strcmp(senha_adm_digitada, "12345") ==0){
        limpa_lcd();
        lcd_cmd (PRIMEIRA_LINHA);
        envia_string ("1)REL 2)IRREG");
        estado_sistema = ADM_MENU;
    }else{
        limpa_lcd();
        lcd_cmd (PRIMEIRA_LINHA);
        envia_string("SENHA INVALIDA");
        estado_sistema = PLACA_INVALIDA;
        TCNT1 = 0; 
        TCCR1B |= (1 << CS12);
    }
}

//validacao da data/hora enviada pelo adm (garante que o dia ou hora digitados existam)
void verifica_hora_adm (){ 
    const int dias_meses[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int dias_maximos = 31; 
    
    int d_novo = (ajuste_tempo[0]-'0')*10 + (ajuste_tempo[1]-'0');
    int m_novo = (ajuste_tempo[2]-'0')*10 + (ajuste_tempo[3]-'0');
    int a_novo = (ajuste_tempo[4]-'0')*1000 + (ajuste_tempo[5]-'0')*100 + (ajuste_tempo[6]-'0')*10 + (ajuste_tempo[7]-'0');
    int h_novo = (ajuste_tempo[8]-'0')*10 + (ajuste_tempo[9]-'0');
    int min_novo = (ajuste_tempo[10]-'0')*10 + (ajuste_tempo[11]-'0');


    if (m_novo >= 1 && m_novo <= 12) {
        //Ano bissexto
        if ((m_novo == 2) && ((a_novo % 4 == 0 && a_novo % 100 != 0) || (a_novo % 400 == 0))) {
            dias_maximos = 29;
        } else {
            dias_maximos = dias_meses[m_novo];
        }
    }
    if (m_novo>=1 && m_novo<=12 && d_novo>=1 && d_novo<=dias_maximos && h_novo>=0 && h_novo<=23 && min_novo>=0 && min_novo<=59){
        dia = d_novo; mes = m_novo; ano= a_novo; hora = h_novo; min = min_novo; 
        ano1 = a_novo / 100; ano2 = a_novo % 100;
        
        limpa_lcd();
        lcd_cmd(PRIMEIRA_LINHA);
        envia_string("ATUALIZADO");
        estado_sistema = PLACA_INVALIDA; // aproveita o estado placa invalida para exibir a mensagem e retornar ao estado inicial
        TCNT1 = 0; TCCR1B |= (1 << CS12);
    }else{
        limpa_lcd();
        lcd_cmd(PRIMEIRA_LINHA);
        envia_string("DATA INVALIDA");
        estado_sistema = PLACA_INVALIDA; 
        TCNT1 = 0; TCCR1B |= (1 << CS12);
    }

}

//funcao para apagar o ultimo caractere digitado

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
    else if (estado_sistema == ADM_SENHA){
        indice_senha_adm--;
        lcd_cmd (SEGUNDA_LINHA + indice_senha_adm);
        lcd_data (' ');
        lcd_cmd (SEGUNDA_LINHA + indice_senha_adm);
        senha_adm_digitada[indice_senha_adm] = '\0';
    }
    else if (estado_sistema == ADM_AJUSTE_TEMPO){
        indice_ajuste_tempo--;
        lcd_cmd (SEGUNDA_LINHA + indice_ajuste_tempo);
        lcd_data (' ');
        lcd_cmd (SEGUNDA_LINHA + indice_ajuste_tempo);
        ajuste_tempo[indice_ajuste_tempo] = '\0';
    }
    
}

//formata e envia o pacote de dados do estacionamento para o servidor externo
void envia_dados_estacionamento(char tempo){
    char pacote_estacionamento[30];
    
    //Formato é 'P''E''PLACA_DO_CARRO''MODALIDADE'
    //fflush(stdin);
    sprintf(pacote_estacionamento,"%s%d", placa_digitada,tempo); 
    UART_transmit('P');
    UART_transmit('E');
    UART_envia_string(pacote_estacionamento);
    //UART_envia_string("\n\n");
    limpa_lcd ();
    lcd_cmd (PRIMEIRA_LINHA);
    envia_string ("REGISTRANDO...");
        
    estado_sistema = AGUARDA_E; //Estado onde está aguardando o 'S''E' do servidor externo
        
}
//formata e envia pacote de dados de pagamento para pagamento via cartao
void envia_dados_pagamento(int preco){
    char pacote_dados [15];

    limpa_lcd ();
    lcd_cmd (PRIMEIRA_LINHA);
    envia_string ("CONFIRMANDO...");
    fflush(stdin);
    int tamanho_payload = sprintf(pacote_dados, "%s%s%d", cartao_digitado, senha_cartao_digitado, preco);
    //Formato é 'P''P''N_BYTES''NUMERO_CARTAO''SENHA_CARTAO''VALOR'
    //UART_envia_string("\n\n");
    UART_transmit ('P');
    UART_transmit ('P');
    UART_transmit((char)tamanho_payload);
    UART_envia_string(pacote_dados);
    //UART_envia_comando(pacote_dados);
    estado_sistema = AGUARDA_P; //Estado onde está aguardando o 'S''P' do servidor externo
}

//formata e envia o comprovante de pagamento
//gerencia o tempo limite para veiculos especiais, novos e recargas

void imprime_comprovante(signed char tempo_escolhido, int preco_a_pagar){
    char payload_comprovante[85];
    int tamanho = 0;
    if (tempo_escolhido == t_ilimitado){
        fflush(stdin);
        tamanho = sprintf(payload_comprovante, 
            "\r\nPARKMICROS\r\nVALORRS%02d\r\nPLACA%s\r\nVALIDADEILIMITADO\r\n", 
            preco_a_pagar, 
            placa_digitada);
    }

    else if (id_carro == -1) { //caso seja veiculo novo 
        int tempo_minutos = 0;
        if (tempo_escolhido == t_30_min) tempo_minutos = 30;
        else if (tempo_escolhido == t_1_hora) tempo_minutos = 60;
        else if (tempo_escolhido == t_1h_30m) tempo_minutos = 90;
        else if (tempo_escolhido == t_2_hora) tempo_minutos = 120;
        
        int min_totais = min + tempo_minutos;
        int min_vencimento = min_totais % 60;
        int hora_vencimento = (hora + (min_totais/60)) %24;

        fflush(stdin);
        tamanho = sprintf(payload_comprovante, 
        "\r\nPARKMICROS\r\nVALORRS%02d\r\nPLACA%s\r\nVALIDADE%02d%02d\r\n", 
        preco_a_pagar , 
        placa_digitada, 
        hora_vencimento, min_vencimento);
    } else { // Caso seja recarga
        int tempo_minutos = 0;
        if (tempo_escolhido == t_30_min) tempo_minutos = 30;
        else if (tempo_escolhido == t_1_hora) tempo_minutos = 60;
        else if (tempo_escolhido == t_1h_30m) tempo_minutos = 90;
        else if (tempo_escolhido == t_2_hora) tempo_minutos = 120;
        
        int min_totais = carros[id_carro].min_entrada + tempo_minutos;
        int min_vencimento = min_totais % 60;
        int hora_vencimento = (carros[id_carro].hora_entrada + (min_totais/60)) %24;

        fflush(stdin);
        tamanho = sprintf(payload_comprovante, 
        "\r\nPARKMICROS\r\nVALORRS%02d\r\nPLACA%s\r\nVALIDADE%02d%02d\r\n", 
        preco_a_pagar , 
        placa_digitada, 
        hora_vencimento, min_vencimento);
        carros[id_carro].tempo_contratado = tempo_escolhido;
    }
   
    //envio do comprovante: 'P''I''ByteAlto''ByteBaixo''\r\n''payload'
    UART_transmit('P');
    UART_transmit('I');
    UART_transmit((char)(tamanho >> 8));   //byte mais significativo
    UART_transmit((char)(tamanho & 0xFF)); //menos significativo 
    //UART_envia_comando("\r\n");
    UART_envia_string(payload_comprovante);
    
    //volta ao estado inicial de digitar placa
    limpa_lcd();
    indice_placa = 0;
    reset_memoria_teclado();
    lcd_cmd(PRIMEIRA_LINHA); 
    envia_string("DIGITE PLACA");
    lcd_cmd(SEGUNDA_LINHA); 
    estado_sistema = DIGITANDO_PLACA;
    
}

//Timers

ISR (TIMER1_COMPA_vect){
    
    //confirma tecla pressionada após passar o tempo (500ms)
    if (estado_sistema == DIGITANDO_PLACA || estado_sistema == SELECAO_TEMPO || estado_sistema == MENU_PAGAMENTO|| 
        estado_sistema == CARTAO || estado_sistema == CARTAO_SENHA || estado_sistema == QUER_COMPROVANTE || estado_sistema == ADM_MENU ||estado_sistema == ADM_IRREG ||estado_sistema == RECARGA ){ 
        
        confirmar_tecla = 1;
        TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); //desliga o timer
    }

    // nos estados de mensagens temporarias na tela, conta 4 ciclos (4*500ms = 2s) para resetar a tela
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

    contador_segundos++;
    contador_inatividade++; //conta o tempo que o display ta inativo
    flag_atualiza_relogio = 1; //avisa que passou um segundo
    
    // ativa led caso carros irregulares
    if ((num_carros_irregulares > 0 || num_carros_parasitas > 0) && eh_horario_comercial()){
        toggle_led();
    }


    if (contador_segundos >= 120) {
        contador_segundos = 0;
        prox_minuto();
    }
}
int main(void) {
    
    signed char tempo_escolhido = -1;
    int preco_a_pagar = 0;
    char msg_valor_pagamento[20];
    char msg_valor_pago[20];
    
    Mensagem msg;

    PINH = 0xFF;
    
    UART_init(); //incia serial

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

    // 0.5 SEGUNDO
    OCR3A = 7812; 

    // Habilita o modo CTC + Prescaler para 1024
    TCCR3B |= (1 << WGM32) | (1 << CS32) | (1 << CS30); 
    
    // Habilita a interrupção de comparação A do Timer3 (OCIE3A)
    TIMSK3 |= (1 << OCIE3A);

    sei(); // Ativa interrupcao global
    
    init_led();    
    delay_1ms();
    delay_1ms();
    delay_1ms();
    delay_1ms();
    delay_1ms();
    lcd_init(); //inicia lcd

    lcd_cmd(PRIMEIRA_LINHA); //posiciona cursor na primeira linha
    envia_string("DIGITE PLACA");
    lcd_cmd(SEGUNDA_LINHA); //cursor na segunda linha
    delay_1ms();
    delay_1ms();
    delay_1ms();
    delay_1ms();
    delay_1ms();

    init_teclado();

    init_gerenciador_msgs();

    while(1) {
        
        if (contador_inatividade >=60 && sistema_inativo == 0){ //30 segundos sem ninguem tocar no teclado
            if (estado_sistema != FORA_SERVICO){
                limpa_lcd();
                estado_sistema = DIGITANDO_PLACA; //Volta para o estado inicial de digitar a placa
                indice_placa = 0;
                reset_memoria_teclado ();
                sistema_inativo = 1;
                flag_atualiza_relogio = 1;
            }
        }


        if (estado_sistema == DIGITANDO_PLACA){ 
            
            if (sistema_inativo == 1 && flag_atualiza_relogio == 1){ //Se estiver inativa, exibe o relogio
                flag_atualiza_relogio = 0;
                char msg_hora[22];
                fflush(stdin);
                sprintf(msg_hora, "%02d/%02d/%04d %02d:%02d", dia, mes, ano, hora, min);
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
                    //char c[50];
                    //sprintf(c,"%d", chave);
                    //envia_string(c);
                    lcd_cmd(0xC0);
                }
            
                else if (saida_teclado == '*'){ // '*' apaga
                    if (indice_placa > 0){
                    apaga_caractere_lcd();
                    }
                }
                else if (saida_teclado == '#'){ // precisa apertar '#' para confirmar
                    if(indice_placa == 7){ //so confirma se foram digitados os 7 digitos da placa
                    placa_digitada [7] = '\0';
                    verifica_placa();
                    }
                }
                else {
                    registra_caractere_confirmado(saida_teclado);
                    
                }
                nova_tecla = TECLADO_LIVRE;
            }

            //Caso esteja fora do horario de funcionamento
            if (!eh_horario_comercial()){
                limpa_lcd();
                lcd_cmd(PRIMEIRA_LINHA);
                envia_string("DESLIGADO");
                estado_sistema = FORA_SERVICO;
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

        //Caso esteja no estado do adm digitar senha
        else if (estado_sistema == ADM_SENHA){
            roda_teclado_numerico();
            if (nova_tecla == TECLADO_PRESSIONADO){
                contador_inatividade = 0;
                if (saida_teclado == '*'){
                    if (indice_senha_adm > 0){
                        apaga_caractere_lcd(); //Se ja digitou alguma coisa na senha, apaga caractere
                    }else { //caso nao tenha digitatado nada, volta para o estado inicial de digitar placa
                        limpa_lcd();
                        lcd_cmd(PRIMEIRA_LINHA);
                        envia_string("DIGITE PLACA");
                        lcd_cmd(SEGUNDA_LINHA);
                        indice_placa = 0;
                        estado_sistema = DIGITANDO_PLACA;
                    }
                }

                else if (saida_teclado == '#'){
                    if (indice_senha_adm == 5){
                        senha_adm_digitada [5] = '\0';
                        verifica_senha_adm();
                    }
                }
                else {
                    registra_caractere_confirmado(saida_teclado);
                }
                nova_tecla = TECLADO_LIVRE;

            }
        }
        //estado do menu do adm
        else if (estado_sistema == ADM_MENU){
            roda_teclado();
            if (nova_tecla == TECLADO_PRESSIONADO){
                contador_inatividade = 0;
                if (saida_teclado == '1'){ //se pressionar a primeira tecla vai para o estado de ajustar tempo
                    limpa_lcd();
                    lcd_cmd (PRIMEIRA_LINHA);
                    envia_string("DDMMAAAAhhmm:");
                    lcd_cmd (SEGUNDA_LINHA);
                    estado_sistema = ADM_AJUSTE_TEMPO;
                    indice_ajuste_tempo = 0;
                }
                else if (saida_teclado == 'A'){ //se pressionar segunda tecla vai para o estado de verificar irregulares
                    indice_irreg = 0;
                    estado_sistema = ADM_IRREG;
                    mostrar_irreg (indice_irreg);
                }
                else if (saida_teclado == '*'){ //Sai do modo admin
                    limpa_lcd ();
                    lcd_cmd(PRIMEIRA_LINHA); //posiciona cursor na primeira linha
                    envia_string("DIGITE PLACA");
                    lcd_cmd(SEGUNDA_LINHA); //cursor na segunda linha
                    indice_placa = 0;
                    reset_memoria_teclado();
                    estado_sistema = DIGITANDO_PLACA; //volta pra o estado inicial
                } 
                nova_tecla = TECLADO_LIVRE;
            }       
        }
        
        //caso o adm tenha selecionado para ajustar data/hora
        else if (estado_sistema == ADM_AJUSTE_TEMPO){
            roda_teclado_numerico();
            if (nova_tecla == TECLADO_PRESSIONADO){
                contador_inatividade = 0;
                if (saida_teclado == '*'){
                    if (indice_ajuste_tempo > 0){
                        apaga_caractere_lcd();
                    }else {
                        limpa_lcd();
                        lcd_cmd(PRIMEIRA_LINHA);
                        envia_string("1)REL 2)IRREG");
                        estado_sistema = ADM_MENU;
                    }
                }
                else if (saida_teclado =='#'){
                    if (indice_ajuste_tempo == 12){
                        ajuste_tempo[12] = '\0';
                        verifica_hora_adm();
                    }
                    
                }
                else{
                    registra_caractere_confirmado(saida_teclado);
                }
                nova_tecla = TECLADO_LIVRE;
            }
        }
        //caso adm tenha selecionado para verificar irregulares
        else if (estado_sistema == ADM_IRREG){
            roda_teclado();
            if (nova_tecla == TECLADO_PRESSIONADO){
                
                if (saida_teclado == '#'){ //Mostrar mais irregulares se tiver
                    if (indice_irreg != -1){
                        mostrar_irreg(indice_irreg +1);
                    }
                }
                else if (saida_teclado == '*'){
                    limpa_lcd ();
                    lcd_cmd (PRIMEIRA_LINHA);
                    envia_string ("1)REL 2)IRREG");
                    estado_sistema = ADM_MENU;
                }
                
                nova_tecla = TECLADO_LIVRE;
            }
        }
        //selecao da rua
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
                    if (flag_vaga_especial){ //caso seja placa especial
                        envia_string("1)30min(R$0)");
                        lcd_cmd(SEGUNDA_LINHA);
                        envia_string("2)1h 3)2h 4)>2h");
                    }else{
                        envia_string("TEMPO: 1)30min");
                        lcd_cmd(SEGUNDA_LINHA);
                        envia_string("2)1h 3)1h30 4)2h");
                    }
                    estado_sistema = SELECAO_TEMPO;
                    
                    reset_memoria_teclado();
                }
                nova_tecla = TECLADO_LIVRE;
            }
        }

        else if (estado_sistema == RECARGA) {
            roda_teclado();
            if (nova_tecla == TECLADO_PRESSIONADO){
                contador_inatividade = 0;
                tempo_escolhido = -1;
                preco_a_pagar = 0;
                
                //lógica para extensão de horas na recarga
                if (carros[id_carro].tempo_contratado == t_30_min){
                    if (saida_teclado == '1'){
                        if (flag_vaga_especial)preco_a_pagar = 0;
                        else preco_a_pagar = 200;
                        tempo_escolhido = t_1_hora; 
                    }
                    else if (saida_teclado == 'A'){
                        if (flag_vaga_especial)preco_a_pagar = 200;
                        else preco_a_pagar = 350;
                        tempo_escolhido = t_1h_30m; 
                    }
                    else if (saida_teclado == 'D'){
                    if (!flag_vaga_especial){
                        preco_a_pagar = 450;
                        tempo_escolhido = t_2_hora; 
                    }
                }
                    else if (saida_teclado =='G'){
                        if (flag_vaga_especial){   
                            preco_a_pagar = 450;
                            tempo_escolhido = t_ilimitado; 
                        }
                    }
                    

                } else if (carros[id_carro].tempo_contratado == t_1_hora) {
                    if (saida_teclado == '1'){
                        if (flag_vaga_especial)preco_a_pagar = 0;
                        else preco_a_pagar = 200;
                        tempo_escolhido = t_1h_30m; 
                    }
                    else if (saida_teclado == 'A'){
                        if (flag_vaga_especial)preco_a_pagar = 200;
                        else preco_a_pagar = 350;
                        tempo_escolhido = t_2_hora; 
                    }
                    else if (saida_teclado =='G'){
                        if (flag_vaga_especial){   
                            preco_a_pagar = 450;
                            tempo_escolhido = t_ilimitado; 
                        }
                    }
                    
                } else if (carros[id_carro].tempo_contratado == t_1h_30m) {
                    if (saida_teclado == '1'){
                        if (flag_vaga_especial)preco_a_pagar = 0;
                        else preco_a_pagar = 200;
                        tempo_escolhido = t_2_hora; 
                    }
                    else if (saida_teclado =='G'){
                        if (flag_vaga_especial){   
                            preco_a_pagar = 450;
                            tempo_escolhido = t_ilimitado; 
                        }
                    }

                } else if (carros[id_carro].tempo_contratado == t_2_hora) {
                    
                } else if (carros[id_carro].tempo_contratado == t_0){
                    //logica de tarifação para carros novos
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
    
                }

                if (tempo_escolhido>=0){
                    limpa_lcd ();
                    lcd_cmd(PRIMEIRA_LINHA);
                    
                    
                    if (preco_a_pagar != 0){
                        fflush(stdin);
                        sprintf(msg_valor_pagamento, "VALOR: R$%d,%02d", preco_a_pagar / 100, preco_a_pagar % 100);
                        envia_string (msg_valor_pagamento); //envia para o display a mensagem do valor a ser pago
                        lcd_cmd(SEGUNDA_LINHA);
                        envia_string ("1)MOD 2)CRT 3)VR"); //opcoes de pagamento 1) moeda, 2) cartao, 3) cartao virtual
                        reset_memoria_teclado();
                        estado_sistema = MENU_PAGAMENTO;
                    }else{
                        estado_sistema = REQ_DADOS_ESTAC; //Se está isento passa direto os dados do estacionamento para os serv externo
                    }   
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

                //logica de tarifação para carros novos
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
                        fflush(stdin);
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
        
        //processamento de pagamentos
        else if (estado_sistema == MENU_PAGAMENTO){
            roda_teclado();
            
            if (nova_tecla == TECLADO_PRESSIONADO){
                contador_inatividade = 0;
                //UART_transmit(saida_teclado);
                if (saida_teclado == '1'){ //opcao de pagar em moedas
                    limpa_lcd();
                    lcd_cmd(PRIMEIRA_LINHA);
                    envia_string ("VALOR DEPOSITADO:");
                    lcd_cmd(SEGUNDA_LINHA);
                    envia_string ("R$0,0");
                    reset_memoria_teclado();
                    estado_sistema = MOEDA;
                }
                else if (saida_teclado == 'A'){//Cartao
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
        
        //caso tenha sido selecionado moeda
        else if (estado_sistema == MOEDA){
            roda_teclado_numerico();
            if(nova_tecla == TECLADO_PRESSIONADO){
                contador_inatividade = 0;
                
                //sao utilizados os dois botões à direita 'A' e 'B' do teclado 4x4
                if (saida_teclado == 'A'){//Caso seja pressionado A, moeda de 1 real
                    preco_pago += 100;
                }
                if (saida_teclado == 'B'){ //pressionado B, moeda de 50 centavos
                    preco_pago += 50;
                }
                limpa_lcd();
                lcd_cmd(PRIMEIRA_LINHA);
                envia_string ("VALOR DEPOSITADO:");
                lcd_cmd(SEGUNDA_LINHA);
                fflush(stdin);
                sprintf(msg_valor_pago, "R$%d,%02d", preco_pago / 100, preco_pago % 100);
                envia_string (msg_valor_pago);

                nova_tecla = TECLADO_LIVRE;
            }
            if(preco_a_pagar <= preco_pago){ //caso tenha sido depositado o preco necessario
                preco_pago = 0;

                estado_sistema = REQ_DADOS_ESTAC; //envio de dados estacionamento
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
                    registra_caractere_confirmado(saida_teclado);
                }
                nova_tecla = TECLADO_LIVRE;
            }
            
            
        }

        else if (estado_sistema == CARTAO_INVALIDO && resetar_tela == 1){ //Se o cartao é invalido(nao foi digitado so numeros) e ja pode resetar a tela
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
            envia_dados_estacionamento(tempo_escolhido); //envia os dados do estacionamento para serv externo    
        }
        else if (estado_sistema == REQ_PAGAMENTO){
            envia_dados_pagamento(preco_a_pagar); //envia dados do pagamento via cartao para serv externo
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
                else if (saida_teclado == '#'){ //Caso queira comprovante
                    imprime_comprovante(tempo_escolhido,preco_a_pagar);
                }
                

                nova_tecla = TECLADO_LIVRE;
            }
        }
        //se ocorreu o pagamento via cartao virtual e ja pode resetar a tela
        else if (estado_sistema == SUCESSO_CART_VIRT && resetar_tela == 1){
            resetar_tela = 0;
            estado_sistema = REQ_DADOS_ESTAC;
        } 
        //se estava fora do tempo de servico e chegou o horario de abrir
        else if (estado_sistema == FORA_SERVICO ){
            if (eh_horario_comercial()){
                limpa_lcd();
                indice_placa = 0;
                reset_memoria_teclado();
                lcd_cmd(PRIMEIRA_LINHA); 
                envia_string("DIGITE PLACA");
                lcd_cmd(SEGUNDA_LINHA); 
                estado_sistema = DIGITANDO_PLACA;
            }
        }

        // novo_caracter_recebido_UART();
        gerenciar_msgs();

        if (novas_mensagens_servidor() > 0){
            msg = le_mensagem();

            if(estado_sistema == AGUARDA_E && msg.tipo == MSG_DADOS_ESTACIONAMENTO){ //Se o tipo da mensagem for 'E', quer dizer que recebeu os dados do estacionamento
                if (id_carro == -1){
                    //Carro novo: cria a entrada (hora de entrada = agora)
                    adiciona_carro(placa_digitada, flag_vaga_especial, tempo_escolhido, rua_escolhida, REGULAR);
                } else {
                    //Recarga: nao adiciona de novo (evita duplicar na lista), so atualiza tempo.
                    carros[id_carro].tempo_contratado = tempo_escolhido;
                    carros[id_carro].esta_regular = REGULAR;
                }

                limpa_lcd();
                lcd_cmd (PRIMEIRA_LINHA);
                envia_string ("COMPROVANTE?");
                lcd_cmd (SEGUNDA_LINHA);
                envia_string ("*-NAO   #-SIM");
                estado_sistema = QUER_COMPROVANTE; 
            }
            
            else if (estado_sistema == AGUARDA_P && msg.tipo == MSG_PAGAMENTO){//Se o tipo da mensagem for P, servidor externo está enviando a resposta do pagamento
                
                if (msg.string[0] == 'S'){//Se a string for 'S', pagamento foi realizado com sucesso
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
            else if (msg.tipo == MSG_HORARIO){ // 
                ajustar_hora(msg.string); 
                UART_envia_comando ("PH"); //mensagem de resposta para o servidor externo
            }

            else if(msg.tipo == MSG_CONDICAO_VIA){
                UART_envia_comando("PV"); //mensagem de resposta para o servidor externo
                verifica_carros_presentes(msg.string, (msg.tamanho - 7)/7);
                verifica_carros_parasitas(msg.string, (msg.tamanho - 7)/7);
            }

            else if(msg.tipo == MSG_CARTAO_INSERIDO){
                UART_envia_comando("PC"); //mensagem de resposta para o servidor externo

                //se está no estado de pagamento por cartao e o cartão é inserido, cartao_digitado[] pega os valores do numero do cartao enviados pelo servidor externo
                if (estado_sistema == CARTAO){
                    cartao_digitado[0] = msg.string[0];
                    cartao_digitado[1] = msg.string[1];
                    cartao_digitado[2] = msg.string[2];
                    cartao_digitado[3] = msg.string[3];
                    cartao_digitado[4] = msg.string[4];
                    cartao_digitado[5] = msg.string[5];
                    verifica_cartao();
                }
                
            }

            else if (msg.tipo == MSG_DEBUG){ // DEBUG
                printa_data();
                printa_lista_carros();
            }

            // UART_envia_string("\nMENSAGEM RECEBIDA\n");
            // UART_envia_string(msg.string);
        }


        gerenciar_parkimetro();

        //se nao está no horario comerical e nao tem carros irregulares, desliga led
        if (!eh_horario_comercial() || (num_carros_irregulares == 0 && num_carros_parasitas == 0)){
            desl_led();
        }
       
    }
    return 0;
}
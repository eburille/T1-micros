#include "hora.h"
#include "uart_lib.h"

volatile char dia = 21;
volatile char mes = 10;
volatile int ano = 2026;
volatile char hora = 10;
volatile char min = 30;

volatile unsigned char ano1 = 20;
volatile unsigned char ano2 = 26;

char eh_horario_comercial(){
    if (hora >= HORA_INICIO_TRABALHO && hora < HORA_FIM_TRABALHO){
        return 1;
    }
    return 0;
}

/// @brief Ajusta a hora baseado em uma string no formato DMAAHm
/// @param string String com as informações de data
void ajustar_hora(unsigned char string[6]){
    dia = string[0];
    mes = string[1];
    ano = (((int)string[2]) << 8) | ((int)string[3]);
    hora = string[4];
    min = string[5];

    ano1 = string[2];
    ano2 = string[3];
}

/// @brief Calcula a diferença de tempo em minutos entre a hora atual e a hora passada por parametro
/// @param dia_inicial 
/// @param mes_inicial 
/// @param ano_inicial 
/// @param hora_inicial 
/// @param min_inicial 
/// @return Se ja tiver se passado mais de 1 dia, retorna 500, caso contrario, retorna o tempo passado em minutos
int calc_delta_time_min(char dia_inicial, char mes_inicial, int ano_inicial, char hora_inicial, char min_inicial){    
    if (ano - ano_inicial){
        return 500;  // Como o maximo de tempo é 120min, o cálculo preciso não importa
    }

    if (mes - mes_inicial){
        return 500;  // Como o maximo de tempo é 120min, o cálculo preciso não importa
    }

    if (dia - dia_inicial){
        return 500;  // Como o maximo de tempo é 120min, o cálculo preciso não importa
    }

    int delta_min = (hora - hora_inicial) * 60 + (min - min_inicial);
    return delta_min;
}

/// @brief Retorna o numero de dias do mes atual
/// @return Numero de dias do mes atual
char num_dia_mes(){
    const int dias[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // Verifica se o ano é bissexto e se o mês é fevereiro
    if ((mes == 2) && ((ano % 4 == 0 && ano % 100 != 0) || (ano % 400 == 0))) {
        return 29;
    }
    return dias[mes];
}

/// @brief Adiciona mais 1 minuto no relógio interno
void prox_minuto(){
    char _num_dia_mes = num_dia_mes();
    min++;
    
    if (min < 60){
        return;
    }
    min = 0;
    hora++;

    if (hora < 24){
        return;
    }
    hora = 0;
    dia++;

    if (dia < _num_dia_mes + 1){
        return;
    }
    dia = 1;
    mes++;

    if (mes < 13){
        return;
    }
    mes = 1;
    ano++;
}

/// @brief Coloca a data e hora atual formatados no ponteiro recebido por parametro
/// @param hora_atual_ptr Ponteiro que irá receber data e hora no formato // DMAAHm
void hora_atual_str(char hora_atual_ptr[6]){
    hora_atual_ptr[0] = dia;
    hora_atual_ptr[1] = mes;
    hora_atual_ptr[2] = (char) (ano >> 8);
    hora_atual_ptr[3] = (char) (ano & 0xF);
    hora_atual_ptr[4] = hora;
    hora_atual_ptr[5] = min;
}

void printa_data(){
    UART_transmit('\n');
    UART_envia_string("Data atual - ");
    UART_transmit(dia + 48);
    UART_transmit('/');
    UART_transmit(mes + 48);
    UART_transmit('/');
    UART_transmit(((char)(ano >> 8)) + 48);
    UART_transmit('_');
    UART_transmit(((char)(ano & 0xF)) + 48);
    UART_transmit('/');
    UART_transmit(hora + 48);
    UART_transmit(':');
    UART_transmit(min + 48);
    UART_transmit('\n');
    UART_envia_string("----------------------------------");
}
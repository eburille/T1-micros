#include "hora.h"

char dia = 0;
char mes = 0;
int ano = 0;
char hora = 0;
char min = 0;

/// @brief Ajusta a hora baseado em uma string no formato DMAAHm
/// @param string String com as informações de data
void ajustar_hora(char string[6]){
    dia = string[0];
    mes = string[1];
    ano = string[2] << 8 | string[3];
    hora = string[4];
    min = string[5];
}

/// @brief Calcula a diferença de tempo em minutos entre a hora atual e a hora passada por parametro
/// @param hora_entrada string com o hora de entrada no formato  DMAAHm
/// @return Se ja tiver se passado mais de 1 dia, retorna 500, caso contrario, retorna o tempo passado em minutos
int calc_delta_time_min(char hora_entrada[6]){    
    char dia_inicial = hora_entrada[0];
    char mes_inicial = hora_entrada[1];
    char ano_inicial = hora_entrada[2] << 8 | hora_entrada[3];
    char hora_inicial = hora_entrada[4];
    char min_inicial = hora_entrada[5];

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
    hora_atual_ptr[2] = ano >> 8;
    hora_atual_ptr[3] = ano & 0xF;
    hora_atual_ptr[4] = hora;
    hora_atual_ptr[5] = min;
}
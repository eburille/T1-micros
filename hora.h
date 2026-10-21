#ifndef HORA__H
#define HORA__H

#define HORA_INICIO_TRABALHO  8
#define HORA_FIM_TRABALHO    18

extern volatile char dia;
extern volatile char mes;
extern volatile int ano;
extern volatile char hora;
extern volatile char min;

extern volatile unsigned char ano1;
extern volatile unsigned char ano2;

void ajustar_hora(unsigned char string[6]);
int calc_delta_time_min(char dia_inicial, char mes_inicial, int ano_inicial, char hora_inicial, char min_inicial);
void prox_minuto();
void hora_atual_str(char hora_atual_ptr[6]);
char eh_horario_comercial();
void printa_data();

#endif
#ifndef HORA__H
#define HORA__H

#define HORA_INICIO_TRABALHO  8
#define HORA_FIM_TRABALHO    18

extern char dia;
extern char mes;
extern int ano;
extern char hora;
extern char min;

void ajusta_hora(char string[6]);
int calc_delta_time_min(char hora_entrada[6]);
void prox_minuto();
void hora_atual_str(char hora_atual_ptr[6]);


#endif
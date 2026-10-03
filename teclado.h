#ifndef TECLADO__H
#define TECLADO__H

#define TECLADO_PRESSIONADO 1
#define TECLADO_LIVRE       0

extern int confirmar_tecla;
extern char nova_tecla;
extern char saida_teclado;

void init_teclado();
char ler_teclado();
char decodifica_tecla(char tecla);
void roda_teclado();

#endif
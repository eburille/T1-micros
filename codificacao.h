#ifndef CODIFICACAO__H
#define CODIFICACAO__H


#define TAM_MAX      78   /* tamanho maximo de uma mensagem (com o '\0') */

extern int chave;

void codifica (char *mensagem, int p_chave);
void decodifica (char *mensagem, int p_chave);
void encontra_chave(char *mensagem);

#endif
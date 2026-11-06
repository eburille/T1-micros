#ifndef CODIFICACAO__H
#define CODIFICACAO__H

#define TAM_MAX      78   

extern int chave;

void codifica (char *mensagem, int p_chave);
void decodifica (char *mensagem, int p_chave);
void encontra_chave(char *mensagem);

#endif
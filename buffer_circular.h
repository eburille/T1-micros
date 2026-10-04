#ifndef BUFFER_CIRC_H
#define BUFFER_CIRC_H

#include "mensagem.h"

extern volatile int i_escrita;

typedef struct {
    Mensagem lista_mensagens[10];
    volatile int i_leitura;
    volatile int i_escrita;    
}BufferCircularMsg;

typedef struct {
    char lista_caracter[10];
    volatile int i_leitura;
    volatile int i_escrita;    
}BufferCircularChar;

extern BufferCircularMsg msg_buff;
extern BufferCircularChar char_buff;

void adiciona_mensagem(Mensagem msg);
Mensagem le_mensagem();
int novas_mensagens();

void adiciona_char(char caracter);
char le_char();
int novos_caracteres();

#endif
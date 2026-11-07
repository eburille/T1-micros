
/// BUFFER CIRCULAR é responsável por armazenar Mensagens e/ou carcteres até o momento em que o processador tenha tempo para processa-las

#ifndef BUFFER_CIRC_H
#define BUFFER_CIRC_H

#include "mensagem.h"

extern volatile int i_escrita;

/// @brief Buffer circular para mensagens recebidas
typedef struct {
    Mensagem lista_mensagens[20];
    volatile int i_leitura;
    volatile int i_escrita;    
}BufferCircularMsg;

/// @brief Buffer circular para caracteres recebidos por UART
typedef struct {
    char lista_caracter[20];
    volatile int i_leitura;
    volatile int i_escrita;    
}BufferCircularChar;

extern BufferCircularMsg msg_buff;
extern BufferCircularChar char_buff;

void adiciona_mensagem(Mensagem msg);
Mensagem le_mensagem();
int novas_mensagens_servidor();

void adiciona_char(char caracter);
char le_char();
int novos_caracteres();

#endif
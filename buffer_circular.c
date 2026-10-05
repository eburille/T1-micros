#include "buffer_circular.h"
#include "uart_lib.h"
#include "delay.h"

int CIRCBUFFSIZE = 7;

BufferCircularMsg msg_buff;
BufferCircularChar char_buff;

void adiciona_mensagem(Mensagem msg){
    msg_buff.lista_mensagens[msg_buff.i_escrita] = msg;

    msg_buff.i_escrita++;
    if (msg_buff.i_escrita >= CIRCBUFFSIZE) {
        msg_buff.i_escrita = msg_buff.i_escrita - CIRCBUFFSIZE;
    }

    if (msg_buff.i_escrita != msg_buff.i_leitura){
        return;
    }

    msg_buff.i_leitura++;
    if (msg_buff.i_leitura == CIRCBUFFSIZE) {
        msg_buff.i_leitura = msg_buff.i_leitura - CIRCBUFFSIZE;
    }
    
    return;
}

void adiciona_char(char caracter){
    char_buff.lista_caracter[char_buff.i_escrita] = caracter;

    char_buff.i_escrita++;
    if (char_buff.i_escrita >= CIRCBUFFSIZE) {
        char_buff.i_escrita = char_buff.i_escrita - CIRCBUFFSIZE;
    }

    if (char_buff.i_escrita != char_buff.i_leitura){
        return;
    }

    char_buff.i_leitura++;
    if (char_buff.i_leitura >= CIRCBUFFSIZE) {
        char_buff.i_leitura = char_buff.i_leitura - CIRCBUFFSIZE;
    }
    return;
}

Mensagem le_mensagem(){

    Mensagem msg = msg_buff.lista_mensagens[msg_buff.i_leitura];
    
    // Atualiza indice de leitura
    msg_buff.i_leitura++;
    if (msg_buff.i_leitura == CIRCBUFFSIZE) {
        msg_buff.i_leitura = msg_buff.i_leitura - CIRCBUFFSIZE;
    }

    return msg;
}

char le_char(){
    char msg = char_buff.lista_caracter[char_buff.i_leitura];
    
    // Atualiza indice de leitura
    char_buff.i_leitura++;
    if (char_buff.i_leitura > CIRCBUFFSIZE - 1) {
        char_buff.i_leitura = char_buff.i_leitura - CIRCBUFFSIZE;
        delay_1ms(); // Por algum motico, só funcina assim
    }

    return msg;
}

int novas_mensagens_servidor(){
    int num_novas_mensagens_servidor = msg_buff.i_escrita - msg_buff.i_leitura;
    if (num_novas_mensagens_servidor < 0){
        num_novas_mensagens_servidor = num_novas_mensagens_servidor + CIRCBUFFSIZE;
    }
    return num_novas_mensagens_servidor;
}

int novos_caracteres(){
    int num_novas_caracteres = char_buff.i_escrita - char_buff.i_leitura;
    if (num_novas_caracteres < 0){
        num_novas_caracteres = num_novas_caracteres + CIRCBUFFSIZE;
    }
    return num_novas_caracteres;
}
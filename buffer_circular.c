#include "buffer_circular.h"
#include "uart_lib.h"
#include "delay.h"

int CIRCBUFFSIZE = 18;

BufferCircularMsg msg_buff;
BufferCircularChar char_buff;

/// @brief Adiciona uma Mensagem no buffer de mensagens
/// @param msg Mensagem a ser adicionada
void adiciona_mensagem(Mensagem msg){
    //Adiciona a mensagem
    msg_buff.lista_mensagens[msg_buff.i_escrita] = msg;

    //Atualiza o indice de escrita
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

/// @brief Adiciona um char no buffer de caracteres
/// @param caracter Caracter a ser adicionado
void adiciona_char(char caracter){
    // Escreve o caracter no buffer
    char_buff.lista_caracter[char_buff.i_escrita] = caracter;

    // Atualiza o indice de escrita
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

/// @brief Pega a ultima Mensagem adicionada no buffer e retorna 
/// @return Ultima Mensagem adicionada no buffer
Mensagem le_mensagem(){
    //Pega ultima mensagem do buffer
    Mensagem msg = msg_buff.lista_mensagens[msg_buff.i_leitura];
    
    // Atualiza indice de leitura
    msg_buff.i_leitura++;
    if (msg_buff.i_leitura == CIRCBUFFSIZE) {
        msg_buff.i_leitura = msg_buff.i_leitura - CIRCBUFFSIZE;
    }

    return msg;
}

/// @brief Pega o ultimo char adicionado no buffer e retorna
/// @return Ultimo char adicionado no buffer
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

/// @brief Verifica se a novas mensagens adicionadas no buffer comparando os indices de leitura e escrita
/// @return numero de mensagens não lidas
int novas_mensagens_servidor(){
    int num_novas_mensagens_servidor = msg_buff.i_escrita - msg_buff.i_leitura;
    if (num_novas_mensagens_servidor < 0){
        num_novas_mensagens_servidor = num_novas_mensagens_servidor + CIRCBUFFSIZE;
    }
    return num_novas_mensagens_servidor;
}

/// @brief Verifica se a novos char adicionado no buffer comparando os indices de leitura e escrita
/// @return Numero de char não lidos
int novos_caracteres(){
    int num_novas_caracteres = char_buff.i_escrita - char_buff.i_leitura;
    if (num_novas_caracteres < 0){
        num_novas_caracteres = num_novas_caracteres + CIRCBUFFSIZE;
    }
    return num_novas_caracteres;
}
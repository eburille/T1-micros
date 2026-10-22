#ifndef MENSAGEM__H
#define MENSAGEM__H

#define TAMANHO_MAX_MSG               78
        
#define MSG_DADOS_ESTACIONAMENTO          'E'
#define MSG_PAGAMENTO                     'P'
#define MSG_IMPRESSAO_COMP                'I'
#define MSG_HORARIO                       'H'
#define MSG_CONDICAO_VIA                  'V'
#define MSG_CARTAO_INSERIDO               'C'
#define MSG_DEBUG                         'D'

#define MSG_DADOS_ESTACIONAMENTO_N_BYTES    0
#define MSG_PAGAMENTO_N_BYTES               1
#define MSG_IMPRESSAO_COMPN_N_BYTES         0
#define MSG_HORARION_N_BYTES                6
#define MSG_CONDICAO_VIA_N_BYTES            1
#define MSG_CARTAO_INSERIDO_N_BYTES         6


typedef struct{
    unsigned char string[TAMANHO_MAX_MSG];
    char tipo;
    unsigned char tamanho;
} Mensagem;

#endif
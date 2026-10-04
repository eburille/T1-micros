#ifndef MENSAGEM__H
#define MENSAGEM__H

#define TAMANHO_MAX_MSG                24
        
#define DADOS_ESTACIONAMENTO          'E'
#define PAGAMENTO                     'P'
#define IMPRESSAO_COMP                'I'
#define HORARIO                       'H'
#define CONDICAO_VIA                  'V'

#define DADOS_ESTACIONAMENTO_N_BYTES    0
#define PAGAMENTO_N_BYTES               1
#define IMPRESSAO_COMPN_N_BYTES         0
#define HORARION_N_BYTES                6
#define CONDICAO_VIA_N_BYTES            1


typedef struct{
    char string[TAMANHO_MAX_MSG];
    char tipo;
} Mensagem;

#endif
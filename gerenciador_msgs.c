#include "gerenciador_msgs.h"
#include "uart_lib.h"

#define ESPERA                              0
#define RECEBENDO_MENSAGEM                  1
#define MENSAGEM_RECEBIDA                   2
#define RECEBENDO_TIPO_MENSAGEM             3
#define RECEBENDO_N_BYTES_MENSAGEM_VIA      4

#define CHAR_NOVA_MSG                      'S'

char estado_gerenciador;

Mensagem mensagem_recebida;

volatile char num_caracteres_a_receber;


void init_gerenciador_msgs(){
    estado_gerenciador = ESPERA;
}

void proc_n_bytes_via(){
    char c;

    if(novos_caracteres() == 0){
        return;
    }

    c = le_char();
    num_caracteres_a_receber = c;
    estado_gerenciador = RECEBENDO_MENSAGEM;
}

void proc_caracter(){
    static char string_index = 0;
    char c;
    int num_novos_caracteres = novos_caracteres();


    if (num_caracteres_a_receber == 0){
        mensagem_recebida.string[string_index] = '\0';
        string_index = 0;
        estado_gerenciador = MENSAGEM_RECEBIDA;
        UART_envia_string("\nMENSAGEM RECEBIDA");
        return;
    } 

    if(num_novos_caracteres == 0){
        return;
    }

    c = le_char();
    mensagem_recebida.string[string_index] = c;
    
    string_index++;
    num_caracteres_a_receber--;

    UART_envia_string("\nCaracter recebido, faltam: ");
    UART_transmit(num_caracteres_a_receber + 48);
}

void verifica_UART(){
    char c;
    int num_novos_caracteres = novos_caracteres();

    // UART_transmit(num_novos_caracteres + 48);
    if (num_novos_caracteres == 0){
        // UART_envia_string("return");
        return;
    }
    
    c = le_char();

    if (c == CHAR_NOVA_MSG){
        estado_gerenciador = RECEBENDO_TIPO_MENSAGEM;
        UART_envia_string("TIPO MSG\n");
    }
    
}

void proc_tipo_mensagem(){
    char c;
    int num_novos_caracteres = novos_caracteres();
    // UART_envia_string("Fora If");
    if (num_novos_caracteres == 0){
        return;
    }
    
    c = le_char();
    mensagem_recebida.tipo = c;
    switch (c) {
        case DADOS_ESTACIONAMENTO:
            num_caracteres_a_receber = (int) DADOS_ESTACIONAMENTO_N_BYTES;
            estado_gerenciador = (int)MENSAGEM_RECEBIDA;
            UART_envia_string("Dados estacionamento\n");
            break;
            
        case PAGAMENTO:
            num_caracteres_a_receber = (int)PAGAMENTO_N_BYTES;
            estado_gerenciador = (int)RECEBENDO_MENSAGEM;
            UART_envia_string("Dados Pagamento\n");
            break;
            
        case IMPRESSAO_COMP:
            num_caracteres_a_receber = (int)IMPRESSAO_COMPN_N_BYTES;
            estado_gerenciador = (int)MENSAGEM_RECEBIDA;
            UART_envia_string("Dados impressao\n");
            break;

        case HORARIO:
            num_caracteres_a_receber = (int)HORARION_N_BYTES;
            estado_gerenciador = (int)RECEBENDO_MENSAGEM;
            UART_envia_string("Dados hora\n");
            break;

        case CONDICAO_VIA:
            num_caracteres_a_receber = (int)CONDICAO_VIA_N_BYTES;
            estado_gerenciador = (int)RECEBENDO_N_BYTES_MENSAGEM_VIA;
            UART_envia_string("Dados Via\n");
            break;

        default:
            UART_envia_string("Caracter errado\n");
            break;
         
    }
}

void gerenciar_msgs(){
    switch (estado_gerenciador){
        case ESPERA:
            verifica_UART();
            // UART_transmit('A');
            break;

        case RECEBENDO_MENSAGEM:
            proc_caracter();
            break;
        
        case MENSAGEM_RECEBIDA:
            UART_envia_string("ESPERA\n");
            
            adiciona_mensagem(mensagem_recebida);
            estado_gerenciador = ESPERA;
            break;

        case RECEBENDO_TIPO_MENSAGEM:
            proc_tipo_mensagem();
            break;

        case RECEBENDO_N_BYTES_MENSAGEM_VIA:
            proc_n_bytes_via();
            break;

        default:
            break;
    }
}
// gerenciador_msgs serve para processar os caracteres recebidos via UART e montar as mensagens descritas no protocolo

#ifndef GERENCIADOR_MSGS__H
#define GERENCIADOR_MSGS__H

#include "mensagem.h"
#include "buffer_circular.h"

void init_gerenciador_msgs();
void gerenciar_msgs();


#endif
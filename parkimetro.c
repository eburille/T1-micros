#include "parkimetro.h"
#include "hora.h"
#include "string.h"
#include <stdio.h>

#include "uart_lib.h"
#include "lcd_lib.h"

#define CARRO_PRESENTE  0
#define CARRO_AUSENTE   1

#define TEM_PARASITA    1
#define N_TEM_PARASITA  0


#define PRIMEIRA_LINHA 0x80
#define SEGUNDA_LINHA 0xC0
// Rua osvaldo = {"OSVALDO", 0, 0};
// Rua sarmento = {"SARMENT", 0, 0};
// Rua joao = {"JPESSOA", 0, 0};

// Rua ruas[3] = {osvaldo, sarmento, joao};

volatile char num_carros = 0;
int tempo_array[] = {30, 60, 90, 120, 500, 0};
Carro carros[NUM_MAX_CARROS];
volatile char num_carros_irregulares = 0;
volatile char num_carros_parasitas = 0;
int indice_irreg = 0;

Carro carros_parasitas[NUM_MAX_CARROS];
Carro carros_parasitas_tolerancia[NUM_MAX_CARROS];

void adiciona_carro(char placa[7], char especial, char tempo_contratado, char rua, char esta_regular){
    char hora_atual[6];
    
    Carro novo_carro; //= {placa, especial, hora_atual, tempo_contratado, REGULAR, rua};
    
    // hora_atual_str(hora_atual); //////// Fazer função para pegar hora atual
    novo_carro.dia_entrada = dia;
    novo_carro.mes_entrada = mes;
    novo_carro.ano_entrada = ano;
    novo_carro.hora_entrada = hora;
    novo_carro.min_entrada = min;

    strcpy(novo_carro.placa, placa);
    novo_carro.especial = especial;
    novo_carro.tempo_contratado = tempo_contratado;
    novo_carro.esta_regular = esta_regular;
    novo_carro.rua = rua;

    carros[num_carros] = novo_carro;
    num_carros++;
}


/// @brief Verificar se um carro ja ultrapassou o tempo limite de permanência contratada + tolerância
/// @param carro Carro a ter a regularidade verificada
/// @return Retorna condição de regularidade do carro
char verifica_carro_irregular(Carro carro){
    // UART_envia_string("ANALISANDO");
    if (calc_delta_time_min(carro.dia_entrada, carro.mes_entrada, carro.ano_entrada, carro.hora_entrada, carro.min_entrada) > (tempo_array[carro.tempo_contratado] + TOLERANCIA_MIN)){
        return IRREGULAR;
    }
    return REGULAR;
}

/// @brief Verifica todos os carros para garantir se algum deles passou do tempo limite
void verifica_carros_regulares(){
    char i = 0;
    num_carros_irregulares = 0;

    for (i; i < num_carros; i++){
        if (verifica_carro_irregular(carros[i]) == IRREGULAR && (carros[i].especial == N_ESPECIAL)){
            carros[i].esta_regular = IRREGULAR;
            num_carros_irregulares++;
        }else{
            carros[i].esta_regular = REGULAR;
        } 
    }
}

/// @brief Remove carros da lista de carros a partir de um indice, e reorganiza a lista
/// @param i indice do carro a ser removido
void remove_carro(char i){
    for (i; i < num_carros - 1; i++){
        carros[i] = carros[i+1];
    }
    num_carros--;
}


char compara_placas(char placa1[7], char placa2[7]){
    UART_envia_string("\nplaca1 - ");
    UART_transmit(placa1[0]);
    UART_transmit(placa1[1]);
    UART_transmit(placa1[2]);
    UART_transmit(placa1[3]);
    UART_transmit(placa1[4]);
    UART_transmit(placa1[5]);
    UART_transmit(placa1[6]);
    UART_transmit('\n');
    char i = 0;
    for (i; i < 7; i++){
        if (placa1[i] != placa2[i]){
            return 0;
        }
    }
    return 1;
}

char pega_rua(char rua[7]){
    UART_transmit(rua[0]);
    UART_transmit(rua[1]);
    UART_transmit(rua[2]);
    UART_transmit(rua[3]);
    UART_transmit(rua[4]);
    UART_transmit(rua[5]);
    UART_transmit(rua[6]);
    UART_transmit('\n');

    if (rua[0] == 'O'){
        UART_envia_string("ENTROU EM OSVALDO");
        return OSVALDO;
    }else if (rua[0] == 'J'){
        UART_envia_string("ENTROU EM JPESSOA");
        return JPESSOA;
    }else if (rua[0] == 'S'){
        UART_envia_string("ENTROU EM SARMENT");
        return SARMENT;
    }
}

/// @brief Recebe uma lista de placas dos carros presentes e remove os carros que não estão nessa lista
/// @param placas Lista dos carros presentes
void verifica_carros_presentes(char placas[][7], char num_placas){
    int i = 0;
    int j = 1;

    char rua = pega_rua(placas[0]);
    char carro_presente = CARRO_AUSENTE;

    for (i; i < num_carros; i++){
        j=1;
        if (carros[i].rua != rua){
            continue;
        }
        for (j; j <= num_placas; j++){
            // UART_envia_string("roda compara placas\n");
            if (compara_placas(carros[i].placa, placas[j])){
                carro_presente = CARRO_PRESENTE;
                break;
            }
        }
        if (carro_presente == CARRO_AUSENTE){
            remove_carro(i);
        }
        carro_presente = CARRO_AUSENTE;
    }
}

/// @brief Recebe uma lista de placas dos carros presentes e remove os carros que não estão nessa lista
/// @param placas Lista dos carros presentes
void verifica_carros_parasitas(char placas[][7], char num_placas){
    int i = 1;
    int j = 0;

    char rua = pega_rua(placas[0]);
    Carro carro_parasita;
    char eh_parasita = N_TEM_PARASITA;

    num_carros_parasitas = 0;

    UART_envia_string("\nNumero placas: ");
    UART_transmit(num_placas + 48);
    UART_transmit('\n');


    // VERIFICA COM AS PLACAS DOS CARROS ESTACIONADOS
    for (i; i <= num_placas; i++){
        eh_parasita = TEM_PARASITA;
        j=0;
        for (j; j < num_carros; j++){
            if (compara_placas(placas[i], carros[j].placa)){
                eh_parasita = N_TEM_PARASITA;
                break;
            }
        }

        if (eh_parasita == TEM_PARASITA){
            adiciona_carro(placas[i], N_ESPECIAL, t_0, rua, REGULAR);
        }
    }
}

void gerenciar_parkimetro(){
    verifica_carros_regulares();
}

void printa_lista_carros(){
    int i = 0;
    Carro carro;
    UART_envia_string("\nCarros estacionados \n ------------------------------------ \n\n");
    UART_transmit('\n');
    for (i; i < num_carros; i++){
        carro = carros[i];

        UART_transmit(carro.placa[0]);
        UART_transmit(carro.placa[1]);
        UART_transmit(carro.placa[2]);
        UART_transmit(carro.placa[3]);
        UART_transmit(carro.placa[4]);
        UART_transmit(carro.placa[5]);
        UART_transmit(carro.placa[6]);

        UART_envia_string(" - ");

        UART_transmit(carro.dia_entrada + 48);
        UART_transmit('/');
        UART_transmit(carro.mes_entrada + 48);
        UART_transmit('/');
        UART_transmit(((char)(carro.ano_entrada >> 8)) + 48);
        UART_transmit('_');
        UART_transmit(((char)(carro.ano_entrada & 0xF))  + 48);
        UART_transmit('-');
        UART_transmit(carro.hora_entrada + 48);
        UART_transmit(':');
        UART_transmit(carro.min_entrada + 48);
        
        UART_transmit('-');
        UART_transmit(carro.esta_regular + 48);
        UART_transmit('\n');
    }
    
  
}

//Mostra no display carros irregulares para o adm
void mostrar_irreg(int indice_inicio){
    int i;
    char encontrou = 0;
    char msg_placa [17];

    for (i=indice_inicio; i<num_carros; i++){
        if (carros[i].esta_regular == IRREGULAR){
            encontrou = 1;
            indice_irreg = i;
            limpa_lcd ();
            lcd_cmd (PRIMEIRA_LINHA);
            sprintf (msg_placa, "PLACA: %s", carros[i].placa);
            envia_string (msg_placa);
            
            lcd_cmd (SEGUNDA_LINHA);
            if (carros[i].rua == OSVALDO){
                envia_string ("RUA: OSVALDO");
            }
            else if (carros[i].rua ==JPESSOA){
                envia_string ("RUA: JPESSOA");
            }
            else if (carros[i].rua == SARMENT){
                envia_string ("RUA:SARMENTO");
            }
            break;
        }
    }

    if (encontrou == 0){
        limpa_lcd();
        lcd_cmd (PRIMEIRA_LINHA);
        envia_string ("SEM IRREGULARES");
        lcd_cmd (SEGUNDA_LINHA);
        envia_string ("* PARA SAIR");
        indice_irreg = -1;
    }

}
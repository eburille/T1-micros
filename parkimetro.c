#include "parkimetro.h"

#define CARRO_PRESENTE  0
#define CARRO_AUSENTE   1

// Rua osvaldo = {"OSVALDO", 0, 0};
// Rua sarmento = {"SARMENT", 0, 0};
// Rua joao = {"JPESSOA", 0, 0};

// Rua ruas[3] = {osvaldo, sarmento, joao};

char num_carros = 0;

void adiciona_carro(char placa[7], char especial, char tempo_contratado, char rua){
    char hora_atual; //////// Fazer função para pegar hora atual
    Carro novo_carro = {placa, especial, hora_atual, tempo_contratado, REGULAR, rua};

    carros[num_carros] = novo_carro;
    num_carros++;
}

char verifica_carro(Carro carro){
//// implementar logica para verificar se carro está irregular ////
}

/// @brief Verifica todos os carros para garantir se algum deles passou do tempo limite
void verifica_carros_regulares(){
    int i = 0;
    for (i; i < num_carros; i++){
        if (verifica_carro(carros[i]) == IRREGULAR){
            carros[i].esta_regular = IRREGULAR;
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

/// @brief Recebe uma lista de placas dos carros presentes e remove os carros que não estão nessa lista
/// @param placas Lista dos carros presentes
void verifica_carros_presentes(char placas[][8], char num_placas){
    int i = 0;
    int j = 0;

    char carro_presente = CARRO_AUSENTE;
    for (i; i < num_carros; i++){
        for (j; j < num_placas; j++){
            if (compara_placas(carros[i].placa, placas[j])){
                carro_presente = CARRO_PRESENTE;
            }
        }

        if (carro_presente == CARRO_AUSENTE){
            remove_carro(i);
        }
        carro_presente = CARRO_AUSENTE;
    }
}
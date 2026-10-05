#ifndef PARKIMETRO__H
#define PARKIMETRO__H

/////  RUAS //////
#define OSVALDO         0
#define SARMENT         1
#define JPESSOA         2

#define t_30_min       30
#define t_1_hora       60
#define t_1h_30m       90
#define t_2_hora      120
#define t_2_mais      500

#define TOLERANCIA_MIN  3

#define REGULAR         0
#define IRREGULAR       1

#define NUM_MAX_CARROS 60

typedef struct{
    char placa[7];
    char especial;
    char hora_entrada[6];  // DMAAHm
    char tempo_contratado;
    char esta_regular;
    char rua;
}Carro;

// typedef struct {
//     char nome_rua[8];
//     char num_carros;
//     Carro carros[20];
// }Rua;

// extern Rua osvaldo;
// extern Rua sarmento;
// extern Rua joao;

char num_carros;
extern Carro carros[NUM_MAX_CARROS];
// extern Carro carros_irregulares[/* NUMERO MAXIMO DE CARROS */];

void adiciona_carro(char placa[7], char especial, char tempo_contratado, char rua);

void verifica_carros_regulares();

void verifica_carros_presentes(char placas[][8], char num_placas);
#endif
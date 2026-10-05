#ifndef PARKIMETRO__H
#define PARKIMETRO__H

/////  RUAS //////
#define OSVALDO  0
#define SARMENT  1
#define JPESSOA  2

#define t_30_min  0
#define t_1_hora  1
#define t_2_hora  2
#define t_2_mais  3

#define REGULAR   0
#define IRREGULAR 1

typedef struct{
    char placa[7];
    char especial;
    char hora_entrada[6];
    char tempo_contratado;
    char esta_regular;
    char rua;
}Carro;

typedef struct {
    char nome_rua[8];
    char num_carros;
    Carro carros[20];
}Rua;

extern Rua osvaldo;
extern Rua sarmento;
extern Rua joao;

char num_carros;
extern Carro carros[/* NUMERO MAXIMO DE CARROS */];;
extern Carro carros_irregulares[/* NUMERO MAXIMO DE CARROS */];

#endif
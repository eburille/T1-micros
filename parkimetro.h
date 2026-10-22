#ifndef PARKIMETRO__H
#define PARKIMETRO__H

/////  RUAS //////
#define OSVALDO              '0'
#define SARMENT              '1'
#define JPESSOA              '2'

#define t_30_min              0 //  30
#define t_1_hora              1 //  60
#define t_1h_30m              2 //  90
#define t_2_hora              3 // 120
#define t_ilimitado           4 // 500

#define TOLERANCIA_MIN        3

#define REGULAR               0
#define IRREGULAR             1

#define N_ESPECIAL            0
#define ESPECIAL              1

#define NUM_MAX_CARROS       30

typedef struct{
    char placa[7];
    char especial;
    char dia_entrada;
    char mes_entrada;
    int ano_entrada;
    char hora_entrada;
    char min_entrada;
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


extern Carro carros[NUM_MAX_CARROS];
extern volatile char num_carros_irregulares;
extern volatile char num_carros_parasitas;
extern int indice_irreg;



// extern Carro carros_irregulares[/* NUMERO MAXIMO DE CARROS */];

void adiciona_carro(char placa[7], char especial, char tempo_contratado, char rua);

void verifica_carros_regulares();

void verifica_carros_presentes(char placas[][7], char num_placas);

void gerenciar_parkimetro();

void printa_lista_carros();

void verifica_carros_parasitas(char placas[][7], char num_placas);

void mostrar_irreg(int indice_inicio);

#endif
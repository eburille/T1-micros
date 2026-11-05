#include <string.h>

#define TAM_MAX      78   /* tamanho maximo de uma mensagem (com o '\0') */
#define TAM_ALFABETO 36
#define CHAVE_MIN    (-36)
#define CHAVE_MAX    36
#define NUM_PALAVRAS 3

static const char ALFABETO[TAM_ALFABETO + 1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static const char *PALAVRAS_CHAVE[NUM_PALAVRAS] = {"OSVALDO", "JPESSOA", "SARMENT"};


int modulo(int a, int m)
{
    int r = a % m;
    if (r < 0)
        r += m;
    return r;
}

int confere_inicio(const char *mensagem, const char *palavra){
    int i=0;

    while(palavra[i] != '\0' ){
        if(mensagem[i]==palavra[i]){
            i++;
        }else{
            return 0;
        }
    }
    return 1;
}

int localiza_alfabeto (char caractere){

    if(caractere>='A' && caractere<='Z'){
        return caractere - 'A';
    }else if(caractere>='0' && caractere<='9'){
        return caractere - '0' + 26;
    }
}

void soma (char *entrada, char *saida){

    
    for(int j=0;j<n;j++){

        int i = localiza_alfabeto(entrada[j]);

        int i_novo = modulo( chave+i , TAM_ALFABETO);

        saida[j] = ALFABETO[i_novo];
    }
}

void desloca(char *entrada, char *saida, int n){

    for(int j=0;j<n;j++){

        int i = modulo( chave+j , n);

        saida[i] = entrada[j];
    }   
    saida[n] = '\0'; 
}

void codifica (char *mensagem){

    char temp[TAM_MAX];

    int n = (int)strlen(mensagem);

    soma(mensagem, temp, chave, n);
    desloca(temp, mensagem, chave, n);
    
}

void decodifica (char *mensagem,  int n){

    char temp[TAM_MAX];

    soma(mensagem, temp, -chave, n);
    desloca(temp, mensagem, -chave, n);
    
}

void encontra_chave(char *mensagem, int n){

    char tentativa[TAM_MAX];
    
    for(int i=CHAVE_MIN;i<=CHAVE_MAX;i++){
        decodifica(mensagem, tentativa, i, n);

        for(int j=0;j<NUM_PALAVRAS;j++){

            if(confere_inicio(tentativa, PALAVRAS_CHAVE[j])){
                chave = i;
            }
        }
    }
}
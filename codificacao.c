#include <string.h>
#include "codificacao.h"

#define TAM_ALFABETO 36
#define CHAVE_MIN    (-36)
#define CHAVE_MAX    36
#define NUM_PALAVRAS 3

static const char ALFABETO[TAM_ALFABETO + 1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static const char PALAVRAS_CHAVE[NUM_PALAVRAS][8] = {"OSVALDO", "JPESSOA", "SARMENT"};

int chave = 0;

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
    return  -1;
}

void soma (char *entrada, char *saida, int p_chave){
    for(int j=0; j<TAM_MAX; j++){
        if (entrada[j] == '\0'){
            saida[j] = '\0'; // <--- O SALVADOR DA MEMÓRIA
            return;
        }

        int i = localiza_alfabeto(entrada[j]);

        if(i<0){
            saida[j]=entrada[j];
        }else{
            int i_novo = modulo( p_chave+i , TAM_ALFABETO);
            saida[j] = ALFABETO[i_novo];
        }
    }
}

void desloca(char *entrada, char *saida, int p_chave){
    int n = (int)strlen(entrada);
    for(int j=0; j<n; j++){
        int i = modulo( p_chave+j , n);
        saida[i] = entrada[j];
    }   
    saida[n] = '\0'; 
}

void codifica (char *mensagem, int p_chave){
    char temp[TAM_MAX];
    soma(mensagem, temp, p_chave);
    desloca(temp, mensagem, p_chave);
}

void decodifica (char *mensagem, int p_chave){
    char temp[TAM_MAX];
    soma(mensagem, temp, -p_chave);
    desloca(temp, mensagem, -p_chave);
}

void encontra_chave(char *mensagem){
    char tentativa[TAM_MAX];

    for(int i = CHAVE_MIN; i <= CHAVE_MAX; i++){
        
        // 1. O backup TEM de ser feito dentro do laço para recomeçar sempre limpo!
        strcpy(tentativa, mensagem); 

        // 2. Testa a chave i
        decodifica(tentativa, i);

        // 3. Varre as ruas
        for(int j = 0; j < NUM_PALAVRAS; j++){
            if(confere_inicio(tentativa, PALAVRAS_CHAVE[j]) == 1){
                chave = i;
                
                // 4. Achou a chave! Copia a string decifrada de volta para a main e vaza.
                //strcpy(mensagem, tentativa);
                return;
            }
        }
    }
}
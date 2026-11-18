// GRR20240011 GABRIEL OLIVEIRA DE MATOS
// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Alocador de memória heap.

#include <stdio.h>
#include <string.h>
#include "tcb.h"

#define HEAP_SIZE (64 * 1024 * 1024)
#define TAM_BLOCO 16
#define MAX_BLOCOS (HEAP_SIZE/TAM_BLOCO)

static char heap[HEAP_SIZE] __attribute__((aligned(16)));

struct bloco {
    char *inicio;
    int tamanho;
    int ocupado;
    int ativo;

    int anterior;
    int proximo;
    
};

static struct bloco blocos[MAX_BLOCOS];
static int primeiro_bloco;
static int quantidade_blocos;
static int disponivel;

//!=================================================================================

static int normalizar_tamanho(int tamanho){
    return ((tamanho+ TAM_BLOCO - 1)/TAM_BLOCO) * TAM_BLOCO;

}


//!=================================================================================
static int free_descritor()
{
    for (int i = 0; i < MAX_BLOCOS; i++) {
        if (!blocos[i].ativo){
            return i;
        }
    }

    return ERROR;
}

//!=================================================================================


static void fundir_blocos(int esquerda, int direita){
    blocos[esquerda].tamanho += blocos[direita].tamanho;

    int proximo = blocos[direita].proximo;

    if (proximo != esquerda) {
        blocos[proximo].anterior = esquerda;
    }else {
        blocos[esquerda].anterior = esquerda;
    }

    blocos[esquerda].proximo = proximo;


    if (primeiro_bloco == direita) {
        primeiro_bloco = esquerda;
    }

    blocos[direita].ativo = 0;

    quantidade_blocos--;
}

//!=================================================================================

// Inicia o subsistema de memória.
void mem_init(){
    for (int i = 0; i < MAX_BLOCOS; i++) {
        blocos[i].ativo = 0;
    }

    primeiro_bloco = 0;
    quantidade_blocos = 1;
    disponivel = HEAP_SIZE;

    blocos[0].inicio = heap;
    blocos[0].tamanho = HEAP_SIZE;
    blocos[0].ocupado = 0;
    blocos[0].ativo = 1;
    blocos[0].anterior = 0;
    blocos[0].proximo = 0;


}

//!=================================================================================

// Encerra o subsistema de memória.
void mem_term()
{
    for (int i = 0; i < MAX_BLOCOS; i++) {
        blocos[i].ativo = 0;
    }

    primeiro_bloco = -1;
    quantidade_blocos = 0;
    disponivel = 0;
}

//!=================================================================================

// Informa a quantidade total de memória.
int mem_size()
{
    return HEAP_SIZE;
}
//!=================================================================================


// Informa a quantidade de memória disponível.
int mem_avail()
{
    return disponivel;
}

//!=================================================================================

// Aloca um bloco de memória usando First-Fit.
void *mem_alloc(int size){
    if (size <= 0 || size > HEAP_SIZE){
        return NULL;
    }

        
    size = normalizar_tamanho(size);

    if (size > disponivel){
        return NULL;
    }
    int atual = primeiro_bloco;

    do{
        if(!blocos[atual].ocupado && blocos[atual].tamanho >= size) {
            int restante = blocos[atual].tamanho - size;

            if(restante > 0) {
                int novo = free_descritor();
        
                if(novo == -1)return NULL;
                
                blocos[novo].tamanho = restante;

                blocos[novo].ocupado = 0;
                blocos[novo].ativo = 1;
                blocos[novo].inicio = blocos[atual].inicio + size;

                blocos[novo].anterior = atual;
                blocos[novo].proximo = blocos[atual].proximo;
                blocos[blocos[atual].proximo].anterior = novo;

                blocos[atual].proximo = novo;
                blocos[atual].tamanho = size;

                quantidade_blocos++;
            }

            blocos[atual].ocupado = 1;
            disponivel -= size;

            return blocos[atual].inicio;
        }

        atual = blocos[atual].proximo;

    }while(atual != primeiro_bloco);

    return NULL;
}

//!=================================================================================


// Libera um bloco previamente alocado.
int mem_free(void *addr)
{
    if (addr == NULL){
         return ERROR;
    }

    int atual = primeiro_bloco;

    do {
        if (blocos[atual].inicio == addr) break;
        atual = blocos[atual].proximo;

    } while (atual != primeiro_bloco);

    if (blocos[atual].inicio != addr){ 
        return ERROR;
    }
    
    if (!blocos[atual].ocupado){ 
        return ERROR;
    }
    
    blocos[atual].ocupado = 0;

    disponivel += blocos[atual].tamanho;
    int esquerda = blocos[atual].anterior;

    if (esquerda != atual && !blocos[esquerda].ocupado) {

        fundir_blocos(esquerda, atual);

        atual = esquerda;
    }

    int direita = blocos[atual].proximo;

    if (direita != atual && !blocos[direita].ocupado) {
        fundir_blocos(atual, direita);
    }

    return NOERROR;
}

//!=================================================================================


// Gera um relatório sobre o uso atual da heap.
void mem_report(){
    int mem_alocada = 0;
    int mem_livre = 0;
    int blocos_alocados = 0;
    int blocos_livres = 0;
    int atual = primeiro_bloco;

    if (atual == -1){
         return;
    }
    
    do {
        if (blocos[atual].ocupado) {
            mem_alocada += blocos[atual].tamanho;
            blocos_alocados++;

        }else{
            mem_livre += blocos[atual].tamanho;
            blocos_livres++;
        }

        atual = blocos[atual].proximo;

    }while(atual != primeiro_bloco);

    int mem_aloc = mem_alocada / 1024; 
    int mem_livre_r = mem_livre/1024;
    printf("heap: %d KB allocated (%d blocks), %d KB free (%d blocks)\n", mem_aloc, blocos_alocados, mem_livre_r, blocos_livres);

    atual = primeiro_bloco;

    do {
        char *inicio = blocos[atual].inicio;
        char *fim = inicio + blocos[atual].tamanho - 1;
        char result[5];

        if (blocos[atual].ocupado) {
            strcpy(result, "aloc");
        } else {
            strcpy(result, "FREE");
        }
        printf("heap: block %5d: %p - %p %s prev %5d next %5d size %d\n", atual, (void *)inicio, (void *)fim,result, blocos[atual].anterior, blocos[atual].proximo, blocos[atual].tamanho);

        atual = blocos[atual].proximo;
    } while (atual != primeiro_bloco);
}
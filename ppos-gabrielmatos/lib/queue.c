// GRR20240011 GABRIEL OLIVEIRA DE MATOS

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.0 -- 06/2025

// Este arquivo PODE/DEVE ser alterado.

// Implementação do TAD fila genérica
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"


typedef struct queue_t {
    int iterador;
    void **items;
    int size;
    int capacidade; 
} queue_t;


// Cria uma fila inicialmente vazia.
// Retorno: ponteiro p/ a nova fila
//          NULL se houver erro
struct queue_t *queue_create(){
    struct queue_t *novo = (queue_t*) malloc(sizeof(queue_t));

    if(!novo){
        return NULL;
    }
    novo->iterador =-1;
    novo->size=0;
    novo->capacidade = 4;
    novo->items = (void**) malloc(novo->capacidade*sizeof(void*));
    if(!novo->items){
        free(novo);
        return NULL;
    }
    return novo;
}

// Destroi uma fila, liberando a memória alocada por ela.
// IMPORTANTE: os itens apontados pela fila NÃO devem ser liberados,
// pois a aplicação que os criou e pôs na fila é responsável por eles.
// Retorno: NOERROR ou ERROR (se a fila não existir)
int queue_destroy(struct queue_t *queue){

    if(!queue){
        return ERROR;
    }
    free(queue->items);
    free(queue);

    return NOERROR;
}

// Adiciona um item no fim da fila; ajusta o iterador para ele
// se for o primeiro item (ou seja, se a fila estiver vazia).
// Retorno: NOERROR ou ERROR (se fila ou item não existir)
int queue_add(struct queue_t *queue, void *item){

    if(!queue || !item){
        return ERROR;
    }

    if(queue->size >= queue->capacidade){
        int nova_capacidade = queue->capacidade * 2;

        void **temp = realloc(queue->items,
                            nova_capacidade * sizeof(void *));

        if (!temp)
            return ERROR;

        queue->items = temp;
        queue->capacidade = nova_capacidade;

    }
    queue->items[queue->size] = item;
    queue->size++;

    return NOERROR;
}

// Retira da fila o item com o valor indicado; se o item estiver
// em mais de uma posição da fila, retira apenas da primeira posição
// encontrada; se o item estiver apontado pelo iterador, este avança
// para o próximo item da fila (ou para NULL, se for o último).
// Retorno: NOERROR ou ERROR (não encontrou ou outro erro).
int queue_del(struct queue_t *queue, void *item)
{
    if (!queue || !item)
        return ERROR;

    int i;

    for (i = 0; i < queue->size; i++) {
        if (queue->items[i] == item)
            break;
    }

    if (i == queue->size)
        return ERROR;

    if (i < queue->iterador)
        queue->iterador--;

    for (; i < queue->size - 1; i++)
        queue->items[i] = queue->items[i + 1];

    queue->size--;

    if (queue->iterador >= queue->size)
        queue->iterador = queue->size;

    return NOERROR;
}

// Informa se o item indicado está na fila.
// Retorno: true/false (error: false).
bool queue_has(struct queue_t *queue, void *item){
    if(!queue || !item){
        return false;
    }

    for(int i=0; i<queue->size; i++){
        if(queue->items[i] == item){
            return true;
        }
    }
    return false;
}

// Informa o número de itens na fila.
// Retorno: número de itens na fila (>= 0)
//          ERROR se a fila não existir
int queue_size(struct queue_t *queue){
    if(!queue ){
        return ERROR;
    }

    return queue->size;
}

// Põe o iterador no início da fila.
// Retorno: ptr para o item apontado pelo iterador
//          NULL se a fila estiver vazia ou não existir
void *queue_head(struct queue_t *queue){
    if(!queue || queue->size == 0){
        return NULL;
    }
    queue->iterador =0;
    return queue->items[queue->iterador];
}

// Avança o iterador ao próximo item na fila.
// Retorno: ptr para o item apontado pelo iterador após avançar
//          NULL se o iterador passou do último item da fila
//          NULL se a fila estiver vazia ou não existir
void *queue_next(struct queue_t *queue)
{
    if (!queue || queue->size == 0)
        return NULL;

    queue->iterador++;

    if (queue->iterador >= queue->size)
        return NULL;

    return queue->items[queue->iterador];
}

// Informa o item atualmente sob o iterador na fila.
// Retorno: ptr para o item apontado pelo iterador
//          NULL se a fila estiver vazia ou não existir
//          NULL se o iterador passou do fim da fila
void *queue_item(struct queue_t *queue){
    if(!queue ){
        return NULL;
    }
    if(queue->size == 0 || queue->iterador < 0|| queue->iterador >= (queue->size)){
        return NULL;
    }


    return queue->items[queue->iterador];
}

// Imprime os elementos de uma fila; a função externa "func"
// deve ser chamada para imprimir cada item.
// Exemplos de saída, com name == "Frutas":
// Frutas: [ banana pera ameixa uva ] (4 items)
// Frutas: [ ] (0 items)
// Frutas: undef   se queue == NULL
// Frutas: [ undef undef undef ] (3 items)  se func == NULL
void queue_print(char *name, struct queue_t *queue, void(func)(void *)){
   if (!queue) {
        printf("%s: undef\n", name);
        return;
    } else if(!name){
        printf("\nundef\n");
        
    }else{
        printf("%s: [", name);
        if(func == NULL){
            for(int i=0; i<queue->size;i++){
                printf(" undef");
            }
        }else{
            for(int i=0; i<queue->size;i++){
                printf(" ");
                func(queue->items[i]);
                
            }
        }
        printf(" ] (%d items)\n", queue->size);
    }
    
    return;
}
// GRR20240011 GABRIEL OLIVEIRA DE MATOS

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 07/2026

// Este arquivo PODE/DEVE ser alterado.

// Implementação do TAD Mapa de objetos
#include <stdio.h>
#include <stdlib.h>
#include "map.h"


typedef struct map_t {
    void **objetos; 
    int size;
    int quantidade;
} map_t;

// Cria um mapa para até N objetos, com IDs entre 0 e N-1.
// Retorno: ponteiro para o mapa ou NULL (erro)
struct map_t *map_create(int size){
if(size <=0){
    return NULL;
}

    struct map_t *mapa = (map_t*) malloc(sizeof(map_t)); 
    if(!mapa){
        return NULL;
    }

    mapa->objetos = (void**) malloc(size * sizeof(void*));
    if(!mapa->objetos){
        free(mapa);
        return NULL;
    }
    mapa->size= size;
    mapa->quantidade=0;
    for(int i=0; i<size; i++){
        mapa->objetos[i] = NULL;
    }
    return mapa;
}

// destrói um mapa existente (mas não destrói os objetos).
// Retorno: 0 em sucesso ou -1 (erro)
int map_destroy(struct map_t *map){

    if(!map){
        return ERROR;
    }

    free(map->objetos);
    free(map);
  
    return NOERROR;
}


// Registra um objeto no mapa, retornando seu ID.
// Retorno: ID atribuído ao objeto ou -1 (erro).
int map_put(struct map_t *map, void *object)
{
    if (!map || !map->objetos || !object)
        return -1;

    for (int i = 0; i < map->size; i++) {
        if (map->objetos[i] == NULL) {
            map->objetos[i] = object;
            map->quantidade++;
            return i;
        }
    }

    return -1;
}

// Informa o objeto registrado no ID indicado do mapa.
// Retorno: ponteiro para o objeto ou NULL (erro)
void *map_get(struct map_t *map, int id){
    if(!map || !map->objetos){
        return NULL;
    }
    if(id<0 || id>= map->size){
        return NULL;
    }
    return map->objetos[id];
}

// Libera um ID do mapa e devolve o objeto associado.
// Retorno: ponteiro para o objeto ou NULL (erro)
void *map_del(struct map_t *map, int id)
{
    if (!map || !map->objetos)
        return NULL;

    if (id < 0 || id >= map->size)
        return NULL;

    void *objeto = map->objetos[id];

    if (objeto != NULL) {
        map->objetos[id] = NULL;
        map->quantidade--;
    }

    return objeto;
}
// Informa o número de objetos registrados no mapa.
// Retorno: número de objetos registrados ou -1 (erro)
int map_items(struct map_t *map){
    if(!map || !map->objetos){
        return ERROR;
    }
    return map->quantidade;
}

// Informa o número de objetos que o mapa pode registrar.
// Retorno: número de objetos ou -1 (erro)
int map_size(struct map_t *map){
    if(!map || !map->objetos){
        return ERROR;
    }
    return map->size;
}

// Imprime o conteúdo do mapa, no seguinte formato:
// Mapa nulo:     nome: undef
// Mapa vazio:    nome: [ - - - - - ] (0/5)
// Mapa qualquer: nome: [ - * - * * ] (3/5)
// As posições "*" no vetor impresso correspondem aos IDs em uso no mapa;
// (3/5) indica que o mapa tem 3 objetos e pode registrar até 5 objetos.
void map_print(char *name, struct map_t *map){
    if(!name){
        printf("\nNome: undef\n");
        return;
    }
    if( !map || !map->size){
        printf("\n%s: undef\n", name);        
    }
    else{
        printf("\n%s: [", name);
        for(int i=0; i<map->size; i++){
            if(map->objetos[i]){
                printf(" *");    
                //printf(" %d", map->objetos[i]);
            }else{
                printf(" -");
            }
        }
        printf(" ] (%d/%d) \n",  map->quantidade, map->size);
    }

    return; 
}

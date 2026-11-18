// GRR20240011 GABRIEL OLIVEIRA DE MATOS
// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência de filas de mensagens
// inicia o subsistema de filas de mensagens
// (chamada pelo núcleo na inicialização).

#include <stdlib.h>
#include <string.h>
#include "tcb.h"
#include "lib/queue.h"
#include "lib/map.h"
#include "semaphore.h"
#include "memory.h"

struct map_t *map_queues;

struct mqueue_t {
    int max_msgs;
    int msg_size;
    int destruida;

    struct queue_t *messages;

    int mutex;
    int vagas;
    int mensagens;
};


// encerra o subsistema de filas de mensagens
// (chamada pelo núcleo no encerramento).
void mqueue_init()
{
    map_queues = map_create(10);
}

void mqueue_term()
{
    if (!map_queues)
        return;

    int tamanho = map_size(map_queues);

    for (int i = 0; i < tamanho; i++) {
        struct mqueue_t *mqueue = map_get(map_queues, i);

        if (!mqueue)
            continue;
        while (queue_size(mqueue->messages) > 0) {
            void *mensagem = queue_head(mqueue->messages);

            queue_del(mqueue->messages, mensagem);
            mem_free(mensagem);
        }

        sem_destroy(mqueue->mutex);
        sem_destroy(mqueue->vagas);


        sem_destroy(mqueue->mensagens);
        queue_destroy(mqueue->messages);

       mem_free(mqueue);
    }

    map_destroy(map_queues);
    map_queues = NULL;
}


// cria uma fila de mensagens.
// Retorno: descritor da nova fila ou ERROR
int mqueue_create(int max_msgs, int msg_size){
    if (max_msgs <= 0 || msg_size <= 0 || !map_queues)
        return ERROR;

    struct mqueue_t *mqueue = mem_alloc(sizeof(struct mqueue_t));

    if (!mqueue)
        return ERROR;

    mqueue->max_msgs = max_msgs;
    mqueue->msg_size = msg_size;
    mqueue->destruida = 0;

    mqueue->messages = queue_create();

    if (!mqueue->messages) {
       mem_free(mqueue);
        return ERROR;
    }
    mqueue->mutex = sem_create(1);
    mqueue->vagas = sem_create(max_msgs);
    mqueue->mensagens = sem_create(0);

    if (mqueue->mutex < 0 || mqueue->vagas < 0 ||mqueue->mensagens < 0) {

        if (mqueue->mutex >= 0){
            sem_destroy(mqueue->mutex);
        }
        if (mqueue->vagas >= 0){
            sem_destroy(mqueue->vagas);
        }
        if (mqueue->mensagens >= 0){
            sem_destroy(mqueue->mensagens);
        }
        queue_destroy(mqueue->messages);
       mem_free(mqueue);

        return ERROR;
    }

    int id = map_put(map_queues, mqueue);

    if (id < 0) {
        sem_destroy(mqueue->mutex);
        sem_destroy(mqueue->vagas);
        sem_destroy(mqueue->mensagens);

        queue_destroy(mqueue->messages);
       mem_free(mqueue);

        return ERROR;
    }

    return id;
}


// destrói uma fila de mensagens, liberando recursos e tarefas
// Retorno: NOERROR ou ERROR
int mqueue_destroy(int mqueue_id)
{
    if (mqueue_id < 0 || !map_queues)
        return ERROR;

    struct mqueue_t *mqueue = map_get(map_queues, mqueue_id);

    if (!mqueue)
        return ERROR;

    if (sem_down(mqueue->mutex) == ERROR)
        return ERROR;
    mqueue->destruida = 1;

    while (queue_size(mqueue->messages) > 0) {
        void *mensagem = queue_head(mqueue->messages);

        queue_del(mqueue->messages, mensagem);
       mem_free(mensagem);
    }

    sem_up(mqueue->mutex);

    sem_destroy(mqueue->vagas);
    sem_destroy(mqueue->mensagens);
    sem_destroy(mqueue->mutex);

    queue_destroy(mqueue->messages);

    map_del(map_queues, mqueue_id);

   mem_free(mqueue);

    return NOERROR;
}


// envia uma mensagem
// Retorno: NOERROR ou ERROR
int mqueue_send(int mqueue_id, void *msg)
{
    if (!msg || mqueue_id < 0 || !map_queues)
        return ERROR;

    struct mqueue_t *mqueue = map_get(map_queues, mqueue_id);

    if (!mqueue || mqueue->destruida)
        return ERROR;

    if (sem_down(mqueue->vagas) == ERROR)
        return ERROR;
    if (sem_down(mqueue->mutex) == ERROR) {
        sem_up(mqueue->vagas);
        return ERROR;
    }

    if (mqueue->destruida) {
        sem_up(mqueue->mutex);
        sem_up(mqueue->vagas);
        return ERROR;
    }

    void *copia = mem_alloc(mqueue->msg_size);

    if (!copia) {
        sem_up(mqueue->mutex);
        sem_up(mqueue->vagas);
        return ERROR;
    }

    memcpy(copia, msg, mqueue->msg_size);
    queue_add(mqueue->messages, copia);
    sem_up(mqueue->mutex);
    sem_up(mqueue->mensagens);

    return NOERROR;
}


// recebe uma mensagem
// Retorno: NOERROR ou ERROR
int mqueue_recv(int mqueue_id, void *msg)
{
    if (!msg || mqueue_id < 0 || !map_queues)
        return ERROR;

    struct mqueue_t *mqueue = map_get(map_queues, mqueue_id);

    if (!mqueue || mqueue->destruida)
        return ERROR;

    if (sem_down(mqueue->mensagens) == ERROR)
        return ERROR;

    if (sem_down(mqueue->mutex) == ERROR) {
        sem_up(mqueue->mensagens);
        return ERROR;
    }
    if (mqueue->destruida) {
        sem_up(mqueue->mutex);
        return ERROR;
    }

    void *mensagem = queue_head(mqueue->messages);

    if (!mensagem) {
        sem_up(mqueue->mutex);
        return ERROR;
    }

    memcpy(msg, mensagem, mqueue->msg_size);

    queue_del(mqueue->messages, mensagem);
    mem_free(mensagem);
    sem_up(mqueue->mutex);
    sem_up(mqueue->vagas);

    return NOERROR;
}


// retorna o numero de mensagens em uma fila
// Retorno: número >= 0 ou ERROR
int mqueue_msgs(int mqueue_id)
{
    if (mqueue_id < 0 || !map_queues)
        return ERROR;

    struct mqueue_t *mqueue = map_get(map_queues, mqueue_id);

    if (!mqueue || mqueue->destruida)
        return ERROR;

    if (sem_down(mqueue->mutex) == ERROR)
        return ERROR;

    if (mqueue->destruida) {
        sem_up(mqueue->mutex);
        return ERROR;
    }

    int quantidade = queue_size(mqueue->messages);

    sem_up(mqueue->mutex);

    return quantidade;
}
// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Semáforos e spinlocks

// Essa estrutura contém um ID, um contador, uma fila e um inteiro para o controle de concorrência no acesso ao semáforo (spinlock). 
#include "lib/queue.h"
#include "lib/map.h"
#include "ctx.h"
#include "task.h"
#include "dispatcher.h"
#include "task.h"
#include <stdlib.h>
#include "memory.h"

struct semaphore_t{
    int id;
    int count;
    struct queue_t * fila;
    int spin;
};
int spin_mapa = 0;
struct map_t * map_semaforos;

void sem_init()
{
    map_semaforos = map_create(10);
    if(!map_semaforos){
        return;
    }

    

}

void sem_term()
{
    if (map_semaforos == NULL)
        return;

    int tamanho = map_size(map_semaforos);

    for (int i = 0; i < tamanho; i++) {
        struct semaphore_t *sem = map_get(map_semaforos, i);

        if (sem != NULL) {
            queue_destroy(sem->fila);
            mem_free(sem);
        }
    }

    map_destroy(map_semaforos);
    map_semaforos = NULL;
}


void spin_lock(int *lock)
{
    if (lock == NULL)
        return;

    while (__sync_fetch_and_or(lock, 1));
}


void spin_unlock(int *lock)
{
    if (lock == NULL)
        return;

    __sync_lock_release(lock);
}


// Cria um novo semáforo, inicializado com value >= 0.
// Retorno: descritor do semáforo ou -1 (erro).
int sem_create(int value)
{
    if (value < 0 || map_semaforos == NULL)
        return ERROR;

    struct semaphore_t *sem = mem_alloc(sizeof(struct semaphore_t));

    if (sem == NULL)
        return ERROR;

    sem->count = value;
    sem->spin = 0;
    sem->fila = queue_create();

    if (sem->fila == NULL) {
        mem_free(sem);
        return ERROR;
    }

    sem->id = map_put(map_semaforos, sem);

    if (sem->id < 0) {
        queue_destroy(sem->fila);
        mem_free(sem);
        return ERROR;
    }

    return sem->id;
}

// destrói um semáforo, liberando recursos e tarefas bloqueadas
// Retorno: NOERROR (0) ou ERROR (<0)
int sem_destroy(int sem_id)
{
    if (sem_id < 0 || map_semaforos == NULL)
        return ERROR;

    spin_lock(&spin_mapa);

    struct semaphore_t *sem = map_get(map_semaforos, sem_id);

    if (sem == NULL) {
        spin_unlock(&spin_mapa);
        return ERROR;
    }

    spin_lock(&sem->spin);

    map_del(map_semaforos, sem_id);

    spin_unlock(&spin_mapa);

    while (queue_size(sem->fila) > 0) {
        struct task_t *tarefa = queue_head(sem->fila);

        if (tarefa == NULL)
            break;

        queue_del(sem->fila, tarefa);

        tarefa->semaforo_res = ERROR;
        task_awake(tarefa);
    }

    queue_destroy(sem->fila);

    spin_unlock(&sem->spin);

    mem_free(sem);

    return NOERROR;
}


// Requisita acesso a um semáforo
// Retorno: NOERROR (0) ou ERROR (<0)
int sem_down(int sem_id)
{
    if (sem_id < 0 || map_semaforos == NULL)
        return ERROR;

    spin_lock(&spin_mapa);

    struct semaphore_t *sem = map_get(map_semaforos, sem_id);

    if (sem == NULL) {
        spin_unlock(&spin_mapa);
        return ERROR;
    }

    spin_lock(&sem->spin);
    spin_unlock(&spin_mapa);

    sem->count--;

    if (sem->count < 0) {
        struct task_t *tarefa = task_atual;

        tarefa->semaforo_res = NOERROR;

        queue_add(sem->fila, tarefa);

        tarefa->status = TASK_SUSPENSA;
        queue_add(fila_suspensas, tarefa);

        spin_unlock(&sem->spin);

        task_switch(NULL);

        return tarefa->semaforo_res;
    }

    spin_unlock(&sem->spin);

    return NOERROR;
}

// libera o acesso a um semáforo
// Retorno: NOERROR (0) ou ERROR (<0)
int sem_up(int sem_id)
{
    if (sem_id < 0 || map_semaforos == NULL)
        return ERROR;

    spin_lock(&spin_mapa);

    struct semaphore_t *sem = map_get(map_semaforos, sem_id);

    if (sem == NULL) {
        spin_unlock(&spin_mapa);
        return ERROR;
    }

    spin_lock(&sem->spin);
    spin_unlock(&spin_mapa);

    sem->count++;

    if (sem->count <= 0) {
        struct task_t *tarefa = queue_head(sem->fila);

        if (tarefa != NULL) {
            queue_del(sem->fila, tarefa);

            tarefa->semaforo_res = NOERROR;
            task_awake(tarefa);
        }
    }

    spin_unlock(&sem->spin);

    return NOERROR;
}
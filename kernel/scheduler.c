// GRR20240011 GABRIEL OLIVEIRA DE MATOS

// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Escalonador de tarefas prontas.
#include "tcb.h"
#include "dispatcher.h"
#include "task.h"
#include "lib/queue.h"




// inicia o subsistema do escalonador de tarefas
// (chamada pelo núcleo na inicialização).
void sched_init(){
return;
}

//!=================================================================================

// encerra o subsistema do escalonador de tarefas
// (chamada pelo núcleo no encerramento).
void sched_term(){
    return;
}

//!=================================================================================

// muda a prioridade de uma tarefa (da tarefa atual se task == NULL)
void sched_setprio(struct task_t *task, int prio)
{
    if (prio > 20)
        prio = 20;

    if (prio < -20)
        prio = -20;

    if (task == NULL){
        task = task_atual;
    }
    task->prioridade = prio;
    task->prioridade_dinamica = prio;
}

//!=================================================================================

// obtém a prioridade de uma tarefa (da tarefa atual se task == NULL)
int sched_getprio(struct task_t *task){
    if(task == NULL){
        return task_atual->prioridade;
    }

    return task->prioridade;
}

//!=================================================================================


struct task_t *scheduler(struct queue_t *ready_queue){
    if (!ready_queue || ready_queue->size == 0){
        return NULL;
    }
    struct task_t *px = ready_queue->items[0];
    for (int i = 1; i < ready_queue->size; i++){
        struct task_t *task = ready_queue->items[i];

        if (task->prioridade_dinamica < px->prioridade_dinamica){
            px = task;
        }
    }

    for (int i = 0; i < ready_queue->size; i++){
        struct task_t *task = ready_queue->items[i];

        if (task != px && task->prioridade_dinamica > -20){
            task->prioridade_dinamica--;
        }
    }
    px->prioridade_dinamica = px->prioridade;

    return px;
}
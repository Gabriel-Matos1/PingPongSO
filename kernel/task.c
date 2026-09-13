// GRR20240011 GABRIEL OLIVEIRA DE MATOS

// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência básica de tarefas.


// inicializa o subsistema de tarefas.
// (chamada pelo núcleo na inicialização).
#define _POSIX_C_SOURCE 200112L
#include "task.h"
#include "dispatcher.h"
#include <stdlib.h>

struct task_t task_kernel;
struct task_t *task_atual;
#define STACK_SIZE 64 * 1024
static int px_id = 1;

//!=================================================================================


void task_init(){

    task_kernel.name = "kernel";
    task_kernel.id=0;
    task_kernel.status= TASK_EXECUTANDO;
    px_id = 1;

    task_atual = &task_kernel;
}   

//!=================================================================================

// encerra o subsistema de tarefas.
// (chamada pelo núcleo no encerramento).
void task_term(){
    
}

//!=================================================================================

// cria uma nova tarefa: "name" é o nome da tarefa, "entry" é a função que
// ela irá executar e "arg" aponta para o valor recebido por "entry" ao
// iniciar (pode ser NULL).
// Retorno: ptr para a tarefa ou NULL se houver erro.
struct task_t * task_create(char *name, void (*entry)(void *), void *arg){
    if(!entry){
        return NULL;
    }

    struct task_t *task = (struct task_t*) malloc(sizeof(struct task_t));
    
    if (!task)
        return NULL;
    
    task->name = name;
    task->status = TASK_CRIADA;
    task->context.size = STACK_SIZE;
    task->id = px_id++;
    task->parente = task_atual;
    task->prioridade = 0;
    task->prioridade_dinamica = 0;
    if (posix_memalign(&task->context.stack,16,task->context.size) != 0){
        
        free(task);
        return NULL;
    }

    int resultado_ctx = ctx_create(&task->context, entry, arg, task->context.stack, task->context.size);

    if (resultado_ctx == ERROR) {
        free(task->context.stack);
        free(task);
        return NULL;
    }

        
    task->status = TASK_PRONTA;
    queue_add(fila_prontas, task);
    return task;
}
//!=================================================================================


// destrói uma tarefa e libera seus recursos; somente deve atuar sobre tarefas
// terminadas. Retorno: NOERROR (0) ou ERROR (<0).
int task_destroy(struct task_t *task){
    if(!task){
        return ERROR;
    }
    if(task->status != TASK_FINALIZADA){
        return ERROR;
    }
    
    
    free(task->context.stack);
    
    free(task); 
    return NOERROR;
}

//!=================================================================================


// informa o ID de uma tarefa (ou da tarefa atual se task == NULL)
int task_id(struct task_t *task){
    if (!task)
        return task_atual->id;

    return task->id;
}

//!=================================================================================


// informa o nome de uma tarefa (ou da tarefa atual se task == NULL)
char *task_name(struct task_t *task){
    if (!task)
        return task_atual->name;

    return task->name;
}

//!=================================================================================


void task_exit(int exit_code)
{
    task_atual->status = TASK_FINALIZADA;

    task_switch(NULL);
}

//!=================================================================================


// a tarefa atual libera a CPU e volta para a fila de prontas; a execução
// retorna ao núcleo/dispatcher.

void task_yield()
{
    task_atual->status = TASK_PRONTA;

    queue_add(fila_prontas, task_atual);

    task_switch(NULL);
}

//!=================================================================================

/*
// suspende a tarefa atual até que a tarefa task termine; a execução retorna
// ao núcleo/dispatcher. Se a tarefa task já terminou, retorna sem suspender.
// Retorno: exit code tarefa que terminou ou ERROR.
int task_wait(struct task_t *task){
    if(task->status == TASK_FINALIZADA){
        return task->id;
    }

    task_atual->status = TASK_SUSPENSA;
    queue_add(fila_suspensas, task_atual);
//precisa rodar a task aqui mesmo ou só deixar a outra suspensa?
    task_run(task);

    return;
}

// suspende a tarefa atual por t milissegundos; a execução retorna ao
// núcleo/dispatcher.
void task_sleep(int t){

}*/

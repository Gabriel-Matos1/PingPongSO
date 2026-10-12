// GRR20240011 GABRIEL OLIVEIRA DE MATOS

// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Dispatcher: gerencia os estados das tarefas.

// inicia o subsistema dispatcher


#include "lib/queue.h"
#include "dispatcher.h"
#include "task.h"
#include "tcb.h"
#include "queue.h"
#include "scheduler.h"
#include "time.h"
#include <stdlib.h>
struct queue_t *fila_prontas;
struct queue_t *fila_suspensas;

//!=================================================================================

/*/
struct queue_t* fila_criado;
*/
// (chamada pelo núcleo na inicialização).
void dispatcher_init(){

    fila_prontas = queue_create();
    
    fila_suspensas = queue_create();
    
/*    fila_terminado = queue_create();
    fila_criado = queue_create();
*/
}

//!=================================================================================

// encerra o subsistema dispatcher
// (chamada pelo núcleo no encerramento).
void dispatcher_term(){
    queue_destroy(fila_prontas);
    queue_destroy(fila_suspensas);
  /*  queue_destroy(fila_terminado);
    queue_destroy(fila_criado);
*/
}

//!=================================================================================


// transfere a CPU da tarefa atual para outra tarefa; se task_id == 0, 
// transfere para o núcleo. Ignora sem erro se "task" já tiver terminado.
// Retorno: NOERROR (0) ou ERROR (<0)
int task_switch(struct task_t *task)
{
    struct task_t *anterior = task_atual;

    if (task == NULL)
        task = &task_kernel;
    else if (task->status == TASK_FINALIZADA)
        return NOERROR;

    unsigned int agora = time();

    // Contabiliza o tempo de CPU de quem está deixando a CPU.
    anterior->tempo_processamento += agora - anterior->tempo_temp_cpu;

    // Prepara a tarefa que vai receber a CPU.
    task->status = TASK_EXECUTANDO;
    task->tempo_temp_cpu = agora;

    // Conta cada vez que o núcleo recebe a CPU.
    if (task == &task_kernel)
        task_kernel.ativacoes++;

    task_atual = task;

    return ctx_switch(&anterior->context, &task->context);
}

//!=================================================================================

// executa a tarefa indicada: retira-a da fila de prontas, muda seu status
// para RODANDO e transfere a CPU para ela.
void task_run(struct task_t *task){
    queue_del(fila_prontas, task);
    task->ativacoes++;
    task->tempo_temp_cpu = time();
    task->status = TASK_EXECUTANDO;
    task->quantum = QUANTUM;
    int result = task_switch(task);
    if(result <0){
        exit(-1);
    }
}

//!=================================================================================

void user_main(void *arg);

// executa o dispatcher (chamada pelo núcleo após a inicialização).
void  dispatcher(){
    struct task_t *task_user;

    task_user = task_create("user_main", user_main, NULL);

    while ( fila_prontas->size > 0 ){
        struct task_t *proximo = scheduler(fila_prontas);      
        if(proximo != NULL){
            task_run(proximo);
            switch (proximo->status){

            case TASK_PRONTA:
                break;
            case TASK_EXECUTANDO:
                continue;
            
            case TASK_FINALIZADA:
            //PPOS: task   5 (ping), 13833 ms run,  2772 ms cpu,   278 acts, exit code   0
           printk("PPOS: task %3d (%s), %u ms run, %u ms cpu, %4u acts, exit code %3d\n",
       proximo->id,
       proximo->name,
       proximo->tempo_finalizacao,
       proximo->tempo_processamento,
       proximo->ativacoes,
       proximo->exit_code);
       break;
            case TASK_CRIADA:
                
                break;

            case TASK_SUSPENSA:
               
                break;

            default:
                break;
            }
        }
    }
    task_destroy(task_user);

    task_kernel.tempo_processamento += time() - task_kernel.tempo_temp_cpu;

    task_kernel.tempo_finalizacao =time() - task_kernel.tempo_criacao;

    task_kernel.exit_code = 0;

    printk("PPOS: task %3d (%s), %u ms run, %u ms cpu, %4u acts, exit code %3d\n",
        task_kernel.id,
        task_kernel.name,
        task_kernel.tempo_finalizacao,
        task_kernel.tempo_processamento,
        task_kernel.ativacoes,
        task_kernel.exit_code);
}

//!=================================================================================


// suspende a tarefa atual: retira-a da fila de prontas, muda seu status para
// SUSPENSA, a insere na fila "queue" (se não for NULL) e retorna ao dispatcher.
void task_suspend(struct queue_t *queue)
{

    task_atual->status = TASK_SUSPENSA;

    if (queue != NULL) queue_add(queue, task_atual);

    task_switch(NULL);
}

//!=================================================================================

// acorda uma tarefa: retira-a da fila onde se encontra suspensa (se estiver
// em uma fila), muda seu status para PRONTA e a insere na fila de prontas,
// para retomar (ou iniciar) sua execução.
void task_awake(struct task_t *task){
    
    queue_del(fila_suspensas, task);
    task->status = TASK_PRONTA;
    queue_add(fila_prontas, task);
    return;
}


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
#include "time.h"
struct task_t task_kernel;
struct task_t *task_atual;
#define STACK_SIZE 64 * 1024
static int px_id = 1;

//!=================================================================================


void task_init()
{
    task_kernel.name = "kernel";
    task_kernel.id = 0;
    task_kernel.tipo_user_ou_sistema = 0;
    task_kernel.status = TASK_EXECUTANDO;

    task_kernel.tempo_criacao = time();
    task_kernel.tempo_processamento = 0;
    task_kernel.tempo_finalizacao = 0;
    task_kernel.tempo_temp_cpu = time();
    task_kernel.ativacoes = 1;
    task_kernel.exit_code = 0;

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
struct task_t *task_create(char *name, void (*entry)(void *), void *arg)
{
    if (entry == NULL)
        return NULL;

    struct task_t *task = malloc(sizeof(struct task_t));

    if (task == NULL)
        return NULL;

    task->name = name;
    task->tipo_user_ou_sistema = 1;
    task->status = TASK_CRIADA;
    task->context.size = STACK_SIZE;
    task->id = px_id++;
    task->parente = task_atual;

    task->prioridade = 0;
    task->prioridade_dinamica = 0;

    task->tempo_criacao = time();
    task->tempo_processamento = 0;
    task->tempo_finalizacao = 0;
    task->tempo_temp_cpu = 0;
    task->ativacoes = 0;
    task->exit_code = 0;

    task->qtde_tasks_esperando = 0;
    task->tamanho_fila_tasks_esperando = 10;

    task->tasks_esperando = malloc(
        task->tamanho_fila_tasks_esperando * sizeof(struct task_t *)
    );

    if (task->tasks_esperando == NULL) {
        free(task);
        return NULL;
    }

    if (posix_memalign(&task->context.stack, 16, task->context.size) != 0) {
        free(task->tasks_esperando);
        free(task);
        return NULL;
    }

    int resultado_ctx = ctx_create(
        &task->context,
        entry,
        arg,
        task->context.stack,
        task->context.size
    );

    if (resultado_ctx == ERROR) {
        free(task->context.stack);
        free(task->tasks_esperando);
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
int task_destroy(struct task_t *task)
{
    if (task == NULL)
        return ERROR;

    if (task->status != TASK_FINALIZADA)
        return ERROR;

    free(task->tasks_esperando);
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
    struct task_t *terminada = task_atual;


    terminada->tempo_finalizacao =time() - terminada->tempo_criacao;

    terminada->exit_code = exit_code;
    terminada->status = TASK_FINALIZADA;

    for (int i = 0; i < terminada->qtde_tasks_esperando; i++) {
        struct task_t *esperando = terminada->tasks_esperando[i];

        if (esperando != NULL && esperando->status == TASK_SUSPENSA)
            task_awake(esperando);
    }

    task_switch(NULL);
}
//!=================================================================================


// a tarefa atual libera a CPU e volta para a fila de prontas; a execução
// retorna ao núcleo/dispatcher.

void task_yield(void)
{
    struct task_t *anterior = task_atual;

    anterior->status = TASK_PRONTA;
    queue_add(fila_prontas, anterior);

    task_switch(NULL);
}

//!=================================================================================


// suspende a tarefa atual até que a tarefa task termine; a execução retorna
// ao núcleo/dispatcher. Se a tarefa task já terminou, retorna sem suspender.
// Retorno: exit code tarefa que terminou ou ERROR.
int task_wait(struct task_t *task)
{
    if (task == NULL)
        return ERROR;

    if (task == task_atual)
        return ERROR;

    if (task->status == TASK_FINALIZADA)
        return task->exit_code;

    if (task->qtde_tasks_esperando >=
        task->tamanho_fila_tasks_esperando) {

        int nova_capacidade =
            task->tamanho_fila_tasks_esperando + 10;

        struct task_t **novo_vetor = realloc(
            task->tasks_esperando,
            nova_capacidade * sizeof(struct task_t *)
        );

        if (novo_vetor == NULL)
            return ERROR;

        task->tasks_esperando = novo_vetor;
        task->tamanho_fila_tasks_esperando = nova_capacidade;
    }

    task->tasks_esperando[task->qtde_tasks_esperando] = task_atual;
    task->qtde_tasks_esperando++;

    task_suspend(fila_suspensas);

    return task->exit_code;
}
// suspende a tarefa atual por t milissegundos; a execução retorna ao
// núcleo/dispatcher.
//void task_sleep(int t){

//}

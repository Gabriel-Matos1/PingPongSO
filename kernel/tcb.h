// GRR20240011 GABRIEL OLIVEIRA DE MATOS

// PingPongOS - PingPong Operating System
// © Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 06/2026

// Este arquivo PODE/DEVE ser alterado.

// Descritor de tarefas (TCB - Task Control Block).

#ifndef __PPOS_TCB__
#define __PPOS_TCB__
#include "ctx.h"
#include "lib/queue.h"

#define TASK_CRIADA 0
#define TASK_PRONTA 1
#define TASK_EXECUTANDO 2
#define TASK_FINALIZADA 3
#define TASK_SUSPENSA 4
extern struct task_t task_kernel;
extern struct task_t *task_atual;
extern struct queue_t* fila_prontas;
extern struct queue_t* fila_suspensas;

//plantas que vamos passar a ensinar os GIS a atender: Palmela, Pune e Bentley.  

typedef struct queue_t {
    int iterador;
    void **items;
    int size;
    int capacidade; 
} queue_t;


// Task Control Block (TCB), infos sobre uma tarefa
struct task_t
{
    int id;         // identificador da tarefa
    char *name;     // nome da tarefa
    struct ctx_t context;  // contexto da tarefa
    int status;     // criada(0), pronta(1), executando(2), terminada(3) 
    struct task_t *parente;
    int prioridade;
    int prioridade_dinamica;
    int quantum;
     int tipo_user_ou_sistema; // usuario(1) e sistema(0)
     int tempo_criacao;
     int tempo_processamento;
     int ativacoes;
     int tempo_finalizacao;
    int exit_code;
     int tempo_temp_cpu;
    struct task_t **tasks_esperando;
    int qtde_tasks_esperando;
     int tamanho_fila_tasks_esperando;
     int dormindo; // sim, por tempo determinado(-1) else  suspensa sem prazo especifico(0)
     int acordar_quando;
     int semaforo_res;
    // ...             // demais informações, a completar
};

#endif
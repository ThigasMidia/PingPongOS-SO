// PingPongOS - PingPong Operating System
// © Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 06/2026

// Este arquivo PODE/DEVE ser alterado.
// Eduarda Saibert - GRR20232368
// Kawai Barzotto - GRR20235687

// Descritor de tarefas (TCB - Task Control Block).

#ifndef __PPOS_TCB__
#define __PPOS_TCB__

#include "ctx.h"

extern struct queue_t *queue_ready; 
extern struct task_t task_kernel;		// Variável global com a tarefa inicial (kernel)
extern struct task_t* curr_task;		// Tarefa atual (contexto atual)

// Enum que define os status de uma tarefa
typedef enum {
	READY,
	RUNNING,
	SUSPENDED,
	FINISHED
} t_status;

typedef struct ctx_t ctx_t;

// Task Control Block (TCB), infos sobre uma tarefa
struct task_t
{
    int id;					// identificador da tarefa
    char *name;				// nome da tarefa
    ctx_t context;			// contexto da tarefa
    t_status status;		// pronta, executando, ...
	char* stack;			// stack
	
	struct task_t* parent;	// tarefa pai
};

#endif

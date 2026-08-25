// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.
// Eduarda Saibert - GRR20232368
// Kawai Barzotto - GRR20235687

// Dispatcher: gerencia os estados das tarefas.

#include "dispatcher.h"
#include "task.h"
#include "lib/pplibc.h"
#include "macros.h"
#include <stdio.h>

extern void user_main (void *arg);

void dispatcher_init()
{
	ppos_debug("subsystem dispatcher initiated\n");/*
	struct task_t* task_user = task_create("user_main", user_main, NULL);
	if (!task_user) return;

	int status = task_switch(task_user);
	if (!status) {
		task_destroy(task_user);
		return;
	}

	task_destroy(task_user);*/
}

void dispatcher_term()
{
}

//Dispatcher simples para execucao de tarefa user_main.
void dispatcher()
{
	ppos_debug("dispatcher initiated\n");
	struct task_t* task_user;
	task_user = task_create("user_main", user_main, NULL);
	task_switch(task_user);
	ppos_debug("dispatcher stopping, no more user tasks\n");
	task_destroy(task_user);
}

int task_switch(struct task_t *task) {
	// Realiza a troca de contexto entre tarefas
	// Se task=NULL, o contexto troca para o pai da tarefa em execução

	struct task_t* next_task = task;
	struct task_t* old_task = curr_task;

	if (!task) 
		next_task = curr_task->parent;	
	if (!next_task) return ERROR;

	curr_task = next_task;

	ppos_debug("task %d (%s) switched to task %d (%s)\n", old_task->id, old_task->name, next_task->id, next_task->name);

	old_task->status = READY;
	next_task->status = RUNNING;

	int status = ctx_switch(&(old_task->context), &(next_task->context));

	if (status == ERROR){
		curr_task = old_task;
		next_task->status = READY;
		old_task->status = RUNNING;
		return ERROR;
	}

	return NOERROR;
}

// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.
// Eduarda Saibert - GRR20232368
// Kawai Barzotto - GRR20235687

// Dispatcher: gerencia os estados das tarefas.

#include "dispatcher.h"
#include "task.h"
#include "scheduler.h"
#include "time.h"
#include "lib/pplibc.h"
#include "macros.h"
#include "lib/queue.h"
#include <stdio.h>

extern void user_main (void *arg);

struct queue_t *queue_ready = NULL;
struct queue_t *queue_sleeping = NULL;

void dispatcher_init()
{
	ppos_debug("subsystem dispatcher initiated\n");
	queue_ready = queue_create();
	queue_sleeping = queue_create();
}

void dispatcher_term()
{		
	ppos_debug("dispatcher stopping, no more user tasks\n");
	queue_destroy(queue_ready);
	queue_destroy(queue_sleeping);
}

void tick_handler_disp(){
	// Função ativada a cada tick
	if(curr_task->user) {
		curr_task->quantum--;

		if(curr_task->quantum <= 0) {
			ppos_debug("tick handler: task %d (%s) preempted\n", curr_task->id, curr_task->name);
			task_yield();
		}
	}
}

// Função que verifica se alguma tarefa pode ser acordada
void wakeup_tasks(){
	struct task_t *temp = queue_head(queue_sleeping);
	int curr_time = time();
	hw_irq_enable(0);

	while (temp != NULL){
		if (curr_time >= temp->wakeup_time){
			queue_del(queue_sleeping, temp);
			task_awake(temp);
			temp = queue_item(queue_sleeping);
		} else temp = queue_next(queue_sleeping);
	}

	hw_irq_enable(1);
}

//Dispatcher simples para execucao de tarefa user_main.
void dispatcher()
{
	ppos_debug("dispatcher started\n");
	struct task_t* next_task = NULL;

	struct task_t *task_user = task_create("user_main", user_main, NULL);
	while(queue_size(queue_ready) > 0 || queue_size(queue_sleeping) > 0)
	{
		wakeup_tasks();

		next_task = scheduler(queue_ready);
		if (next_task) task_run(next_task);
	
	}
    task_destroy(task_user);
}

int task_switch(struct task_t *task) {
	// Realiza a troca de contexto entre tarefas
	// Se task=NULL, o contexto troca para o pai da tarefa em execução

	struct task_t* next_task = task;
	struct task_t* old_task = curr_task;

	if (!task) next_task = curr_task->parent;	

	if (!next_task) return ERROR;

	curr_task = next_task;
	unsigned int curr_time = time();

	old_task->cpu_time += (curr_time - old_task->last_time_used);
	curr_task->last_time_used = curr_time;

	next_task->quantum = QUANTUM;
	next_task->acts++;

	ppos_debug("task %d (%s) switched to task %d (%s)\n", old_task->id, old_task->name, next_task->id, next_task->name);

	if (old_task->status != FINISHED) old_task->status = READY;

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


void task_run(struct task_t *task)
{
	if(queue_del(queue_ready, task) == ERROR) return;
	
	task->status = RUNNING;
	task_switch(task);
}


void task_suspend(struct queue_t *queue)
{	
	if(!curr_task || !queue) return;
	ppos_debug("Suspending task %d\n", curr_task->id);

	curr_task->status = SUSPENDED;
	queue_add(queue, curr_task);
	task_switch(&task_kernel);
}


void task_awake(struct task_t *task)
{
    
	ppos_debug("task %d (%s) woke up.\n", task->id, task->name);
	task->status = READY;
	queue_add(queue_ready, task);
}

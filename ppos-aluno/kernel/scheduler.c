// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.
// Eduarda Saibert - GRR20232368
// Kawai Barzotto - GRR20235687


// Escalonador de tarefas prontas.

#include "task.h"
#include "dispatcher.h"
#include "lib/pplibc.h"
#include "macros.h"

#define AGING_STEP 1	

void sched_init()
{
	ppos_debug("subsystem scheduler initiated\n");
}

void sched_term()
{
	ppos_debug("scheduler stopping\n");
}


struct task_t *select_next_task(struct queue_t *ready_queue){
	struct task_t* curr = (struct task_t*) queue_head(ready_queue); 
	struct task_t* best = NULL;

	while (curr != NULL) {
		if (best == NULL || curr->dynamic_priority < best->dynamic_priority)
			best = curr;

		curr = (struct task_t*) queue_next(ready_queue);
	}
	return best;
}

struct task_t *scheduler(struct queue_t *ready_queue)
{
	if (!ready_queue) return NULL; 

	// Seleciona próxima tarefa com base nas propriedades dinâmicas
	struct task_t* best = select_next_task(ready_queue); 
	if (!best) return NULL;

	// Envelhece as tarefas não escolhidas
	struct task_t* curr = (struct task_t*) queue_head(ready_queue);
	while (curr != NULL){
		if (curr != best)
			curr->dynamic_priority -= AGING_STEP;
		curr = (struct task_t*) queue_next(ready_queue);
	}

	// Tarefa escolhida tem prioridade dinâmica resetada
	best->dynamic_priority = best->static_priority;

	ppos_debug("selected task from the queue: %d\n", task_id(best));
	return best;
}

void sched_setprio(struct task_t *task, int prio)
{
    if(!task) task = curr_task;

	// Cap para a variável de prioridade
	if (prio > 20) prio = 20;
	if (prio < -20) prio = -20;

    task->static_priority = prio;
    task->dynamic_priority = prio;

	ppos_debug("setting static priority %d on task %d\n", prio, task_id(task));
}

int sched_getprio(struct task_t *task)
{
	// Caso task seja nulo, retorna a prioridade estática da tarefa atual
    if(!task) return curr_task->static_priority;
    return task->static_priority;
}

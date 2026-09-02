// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.
// Eduarda Saibert - GRR20232368
// Kawai Barzotto - GRR20235687


// Escalonador de tarefas prontas.

#include "task.h"
#include "dispatcher.h"

void sched_init()
{
}

void sched_term()
{
}

struct task_t *scheduler(struct queue_t *ready_queue)
{
    
}

void sched_setprio(struct task_t *task, int prio)
{
    if(!task) task = curr_task;

    task->static_priority = prio;
    task->dynamic_priority = prio;
}

int sched_getprio(struct task_t *task)
{
    if(!task) return curr_task->static_priority;

    return task->static_priority;
}
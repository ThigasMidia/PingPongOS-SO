// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.
// Eduarda Saibert - GRR20232368
// Kawai Barzotto - GRR20235687


// Gerência básica do tempo.
#include "hardware/cpu.h"
#include "macros.h"
#include "time.h"
#include "dispatcher.h"

#define QUANTUM 10

int ppos_time = 0;

extern struct task_t* curr_task;

void task_yield();
void tick_handler();

void time_init()
{
    hw_irq_enable(1);
    hw_timer(1, 1);
    hw_irq_handle(IRQ_TIMER, tick_handler);
}

void time_term()
{
    hw_irq_enable(0);
    hw_timer(0, 0);
}

//Funcao handler de IRQ_TIMER
void tick_handler()
{
    ppos_time++;

    if(curr_task->user) curr_task->quantum--;

    if(curr_task->quantum <= 0) {
        ppos_debug("tick handler: task %d (%s) preempted\n", curr_task->id, curr_task->name);
        task_yield();
    }
    
}

unsigned int time()
{
    return (ppos_time);
}


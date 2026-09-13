// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.
// Eduarda Saibert - GRR20232368
// Kawai Barzotto - GRR20235687


// Gerência básica do tempo.
#include "hardware/cpu.h"
#include "macros.h"
#include "time.h"
//#include "dispatcher.h"

unsigned int ppos_time;

void tick_handler();				// definido em time.c
void tick_handler_disp();			// definido em dispatcher.c

void time_init()
{
	// inicializa variável global
	ppos_time = 0;

	// inicializa funções do emulador de hardware
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
	tick_handler_disp();
}

unsigned int time()
{
    return (ppos_time);
}


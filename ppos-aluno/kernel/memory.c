// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.
// Eduarda Saibert - GRR20232368
// Kawai Barzotto - GRR20235687

// Alocador básico de memória heap.

// somente para a implementação trivial
#include <stdlib.h>
#include "ctx.h"

// implementação trivial, a ser substituída
void *mem_alloc(int size)
{
    return (malloc(size));
}

// implementação trivial, a ser substituída
int mem_free(void *addr)
{
    free(addr);
    return (NOERROR);
}

void mem_init()
{
}

void mem_term()
{
}


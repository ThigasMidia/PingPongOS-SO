// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência básica de tarefas.

#ifndef STACKSIZE
#define STACKSIZE 32 * 1024		// Tamanho da pilha das tarefas
#endif

#include "task.h"
#include "memory.h"
#include "lib/pplibc.h"

struct task_t task_kernel;		// Variável global com a tarefa inicial (kernel)
struct task_t* curr_task;		// Tarefa atual (contexto atual)

int next_t_id = 0;						// Próximo ID a ser definido a uma tarefa


void task_init()
{
	// Inicia uma tarefa do kernel "task_kernel", com nome “kernel” e ID 0.
	
	task_kernel.id = 0;						
	task_kernel.name = "kernel";
	task_kernel.status = RUNNING;
	task_kernel.stack = NULL;
	task_kernel.parent = NULL;

	next_t_id++;				// Incrementa o próximo ID
	
	curr_task = &task_kernel;	// Define que a tarefa atual é a tarefa do kernel
	// contexto (?)
}


void task_term()
{
}


struct task_t * task_create(char *name, void (*entry)(void *), void *arg){
	// Cria uma nova tarefa, retornando a nova estrutura
	// Retorno: struct task_t (tarefa) ou NULL
	
	struct task_t* task = (struct task_t*) mem_alloc(sizeof(struct task_t));
	if (!task) return NULL;

	task->name = name;
	task->id = next_t_id;
	task->status = READY;
	task->parent = curr_task;

	next_t_id++;						// Incrementa o próximo ID
	
    char* stack = mem_alloc(STACKSIZE);	// Aloca a pilha da tarefa
	if (!stack){
		mem_free(task);						// (!) Verificar se é necessário trocar por destroy_task()
		return NULL;
	}

    //VALGRIND_STACK_REGISTER(stack, stack + STACKSIZE);	// Registra pilha no Valgrind

	static struct ctx_t ctx;			// Cria contexto da tarefa
    int status = ctx_create(&ctx, entry, arg, stack, STACKSIZE);
	if (status == ERROR){
		mem_free(task);						// (!) Verificar se é necessário trocar por destroy_task()
		mem_free(stack);
		return NULL;
	}

	task->context = ctx;
	task->stack = stack;

	return task;
}


int task_destroy(struct task_t *task){
	// Destrói uma tarefa
	// Retorno: ERROR ou NOERROR
	
	if (!task) return ERROR;
	if (task->stack) mem_free(task->stack);
	mem_free(task);

	return NOERROR;
}


int task_id(struct task_t *task){
	// informa o ID de uma tarefa (ou da tarefa atual se task == NULL)
	
	if (!task) return curr_task->id;
	return task->id;
}


char *task_name(struct task_t *task){
	// informa o nome de uma tarefa (ou da tarefa atual se task == NULL)

	if (!task) return curr_task->name;
	return task->name;
}



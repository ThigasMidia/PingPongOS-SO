// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.
// Eduarda Saibert - GRR20232368
// Kawai Barzotto - GRR20235687

// Semáforos e spinlocks

#include "task.h"
#include "memory.h"
#include "queue.h"
#include <assert.h>
#include "lib/pplibc.h"
#include "syscall.h"

#define NOERROR 0
#define ERROR -1

extern void task_suspend(struct queue_t *queue);
extern void task_awake(struct task_t *task);

struct semaphore **semaphore_array;
int next_semaphore_id = 0;

struct semaphore
{
    int s_lock;                 // Controle de acesso ao semaforo

    int s_id;                   // ID do semaforo
    int s_counter;              // Contador do semaforo
    struct queue_t *s_queue;    // Fila de tarefas associada ao semaforo
};

void sem_init()
{
    semaphore_array = (struct semaphore**) mem_alloc(sizeof(struct semaphore*) * 100);
}

void sem_term()
{
    mem_free(semaphore_array);
}

void spin_lock(int *lock)
{
    if (!lock) return;
    while( __sync_fetch_and_or(lock, 1)) {};
}

void spin_unlock(int *lock)
{
    if (!lock) return;
    (*lock) = 0;
}

int sem_create(int value)
{   
    struct semaphore *semaphore = (struct semaphore*) mem_alloc(sizeof(struct semaphore));
    if (!semaphore) return ERROR;

    semaphore->s_counter = value;
    semaphore->s_id = next_semaphore_id;
    semaphore->s_queue = queue_create();
    semaphore->s_lock = 0;

    semaphore_array[next_semaphore_id] = semaphore;

    next_semaphore_id++;

    return semaphore->s_id;
}

int sem_destroy(int sem_id)
{  
    printk("TA DESTRUINDO O SEMAFOROOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
    if(!semaphore_array[sem_id]) return ERROR;

    queue_destroy(semaphore_array[sem_id]->s_queue); // Libera fila associada ao semaforo
    mem_free(semaphore_array[sem_id]); // Libera semaforo
    
    semaphore_array[sem_id] = NULL;

    return NOERROR;
}

int sem_down(int sem_id)
{   
    if(!semaphore_array[sem_id]) return ERROR;

    struct semaphore *semaphore = semaphore_array[sem_id];
    spin_lock(&semaphore->s_lock);

    semaphore->s_counter--;

    // Se o semaforo fechou
    if(semaphore->s_counter < 0) {
        spin_unlock(&semaphore->s_lock);
        task_suspend(semaphore->s_queue);
    } else spin_unlock(&semaphore->s_lock);
    
    return NOERROR;
}

int sem_up(int sem_id)
{
    if(!semaphore_array[sem_id]) return ERROR;

    struct semaphore *semaphore = semaphore_array[sem_id];
    spin_lock(&semaphore->s_lock);

    semaphore->s_counter++;

    // Se havia alguem esperando o semaforo abrir
    if(semaphore->s_counter <= 0) {
        struct task_t *awoken_task = queue_head(semaphore->s_queue);
        queue_del(semaphore->s_queue, awoken_task);
        task_awake(awoken_task);
    }
    
    spin_unlock(&semaphore->s_lock);
    
    return NOERROR;
}
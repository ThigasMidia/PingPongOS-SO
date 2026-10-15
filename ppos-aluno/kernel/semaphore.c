// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.
// Eduarda Saibert - GRR20232368
// Kawai Barzotto - GRR20235687

// Semáforos e spinlocks

#include "task.h"
#include "lib/map.h"
#include "memory.h"
#include "queue.h"

#define NOERROR 0
#define ERROR -1
#define SEMAPHORE_QUANTITY 100

extern void task_suspend(struct queue_t *queue);
extern void task_awake(struct task_t *task);

struct map_t *semaphore_map;

struct semaphore
{
    int s_lock;                 // Controle de acesso ao semaforo

    int s_id;                   // ID do semaforo
    int s_counter;              // Contador do semaforo
    struct queue_t *s_queue;    // Fila de tarefas associada ao semaforo
};

void sem_init()
{
    semaphore_map = map_create(SEMAPHORE_QUANTITY);
}

void sem_term()
{
    map_destroy(semaphore_map);
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
    semaphore->s_queue = queue_create();
    semaphore->s_lock = 0;
    semaphore->s_id = 0;

    semaphore->s_id = map_put(semaphore_map, semaphore);

    return semaphore->s_id;
}

int sem_destroy(int sem_id)
{  
    struct semaphore *semaphore = map_get(semaphore_map, sem_id);
    if(!semaphore) return ERROR;

    while(queue_size(semaphore->s_queue))
    {
        struct task_t *awoken_task = queue_head(semaphore->s_queue);
        queue_del(semaphore->s_queue, awoken_task);
        task_awake(awoken_task);
    }
    queue_destroy(semaphore->s_queue);  // Libera fila associada ao semaforo
    map_del(semaphore_map, sem_id);     //Deleta do mapa
    mem_free(semaphore);                // Libera semaforo
    
    semaphore = NULL;

    return NOERROR;
}

int sem_down(int sem_id)
{   
    struct semaphore *semaphore = map_get(semaphore_map, sem_id);
    if(!semaphore) return ERROR;

    spin_lock(&semaphore->s_lock);

    semaphore->s_counter--;

    // Se o semaforo fechou
    if(semaphore->s_counter < 0) {
        spin_unlock(&semaphore->s_lock);
        task_suspend(semaphore->s_queue);
        if(!semaphore)
            return ERROR;
    } else spin_unlock(&semaphore->s_lock);
    
    return NOERROR;
}

int sem_up(int sem_id)
{
    struct semaphore *semaphore = map_get(semaphore_map, sem_id);
    if(!semaphore) return ERROR;

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
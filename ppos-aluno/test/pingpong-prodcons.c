// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 07/2026

// Este arquivo PODE/DEVE ser alterado.
// Eduarda Saibert - GRR20232368
// Kawai Barzotto - GRR20235687
// Produtor/Consumidor

#include <assert.h>
#include <stdlib.h>
#include "kernel/task.h"
#include "lib/queue.h"
#include "lib/pplibc.h"
#include "syscall.h"

struct task_t *prod1, *prod2, *prod3, *cons1, *cons2;
struct queue_t *buffer;
int sem_vaga, sem_buffer, sem_item;

void body_producer(void *arg)
{
    while(1)
    {
        task_sleep(1000);

        int *item = malloc(sizeof(int));
        *item = rand() % 100;

        sem_down(sem_vaga);
        sem_down(sem_buffer);

        queue_add(buffer, (int*)item);
        printk("%s produziu %d\n", (char *)arg, *item);

        sem_up(sem_buffer);
        sem_up(sem_item);
    }
}

void body_consumer(void *arg)
{  
    while(1)
    {
        sem_down(sem_item);

        sem_down(sem_buffer);
        int *item = queue_head(buffer);
        printk("%s consumiu %d\n", (char *)arg, *item);
        queue_del(buffer, item);
        sem_up(sem_buffer);

        sem_up(sem_vaga);

        task_sleep(1000);
    }
}

void user_main(void *arg) 
{   
    int status, id;
    char *name;

    name = task_name(NULL);
    id   = task_id(NULL);
    printk("%5u ms: %s (id %d): inicio\n", time(), name, id);

    // seed para gerar numeros aleatorios
    srand(time());

    // inicia buffer
    buffer = queue_create();


    // inicia semáforos
    sem_buffer = sem_create(1);
    assert(sem_buffer >= 0);
    sem_vaga = sem_create(5);
    assert(sem_vaga >= 0);
    sem_item = sem_create(0);
    assert(sem_item >= 0);

    // cria tarefas

    prod1 = task_create("p1", body_producer, "P1");
    assert(prod1);
    prod2 = task_create("p2", body_producer, "P2");
    assert(prod2);
    prod3 = task_create("a2", body_producer, "P3");
    assert(prod3);
    cons1 = task_create("b1", body_consumer, "\t\t\tC1");
    assert(cons1);
    cons2 = task_create("b2", body_consumer, "\t\t\tC2");
    assert(cons2);


    status = task_wait(prod1);
    assert(status == NOERROR);
}
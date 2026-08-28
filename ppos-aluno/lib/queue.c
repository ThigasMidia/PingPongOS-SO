// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 07/2026

// Este arquivo PODE/DEVE ser alterado.
// Eduarda Saibert - GRR20232368
// Kawai Barzotto - GRR20235687

// Implementação do TAD fila genérica
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

struct queue_t {
    struct queue_t_node *head;
    struct queue_t_node *tail;
    struct queue_t_node *iterator;
    int queue_size;
};      

struct queue_t_node {
    void *item;
    struct queue_t_node *next;
};

struct queue_t *queue_create() {
    struct queue_t *new_queue = malloc(sizeof(struct queue_t));
    if(!new_queue)
        return NULL;

    new_queue->head = NULL;
    new_queue->iterator = NULL;
    new_queue->queue_size = 0;

    return new_queue;
}

int queue_destroy(struct queue_t *queue) {
    if(!queue)
        return ERROR;

    free(queue);
    return NOERROR;
}

int queue_add(struct queue_t *queue, void *item) {
    if(!queue || !item)
        return ERROR;
    
    if(!queue_size(queue)) {
        queue->head = malloc(sizeof(struct queue_t_node));
        queue->tail = queue->head;
        queue->iterator = queue->head;
    }

    else {
        queue->tail->next = malloc(sizeof(struct queue_t_node));
        queue->tail = queue->tail->next;
    }

    queue->tail->next = NULL;
    queue->tail->item = item;
    queue->queue_size++;
    
    return NOERROR;
}   

int queue_del(struct queue_t *queue, void *item) {
    if(!queue || !item || !queue->queue_size)
        return ERROR;

    struct queue_t_node *finder_iterator = queue->head;

    bool found = false, head_found = false;
    if(queue->head->item == item)
        head_found = true;

    while(finder_iterator->next && !found && !head_found) {
        if(finder_iterator->next->item == item) {
            found = true;
        }
        if(!found)
            finder_iterator = finder_iterator->next;
    }
    
    if(head_found) {
        queue->head = finder_iterator->next;
        if(queue->iterator == finder_iterator) {
            if(queue->iterator == queue->tail)
                queue->iterator = NULL;
            else
                queue_next(queue);
        }
        free(finder_iterator);
        finder_iterator = NULL;
    }

    else if(!found)
        return ERROR;

    else {

        struct queue_t_node *remove_iterator = finder_iterator->next;
            
        if(queue->iterator == remove_iterator) {
            if(queue->iterator == queue->tail)
                queue->iterator = NULL;
            else
                queue_next(queue);
        }

        if(queue->tail == remove_iterator) {
            finder_iterator->next = NULL;
            queue->tail = finder_iterator;
        }

        else
            finder_iterator->next = remove_iterator->next;

        free(remove_iterator);
        remove_iterator = NULL;
    }
    queue->queue_size--;
    return NOERROR;
}


bool queue_has(struct queue_t *queue, void *item) { 
    if(!queue || !item || !queue->queue_size)
        return false;
    
    struct queue_t_node *finder_iterator = queue->head;
    bool found = false;
    if(queue->head->item == item)
        found = true;

    while(finder_iterator->next && !found) {
        if(finder_iterator->next->item == item) {
            found = true;
        }
    }
    
    return found;
}


int queue_size(struct queue_t *queue) {
    if (!queue)
        return ERROR;
    
    return queue->queue_size;
}


void *queue_head(struct queue_t *queue) {
    if (!queue || !queue->queue_size)
        return NULL;

    queue->iterator = queue->head;
    
    return queue->iterator->item;
}

void *queue_next(struct queue_t *queue) {
    if (!queue || !queue->queue_size)
        return NULL;

    if(queue->iterator == queue->tail) {
        queue->iterator = NULL;
        return NULL;
    }
    else
        queue->iterator = queue->iterator->next;

    return queue->iterator->item;
}

void *queue_item(struct queue_t *queue) {
    if (!queue || !queue->queue_size || queue->iterator == NULL)
        return NULL;

    return queue->iterator->item;
}

void queue_print(char *name, struct queue_t *queue, void(func)(void *)) {
    
    printf("%s: ", name);
    if(!queue) {
        printf("undef\n");
        return;
    }
    printf("[");

    int items = queue->queue_size;
    struct queue_t_node *print_iterator = queue->head;

    for(int i = 0; i < items; i++) {
        printf(" ");
        if(!func)
            printf("undef");
        else
            func(print_iterator->item);
        print_iterator = print_iterator->next;
    }
    printf(" ] (%d items)\n", items);
}


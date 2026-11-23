// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.
// Eduarda Saibert - GRR20232368
// Kawai Barzotto - GRR20235687

// Alocador básico de memória heap.

// somente para a implementação trivial
#include <stdlib.h>
#include "ctx.h"
#include "lib/pplibc.h"

#define HEAP_SIZE 64*1024*1024 //2^26, entao precisa de 26 bits
#define MAX_BLOCKS HEAP_SIZE / 16

static char heap[HEAP_SIZE];

typedef struct {
    void* initial_address;
    char alocated;
    char active;
    int size;
    int left_neighbor_id;
    int right_neighbor_id;
} block_descriptor;

static block_descriptor descriptor_array[MAX_BLOCKS];

int bytes_allocated = 0;
int blocks_allocated = 0;
int first_block;

void *mem_alloc(int size)
{
    if(size <= 0) return NULL;
    size = ((size - 1) | 0x00000F) + 1;

    int block_adress = first_block;
    int found_block = 0;
    //procura bloco de tamanho maior ou igual
    do {
        if(!descriptor_array[block_adress].alocated && descriptor_array[block_adress].size >= size) {
            found_block = 1;
            break;
        }
        block_adress = descriptor_array[block_adress].right_neighbor_id;
    } while(block_adress != first_block);

    //nao há bloco desalocado ou com espaço
    if(!found_block) return NULL;

    //fatia bloco em 2 se size for menor
    if(descriptor_array[block_adress].size > size) {
        int new_empty_block = 0;

        while(new_empty_block < MAX_BLOCKS) {
            //procura descritor ativo pro proximo bloco vazio
            if(!descriptor_array[new_empty_block].active)
                break;
            new_empty_block++;
        }
        //cria bloco novo
        if(new_empty_block != MAX_BLOCKS) {
            descriptor_array[new_empty_block].size = descriptor_array[block_adress].size - size;

            descriptor_array[new_empty_block].active = 1;
            descriptor_array[new_empty_block].alocated = 0;

            //redireciona vizinhos do novo bloco
            descriptor_array[new_empty_block].left_neighbor_id = block_adress;
            descriptor_array[new_empty_block].right_neighbor_id = descriptor_array[block_adress].right_neighbor_id;

            //redireciona vizinho do proximo bloco
            descriptor_array[descriptor_array[block_adress].right_neighbor_id].left_neighbor_id = new_empty_block;

            //atualiza tamanho e vizinhos do atual e do novo
            descriptor_array[new_empty_block].initial_address = descriptor_array[block_adress].initial_address + size;
            descriptor_array[block_adress].size = size;
            descriptor_array[block_adress].right_neighbor_id = new_empty_block;
        }
    }

    if(descriptor_array[descriptor_array[block_adress].left_neighbor_id].left_neighbor_id == -1)
        descriptor_array[descriptor_array[block_adress].left_neighbor_id].left_neighbor_id = block_adress;
    descriptor_array[block_adress].alocated = 1;

    bytes_allocated += size;
    blocks_allocated++;
    return descriptor_array[block_adress].initial_address;
}

int mem_free(void *addr)
{
    if(!addr) return ERROR;

    //procura pelo bloco atraves do endereco
    int block_id = first_block;
    int found_block = 0;
    do {
        if(descriptor_array[block_id].initial_address == addr) {
            found_block = 1;
            break;
        }
        block_id = descriptor_array[block_id].right_neighbor_id;
    } while(block_id != first_block);

    //se nao achou ou se ja foi liberado, erro
    if(!found_block || !descriptor_array[block_id].alocated) return ERROR;

    int id_esquerdo = descriptor_array[block_id].left_neighbor_id;
    int id_direito = descriptor_array[block_id].right_neighbor_id;
    //marca como livre
    descriptor_array[block_id].alocated = 0;
    bytes_allocated -= descriptor_array[block_id].size;
    if(!descriptor_array[id_esquerdo].alocated) {
        if(first_block == block_id) first_block = id_esquerdo;
        //desativa, atualiza vizinhos, aumenta tamanho do vizinho esquerdo
        descriptor_array[block_id].active = 0;
        descriptor_array[id_esquerdo].size += descriptor_array[block_id].size;
        descriptor_array[id_esquerdo].right_neighbor_id = id_direito;

        //aterra o tamanho e troca block_id pelo id do vizinho esquerdo caso precise se fundir com o direito
        descriptor_array[block_id].size = 0;
        block_id = id_esquerdo;
    }

    if(!descriptor_array[id_direito].alocated) {
        if(first_block == id_direito) first_block = block_id;
        descriptor_array[id_direito].active = 0;
        descriptor_array[block_id].size += descriptor_array[id_direito].size;
        descriptor_array[block_id].right_neighbor_id = descriptor_array[id_direito].right_neighbor_id;

        descriptor_array[id_direito].size = 0;
    }
    
    blocks_allocated--;
    return (NOERROR);
}

void print_block(int block_id) {
    void* block_end = descriptor_array[block_id].initial_address + descriptor_array[block_id].size - 1;

    printk("heap: block %5d: 0x%p - 0x%p", block_id, descriptor_array[block_id].initial_address, block_end);

    if(descriptor_array[block_id].alocated)
        printk(" aloc ");
    else
        printk(" FREE ");
    
    printk("prev %5d next %5d size %d\n", descriptor_array[block_id].left_neighbor_id, 
        descriptor_array[block_id].right_neighbor_id, descriptor_array[block_id].size);
}

void mem_report() {
    printk("heap: %d KB allocated (%d blocks), %d KB free (%d blocks)\n",
        bytes_allocated, blocks_allocated, HEAP_SIZE - bytes_allocated, MAX_BLOCKS - blocks_allocated);

    int block_id = first_block;
    do {
        if(descriptor_array[block_id].active)
            print_block(block_id);
        block_id = descriptor_array[block_id].right_neighbor_id;
    } while(block_id != first_block);
}

int mem_avail() {
    return HEAP_SIZE - bytes_allocated;
}

int mem_size() {
    return HEAP_SIZE;
}

void mem_init()
{
    for(int i = 0; i < MAX_BLOCKS; i++) {
        descriptor_array[i].alocated = 0;
        descriptor_array[i].active = 0;
    }

    descriptor_array[0].initial_address = heap;
    descriptor_array[0].size = HEAP_SIZE;
    descriptor_array[0].active = 1;
    descriptor_array[0].left_neighbor_id = 0;
    descriptor_array[0].right_neighbor_id = 0;
    first_block = 0;
}

void mem_term()
{
}
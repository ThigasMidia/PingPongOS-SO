// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 07/2026

// Este arquivo PODE/DEVE ser alterado.
// Eduarda Saibert - GRR20232368
// Kawai Barzotto - GRR20235687
// Implementação do TAD Mapa de objetos

#include <stdio.h>
#include <stdlib.h>
#include "map.h"

typedef struct map_t {
	void** items;			// itens
	int tot_size;			// capacidade total
	int act_size;			// capacidade atual
	int top;				// id do último elemento
} map_t;

// Cria um mapa para até N objetos, com IDs entre 0 e N-1.
// Retorno: ponteiro para o mapa ou NULL (erro)
struct map_t *map_create(int size){
	if (size <= 0) {
		return NULL;
	}
	
	map_t* map = (map_t*) malloc(sizeof(map_t));
	if (!map) return NULL;

	map->items = (void**) malloc(size*sizeof(void*));
	if (!map->items){
		free(map);
		return NULL;
	}

	map->tot_size = size;
	map->act_size = size;
	map->top = -1;

	return map;
}


// destrói um mapa existente (mas não destrói os objetos).
// Retorno: 0 em sucesso ou -1 (erro)
int map_destroy(struct map_t *map){
	if (!map) {
		return ERROR;
	}

	if (map->items) {
		free(map->items);
		map->items = NULL;
	}
	free(map);
	map = NULL;

	return NOERROR;
}


// Registra um objeto no mapa, retornando seu ID.
// Retorno: ID atribuído ao objeto ou -1 (erro).
int map_put(struct map_t *map, void *object){
	if (!map) return ERROR;
	if (!object) return ERROR;

	if (map->act_size == 0) return ERROR;		// Se não há mais capacidade no mapa, retornar -1

	// Retorna 0 caso último elemento registrado seja no limite do mapa
	int top = (map->top < 0) ? 0 : (map->top + 1) % map->tot_size;	

	for (int i = 0; i < map->tot_size; i++) {
		int cur_id = (top + i) % map->tot_size;

		void* cur_ptr = map->items[cur_id];
		if (cur_ptr) continue;

		map->items[cur_id] = object;
		map->top = cur_id;
		map->act_size = map->act_size - 1;

		return cur_id;
	}

	return ERROR;
}

		
// Informa o objeto registrado no ID indicado do mapa.
// Retorno: ponteiro para o objeto ou NULL (erro)
void *map_get(struct map_t *map, int id){
	if (!map) return NULL;
	if (id < 0 || id >= map->tot_size) return NULL;

	return map->items[id];
}

// Libera um ID do mapa e devolve o objeto associado.
// Retorno: ponteiro para o objeto ou NULL (erro)
void *map_del(struct map_t *map, int id){
	if (!map) return NULL;
	if (id < 0 || id >= map->tot_size) return NULL;

	void* item = map_get(map, id);
	if (!item) return NULL;

	map->items[id] = NULL;
	map->act_size = map->act_size + 1;
	if (map->top == id) map->top = map->top - 1;
	
	return item;
}

// Informa o número de objetos registrados no mapa.
// Retorno: número de objetos registrados ou -1 (erro)
int map_items(struct map_t *map){
	if (!map) return ERROR;

	return map->tot_size - map->act_size;
}

// Informa o número de objetos que o mapa pode registrar.
// Retorno: número de objetos ou -1 (erro)
int map_size(struct map_t *map){
	if (!map) return ERROR;

	return map->act_size;
}

// Imprime o conteúdo do mapa, no seguinte formato:
// Mapa nulo:     nome: undef
// Mapa vazio:    nome: [ - - - - - ] (0/5)
// Mapa qualquer: nome: [ - * - * * ] (3/5)
// As posições "*" no vetor impresso correspondem aos IDs em uso no mapa;
// (3/5) indica que o mapa tem 3 objetos e pode registrar até 5 objetos.
void map_print(char *name, struct map_t *map){
	if (!map) {
		printf("%s: undef\n", name);
		return;
	}

	printf("%s: [ ", name);	
	
	int size = map->tot_size;
	int act_size = map->act_size;
	for (int i = 0; i < size; i++){
		void* cur_ptr = map->items[i];
		if (cur_ptr) printf("* ");
		else printf("- ");
	}
	printf("] (%d/%d)\n", size-act_size, size);

}


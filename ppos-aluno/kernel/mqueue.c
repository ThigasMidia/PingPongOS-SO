// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.
// Eduarda Saibert - GRR20232368
// Kawai Barzotto - GRR20235687
// Gerência de filas de mensagens

// Mapa das filas de mensagens no sistema
#include <stdlib.h>
#include <string.h>
#include "macros.h"
#include "mqueue.h"
#include "semaphore.h"
#include "lib/pplibc.h"
#include "lib/map.h"
#include "lib/queue.h"


typedef struct mqueue_t {
	int max_msgs;		// Máximo de mensagens permitidas no buffer 
	int	msg_size;		// Tamanho das mensagens a serem transmitidas
	int msg_count;		// Quantas mensagens existem no buffer
	
	int sem_vaga;		// Valor de vagas no semáforo
	int sem_buffer;		// Mutex que controla o acesso ao agrupador
	int sem_item;		// Valor de itens no semáforo

	struct queue_t* buffer;	// Buffer 
} mqueue_t;

#define MAX_QUEUES 100	// Quantas message queues o mapa aguenta simultâneamente
struct map_t* queue_map = NULL;// Mapa que gerencia as message queues no sistema


void mqueue_init()
{
	queue_map = map_create(MAX_QUEUES);	
	if (!queue_map) return;
}

void mqueue_term()
{
	if (queue_map) map_destroy(queue_map);
}

// cria uma fila de mensagens.
// Retorno: descritor da nova fila ou ERROR
int mqueue_create(int max_msgs, int msg_size){
	if (!queue_map) return ERROR;

	// Inicializa estrutura de dados
	struct mqueue_t* mqueue = (struct mqueue_t*) malloc(sizeof(mqueue_t));
	if (!mqueue) return ERROR;

	// Inicializa buffer interno
	mqueue->buffer = queue_create();
	if (!(mqueue->buffer)) return ERROR;

	// Inicializa variáveis
	mqueue->max_msgs = max_msgs;
	mqueue->msg_size = msg_size;
	mqueue->msg_count = 0;

	// Inicializa semáforos
	mqueue->sem_vaga = sem_create(max_msgs);
	mqueue->sem_buffer = sem_create(1);
	mqueue->sem_item = sem_create(0);

	// Insere a mensage queue no mapa
	int mqueue_id = map_put(queue_map, (void*) mqueue);
	if (mqueue_id == -1) return ERROR;

	return mqueue_id;
}

// destrói uma fila de mensagens, liberando recursos e tarefas
// Retorno: NOERROR ou ERROR
int mqueue_destroy(int mqueue_id){
	if (!queue_map) return ERROR;

	// Obtém estrutura de dados pelo descritor
	struct mqueue_t* mqueue = (struct mqueue_t*) map_get(queue_map, mqueue_id);
	if (!mqueue) return ERROR;

	// Realiza free das estruturas alocadas
	int res_sv = sem_destroy(mqueue->sem_vaga);
	int res_sb = sem_destroy(mqueue->sem_buffer);
	int res_si = sem_destroy(mqueue->sem_item);

	// Realize free dos itens alocados pelo buffer
	while (queue_size(mqueue->buffer) > 0) {
		void *msg = queue_head(mqueue->buffer);

		queue_del(mqueue->buffer, msg);
		free(msg);
	}
	int res_q = queue_destroy(mqueue->buffer);

	if ((res_q == ERROR) || (res_sv == ERROR) || (res_sb == ERROR) || (res_si == ERROR)) return ERROR;

	// Retira estrutura do mapa
	if (!map_del(queue_map, mqueue_id)) return ERROR;

	free(mqueue);
	mqueue = NULL;
	return NOERROR;
}

// envia uma mensagem
// Retorno: NOERROR ou ERROR
int mqueue_send(int mqueue_id, void *msg){
    if (!queue_map || !msg)
        return ERROR;

    struct mqueue_t *mqueue = (struct mqueue_t *) map_get(queue_map, mqueue_id);

    if (!mqueue)
        return ERROR;

	// Pede acesso ao semaforo, juntamente de check se semaforo foi destruido
    if (sem_down(mqueue->sem_vaga) < 0)
        return ERROR;

    if (sem_down(mqueue->sem_buffer) < 0) {
        sem_up(mqueue->sem_vaga);
        return ERROR;
    }

	// Adiciona item ao buffer

    void *cp_msg = malloc(mqueue->msg_size);
    if (!cp_msg) {
        sem_up(mqueue->sem_buffer);
        sem_up(mqueue->sem_vaga);
        return ERROR;
    }
    memcpy(cp_msg, msg, mqueue->msg_size);

    if (queue_add(mqueue->buffer, cp_msg) < 0) {
        free(cp_msg);
        sem_up(mqueue->sem_buffer);
        sem_up(mqueue->sem_vaga);
        return ERROR;
    }

    mqueue->msg_count++;
    ppos_debug("Fila %d produziu mensagem\n", mqueue_id);

	// Libera semaforos
    sem_up(mqueue->sem_buffer);
    sem_up(mqueue->sem_item);

    return NOERROR;
}

// recebe uma mensagem
// Retorno: NOERROR ou ERROR
int mqueue_recv(int mqueue_id, void *msg){
    if (!queue_map || !msg)
        return ERROR;

    struct mqueue_t *mqueue =
        (struct mqueue_t *) map_get(queue_map, mqueue_id);

    if (!mqueue)
        return ERROR;

	// Pede acesso ao semaforo, juntamente de check se semaforo foi destruido
    if (sem_down(mqueue->sem_item) < 0)
        return ERROR;

    if (sem_down(mqueue->sem_buffer) < 0) {
        sem_up(mqueue->sem_item);
        return ERROR;
    }

	// Adiciona item ao buffer
    void *cp_msg = queue_head(mqueue->buffer);
    if (!cp_msg) {
        sem_up(mqueue->sem_buffer);
        sem_up(mqueue->sem_item);
        return ERROR;
    }

    memcpy(msg, cp_msg, mqueue->msg_size);

    if (queue_del(mqueue->buffer, cp_msg) == ERROR) {
        sem_up(mqueue->sem_buffer);
        sem_up(mqueue->sem_item);
        return ERROR;
    }
    free(cp_msg);

    mqueue->msg_count--;

    ppos_debug("Fila %d consumiu mensagem\n", mqueue_id);

    // Libera semáforos
    sem_up(mqueue->sem_buffer);
    sem_up(mqueue->sem_vaga);

    return NOERROR;
}

// retorna o numero de mensagens em uma fila
// Retorno: número >= 0 ou ERROR
int mqueue_msgs(int mqueue_id){
	if (!queue_map) return ERROR;

	struct mqueue_t* mqueue = map_get(queue_map, mqueue_id);
	if (!mqueue) return ERROR;

	// Pede acesso ao semáforo
	if (sem_down(mqueue->sem_buffer) < 0) return ERROR;	

	// Acessa a contagem de mesagens no buffer
	int msg_count = mqueue->msg_count;

	// Libera o semáforo
	sem_up(mqueue->sem_buffer);

	return msg_count;
}



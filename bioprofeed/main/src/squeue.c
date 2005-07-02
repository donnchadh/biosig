#include <stdio.h>
#include <sys/queue.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

#include "squeue.h"

/* Defines ********************************************************************/

#define MAXSIZE		256


/* Typedef ********************************************************************/

typedef struct sq_item {
	CIRCLEQ_ENTRY(sq_item)	entries;
	void*					data;
	int						size;
} sq_item_t;


/* Variables ******************************************************************/

static CIRCLEQ_HEAD(circleq, sq_item)	head;
static pthread_mutex_t					mtx;
static int								cnt;
static sem_t							sem;


/* Public Functions ***********************************************************/

void sq_init(void)
{
	CIRCLEQ_INIT(&head);
	pthread_mutex_init(&mtx, NULL);
	cnt	= -1;
}

void sq_destroy(void)
{
	sq_cleanup();

	pthread_mutex_destroy(&mtx);
}

int sq_setup(void)
{
	sem_init(&sem, 0, 0);

	pthread_mutex_lock(&mtx);
	cnt		= 0;
	pthread_mutex_unlock(&mtx);

	return 0;
}

void sq_cleanup(void)
{
	sq_item_t*	item;

	pthread_mutex_lock(&mtx);
	while(head.cqh_first != (void*)&head) {
		item	= head.cqh_first;
		CIRCLEQ_REMOVE(&head, head.cqh_first, entries);
		free(item->data);
		free(item);
	}

	/* Exit */
	cnt	= -1;
	pthread_mutex_unlock(&mtx);

	sem_post(&sem);
}

void sq_exit(void)
{
	pthread_mutex_lock(&mtx);
	cnt	= -1;
	pthread_mutex_unlock(&mtx);

	sem_post(&sem);
}

int sq_append_data(void* data, int size)
{
	sq_item_t*	item;

	pthread_mutex_lock(&mtx);
	if( cnt < 0 || cnt > MAXSIZE) {
		pthread_mutex_unlock(&mtx);
		return -1;
	}
	pthread_mutex_unlock(&mtx);

	item	= malloc(sizeof(sq_item_t));
	if( !item ) {
		return -1;
	}

	item->data	= malloc(size);
	if( !(item->data) ) {
		free(item);
		return -1;
	}

	memcpy(item->data, data, size);

	pthread_mutex_lock(&mtx);
	CIRCLEQ_INSERT_TAIL(&head, item, entries);

	/* Data available */
	cnt++;
	pthread_mutex_unlock(&mtx);

	sem_post(&sem);

	return 0;
}

void* sq_remove_data(void)
{
	sq_item_t*	item;
	void*		data;

	/* If setup has not been called don't wait */
	pthread_mutex_lock(&mtx);
	if( cnt < 0 ) {
		pthread_mutex_unlock(&mtx);
		return NULL;
	}
	pthread_mutex_unlock(&mtx);

	/* Wait */
	sem_wait(&sem);

	/* Return data or exit */
	pthread_mutex_lock(&mtx);
	if( cnt < 0 ) {
		pthread_mutex_unlock(&mtx);
		return NULL;
	}

	if(head.cqh_first == (void*)&head) {
		pthread_mutex_unlock(&mtx);
		return NULL;
	}
	
	item	= head.cqh_first;
	CIRCLEQ_REMOVE(&head, head.cqh_first, entries);
	cnt--;
	pthread_mutex_unlock(&mtx);

	data	= item->data;
	free(item);

	return data;
}

/* vim: set ts=4: */
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
static GMutex*							mtx_ptr;
static GMutex*							con_mtx_ptr;
static GCond*							con_ptr;
static int								cnt;
static sem_t							sem;


/* Public Functions ***********************************************************/

void sq_init(void)
{
	CIRCLEQ_INIT(&head);
	mtx_ptr		= g_mutex_new();
	con_mtx_ptr	= g_mutex_new();
	con_ptr		= g_cond_new();
	cnt		= -1;
}

void sq_destroy(void)
{
	sq_cleanup();

	g_mutex_free(mtx_ptr);
	g_mutex_free(con_mtx_ptr);
	g_cond_free(con_ptr);
}

int sq_setup(void)
{
	g_mutex_lock(mtx_ptr);
	cnt		= 0;
	g_mutex_unlock(mtx_ptr);

	return 0;
}

void sq_cleanup(void)
{
	sq_item_t*	item;

	g_mutex_lock(mtx_ptr);
	while(head.cqh_first != (void*)&head) {
		item	= head.cqh_first;
		CIRCLEQ_REMOVE(&head, head.cqh_first, entries);
		free(item->data);
		free(item);
	}

	/* Exit */
	cnt	= -1;
	g_mutex_unlock(mtx_ptr);

	g_cond_signal(con_ptr);
}

void sq_exit(void)
{
	g_mutex_lock(mtx_ptr);
	cnt	= -1;
	g_mutex_unlock(mtx_ptr);

	g_cond_signal(con_ptr);
}

int sq_append_data(void* data, int size)
{
	sq_item_t*	item;

	g_mutex_lock(mtx_ptr);
	if( cnt < 0 || cnt > MAXSIZE) {
		g_mutex_unlock(mtx_ptr);
		return -1;
	}
	g_mutex_unlock(mtx_ptr);

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

	g_mutex_lock(mtx_ptr);
	CIRCLEQ_INSERT_TAIL(&head, item, entries);

	/* Data available */
	cnt++;
	g_mutex_unlock(mtx_ptr);

	g_cond_signal(con_ptr);

	return 0;
}

void* sq_remove_data(void)
{
	sq_item_t*	item;
	void*		data;

	/* If setup has not been called don't wait */
	g_mutex_lock(mtx_ptr);
	if( cnt < 0 ) {
		g_mutex_unlock(mtx_ptr);
		return NULL;
	}
	g_mutex_unlock(mtx_ptr);

	/* Wait */
	g_mutex_lock(con_mtx_ptr);
	g_cond_wait(con_ptr, con_mtx_ptr);
	g_mutex_unlock(con_mtx_ptr);

	/* Return data or exit */
	g_mutex_lock(mtx_ptr);
	if( cnt < 0 ) {
		g_mutex_unlock(mtx_ptr);
		return NULL;
	}

	if(head.cqh_first == (void*)&head) {
		g_mutex_unlock(mtx_ptr);
		return NULL;
	}
	
	item	= head.cqh_first;
	CIRCLEQ_REMOVE(&head, head.cqh_first, entries);
	cnt--;
	g_mutex_unlock(mtx_ptr);

	data	= item->data;
	free(item);

	return data;
}

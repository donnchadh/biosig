/* vim: set ts=4: */
#ifndef _SQUEUE_H_
#define _SQUEUE_H_

void sq_init(void);
void sq_destroy(void);

int sq_setup(void);
void sq_cleanup(void);
void sq_exit(void);

int sq_append_data(void*, int);
void* sq_remove_data(void);

#endif /* _SQUEUE_H_ */

/* vim: set ts=4: */
#ifndef _DRAW_H_
#define _DRAW_H_

void d_init(void);
void d_destroy(void);

int d_setup(void);
void d_cleanup(void);

int d_set_samples(void*);

int d_set_offset(int);

void d_redraw(void);

char* d_get_errormsg(void);

void d_control_load(int);
void d_control_apply(int, int, int);

#endif /* _DRAW_H_ */

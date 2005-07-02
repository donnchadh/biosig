#ifndef _GDFFILE_H_
#define _GDFFILE_H_

void gf_init(void);
void gf_destroy(void);

int gf_setup(void);
void gf_cleanup(void);

int gf_set_samples(void*);

char* gf_get_errormsg(void);

#endif /* _GDFFILE_H_ */

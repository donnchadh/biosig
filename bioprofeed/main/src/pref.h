#ifndef _PREF_H_
#define _PREF_H_

/**
 * Constructor
 * @param path Path were the last preference configuration has been saved
 */
void p_init(const char* path);

/**
 * Destructor
 * @param path Path were the preference configuration should be saved
 */
void p_destroy(const char* path);


void p_set_logfile(const char* filepath);
char* p_get_logfile(void);

void p_set_patid(const char* patid);
char* p_get_patid(void);

void p_set_recid(const char* recid);
char* p_get_recid(void);

void p_set_hrange(int range);
int p_get_hrange(void);

void p_set_chanperscreen(int chanperscreen);
int p_get_chanperscreen(void);

#endif /* _PREF_H_ */

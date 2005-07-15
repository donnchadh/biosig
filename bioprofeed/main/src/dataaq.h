/* vim: set ts=4: */
#ifndef _DATAAQ_H_
#define _DATAAQ_H_

/**
 * Constructor
 */
void da_init(void);

/**
 * Destructor
 */ 
void da_destroy(void);

/**
 * Start aquisition
 */
void da_start(void);

/**
 * Stop aquisition
 */
void da_stop(void);

#endif /* _DATAAQ_H_ */

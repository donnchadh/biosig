#ifndef _BSV_MODULE_H_
#define _BSV_MODULE_H_

/* Defines ********************************************************************/

#define BSV_CHTYPE_CHAR			0	/* char */
#define BSV_CHTYPE_INT8			1	/* char */
#define BSV_CHTYPE_UINT8		2	/* unsigned char */
#define BSV_CHTYPE_INT16		3	/* short */
#define BSV_CHTYPE_UINT16		4	/* unsigned short */
#define BSV_CHTYPE_INT32		5	/* long */
#define BSV_CHTYPE_UINT32		6	/* unsigned long */
#define BSV_CHTYPE_INT64		7	/* long long */
#define BSV_CHTYPE_FLT32		16	/* float */
#define BSV_CHTYPE_FLT64		17	/* double */


/* Macros *********************************************************************/

#define BSV_IS_MODID(x, y)		((x) == (y))
#define BSV_GET_MODNAME(x)		((x)->name)
#define BSV_GET_MODID(x)		((x)->id)
#define BSV_GET_MODLIC(x)		((x)->licence)
#define BSV_GET_MODAUT(x)		((x)->author)
#define BSV_GET_CHANNUM(x)		((x)->channum)
#define BSV_GET_EPID(x)			((x)->epid)
#define BSV_GET_LABID(x)		((x)->labid)
#define BSV_GET_TID(x)			((x)->tid)
#define BSV_GET_SNUM(x)			((x)->sernum)
#define BSV_GET_SRATENOM(x)		((x)->sratenom)
#define BSV_GET_SRATEDENOM(x)	((x)->sratedenom)

#define BSV_CH_GET_LABEL(x)		((x)->label)
#define BSV_CH_GET_TTYPE(x)		((x)->ttype)
#define BSV_CH_GET_PDIM(x)		((x)->pdim)
#define BSV_CH_GET_PMIN(x)		((x)->pmin)
#define BSV_CH_GET_PMAX(x)		((x)->pmax)
#define BSV_CH_GET_DMIN(x)		((x)->dmin)
#define BSV_CH_GET_DMAX(x)		((x)->dmax)
#define BSV_CH_GET_PFILT(x)		((x)->pfilt)
#define BSV_CH_GET_SRECNUM(x)	((x)->srecnum)
#define BSV_CH_GET_CHTYPE(x)	((x)->chtype)


/* Typedef ********************************************************************/

typedef unsigned long	bsv_modid_t;

/**
 * Info of this module needed to display and save data
 * coming from the driver
 */
typedef struct bsv_modinfo {
	const char*			name;
	bsv_modid_t			id;
	const char*			licence;
	const char*			author;
	int					channum;
	unsigned long long	epid;
	unsigned long long	labid;
	unsigned long long	tid;
	const char*			sernum;
	unsigned long		sratenom;
	unsigned long		sratedenom;
} bsv_modinfo_t;

/**
 * Info of the channels needed to display and save data
 * coming from the driver
 */
typedef struct bsv_channelinfo {
	const char*			label;
	const char*			ttype;
	const char*			pdim;
	double				pmin;
	double				pmax;
	long long			dmin;
	long long			dmax;
	const char*			pfilt;
	unsigned long		srecnum;
	unsigned long		chtype;
} bsv_channelinfo_t;

/**
 * The data structure which contains samples from the 
 * driver
 */
typedef struct bsv_data_t {
	const char*			samples;
	int					size;
} bsv_data_t;

/** 
 * "Constructor" of the module
 * Takes the path were to get the last configuration.
 */
typedef int (*bsv_init_f)(const char*);

/** 
 * "Destructor" of the module
 * Takes the path were to save the last configuration
 */
typedef int (*bsv_destroy_f)(const char*);

/**
 * Start aquistion
 */
typedef int (*bsv_start_f)(void);

/**
 * Stop aquistion
 */
typedef int (*bsv_stop_f)(void);

/**
 * Returns the instance of the configuration window
 */
typedef void* (*bsv_get_config_win_f)(void);

/**
 * Returns the channel info needed to display and save the data
 */
typedef bsv_channelinfo_t* (*bsv_get_channelinfo_f)(int);

/** 
 * Returns the module info needed to display and save the data
 */
typedef bsv_modinfo_t* (*bsv_get_moduleinfo_f)(void);

/**
 * Retrurns samples from the driver. May block in case
 * there are no samples.
 */
typedef bsv_data_t* (*bsv_get_data_f)(void);

/**
 * Retrurns a message which describes the last error which 
 * has occured.
 */
typedef char* (*bsv_get_errormsg_f)(void);


/* Global Variables ***********************************************************/

extern bsv_init_f				bsv_init;
extern bsv_destroy_f			bsv_destroy;
extern bsv_start_f				bsv_start;
extern bsv_stop_f				bsv_stop;
extern bsv_get_config_win_f		bsv_get_config_win;
extern bsv_get_channelinfo_f	bsv_get_channelinfo;
extern bsv_get_moduleinfo_f		bsv_get_moduleinfo;
extern bsv_get_data_f			bsv_get_data;
extern bsv_get_errormsg_f		bsv_get_errormsg;

#endif /* _BSV_MODULE_H_ */

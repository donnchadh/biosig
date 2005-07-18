/* vim: set ts=4: */
#include <stdio.h>
#include <gtk/gtk.h>
#include <sys/queue.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "gdffile.h"
#include "pref.h"
#include "bsv_module.h"
#include "support.h"


/* Debugging ******************************************************************/

//#define DEBUG
#undef DEBUG
#ifdef DEBUG
#define say(fun, fmt, args...)  printf("DEBUG: "fun": "fmt"\n", ##args) 
#else
#define say(fun, fmt, args...)
#endif


/* Define *********************************************************************/

#define GF_VER_SIZE		8
#define GF_PATID_SIZE	80
#define GF_RECID_SIZE	80
#define GF_RECST_SIZE	16
#define GF_SNUM_SIZE	20
#define GF_DRD_SIZE		2

#define GF_LABEL_SIZE	16
#define GF_TTYPE_SIZE	80
#define GF_PDIM_SIZE	8
#define GF_PFILT_SIZE	80
#define GF_RSV_SIZE		32

#define DEAD			0
#define ALIVE			1
#define RUNNING			3
#define DYING			4


/* Macro **********************************************************************/

#define write_str(__str, __size, fmt, arg...) do { \
		int	i;	\
		memset((__str), ' ', (__size)); \
		i	= snprintf((__str), (__size)+1, fmt, ##arg); \
		(__str)[i]	= ' '; \
	} while(0)


/* Typedef ********************************************************************/

typedef struct gf_fheader {
	char				version[GF_VER_SIZE+1];
	char				patid[GF_PATID_SIZE+1];
	char				recid[GF_RECID_SIZE+1];
	char				recst[GF_RECST_SIZE+1];
	long long			hrecbnum;				/* number of bytes in header 
												   record */
	unsigned long long	epid;					/* equipment provider id */
	unsigned long long	labid;
	unsigned long long	tid;					/* technician id */
	char				sernum[GF_SNUM_SIZE+1];
	long long			drecnum;				/* number of data records */
	unsigned long		drecdur[GF_DRD_SIZE+1];	/* duration of a data record */
	unsigned long 		snum;					/* number of signals */
} gf_fheader_t;

typedef struct gf_vheader {
	CIRCLEQ_ENTRY(gf_vheader)	entries;
	char						label[GF_LABEL_SIZE+1];	/* label */
	char						ttype[GF_TTYPE_SIZE+1];	/* transducer type */
	char						pdim[GF_PDIM_SIZE+1];	/* physical dimension */
	double						pmin;					/* physical minimum */
	double						pmax;					/* physical maximum */
	long long					dmin;					/* digital minimum */
	long long					dmax;					/* digital maximum */
	char						pfilt[GF_PFILT_SIZE+1];	/* prefiltering */
	unsigned long				srecnum;				/* number of samples per
														   record */
	unsigned long				chtype;					/* channel type */
	char						rsv[GF_RSV_SIZE+1];		/* reserved size */
} gf_vheader_t;


/* External Variables *********************************************************/

extern GtkWidget*				bsv_main_win;


/* Variable *******************************************************************/

static gf_fheader_t							fheader;
static CIRCLEQ_HEAD(circleq, gf_vheader)	head;
static int									fd				= -1;
static char*								errormsg_ptr	= NULL;
static char* 								terrormsg_ptr	= NULL;
static GThread*								writer_ptr;
static volatile unsigned char				state			= DEAD;
static GMutex*								state_mtx_ptr;
static unsigned long						sbarcnt			= 0;
static unsigned long						rectime			= 0;
static GAsyncQueue*							sample_q_ptr;


/* Private functions **********************************************************/

static void clear_circleq(void)
{
	gf_vheader_t*	vheader_ptr;

	while(head.cqh_first != (void*)&head) {
		vheader_ptr	= head.cqh_first;

		/* remove item from list */
		CIRCLEQ_REMOVE(&head, head.cqh_first, entries);

		/* dealloc item */
		free(vheader_ptr);
	}
}

static void setup_fheader(void)
{
	bsv_modinfo_t*		modinfo;
	
	modinfo					= bsv_get_moduleinfo();

	write_str(fheader.version, GF_VER_SIZE, "GDF 1.24");
	write_str(fheader.patid, GF_PATID_SIZE, "%s", p_get_patid());
	write_str(fheader.recid, GF_RECID_SIZE, "%s", p_get_recid());
	fheader.hrecbnum		= 256;						/* fixed header size */
	fheader.epid			= BSV_GET_EPID(modinfo);
	fheader.labid			= BSV_GET_LABID(modinfo);
	fheader.tid				= BSV_GET_TID(modinfo);
	write_str(fheader.sernum, GF_SNUM_SIZE, "%s", BSV_GET_SNUM(modinfo));
	fheader.drecnum			= 0;						/* sample count is 0 */
	fheader.drecdur[0]		= BSV_GET_SRATENOM(modinfo);
	fheader.drecdur[1]		= BSV_GET_SRATEDENOM(modinfo);
	fheader.snum			= 0;						/* channel num is 0 */

	say("setup_fheader",
		"fheader");
}

static void setup_fheader_starttime(void)
{
	time_t			start_time;
	struct tm*		tm_ptr;
	
	time(&start_time);
	tm_ptr	= localtime(&start_time);
	write_str(fheader.recst, GF_RECST_SIZE, "%04d%02d%02d%02d%02d%02d%02d",
											tm_ptr->tm_year+1900,
											tm_ptr->tm_mon,
											tm_ptr->tm_mday,
											tm_ptr->tm_hour,
											tm_ptr->tm_min,
											tm_ptr->tm_sec,
											0);
	say("setup_fheader_starttime",
		"starttime");
}

static int setup_vheader(void)
{
	bsv_channelinfo_t*	chaninfo;
	int					cnt;
	gf_vheader_t*		vheader_ptr;

	cnt	= 0;
	while( (chaninfo = bsv_get_channelinfo(cnt)) ) {
		if( !(vheader_ptr = (gf_vheader_t*)malloc(sizeof(gf_vheader_t))) ) {
			errormsg_ptr	= "setup_vheader: Error: Out of memory";
			return -1;
		}
	
		CIRCLEQ_INSERT_TAIL(&head, vheader_ptr, entries);
	
		write_str(vheader_ptr->label, GF_LABEL_SIZE, 
				  "%s", BSV_CH_GET_LABEL(chaninfo));
		write_str(vheader_ptr->ttype, GF_TTYPE_SIZE,
				  "%s", BSV_CH_GET_TTYPE(chaninfo));
		write_str(vheader_ptr->pdim, GF_PDIM_SIZE, 
				  "%s", BSV_CH_GET_PDIM(chaninfo));
		vheader_ptr->pmin	= BSV_CH_GET_PMIN(chaninfo);
		vheader_ptr->pmax	= BSV_CH_GET_PMAX(chaninfo);
		vheader_ptr->dmin	= BSV_CH_GET_DMIN(chaninfo);
		vheader_ptr->dmax	= BSV_CH_GET_DMAX(chaninfo);
		write_str(vheader_ptr->pfilt, GF_PFILT_SIZE,
				  "%s", BSV_CH_GET_PFILT(chaninfo));
		vheader_ptr->srecnum= BSV_CH_GET_SRECNUM(chaninfo);
		vheader_ptr->chtype	= BSV_CH_GET_CHTYPE(chaninfo);
	
		/* Update fheader */
		fheader.snum++;
		fheader.hrecbnum+=256;

		/* Increment channel counter */
		cnt++;
	}

	say("setup_vheader",
		"vheader");

	return 0;
}

static inline int write_short(int fd, short* val, int cnt) 
{
#ifdef LENDIAN
    return (write(fd, val, sizeof(short)*cnt) != sizeof(short)*cnt) ? -1 : 0;
#else
    int i, o;
    for(o=0; o<cnt; o++) {
        for(i=sizeof(short)-1; i>=0; i--) {
            if( write(fd, &((char*)&val[o])[i], 1) != 1 ) {
                return -1;
            }
        }
    }
    return 0;
#endif /* LENDIAN */
}

static inline int write_longlong(int fd, long long* val, int cnt)
{
#ifdef LENDIAN
	return (write(fd, val, sizeof(long long)*cnt) !=
			sizeof(long long)*cnt) ? -1: 0;
#else
	int i, o;
	for(o=0; o<cnt; o++) {
		for(i=sizeof(long long)-1; i>=0; i--) {
			if( write(fd, &((char*)&val[o])[i], 1) != 1) {
				return -1;
			}
		}
	}
	return 0;
#endif /* LENDIAN */
}

static inline int write_ulonglong(int fd, unsigned long long val)
{
#ifdef LENDIAN
	return (write(fd, &val, sizeof(unsigned long long)) !=
			sizeof(unsigned long long)) ? -1 : 0;
#else
	int i;
	for(i=sizeof(unsigned long long)-1; i>=0; i--) {
		if( write(fd, &((char*)&val)[i], 1) != 1) {
			return -1;
		}
	}
	return 0;
#endif /* LENDIAN */
}

static inline int write_ulong(int fd, unsigned long* val, int cnt)
{
#ifdef LENDIAN
	return (write(fd, val, sizeof(unsigned long)*cnt) !=
			sizeof(unsigned long)*cnt) ? -1 : 0;
#else
	int i, o;
	for(o=0; o<cnt; o++) {
		for(i=sizeof(unsigned long)-1; i>=0; i--) {
			if( write(fd, &((char*)&val[o])[i], 1) != 1 ) {
				return -1;
			}
		}
	}
	return 0;
#endif /* LENDIAN */
}

static inline int write_double(int fd, double* val, int cnt)
{
#ifdef LENDIAN

#ifdef IPAQ
	int ret, i;
	for(i=0; i<cnt; i++) {
		ret = write(fd, ((char*)(val+i))+sizeof(double)/2, sizeof(double)/2);
		ret +=write(fd, ((char*)(val+i)), sizeof(double)/2);
		if(ret != sizeof(double)) {
			return -1;
		}
	}
	return 0;
#else
	return (write(fd, val, sizeof(double)*cnt) !=
			sizeof(double)*cnt) ? -1 : 0;
#endif /* IPAQ */

#else
	int i, o;
	for(o=0; o<cnt; o++) {
		for(i=sizeof(double)-1; i>=0; i--) {
			if( write(fd, &((char*)&val[o])[i], 1) != 1 ) {
				return -1;
			}
		}
	}
	return 0;
#endif /* LENDIAN */
}

static void write_drecnum(void)
{
	if( lseek(fd, 236, SEEK_SET) != 236 ) {
		return;
	}

	(void)write_longlong(fd, &fheader.drecnum, 1);

	say("write_drecnum",
		"drecnum:%lld",
		fheader.drecnum);
}

static int write_header(void)
{
	gf_vheader_t*	vh_ptr;

	if( lseek(fd, 0, SEEK_SET) != 0 ) {
		return -1;
	}

	/* write fixed header */
	if( write(fd, fheader.version, GF_VER_SIZE) != GF_VER_SIZE ) {
		return -2;
	}
	if( write(fd, fheader.patid, GF_PATID_SIZE) != GF_PATID_SIZE ) {
		return -3;
	}
	if( write(fd, fheader.recid, GF_RECID_SIZE) != GF_RECID_SIZE ) {
		return -4;
	}
	if( write(fd, fheader.recst, GF_RECST_SIZE) != GF_RECST_SIZE ) {
		return -5;
	}
	if( write_longlong(fd, &fheader.hrecbnum, 1) ) {
		return -6;
	}
	if( write_ulonglong(fd, fheader.epid) ) {
		return -7;
	}
	if( write_ulonglong(fd, fheader.labid) ) {
		return -8;
	}
	if( write_ulonglong(fd, fheader.tid) ) {
		return -9;
	}
	if( write(fd, fheader.sernum, GF_SNUM_SIZE) != GF_SNUM_SIZE ) {
		return -10;
	}
	if( write_longlong(fd, &fheader.drecnum, 1) ) {
		return -11;
	}
	if( write_ulong(fd, fheader.drecdur, 2) ) {
		return -12;
	}
	if( write_ulong(fd, &fheader.snum, 1) ) {
		return -13;
	}

	/* write variable header */
	for( vh_ptr = head.cqh_first; vh_ptr != (void*)&head; 
		 vh_ptr = vh_ptr->entries.cqe_next) {
		if( write(fd, vh_ptr->label, GF_LABEL_SIZE) != GF_LABEL_SIZE ) {
			return -14;
		}
	}
	for( vh_ptr = head.cqh_first; vh_ptr != (void*)&head; 
		 vh_ptr = vh_ptr->entries.cqe_next) {
		if( write(fd, vh_ptr->ttype, GF_TTYPE_SIZE) != GF_TTYPE_SIZE ) {
			return -15;
		}
	}
	for( vh_ptr = head.cqh_first; vh_ptr != (void*)&head; 
		 vh_ptr = vh_ptr->entries.cqe_next) {
		if( write(fd, vh_ptr->pdim, GF_PDIM_SIZE) != GF_PDIM_SIZE ) {
			return -16;
		}
	}
	for( vh_ptr = head.cqh_first; vh_ptr != (void*)&head; 
		 vh_ptr = vh_ptr->entries.cqe_next) {
		if( write_double(fd, &vh_ptr->pmin, 1) ) {
			return -17;
		}
	}
	for( vh_ptr = head.cqh_first; vh_ptr != (void*)&head; 
		 vh_ptr = vh_ptr->entries.cqe_next) {
		if( write_double(fd, &vh_ptr->pmax, 1) ) {
			return -18;
		}
	}
	for( vh_ptr = head.cqh_first; vh_ptr != (void*)&head; 
		 vh_ptr = vh_ptr->entries.cqe_next) {
		if( write_longlong(fd, &vh_ptr->dmin, 1) ) {
			return -19;
		}
	}
	for( vh_ptr = head.cqh_first; vh_ptr != (void*)&head; 
		 vh_ptr = vh_ptr->entries.cqe_next) {
		if( write_longlong(fd, &vh_ptr->dmax, 1) ) {
			return -20;
		}
	}
	for( vh_ptr = head.cqh_first; vh_ptr != (void*)&head; 
		 vh_ptr = vh_ptr->entries.cqe_next) {
		if( write(fd, vh_ptr->pfilt, GF_PFILT_SIZE) != GF_PFILT_SIZE ) {
			return -21;
		}
	}
	for( vh_ptr = head.cqh_first; vh_ptr != (void*)&head; 
		 vh_ptr = vh_ptr->entries.cqe_next) {
		if( write_ulong(fd, &vh_ptr->srecnum, 1) ) {
			return -22;
		}
	}
	for( vh_ptr = head.cqh_first; vh_ptr != (void*)&head; 
		 vh_ptr = vh_ptr->entries.cqe_next) {
		if( write_ulong(fd, &vh_ptr->chtype, 1) ) {
			return -23;
		}
	}
	for( vh_ptr = head.cqh_first; vh_ptr != (void*)&head; 
		 vh_ptr = vh_ptr->entries.cqe_next) {
		if( write(fd, vh_ptr->rsv, GF_RSV_SIZE) != GF_RSV_SIZE ) {
			return -24;
		}
	}

	say("write_header",
		"write_header");

	return 0;
}

static int write_samples(void)
{
	gf_vheader_t*	vh_ptr;
	int				offset;
	bsv_data_t*		sample_ptr;

	sample_ptr	= (bsv_data_t*)g_async_queue_pop(sample_q_ptr);

	/* Should exit? */
	if( !sample_ptr->size ) {
		g_free(sample_ptr);

		return -1;
	}

	offset	= 0;
	for( vh_ptr = head.cqh_first; vh_ptr != (void*)&head;
		 vh_ptr = vh_ptr->entries.cqe_next) {
		switch( vh_ptr->chtype ) {
			case BSV_CHTYPE_CHAR:
			case BSV_CHTYPE_INT8:
			case BSV_CHTYPE_UINT8:
				if( write(fd, 
						  &(sample_ptr->samples)[offset], 
						  vh_ptr->srecnum) != vh_ptr->srecnum ) {

					g_free((void*)sample_ptr->samples);
					g_free(sample_ptr);
					return -2;
				}
				offset	+= sizeof(char)*vh_ptr->srecnum;
			break;

			case BSV_CHTYPE_INT16:
			case BSV_CHTYPE_UINT16:
				if( write_short(fd, 
								(short*)&(sample_ptr->samples)[offset], 
								vh_ptr->srecnum) ) {
	
					g_free((void*)sample_ptr->samples);
					g_free(sample_ptr);
					return -3;
				}
				offset	+= sizeof(short)*vh_ptr->srecnum;
			break;

			case BSV_CHTYPE_INT32:
			case BSV_CHTYPE_UINT32:
				if( write_ulong(fd, 
								(unsigned long*)&(sample_ptr->samples)[offset], 
								vh_ptr->srecnum) ) {
	
					g_free((void*)sample_ptr->samples);
					g_free(sample_ptr);
					return -4;
				}
				offset	+= sizeof(unsigned long)*vh_ptr->srecnum;
			break;

			case BSV_CHTYPE_INT64:
				if( write_longlong(fd, 
								   (long long*)&(sample_ptr->samples)[offset], 
								   vh_ptr->srecnum) ) {
	
					g_free((void*)sample_ptr->samples);
					g_free(sample_ptr);
					return -5;
				}
				offset	+= sizeof(long long)*vh_ptr->srecnum;
			break;

			case BSV_CHTYPE_FLT32:
				if( write_ulong(fd, 
								(unsigned long*)&(sample_ptr->samples)[offset], 
								vh_ptr->srecnum) ) {
	
					g_free((void*)sample_ptr->samples);
					g_free(sample_ptr);
					return -6;
				}
				offset	+= sizeof(unsigned long)*vh_ptr->srecnum;
			break;

			case BSV_CHTYPE_FLT64:
				if( write_double(fd, 
								 (double*)&(sample_ptr->samples)[offset], 
								 vh_ptr->srecnum) ) {
	
					g_free((void*)sample_ptr->samples);
					g_free(sample_ptr);
					return -7;
				}
				offset	+= sizeof(double)*vh_ptr->srecnum;
			break;

			default:
				terrormsg_ptr= "write_samples: Error: Chtype not implemented";

				g_free((void*)sample_ptr->samples);
				g_free(sample_ptr);
				return -8;
		}
	}

	/* increment record count */
	fheader.drecnum++;

	/* free sample ptr */
	g_free((void*)sample_ptr->samples);
	g_free(sample_ptr);
	return 0;
}

static void init_statusbar(GtkStatusbar* sbar)
{
	guint		ctxid;

	sbarcnt		= 0;
	rectime		= 0;

	say("init_statusbar",
		"sbar:0x%lx",
		(unsigned long*)sbar);

	gdk_threads_enter();
	ctxid	= gtk_statusbar_get_context_id(sbar, "file");
	gtk_statusbar_pop(sbar, ctxid);
	gtk_statusbar_push(sbar, ctxid, "Recording time: 00:00:00");
	gdk_threads_leave();

	say("init_statusbar",
		"init");
}

static void update_statusbar(GtkStatusbar* sbar)
{
	guint			ctxid;
	unsigned long	rectime_new;
	int				sec, min, hour;
	char			str[50];

	sbarcnt++;

	rectime_new	= 
			(unsigned long)((sbarcnt*fheader.drecdur[0])/fheader.drecdur[1]);
	if( rectime_new != rectime ) {
		sec	= rectime_new%60;
		min	= (rectime_new/60)%60;
		hour= (rectime_new/(60*60))%60;
	
		sprintf(str, "Recording time: %02d:%02d:%02d", hour, min, sec);
	
		/* Update statusbar */
		gdk_threads_enter();
		ctxid	= gtk_statusbar_get_context_id(sbar, "file");
		gtk_statusbar_pop(sbar, ctxid);
		gtk_statusbar_push(sbar, ctxid, str);
		gdk_threads_leave();

		/* Save rectime */
		rectime	= rectime_new;	
	}
}

static void clear_statusbar(GtkStatusbar* sbar)
{
	guint	ctxid;

	gdk_threads_enter();
	ctxid	= gtk_statusbar_get_context_id(sbar, "file");
	gtk_statusbar_pop(sbar, ctxid);
	gdk_threads_leave();
}

static void* write_data(void* param)
{
	GtkStatusbar*	sbar;
	int				ret;

	/* Fill in start time */
	setup_fheader_starttime();

	/* Init statusbar */
	sbar	= (GtkStatusbar*)lookup_widget(bsv_main_win, "statusbar");
	init_statusbar(sbar);

	/* Write fheader and vheader */
	if( (ret=write_header()) ) {
		terrormsg_ptr	= "write_data: Error: Write header failed";

		goto exit;
	}

	/* Set new state */
	g_mutex_lock(state_mtx_ptr);
	if( state == ALIVE ) {
		state	= RUNNING;
	} else {
		g_mutex_unlock(state_mtx_ptr);
		goto exit;
	}
	g_mutex_unlock(state_mtx_ptr);

	say("write_data",
		"RUNNING");

	while(1) {
		if( (ret = write_samples()) ) {

			/* Error */
			goto exit;
		}

		update_statusbar(sbar);
	}

exit:
	write_drecnum();
	clear_statusbar(sbar);

	g_mutex_lock(state_mtx_ptr);
	state	= DEAD;
	g_mutex_unlock(state_mtx_ptr);
	
	say("write_data",
		"DEAD");
	
	g_thread_exit(NULL);
}

static void sample_q_exit(void)
{
	bsv_data_t*		ptr;

	/* Signal thread to exit */
	ptr	= (bsv_data_t*)g_malloc(sizeof(bsv_data_t));
	ptr->samples	= NULL;
	ptr->size		= 0;
	g_async_queue_push(sample_q_ptr, ptr);
}

static void sample_q_clear(void)
{
	bsv_data_t*		ptr;

	while( (ptr = (bsv_data_t*)g_async_queue_try_pop(sample_q_ptr)) ) {
		g_free((void*)ptr->samples);
		g_free(ptr);
	}
}


/* Public functions ***********************************************************/

void gf_init(void)
{
	CIRCLEQ_INIT(&head);
	state_mtx_ptr	= g_mutex_new();

	sample_q_ptr	= g_async_queue_new();
}

void gf_destroy(void)
{
	gf_cleanup();

	g_mutex_free(state_mtx_ptr);	

	g_async_queue_unref(sample_q_ptr);
}

int gf_setup(void)
{
	say("gf_setup",
		"setup");

	/* Try to open file */
	if( (fd = open(p_get_logfile(), O_RDWR | O_CREAT | O_TRUNC,
									S_IRUSR | S_IWUSR)) == -1 ) {
		/* This is no error */
		return 0;
	}

	/* Setup */
	setup_fheader();
	
	if( setup_vheader() ) {
		errormsg_ptr	= "gf_setup: Error: Could not setup variable header";	
		return -1;
	}

	g_mutex_lock(state_mtx_ptr);
	if( state != DEAD ) {
		/* Set new state */
		state	= DEAD;
		g_mutex_unlock(state_mtx_ptr);

		/* Error */
		errormsg_ptr	= "gf_setup: Error: Writer thread was running";
		return -1;
	}

	/* set new state */
	state	= ALIVE;
	g_mutex_unlock(state_mtx_ptr);

	/* start thread */
	writer_ptr	= g_thread_create(write_data, NULL, TRUE, NULL);	

	return 0;
}

void gf_cleanup(void)
{
	if( fd == -1 ) {
		return;
	}

	g_mutex_lock(state_mtx_ptr);
	if( state != RUNNING && state != ALIVE ) {
		g_mutex_unlock(state_mtx_ptr);

		/* Error */
		goto exit;
	}
	
	/* set new state */
	state	= DYING;
	g_mutex_unlock(state_mtx_ptr);

	sample_q_exit();
	g_thread_join(writer_ptr);

exit:
	clear_circleq();
	sample_q_clear();
	close(fd);
	fd				= -1;
	terrormsg_ptr	= NULL;
}

int gf_set_samples(void* s_ptr)
{
	bsv_data_t*		sample_ptr;
	bsv_data_t*		cpy_sample_ptr;

	if( fd == -1 ) {
		return 0;
	}

	/* check if thread is running */
	g_mutex_lock(state_mtx_ptr);
	switch( state ) {
		case RUNNING:
			g_mutex_unlock(state_mtx_ptr);

			/* Continue */
			break;

		case ALIVE:
			g_mutex_unlock(state_mtx_ptr);

			/* Ignore samples */
			return 0;

		default:
			g_mutex_unlock(state_mtx_ptr);
			if( terrormsg_ptr ) {
				errormsg_ptr	= terrormsg_ptr;
				terrormsg_ptr	= NULL;
			} else {
				errormsg_ptr	= "gf_set_samples: Error: Thread has died "
								  "unexpectedly";
			}
			return -1;
	}
	g_mutex_unlock(state_mtx_ptr);

	sample_ptr		= (bsv_data_t*)s_ptr;

	/* Append samples */
	cpy_sample_ptr	
				= (bsv_data_t*)g_memdup(sample_ptr, sizeof(bsv_data_t));
	cpy_sample_ptr->samples	
				= (char*)g_memdup(sample_ptr->samples, sample_ptr->size);
	g_async_queue_push(sample_q_ptr, cpy_sample_ptr);

	say("gf_set_samples",
		"s:%d",
		*(short*)(sample_ptr->samples));

	return 0;
}

char* gf_get_errormsg(void)
{
	return errormsg_ptr;
}

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include <spa20a.h>

#include "interface.h"
#include "support.h"

#include "bsv_module.h"
#include "main.h"


/* Debug **********************************************************************/

//#define DEBUG
#undef DEBUG
#ifdef DEBUG
#define say(fu, fmt, arg...)	printf(fu":"fmt"\n", ##arg)
#else
#define say(fu, fmt, arg...)
#endif


/* Exported functions *********************************************************/

#define bsv_init			gmobilab_LTX_bsv_init
#define bsv_destroy			gmobilab_LTX_bsv_destroy
#define bsv_start			gmobilab_LTX_bsv_start
#define bsv_stop			gmobilab_LTX_bsv_stop
#define bsv_get_config_win	gmobilab_LTX_bsv_get_config_win
#define bsv_get_channelinfo	gmobilab_LTX_bsv_get_channelinfo
#define bsv_get_moduleinfo	gmobilab_LTX_bsv_get_moduleinfo
#define bsv_get_data		gmobilab_LTX_bsv_get_data
#define bsv_get_errormsg	gmobilab_LTX_bsv_get_errormsg


/* Defines ********************************************************************/

#define MAXCHANNUM			10
#define MAXDEVNOD			60
#define DCHANNUM			2


/* Typdefs ********************************************************************/

typedef struct conf {
	bsv_modid_t		modid;
	char			devnod[MAXDEVNOD];
	unsigned char	chanactivelist[MAXCHANNUM];	
} conf_t;


/* Global variables ***********************************************************/

GtkWidget*			config_win_ptr	= NULL;
GtkWidget*			file_select_ptr	= NULL;
bsv_modinfo_t		modinfo			
	= {	/* author */	
		"g.mobilab",			/* name */
		333,					/* id */
		"GPL-2",				/* licence */
		"gandy@sbox.tugraz.at",	/* author */
		MAXCHANNUM,				/* channum */
		33,						/* epid */
		0,						/* labid */
		0,						/* tid */
		"g.mobilab",			/* sernum */
		1,						/* sratenom */
		256						/* sratedenom */
	};
bsv_channelinfo_t	chaninfo[MAXCHANNUM]	
	= {		
		{
			"EEGEOG1",			/* label */
			"Ag/AgCl",			/* transducer type */
			"uV",				/* phys. dimension */
			-100,				/* phys. minimum */
			100,				/* phys. maximum */
			-1*(1<<15),			/* digital minimum */
			(1<<15)-1,			/* digital maximum */
			"LP:30Hz HP:2Hz",	/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_INT16	/* channel type */
		},
		{
			"EEGEOG2",			/* label */
			"Ag/AgCl",			/* transducer type */
			"uV",				/* phys. dimension */
			-100,				/* phys. minimum */
			100,				/* phys. maximum */
			-1*(1<<15),			/* digital minimum */
			(1<<15)-1,			/* digital maximum */
			"LP:30Hz HP:2Hz",	/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_INT16	/* channel type */
		},
		{
			"EEGEOG3",			/* label */
			"Ag/AgCl",			/* transducer type */
			"uV",				/* phys. dimension */
			-500,				/* phys. minimum */
			500,				/* phys. maximum */
			-1*(1<<15),			/* digital minimum */
			(1<<15)-1,			/* digital maximum */
			"LP:30Hz HP:0.01Hz",/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_INT16	/* channel type */
		},
		{
			"EEGEOG4",			/* label */
			"Ag/AgCl",			/* transducer type */
			"uV",				/* phys. dimension */
			-500,				/* phys. minimum */
			500,				/* phys. maximum */
			-1*(1<<15),			/* digital minimum */
			(1<<15)-1,			/* digital maximum */
			"LP:30Hz HP:0.01Hz",/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_INT16	/* channel type */
		},
		{
			"ECGEMG1",			/* label */
			"Ag/AgCl",			/* transducer type */
			"mV",				/* phys. dimension */
			-5,					/* phys. minimum */
			5,					/* phys. maximum */
			-1*(1<<15),			/* digital minimum */
			(1<<15)-1,			/* digital maximum */
			"LP:100Hz HP:0.5Hz",/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_INT16	/* channel type */
		},
		{
			"ECGEMG2",			/* label */
			"Ag/AgCl",			/* transducer type */
			"mV",				/* phys. dimension */
			-5,					/* phys. minimum */
			5,					/* phys. maximum */
			-1*(1<<15),			/* digital minimum */
			(1<<15)-1,			/* digital maximum */
			"LP:100Hz HP:0.5Hz",/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_INT16	/* channel type */
		},
		{
			"Analog1",			/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-5,					/* phys. minimum */
			5,					/* phys. maximum */
			-1*(1<<15),			/* digital minimum */
			(1<<15)-1,			/* digital maximum */
			"LP:100Hz HP:DC",	/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_INT16	/* channel type */
		},
		{
			"Analog2",			/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-5,					/* phys. minimum */
			5,					/* phys. maximum */
			-1*(1<<15),			/* digital minimum */
			(1<<15)-1,			/* digital maximum */
			"LP:100Hz HP:DC",	/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_INT16	/* channel type */
		},
		{
			"Digital1",			/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-5,					/* phys. minimum */
			5,					/* phys. maximum */
			-1*(1<<15),			/* digital minimum */
			(1<<15)-1,			/* digital maximum */
			"",					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_INT16	/* channel type */
		},
		{
			"Digital2",			/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-5,					/* phys. minimum */
			5,					/* phys. maximum */
			-1*(1<<15),			/* digital minimum */
			(1<<15)-1,			/* digital maximum */
			"",					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_INT16	/* channel type */
		}
	};


static HANDLE				fd			= 0;
static _ERRSTR				errstr;
static _BUFFER_ST			b_st;
static char					samples_ping[MAXCHANNUM*sizeof(short)];
static char					samples_pong[MAXCHANNUM*sizeof(short)];
static int					state		= 0;
static bsv_data_t			data;
static conf_t				c_conf;
static int					chanactivcnt= 0;
static short				dchanmask	= 0;

/* Private functions **********************************************************/

static void c_load_default(void)
{
	int	i;

	c_conf.modid					= BSV_GET_MODID(&modinfo);
	c_conf.devnod[0]				= 0;
	for(i=0; i<MAXCHANNUM; i++) {
		c_conf.chanactivelist[i]	= 0;
	}
	chanactivcnt					= 0;
}

static void c_set_checkbox(void)
{
	GtkToggleButton*	tb;

	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "eegeog1_chk");
	gtk_toggle_button_set_active(tb, c_is_active(0));
	
	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "eegeog2_chk");
	gtk_toggle_button_set_active(tb, c_is_active(1));
	
	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "eegeog3_chk");
	gtk_toggle_button_set_active(tb, c_is_active(2));
	
	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "eegeog4_chk");
	gtk_toggle_button_set_active(tb, c_is_active(3));
	
	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "ecgemg1_chk");
	gtk_toggle_button_set_active(tb, c_is_active(4));
	
	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "ecgemg2_chk");
	gtk_toggle_button_set_active(tb, c_is_active(5));
	
	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "analog1_chk");
	gtk_toggle_button_set_active(tb, c_is_active(6));
	
	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "analog2_chk");
	gtk_toggle_button_set_active(tb, c_is_active(7));
	
	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "digital1_chk");
	gtk_toggle_button_set_active(tb, c_is_active(8));
	
	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "digital2_chk");
	gtk_toggle_button_set_active(tb, c_is_active(9));	
}

static void c_get_checkbox(void)
{
	GtkToggleButton*	tb;

	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "eegeog1_chk");
	c_conf.chanactivelist[0]	= gtk_toggle_button_get_active(tb);

	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "eegeog2_chk");
	c_conf.chanactivelist[1]	= gtk_toggle_button_get_active(tb);

	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "eegeog3_chk");
	c_conf.chanactivelist[2]	= gtk_toggle_button_get_active(tb);

	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "eegeog4_chk");
	c_conf.chanactivelist[3]	= gtk_toggle_button_get_active(tb);

	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "ecgemg1_chk");
	c_conf.chanactivelist[4]	= gtk_toggle_button_get_active(tb);

	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "ecgemg2_chk");
	c_conf.chanactivelist[5]	= gtk_toggle_button_get_active(tb);

	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "analog1_chk");
	c_conf.chanactivelist[6]	= gtk_toggle_button_get_active(tb);

	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "analog2_chk");
	c_conf.chanactivelist[7]	= gtk_toggle_button_get_active(tb);

	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "digital1_chk");
	c_conf.chanactivelist[8]	= gtk_toggle_button_get_active(tb);

	tb	= (GtkToggleButton*)lookup_widget(config_win_ptr, "digital2_chk");
	c_conf.chanactivelist[9]	= gtk_toggle_button_get_active(tb);	
}

static void c_set_entry(void)
{
	GtkEntry*	entry;

	entry	= (GtkEntry*)lookup_widget(config_win_ptr, "devnode_entry");
	gtk_entry_set_text(entry, c_conf.devnod);
}

static void c_get_entry(void)
{
	GtkEntry*	entry;

	entry	= (GtkEntry*)lookup_widget(config_win_ptr, "devnode_entry");
	g_strlcpy(c_conf.devnod,
			  (char*)gtk_entry_get_text(entry),
			  MAXDEVNOD);
}


/* Public functions ***********************************************************/

void c_load(const char* path)
{
	char*	str;
	int		fd;
	conf_t	last_conf;

	c_load_default();

	str	= g_strconcat(path, "/.bsview_driver_gmobilab", NULL);
	if( (fd = open(str, O_RDONLY)) != -1 ) {
		if( read(fd, &last_conf, sizeof(conf_t)) == sizeof(conf_t) ) {
			if( BSV_IS_MODID(last_conf.modid, c_conf.modid) ) {
				memcpy(&c_conf, &last_conf, sizeof(conf_t));
			}
		}

		close(fd);
	}

	g_free(str);

	c_set_checkbox();
	c_set_entry();
}

void c_save(const char* path)
{
	char*	str;
	int		fd;

	str	= g_strconcat(path, "/.bsview_driver_gmobilab", NULL);
	if( (fd	= open(str,
				   O_RDWR | O_TRUNC | O_CREAT,
				   S_IRUSR | S_IWUSR)) != -1 ) {
		write(fd, &c_conf, sizeof(conf_t));
		close(fd);
	}

	g_free(str);
}

void c_update(void)
{
	c_get_entry();
	c_get_checkbox();
}

const char* c_get_devnode(void)
{
	return c_conf.devnod;
}

bsv_channelinfo_t* c_get_chaninfo(int cnt)
{
	int					my_cnt;
	int 				index;
	bsv_channelinfo_t*	ret		= NULL;

	for(my_cnt=0, index=0; index < MAXCHANNUM; index++) {
		if(c_is_active(index)) {
			/* Found */
			my_cnt++;

			if( my_cnt >= (cnt+1) ) {
				/* Reached cnt */
				ret	= &chaninfo[index];
				break;
			}
		}
	}

	say("c_get_chaninfo",
		"index:%d cnt:%d ret:0x%lx",
		index, cnt, (unsigned long)ret);

	return ret;
}

unsigned char c_is_active(int channum)
{
	say("c_is_active",
		"channum:%d act:%d", 
		channum, c_conf.chanactivelist[channum]);

	return c_conf.chanactivelist[channum];
}


/* Public exported functions **************************************************/

int bsv_init(const char* config_path)
{
	config_win_ptr	= create_dialog1();
	file_select_ptr	= create_fileselection1();
	if( !config_win_ptr || !file_select_ptr ) {
		sprintf(errstr.Error, "bsv_init: Error: Could not create dialog\n");
		return -1;
	}

	c_load(config_path);

	return 0;
}

int bsv_destroy(const char* config_path)
{
	bsv_stop();

	c_save(config_path);

	return 0;
}

int bsv_start(void)
{
	int		i;
	_AIN	ain;
	_DIO	dio;

	fd	= GT_OpenDevice( c_get_devnode() );
	if( !fd ) {
		GT_TranslateErrorCode(&errstr, 0);
		return -1;
	}

	/* Analog channels */
	for(i=0, chanactivcnt=0; i<MAXCHANNUM-DCHANNUM; i++) {
		if( c_is_active(i) ) {
			chanactivcnt++;
		}
	}
	/* Digital channels */
	if( c_is_active(8) || c_is_active(9) ) {
		chanactivcnt++;
	}
	dchanmask	= ((c_is_active(8))?1:0)+((c_is_active(9))?2:0);

	ain.ain1	= (c_is_active(0)) ? TRUE : FALSE;
	ain.ain2	= (c_is_active(1)) ? TRUE : FALSE;
	ain.ain3	= (c_is_active(2)) ? TRUE : FALSE;
	ain.ain4	= (c_is_active(3)) ? TRUE : FALSE;
	ain.ain5	= (c_is_active(4)) ? TRUE : FALSE;
	ain.ain6	= (c_is_active(5)) ? TRUE : FALSE;
	ain.ain7	= (c_is_active(6)) ? TRUE : FALSE;
	ain.ain8	= (c_is_active(7)) ? TRUE : FALSE;

	if( c_is_active(8) || c_is_active(9) ) {
		dio.scan		= TRUE;
	} else {
		dio.scan		= FALSE;
	}
	dio.dio1_direction	= (c_is_active(8)) ? TRUE : FALSE;
	dio.dio2_direction	= (c_is_active(9)) ? TRUE : FALSE;

	if( !GT_InitChannels(fd, ain, dio) ) {
		GT_TranslateErrorCode(&errstr, 0);
		GT_CloseDevice(fd);
		fd	= 0;
		return -1;
	}

	if( !GT_StartAcquistion(fd) ) {
		GT_TranslateErrorCode(&errstr, 0);
		GT_CloseDevice(fd);
		fd	= 0;
		return -1;
	}

	state	= 0;

	return 0;
}

int bsv_stop(void)
{
	if( fd != 0 ) {
		GT_StopAcquistion(fd);
		GT_CloseDevice(fd);
	}

	return 0;
}

void* bsv_get_config_win(void)
{
	return (void*)config_win_ptr;
}

bsv_channelinfo_t* bsv_get_channelinfo(int channel_num)
{
	return c_get_chaninfo(channel_num);
}

bsv_modinfo_t* bsv_get_moduleinfo(void)
{
	return &modinfo;
}

bsv_data_t* bsv_get_data(void)
{
	short val;
	int	  cnt, i;

	if( !state ) {
		b_st.pBuffer= (short*)samples_ping;
		state		= 1;
	} else {
		b_st.pBuffer= (short*)samples_pong;
		state		= 0;
	}
	b_st.size		= chanactivcnt*sizeof(short);
	b_st.validPoints= 0;

	if( !GT_GetData(fd, &b_st, TRUE) ) {
		GT_TranslateErrorCode(&errstr, 0);
		return NULL;
	}

	if( !b_st.validPoints ) {
		GT_TranslateErrorCode(&errstr, 0);
		return NULL;
	}

	if( dchanmask ) {
		/* expand samples from digital channels */
		val	= b_st.pBuffer[(cnt=(b_st.validPoints-1))];
		for(i=0; i<DCHANNUM; i++) {
			if( dchanmask&(1<<i) ) {
				b_st.pBuffer[cnt]	= ( (val&(1<<i)) ? (1<<15)-2 : 0 );
				cnt++;
			}
		}
	}

	data.samples	= (char*)b_st.pBuffer;
	data.size		= chanactivcnt*sizeof(short);

	return &data;
}

char* bsv_get_errormsg(void)
{
	return errstr.Error;
}

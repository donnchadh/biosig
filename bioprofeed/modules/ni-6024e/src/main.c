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
#include <comedilib.h>

#include "interface.h"
#include "support.h"

#include "bsv_module.h"
#include "main.h"


/* Debug **********************************************************************/

//#define MYDEBUG
#undef MYDEBUG
#ifdef MYDEBUG
#define say(fu, fmt, arg...)	fprintf(stderr, fu":"fmt"\n", ##arg)
#else
#define say(fu, fmt, arg...)
#endif


/* Exported functions *********************************************************/

#define bsv_init			ni6024e_LTX_bsv_init
#define bsv_destroy			ni6024e_LTX_bsv_destroy
#define bsv_start			ni6024e_LTX_bsv_start
#define bsv_stop			ni6024e_LTX_bsv_stop
#define bsv_get_config_win	ni6024e_LTX_bsv_get_config_win
#define bsv_get_channelinfo	ni6024e_LTX_bsv_get_channelinfo
#define bsv_get_moduleinfo	ni6024e_LTX_bsv_get_moduleinfo
#define bsv_get_data		ni6024e_LTX_bsv_get_data
#define bsv_get_errormsg	ni6024e_LTX_bsv_get_errormsg


/* Defines ********************************************************************/

#define MAXCHANNUM			16
#define MAXHP				4+1
#define MAXTP				4+1
#define COMEDI_DEVNOD		"/dev/comedi0"
#define MAXRANGE			16
#define MAXSRATE			5
#define MAXREF				4

/* Typdefs ********************************************************************/

typedef struct chan {
	unsigned char		activ;
	int					range;
	int					ref;
	char				hp[MAXHP];
	char				tp[MAXTP];
	bsv_channelinfo_t*	chaninfo_ptr;
} chan_t;

typedef struct conf {
	bsv_modid_t			modid;
	bsv_modinfo_t*		modinfo_ptr;
	int					srate;
	chan_t				chanlist[MAXCHANNUM];
} conf_t;

typedef struct range {
	double		min;
	double		max;
} range_t;

typedef unsigned long	srate_t;

/* Global variables ***********************************************************/

GtkWidget*			config_win_ptr	= NULL;

bsv_modinfo_t		modinfo			
	= {		
		"NI 6024E",				/* name */
		355,					/* id */
		"GPL-2",				/* licence */
		"gandy@sbox.tugraz.at",	/* author */
		MAXCHANNUM,				/* channum */
		22,						/* epid */
		0,						/* labid */
		0,						/* tid */
		"NI 6024E",				/* sernum */
		1,						/* sratenom */
		0						/* sratedenom */
	};

bsv_channelinfo_t	chaninfo[MAXCHANNUM]	
	= {		
		{
			"AnalogIn1",		/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-10,				/* phys. minimum */
			10,					/* phys. maximum */
			0,					/* digital minimum */
			4095,				/* digital maximum */
			0,					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_UINT16	/* channel type */
		},
		{
			"AnalogIn2",		/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-10,				/* phys. minimum */
			10,					/* phys. maximum */
			0,					/* digital minimum */
			4095,				/* digital maximum */
			0,					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_UINT16	/* channel type */
		},
		{
			"AnalogIn3",		/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-10,				/* phys. minimum */
			10,					/* phys. maximum */
			0,					/* digital minimum */
			4095,				/* digital maximum */
			0,					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_UINT16	/* channel type */
		},
		{
			"AnalogIn4",		/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-10,					/* phys. minimum */
			10,					/* phys. maximum */
			0,					/* digital minimum */
			4095,				/* digital maximum */
			0,					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_UINT16	/* channel type */
		},
		{
			"AnalogIn5",		/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-10,				/* phys. minimum */
			10,					/* phys. maximum */
			0,					/* digital minimum */
			4095,				/* digital maximum */
			0,					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_UINT16	/* channel type */
		},
		{
			"AnalogIn6",		/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-10,				/* phys. minimum */
			10,					/* phys. maximum */
			0,					/* digital minimum */
			4095,				/* digital maximum */
			0,					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_UINT16	/* channel type */
		},
		{
			"AnalogIn7",		/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-10,				/* phys. minimum */
			10,					/* phys. maximum */
			0,					/* digital minimum */
			4095,				/* digital maximum */
			0,					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_UINT16	/* channel type */
		},
		{
			"AnalogIn8",		/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-10,				/* phys. minimum */
			10,					/* phys. maximum */
			0,					/* digital minimum */
			4095,				/* digital maximum */
			0,					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_UINT16	/* channel type */
		},
		{
			"AnalogIn9",		/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-10,				/* phys. minimum */
			10,					/* phys. maximum */
			0,					/* digital minimum */
			4095,				/* digital maximum */
			0,					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_UINT16	/* channel type */
		},
		{
			"AnalogIn10",		/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-10,				/* phys. minimum */
			10,					/* phys. maximum */
			0,					/* digital minimum */
			4095,				/* digital maximum */
			0,					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_UINT16	/* channel type */
		},
		{
			"AnalogIn11",		/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-10,				/* phys. minimum */
			10,					/* phys. maximum */
			0,					/* digital minimum */
			4095,				/* digital maximum */
			0,					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_UINT16	/* channel type */
		},
		{
			"AnalogIn12",		/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-10,				/* phys. minimum */
			10,					/* phys. maximum */
			0,					/* digital minimum */
			4095,				/* digital maximum */
			0,					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_UINT16	/* channel type */
		},
		{
			"AnalogIn13",		/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-10,				/* phys. minimum */
			10,					/* phys. maximum */
			0,					/* digital minimum */
			4095,				/* digital maximum */
			0,					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_UINT16	/* channel type */
		},
		{
			"AnalogIn14",		/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-10,				/* phys. minimum */
			10,					/* phys. maximum */
			0,					/* digital minimum */
			4095,				/* digital maximum */
			0,					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_UINT16	/* channel type */
		},
		{
			"AnalogIn15",		/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-10,				/* phys. minimum */
			10,					/* phys. maximum */
			0,					/* digital minimum */
			4095,				/* digital maximum */
			0,					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_UINT16	/* channel type */
		},
		{
			"AnalogIn16",		/* label */
			"Ag/AgCl",			/* transducer type */
			"V",				/* phys. dimension */
			-10,				/* phys. minimum */
			10,					/* phys. maximum */
			0,					/* dig. minimum */
			4095,				/* dig. maximum */
			0,					/* prefiltering */
			1,					/* numbers of samples */
			BSV_CHTYPE_UINT16	/* channel type */
		}
	};

static srate_t	srate_tbl[MAXSRATE]	= 
	{
		256,
		128,
		64,
		32,
		16
	};

static range_t	range_tbl[MAXRANGE]	= 
	{
		{-10, 10},
		{-5, 5},
		{-2.5, 2.5},
		{-1, 1},
		{-0.5, 0.5},
		{-0.25, 0.25},
		{-0.1, 0.1},
		{-0.05, 0.05},
		{0, 20},
		{0, 10},
		{0, 5},
		{0, 2},
		{0, 1},
		{0, 0.5},
		{0, 0.2},
		{0, 0.1}
	};

static const char*	ref_tbl[MAXREF]	=
	{
		"GND",
		"COMMON",
		"DIFF",
		"OTHER"
	};

static sampl_t				samples_ping[MAXCHANNUM];
static sampl_t				samples_pong[MAXCHANNUM];
static int					state		= 0;
static bsv_data_t			data;
static conf_t				c_conf;
static int					chanactivcnt= 0;
static char*				errstr_ptr	= NULL;
static comedi_t*			dev;
static int					subdev;
static unsigned char		is_open		= 0;


/* Private c_conf member functions ********************************************/

static void c_load_default(void)
{
	int	i;

	c_conf.srate						= 0;
	c_conf.modid						= BSV_GET_MODID(&modinfo);
	c_conf.modinfo_ptr					= &modinfo;
	for(i=0; i<MAXCHANNUM; i++) {
		c_conf.chanlist[i].range		= 0;
		c_conf.chanlist[i].ref			= 0;
		c_conf.chanlist[i].hp[0]		= 0;
		c_conf.chanlist[i].tp[0]		= 0;
		c_conf.chanlist[i].activ		= 0;
		c_conf.chanlist[i].chaninfo_ptr	= &chaninfo[i];	
	}
}

static void c_dealloc(void)
{
	int	i;

	for(i=0; i<MAXCHANNUM; i++) {
		if( BSV_CH_GET_PFILT(c_conf.chanlist[i].chaninfo_ptr) ) {
			g_free((void*)BSV_CH_GET_PFILT(
					c_conf.chanlist[i].chaninfo_ptr));
		}
	}
}

/* Public c_conf member functions *********************************************/

static void c_load(const char* path)
{
	char*	str;
	int		fd;
	conf_t	last_conf;

	c_load_default();

	str	= g_strconcat(path, "/.bsview_driver_ni6024e", NULL);
	if( (fd = open(str, O_RDONLY)) != -1 ) {
		if( read(fd, &last_conf, sizeof(conf_t)) == sizeof(conf_t) ) {
			if( BSV_IS_MODID(last_conf.modid, c_conf.modid) ) {
				memcpy(&c_conf, &last_conf, sizeof(conf_t));
			}
		}

		close(fd);
	}

	g_free(str);
}

static void c_save(const char* path)
{
	char*	str;
	int		fd;

	str	= g_strconcat(path, "/.bsview_driver_ni6024e", NULL);
	if( (fd	= open(str,
				   O_RDWR | O_TRUNC | O_CREAT,
				   S_IRUSR | S_IWUSR)) != -1 ) {
		write(fd, &c_conf, sizeof(conf_t));
		close(fd);
	}

	g_free(str);

	c_dealloc();
}

static bsv_channelinfo_t* c_get_chaninfo(int cnt)
{
	int					my_cnt;
	int 				index;
	char				str[40];
	bsv_channelinfo_t*	ret		= NULL;

	for(my_cnt=0, index=0; index < MAXCHANNUM; index++) {
		if( c_conf.chanlist[index].activ ) {
			/* Found */
			my_cnt++;

			if( my_cnt >= (cnt+1) ) {
				/* Reached cnt */
				ret	= c_conf.chanlist[index].chaninfo_ptr;

				/* update range */
				BSV_CH_GET_PMIN(ret)
					= range_tbl[c_conf.chanlist[index].range].min;
				BSV_CH_GET_PMAX(ret)
					= range_tbl[c_conf.chanlist[index].range].max;

				/* update pfilt */
				snprintf(str, 40, "LP:%s HP:%s", c_conf.chanlist[index].tp, 
												c_conf.chanlist[index].hp);
				if( BSV_CH_GET_PFILT(ret) ) {
					g_free((void*)BSV_CH_GET_PFILT(ret));
				}
				BSV_CH_GET_PFILT(ret)	= g_strdup(str);

				break;
			}
		}
	}

	return ret;
}

static bsv_modinfo_t* c_get_modinfo(void)
{
	/* update sratedenom */
	BSV_GET_SRATEDENOM(c_conf.modinfo_ptr)	= srate_tbl[c_conf.srate];

	return c_conf.modinfo_ptr;
}

static void c_set_chan_activ(int channum, int activ)
{
	c_conf.chanlist[channum].activ	= activ;
}

static void c_set_chan_range(int channum, int range)
{
	c_conf.chanlist[channum].range	= range;
}

static void c_set_chan_hp(int channum, char* hp)
{
	g_strlcpy(c_conf.chanlist[channum].hp, 
			  hp, 
			  MAXHP);
}

static void c_set_chan_tp(int channum, char* tp)
{
	g_strlcpy(c_conf.chanlist[channum].tp,
			  tp,
			  MAXTP);
}

static void c_set_chan_ref(int channum, int ref)
{
	c_conf.chanlist[channum].ref	= ref;
}

static void c_set_srate(int srate)
{
	c_conf.srate	= srate;
}

static int c_get_srate(void)
{
	return c_conf.srate;
}

static int c_get_chan_ref(int channum)
{
	return c_conf.chanlist[channum].ref;
}

static int c_get_chan_range(int channum)
{
	return c_conf.chanlist[channum].range;
}

static char* c_get_chan_hp(int channum)
{
	return c_conf.chanlist[channum].hp;
}

static char* c_get_chan_tp(int channum)
{
	return c_conf.chanlist[channum].tp;
}

static unsigned char c_get_chan_activ(int channum)
{
	return c_conf.chanlist[channum].activ;
}


/* Public GUI function ********************************************************/

static void gui_init(void)
{
	int					i, u;
	GtkComboBox*		combo;
	GtkToggleButton*	toggle;
	GtkEntry*			entry;
	char				str[30];

	/* srate */
	combo	= (GtkComboBox*)lookup_widget(config_win_ptr, "srate_combo");
	for(u=0; u<MAXSRATE; u++) {
		snprintf(str, 30, "%ldHz", srate_tbl[u]);
		gtk_combo_box_append_text(combo, str);
	}

	for(i=0; i<MAXCHANNUM; i++) {
		/* range */
		snprintf(str, 30, "ch%d_rng_combo", i+1);
		combo	= (GtkComboBox*)lookup_widget(config_win_ptr, str);
		for(u=0; u<MAXRANGE; u++) {
			snprintf(str, 30, "(%5.2fV, %5.2fV)", range_tbl[u].min, 
											range_tbl[u].max);
			gtk_combo_box_append_text(combo, str);
		}
		/* ref */
		snprintf(str, 30, "ch%d_ref_combo", i+1);
		combo	= (GtkComboBox*)lookup_widget(config_win_ptr, str);
		for(u=0; u<MAXREF; u++) {
			snprintf(str, 30, "%s", ref_tbl[u]);
			gtk_combo_box_append_text(combo, str);
		}
	}

	/* srate */
	combo	= (GtkComboBox*)lookup_widget(config_win_ptr, "srate_combo");
	gtk_combo_box_set_active(combo, c_get_srate());

	for(i=0; i<MAXCHANNUM; i++) {
		/* checkbox */
		snprintf(str, 30, "ch%d_chk", i+1);
		toggle	= (GtkToggleButton*)lookup_widget(config_win_ptr, str);
		gtk_toggle_button_set_active(toggle, c_get_chan_activ(i));

		/* hp entry */
		snprintf(str, 30, "ch%d_hp_entry", i+1);
		entry	= (GtkEntry*)lookup_widget(config_win_ptr, str);
		gtk_entry_set_text(entry, c_get_chan_hp(i));

		/* tp entry */
		snprintf(str, 30, "ch%d_tp_entry", i+1);
		entry	= (GtkEntry*)lookup_widget(config_win_ptr, str);
		gtk_entry_set_text(entry, c_get_chan_tp(i));

		/* range */
		snprintf(str, 30, "ch%d_rng_combo", i+1);
		combo	= (GtkComboBox*)lookup_widget(config_win_ptr, str);
		gtk_combo_box_set_active(combo, c_get_chan_range(i));

		/* ref */
		snprintf(str, 30, "ch%d_ref_combo", i+1);
		combo	= (GtkComboBox*)lookup_widget(config_win_ptr, str);
		gtk_combo_box_set_active(combo, c_get_chan_ref(i));
	}
}

void gui_update(void)
{
	int					i;
	GtkComboBox*		combo;
	GtkToggleButton*	toggle;
	GtkEntry*			entry;
	char				str[30];

	/* srate */
	combo	= (GtkComboBox*)lookup_widget(config_win_ptr, "srate_combo");
	c_set_srate(gtk_combo_box_get_active(combo));

	for(i=0; i<MAXCHANNUM; i++) {
		/* checkbox */
		snprintf(str, 30, "ch%d_chk", i+1);
		toggle	= (GtkToggleButton*)lookup_widget(config_win_ptr, str);
		c_set_chan_activ(i, gtk_toggle_button_get_active(toggle));

		/* hp entry */
		snprintf(str, 30, "ch%d_hp_entry", i+1);
		entry	= (GtkEntry*)lookup_widget(config_win_ptr, str);
		c_set_chan_hp(i, (char*)gtk_entry_get_text(entry));

		/* tp entry */
		snprintf(str, 30, "ch%d_tp_entry", i+1);
		entry	= (GtkEntry*)lookup_widget(config_win_ptr, str);
		c_set_chan_tp(i, (char*)gtk_entry_get_text(entry));

		/* range */
		snprintf(str, 30, "ch%d_rng_combo", i+1);
		combo	= (GtkComboBox*)lookup_widget(config_win_ptr, str);
		c_set_chan_range(i, gtk_combo_box_get_active(combo));

		/* ref */
		snprintf(str, 30, "ch%d_ref_combo", i+1);
		combo	= (GtkComboBox*)lookup_widget(config_win_ptr, str);
		c_set_chan_ref(i, gtk_combo_box_get_active(combo));
	}
}


/* Public exported functions **************************************************/

int bsv_init(const char* config_path)
{
	comedi_range*	rng;
	int				i;

	config_win_ptr	= create_dialog1();
	if( !config_win_ptr ) {
		errstr_ptr	= "bsv_init: Error: Could not create dialog";
		return -1;
	}

	c_load(config_path);

	gui_init();

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
	comedi_cmd		cmd;
	unsigned int	chanlist[MAXCHANNUM];
	int				i;

	dev	= comedi_open( COMEDI_DEVNOD );
	if( !dev ) {
		errstr_ptr	= comedi_strerror( comedi_errno() );
		return -1;
	}

	subdev	= comedi_find_subdevice_by_type(dev, COMEDI_SUBD_AI, 0);
	comedi_lock(dev, subdev);

	is_open	= 1;

	chanactivcnt	= 0;
	for(i=0; i<MAXCHANNUM; i++) {
		if( c_get_chan_activ(i) ) {
			chanlist[chanactivcnt]	= CR_PACK(i, 
											  c_get_chan_range(i),
											  c_get_chan_ref(i));

			chanactivcnt++;
		}
	}

	memset(&cmd, 0, sizeof(cmd));

	cmd.subdev			= subdev;
	cmd.flags			= TRIG_RT | TRIG_WAKE_EOS;
	cmd.start_src		= TRIG_NOW;
	cmd.start_arg		= 0;
	cmd.scan_begin_src	= TRIG_TIMER;
	cmd.scan_begin_arg	= ((unsigned int)1000000000/srate_tbl[c_get_srate()]);
	cmd.convert_src		= TRIG_TIMER;
	cmd.convert_arg		= 5000;
	cmd.scan_end_src	= TRIG_COUNT;
	cmd.scan_end_arg	= chanactivcnt;
	cmd.stop_src		= TRIG_NONE;
	cmd.stop_arg		= 0;

	cmd.chanlist		= chanlist;
	cmd.chanlist_len	= chanactivcnt;

	if( (comedi_command(dev, &cmd)) < 0 ) {
		errstr_ptr	= comedi_strerror( comedi_errno() );
		return -1;
	}

	return 0;
}

int bsv_stop(void)
{
	if( is_open ) {
		comedi_cancel(dev, subdev);
		comedi_unlock(dev, subdev);
		comedi_close(dev);

		is_open	= 0;
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
	return c_get_modinfo();
}

bsv_data_t* bsv_get_data(void)
{
	struct timeval	tv;
	fd_set			set;
	int				help;
	int				size;

	tv.tv_sec	= 5;
	tv.tv_usec	= 0;

	FD_ZERO(&set);
	FD_SET(comedi_fileno(dev), &set);

	help	= select(comedi_fileno(dev)+1, &set, NULL, NULL, &tv);
	if( !help ) {
		errstr_ptr	= "bsv_get_data: Error: Timeout";
		return NULL;
	}

	if( state ) {
		data.samples	= (char*)samples_ping;
		size			= chanactivcnt*sizeof(sampl_t);
		data.size		= size;

		if( read(comedi_fileno(dev), samples_ping, size) != size ) {
			errstr_ptr	= "bsv_get_data: Error: Could not read data";
		}

		state	= 0;
	} else {
		data.samples	= (char*)samples_pong;
		size			= chanactivcnt*sizeof(sampl_t);
		data.size		= size;

		if( read(comedi_fileno(dev), samples_pong, size) != size ) {
			errstr_ptr	= "bsv_get_data: Error: Could not read data";
		}

		state	= 1;
	}

	return &data;
}

char* bsv_get_errormsg(void)
{
	return errstr_ptr;
}

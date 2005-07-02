#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <stdio.h>

#include <gtk/gtk.h>

#include "interface.h"
#include "support.h"

#include "bsv_module.h"

/* Exported functions *********************************************************/

#define bsv_init			testmodule_LTX_bsv_init
#define bsv_destroy			testmodule_LTX_bsv_destroy
#define bsv_start			testmodule_LTX_bsv_start
#define bsv_stop			testmodule_LTX_bsv_stop
#define bsv_get_config_win	testmodule_LTX_bsv_get_config_win
#define bsv_get_channelinfo	testmodule_LTX_bsv_get_channelinfo
#define bsv_get_moduleinfo	testmodule_LTX_bsv_get_moduleinfo
#define bsv_get_data		testmodule_LTX_bsv_get_data
#define bsv_get_errormsg	testmodule_LTX_bsv_get_errormsg


/* Global variables ***********************************************************/

GtkWidget*			config_win_ptr	= NULL;
bsv_modinfo_t		modinfo			= {	
										"testmodule",	/* name */
										100,			/* id */
										"GPL-2",		/* licence */
										"gandy@sbox.tugraz.at",
										3,				/* channum */
										19,				/* epid */
										20,				/* labid */
										99,				/* tid */
										"XXXXXXXX",		/* sernum */
										1,				/* sratenom */
										256				/* sratedenom */
									  };
bsv_channelinfo_t	chan1info		= {
										"testch1",		/* label */
										"Ag/AgCl",		/* transducer type */
										"uV",			/* phys. dimension */
										-100,			/* phys. minimum */
										100,			/* phys. maximum */
										-1*(1<<15),		/* digital minimum */
										(1<<15)-1,		/* digital maximum */
										"",				/* prefiltering */
										1,				/* numbers of samples */
										BSV_CHTYPE_INT16/* channel type */
									  };
bsv_channelinfo_t	chan2info		= {
										"testch2",		/* label */
										"Ag/AgCl",		/* transducer type */
										"uV",			/* phys. dimension */
										-100,			/* phys. minimum */
										100,			/* phys. maximum */
										-1*(1<<15),		/* digital minimum */
										(1<<15)-1,		/* digital maximum */
										"",				/* prefiltering */
										1,				/* numbers of samples */
										BSV_CHTYPE_INT16/* channel type */
									  };
bsv_channelinfo_t	chan3info		= {
										"testch3",		/* label */
										"Ag/AgCl",		/* transducer type */
										"uV",			/* phys. dimension */
										-100,			/* phys. minimum */
										100,			/* phys. maximum */
										-1*(1<<15),		/* digital minimum */
										(1<<15)-1,		/* digital maximum */
										"",				/* prefiltering */
										1,				/* numbers of samples */
										BSV_CHTYPE_INT16/* channel type */
									  };

static bsv_data_t	data;
static short		buf[3];

/* Public functions ***********************************************************/

int bsv_init(const char* config_path)
{
	config_win_ptr	= create_dialog1();
	if( !config_win_ptr ) {
		return -1;
	}

	return 0;
}

int bsv_destroy(const char* config_path)
{
	bsv_stop();

	return 0;
}

int bsv_start(void)
{
	buf[0]	= 0;
	buf[1]	= 0;
	buf[2]	= 0;

	return 0;
}

int bsv_stop(void)
{
	return 0;
}

void* bsv_get_config_win(void)
{
	return (void*)config_win_ptr;
}

bsv_channelinfo_t* bsv_get_channelinfo(int channel_num)
{
	switch( channel_num ) {
		case 0: return &chan1info;
		case 1: return &chan2info;
		case 2: return &chan3info;
		default:return NULL;
	}
}

bsv_modinfo_t* bsv_get_moduleinfo(void)
{
	return &modinfo;
}

bsv_data_t* bsv_get_data(void)
{
	buf[0]	+= 10;
	buf[1]	+= 20;
	buf[2]	+= 30;

	if( buf[0] > (1<<15)-40 ) {
		buf[0]	= -1*(1<<15)+40;
	}
	if( buf[1] > (1<<15)-40 ) {
		buf[1]	= -1*(1<<15)+40;
	}
	if( buf[2] > (1<<15)-40 ) {
		buf[2]	= -1*(1<<15)+40;
	}
	
	data.samples	= (char*)buf;
	data.size		= 3;

	usleep(4000);

	return &data;
}

char* bsv_get_errormsg(void)
{
	return "bsv_get_errormsg: Error";
}

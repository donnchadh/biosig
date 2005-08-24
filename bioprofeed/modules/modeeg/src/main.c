/* vim: set ts=4: */
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <gtk/gtk.h>

#include "interface.h"
#include "support.h"

#include "bsv_module.h"

#define	TIMEOUT				2	/* s */
#define BUF_LEN				6

/* Exported functions *********************************************************/

#define bsv_init			modeeg_LTX_bsv_init
#define bsv_destroy			modeeg_LTX_bsv_destroy
#define bsv_start			modeeg_LTX_bsv_start
#define bsv_stop			modeeg_LTX_bsv_stop
#define bsv_get_config_win	modeeg_LTX_bsv_get_config_win
#define bsv_get_channelinfo	modeeg_LTX_bsv_get_channelinfo
#define bsv_get_moduleinfo	modeeg_LTX_bsv_get_moduleinfo
#define bsv_get_data		modeeg_LTX_bsv_get_data
#define bsv_get_errormsg	modeeg_LTX_bsv_get_errormsg


/* Global variables ***********************************************************/

GtkWidget*			config_win_ptr	= NULL;
bsv_modinfo_t		modinfo			= {	
										"modeeg",		/* name */
										400,			/* id */
										"GPL-2",		/* licence */
										"gandy@sbox.tugraz.at",
										2,				/* channum */
										19,				/* epid */
										20,				/* labid */
										99,				/* tid */
										"XXXXXXXX",		/* sernum */
										1,				/* sratenom */
										256				/* sratedenom */
									  };
bsv_channelinfo_t	chan1info		= {
										"ch1",			/* label */
										"Ag/AgCl",		/* transducer type */
										"mV",			/* phys. dimension */
										-5,				/* phys. minimum */
										5,				/* phys. maximum */
										0,				/* digital minimum */
										1023,			/* digital maximum */
										"HP:0.16Hz LP:59Hz",/* prefiltering */
										1,				/* numbers of samples */
										BSV_CHTYPE_UINT16	/* channel type */
									  };
bsv_channelinfo_t	chan2info		= {
										"ch2",			/* label */
										"Ag/AgCl",		/* transducer type */
										"mV",			/* phys. dimension */
										-5,				/* phys. minimum */
										5,				/* phys. maximum */
										0,				/* digital minimum */
										1023,			/* digital maximum */
										"HP:0.16Hz LP:59Hz",/* prefiltering */
										1,				/* numbers of samples */
										BSV_CHTYPE_UINT16	/* channel type */
									  };

static bsv_data_t	data;
static char*		errormsg	= NULL;
static int			handle		= -1;

/* Private functions **********************************************************/

static inline int read_one_byte(int handle, unsigned char* byte,
								int timeout)
{
	struct timeval	tv;
	fd_set			set;
	int				help;

	if( timeout >= 0 ) {
		/* do blocking read */
		tv.tv_sec	= timeout;
		tv.tv_usec	= 0;

		FD_ZERO(&set);
		FD_SET(handle, &set);

		help	= select(handle+1, &set, NULL, NULL, &tv);
		if( !help ) {
			/* timeout */
			return -2;
		}
	}

	/* read something */
	help	= read(handle, byte, 1);

	if( help > 0 ) {
		/* data read */
		return 0;
	} else 
	if( !help ) {
		/* no data */
		return -3;
	} else {
		/* general error */
		return -1;
	}
}

static int open_device(const char* str)
{
	int				fd;
	struct termios	newtio;
	int				status;

	fd	= open(str,	O_RDWR |
					O_NOCTTY |
					O_NDELAY |
					O_NONBLOCK);
	if( fd == -1 ) {
		/* Error */
		return -1;
	}

	tcgetattr(fd, &newtio);
	newtio.c_cflag	&=	~CBAUD;
	newtio.c_cflag	|=	B57600;
	newtio.c_cflag	&=	~PARENB;
	newtio.c_cflag	&=	~CSTOPB;
	newtio.c_cflag	&=	~CSIZE;
	newtio.c_cflag	|=	CS8;
	newtio.c_cflag	&=	~CRTSCTS;
	newtio.c_cflag	|=	(CLOCAL |
						 CREAD);
	newtio.c_lflag	= 	0;
	newtio.c_iflag	=	IGNPAR | ICRNL;
	newtio.c_oflag	= 	0;
	tcsetattr(fd, TCSAFLUSH, &newtio);

	ioctl(fd, TIOCMGET, &status);
	status			|=	TIOCM_DTR;
	status			|=	TIOCM_RTS;
	ioctl(fd, TIOCMSET, &status);

	return fd;
}


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
	handle	= open_device("/dev/ttyUSB0");
	if( handle == -1 ) {
		errormsg	= "Error: bsv_start";
		return -1;
	}

	return 0;
}

int bsv_stop(void)
{
	close(handle);

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
		default:return NULL;
	}
}

bsv_modinfo_t* bsv_get_moduleinfo(void)
{
	return &modinfo;
}

#define S_START1		0
#define	S_START2		1
#define S_VER			2
#define S_NUM			3
#define S_DATA1			4
#define S_DATA2			5
#define S_DIGI			6

static char				frame_num;
static int				data_cnt;
static unsigned short	buf[BUF_LEN];
static char				goon;

bsv_data_t* bsv_get_data(void)
{
	int				ret, state;
	unsigned char	byte;

	goon	= 1;
	state	= S_START1;
	do {
		/* read one byte */
		ret	= read_one_byte(handle, &byte, TIMEOUT); 
		if( ret == -1 ) {
			errormsg	= "Error: read";
			return NULL;
		}
		if( ret == -2 ) {
			errormsg	= "Error: no data: timeout";
			return NULL;
		}
		if( ret == -3 ) {
			errormsg	= "Error: no data: blocking read";
			return NULL;
		}

		/* state */
		switch( state ) {
			case S_START1:
				if( byte == 0xa5 ) {
					/* Found first start byte */
					state		= S_START2;
				}
			break;

			case S_START2:
				if( byte == 0x5a ) {
					/* Found second start byte */
					state		= S_VER;
				} else {
					state		= S_START1;
				}
			break;

			case S_VER:
				if( byte == 0x02 ) {
					/* Found version */
					state		= S_NUM;
				} else {
					state		= S_START1;
				}
			break;

			case S_NUM:
				if( frame_num == 255 ) {
					frame_num	= 0;
				} else { 
					frame_num++;
				}
				if( frame_num != 13 && frame_num != byte ) {
					printf("wawawa %d %d\n", frame_num, byte);
					frame_num	= byte;
				}
				
				state			= S_DATA1;
				data_cnt		= 0;
			break;

			case S_DATA1:
				buf[data_cnt]	= (unsigned short)byte*0x100;
				state			= S_DATA2;
			break;
			
			case S_DATA2:
				buf[data_cnt]	+=(unsigned short)byte;
				
				data_cnt++;
				if( data_cnt >= BUF_LEN ) {
					state		= S_DIGI;
				} else {
					state		= S_DATA1;
				}
			break;

			case S_DIGI:
				/* Ignore byte and exit loop */
				goon			= 0;
			break;

			default:
				/* Exit loop */
				goon			= 0;
		}
	} while(goon);

	/* Exchange byte */
	buf[1]	= buf[3];

	data.samples	= (char*)buf;
	data.size		= 2;

	return &data;
}

char* bsv_get_errormsg(void)
{
	return errormsg;
}

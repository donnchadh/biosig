/* vim: set ts=4: */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include "pref.h"
#include "bsv_module.h"


/* Defines ********************************************************************/

#define LOGFILE_SIZE	100
#define PATID_SIZE		80	
#define RECID_SIZE		80


/* Typedef ********************************************************************/

typedef struct pref {
	char 		logfile[LOGFILE_SIZE+1];
	char 		patid[PATID_SIZE+1];
	char 		recid[RECID_SIZE+1];
	int 		hrange;
	int 		chanperscreen;
	bsv_modid_t	modid;	
} pref_t;


/* Global Variables ***********************************************************/

static pref_t		p_pref;


static void p_init_default(void)
{
	p_pref.logfile[0]	= 0;
	p_pref.patid[0]		= 0;
	p_pref.recid[0]		= 0;
	p_pref.hrange		= 1;
	p_pref.chanperscreen= BSV_GET_CHANNUM(bsv_get_moduleinfo());
	p_pref.modid		= BSV_GET_MODID(bsv_get_moduleinfo());
}

void p_init(const char* path)
{
	char*	str;
	int		fd;
	pref_t	last_pref;

	/* Load default configuration */
	p_init_default();

	/* Try to read last configuration */
	str	= g_strconcat(path, "/.bsview_pref", NULL);
	if( (fd = open(str, O_RDONLY)) != -1 ) {
		if( read(fd, &last_pref, sizeof(pref_t)) == sizeof(pref_t) ) {
			if( BSV_IS_MODID(last_pref.modid, p_pref.modid) ) {
				memcpy(&p_pref, &last_pref, sizeof(pref_t));
			}
		}

		close(fd);
	}

	g_free(str);
}

void p_destroy(const char* path)
{
	char*	str;
	int		fd;

	str	= g_strconcat(path, "/.bsview_pref", NULL);
	if( (fd = open(str, 
				   O_RDWR | O_TRUNC | O_CREAT,
				   S_IRUSR | S_IWUSR)) != -1 ) {
		write(fd, &p_pref, sizeof(pref_t));
		close(fd);
	}

	g_free(str);
}

void p_set_logfile(const char* logfile)
{
	g_strlcpy(p_pref.logfile, logfile, LOGFILE_SIZE);
}

char* p_get_logfile(void)
{
	return p_pref.logfile;
}

void p_set_patid(const char* patid)
{
	g_strlcpy(p_pref.patid, patid, PATID_SIZE);
}

char* p_get_patid(void)
{
	return p_pref.patid;
}

void p_set_recid(const char* recid)
{
	g_strlcpy(p_pref.recid, recid, RECID_SIZE);
}

char* p_get_recid(void)
{
	return p_pref.recid;
}

void p_set_hrange(int hrange)
{
	p_pref.hrange	= hrange;
}

int p_get_hrange(void)
{
	return p_pref.hrange;
}

void p_set_chanperscreen(int chanperscreen)
{
	p_pref.chanperscreen	= chanperscreen;
}

int p_get_chanperscreen(void)
{
	return p_pref.chanperscreen;
}

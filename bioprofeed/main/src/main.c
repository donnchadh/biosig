/* vim: set ts=4: */
#include <stdio.h>
#include <stdlib.h>
#ifndef EXIT_FAILURE
#	define EXIT_FAILURE
#	define EXIT_SUCCESS
#endif

#include <limits.h>
#ifndef PATH_MAX
#	define PATH_MAX
#endif

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gtk/gtk.h>
#include <string.h>
#include <ltdl.h>

#include "interface.h"
#include "support.h"
#include "bsv_module.h"
#include "pref.h"
#include "gdffile.h"
#include "draw.h"

#ifndef BSV_MODULE_PATH_ENV
#	define BSV_MODULE_PATH_ENV	"BSV_MODULE_PATH"
#endif

/* Exported functions in bsv_module.h *****************************************/

bsv_init_f				bsv_init			= NULL;
bsv_destroy_f			bsv_destroy			= NULL;	
bsv_start_f				bsv_start			= NULL;
bsv_stop_f				bsv_stop			= NULL;
bsv_get_config_win_f	bsv_get_config_win	= NULL;
bsv_get_channelinfo_f	bsv_get_channelinfo	= NULL;
bsv_get_moduleinfo_f	bsv_get_moduleinfo	= NULL;
bsv_get_data_f			bsv_get_data		= NULL;
bsv_get_errormsg_f		bsv_get_errormsg	= NULL;


/* Global Variables ***********************************************************/

GtkWidget*				bsv_main_win;
GtkWidget*				bsv_viewer_pref_dlg;
GtkWidget*				bsv_fs_dlg;
GtkWidget*				bsv_driver_info_dlg;


/* Helper functions ***********************************************************/

static char* dlerrordup(char* errormsg)
{
	char*	error	= (char*)lt_dlerror();
	if( error && !errormsg ) {
		errormsg	= strdup(error);
	}
	return errormsg;
}

static char* get_path(void)
{
	return getenv("HOME");
}

static void setup_pref_widgets(void)
{
	GtkEntry*		entry;
	GtkSpinButton*	spinbut;

	entry	= GTK_ENTRY(lookup_widget(bsv_viewer_pref_dlg, "logfile_entry")); 
	gtk_entry_set_text(entry, p_get_logfile());

	entry	= GTK_ENTRY(lookup_widget(bsv_viewer_pref_dlg, "patid_entry"));
	gtk_entry_set_text(entry, p_get_patid());

	entry	= GTK_ENTRY(lookup_widget(bsv_viewer_pref_dlg, "recid_entry"));
	gtk_entry_set_text(entry, p_get_recid());

	spinbut	= GTK_SPIN_BUTTON(lookup_widget(bsv_viewer_pref_dlg, 
											"chanperscreen_spinb"));
	gtk_spin_button_set_range(spinbut, 
							  1, BSV_GET_CHANNUM(bsv_get_moduleinfo()));
	gtk_spin_button_set_value(spinbut, p_get_chanperscreen());

	spinbut	= GTK_SPIN_BUTTON(lookup_widget(bsv_viewer_pref_dlg,
											"hrange_spinb"));
	gtk_spin_button_set_value(spinbut, p_get_hrange());
}

static void setup_info_widget(void)
{
	GtkLabel*			label_desc;
	GtkLabel*			label_val;
	bsv_modinfo_t*		modinfo_ptr;
	char				str[400];
	int					off;

	label_desc	= (GtkLabel*)lookup_widget(bsv_driver_info_dlg, "label_desc");

	off	= 0;
	off	+= snprintf(str+off, 400-off, "Module Name: \n");
	off += snprintf(str+off, 400-off, "Module ID: \n");
	off += snprintf(str+off, 400-off, "Module Licence: \n");
	off += snprintf(str+off, 400-off, "Module Author: \n");
	off += snprintf(str+off, 400-off, "Channel Number: \n");
	off += snprintf(str+off, 400-off, "Equipment Provider ID: \n");
	off += snprintf(str+off, 400-off, "Serial Number: \n");

	gtk_label_set_text(label_desc, str);

	label_val	= (GtkLabel*)lookup_widget(bsv_driver_info_dlg, "label_val");
	modinfo_ptr	= bsv_get_moduleinfo();

	off	= 0;
	off	+= snprintf(str+off, 400-off, "%s\n", BSV_GET_MODNAME(modinfo_ptr));
	off += snprintf(str+off, 400-off, "%ld\n", BSV_GET_MODID(modinfo_ptr));
	off += snprintf(str+off, 400-off, "%s\n", BSV_GET_MODLIC(modinfo_ptr));
	off += snprintf(str+off, 400-off, "%s\n", BSV_GET_MODAUT(modinfo_ptr));
	off += snprintf(str+off, 400-off, "%d\n", BSV_GET_CHANNUM(modinfo_ptr));
	off += snprintf(str+off, 400-off, "%lld\n", BSV_GET_EPID(modinfo_ptr));
	off += snprintf(str+off, 400-off, "%s\n", BSV_GET_SNUM(modinfo_ptr));
										 
	gtk_label_set_text(label_val, str);
}


/* Main function **************************************************************/

int main(int argc, char* argv[])
{
	char*		errormsg	= NULL;
	lt_dlhandle	module_ptr	= NULL;
	int			errors		= 0;


	if( argc < 2 ) {
		fprintf(stderr, "USAGE: bsview MODULE ARGUMENTS\n");
		exit(EXIT_FAILURE);
	}

	/* Init gtk stuff */
	g_thread_init(NULL);
	gdk_threads_init();
	
	gtk_set_locale();
	gtk_init(&argc, &argv);

	add_pixmap_directory(PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps");

	bsv_main_win	= create_bsv_main_win();
	gtk_widget_show(bsv_main_win);

	bsv_viewer_pref_dlg	= create_bsv_viewer_pref_dlg();
	/* dont show */

	bsv_fs_dlg			= create_bsv_fs_dlg();
	/* dont show */

	bsv_driver_info_dlg	= create_bsv_driver_info_dlg();
	/* dont show */

	/* Initialize libltdl */
	errors	= lt_dlinit();

	/* Set the module search path */
	if( !errors ) {
		const char*	path	= getenv(BSV_MODULE_PATH_ENV);

		if( path != NULL ) {
			errors	= lt_dlsetsearchpath(path);
		}
	}

	/* Load the module */
	if( !errors ) {
		module_ptr	= lt_dlopenext(argv[1]);
	}

	/* Find the entry point */
	if( module_ptr ) {
		bsv_init			= (bsv_init_f) lt_dlsym(module_ptr, 
														"bsv_init");
		bsv_destroy			= (bsv_destroy_f) lt_dlsym(module_ptr, 
														"bsv_destroy");
		bsv_start			= (bsv_start_f) lt_dlsym(module_ptr,
														"bsv_start");
		bsv_stop			= (bsv_stop_f) lt_dlsym(module_ptr,
														"bsv_stop");
		bsv_get_config_win	= (bsv_get_config_win_f) lt_dlsym(module_ptr, 
														"bsv_get_config_win");
		bsv_get_channelinfo	= (bsv_get_channelinfo_f) lt_dlsym(module_ptr,
														"bsv_get_channelinfo");
		bsv_get_moduleinfo	= (bsv_get_moduleinfo_f) lt_dlsym(module_ptr,
														"bsv_get_moduleinfo");
		bsv_get_data		= (bsv_get_data_f) lt_dlsym(module_ptr,
														"bsv_get_data");
		bsv_get_errormsg	= (bsv_get_errormsg_f) lt_dlsym(module_ptr,
														"bsv_get_errormsg");
		if( !bsv_init || 
			!bsv_destroy ||
			!bsv_start ||
			!bsv_stop ||
			!bsv_get_config_win ||
			!bsv_get_channelinfo ||
			!bsv_get_moduleinfo ||
			!bsv_get_data ||
			!bsv_get_errormsg) {
			errors	= 1;
		}
	} else {
		errors	= 1;
	}

	if( !errors ) {
		/* Module constructor */
		errors	= bsv_init(get_path());
	}

	if( !errors ) {
		p_init(get_path());
		setup_pref_widgets();
		setup_info_widget();
		da_init();
		d_init();
		gf_init();
	
		/* Main loop */
		gdk_threads_enter();
		gtk_main();
		gdk_threads_leave();

		gf_destroy();
		d_destroy();
		da_destroy();
		p_destroy(get_path());

		/* Module destructor */
		errors	= bsv_destroy(get_path());
	}

	if( !errors ) {
		errors	= lt_dlclose(module_ptr);
	}

	if( errors ) {
		/* Diagnose the encountered error */
		errormsg	= dlerrordup(errormsg);

		if( !errormsg ) {
			fprintf(stderr, "%s: dlerror() failed.\n", argv[0]);
			return EXIT_FAILURE;
		}
	}

	/* Finished with ltdl now */
	if( !errors ) {
		if( lt_dlexit() != 0 ) {
			errormsg	= dlerrordup(errormsg);
		}
	}

	if( errormsg ) {
		fprintf(stderr, "%s: %s.\n", argv[0], errormsg);
		free(errormsg);
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

/* vim: set ts=4: */
#include <gtk/gtk.h>
#include <unistd.h>
#include <sys/types.h>
#include <gdk/gdkx.h>
#include <glib.h>

#include "dataaq.h"
#include "bsv_module.h"
#include "draw.h"
#include "gdffile.h"
#include "support.h"


/* Defines ********************************************************************/

#define DEAD		0
#define ALIVE		1
#define RUNNING		2
#define DYING		3

/* Exported Variables *********************************************************/

extern GtkWidget*				bsv_main_win;


/* Global Variables ***********************************************************/

static GThread*					reader_ptr;
static GMutex*					state_mtx_ptr;
static volatile unsigned char	state		= DEAD;


/* Private Functions **********************************************************/

static void display_errormsg(char* msg)
{
	guint			ctxid;
	GtkStatusbar*	sbar;

	sbar	= (GtkStatusbar*)lookup_widget(bsv_main_win, "statusbar");

	gdk_threads_enter();
	ctxid	= gtk_statusbar_get_context_id(sbar, "error");

	/* Pop previous errormsg */
	gtk_statusbar_pop(sbar, ctxid);

	/* Push new errormsg */
	gtk_statusbar_push(sbar, ctxid, msg);
	gdk_threads_leave();
}

static void __change_but_state(void)
{
	GtkWidget*	start_widget;
	GtkWidget*	stop_widget;
	GtkWidget*	pref_driver;
	GtkWidget*	pref_main;

	start_widget= (GtkWidget*)lookup_widget(bsv_main_win, "start_but");
	stop_widget	= (GtkWidget*)lookup_widget(bsv_main_win, "stop_but");
	pref_driver	= (GtkWidget*)lookup_widget(bsv_main_win, "preferences_driver");
	pref_main	= (GtkWidget*)lookup_widget(bsv_main_win, "preferences_main");

	/* Set new button state */
	switch(state) {
		case RUNNING:
			gtk_widget_set_sensitive(start_widget, 0);
			gtk_widget_set_sensitive(stop_widget, 1);
			gtk_widget_set_sensitive(pref_driver, 0);
			gtk_widget_set_sensitive(pref_main, 0);
			break;
		case DEAD:
			gtk_widget_set_sensitive(start_widget, 1);
			gtk_widget_set_sensitive(stop_widget, 0);
			gtk_widget_set_sensitive(pref_driver, 1);
			gtk_widget_set_sensitive(pref_main, 1);
			break;
		default:
			gtk_widget_set_sensitive(start_widget, 0);
			gtk_widget_set_sensitive(stop_widget, 0);
			gtk_widget_set_sensitive(pref_driver, 0);
			gtk_widget_set_sensitive(pref_main, 0);
	}
}

static void* read_data(void* param)
{
	bsv_data_t*		s_ptr;

	/* Clear previous errormsg */
	display_errormsg("");	

	/* Setup */
	if( d_setup() ) {
		display_errormsg( d_get_errormsg() );
	
		goto exit;
	}
	if( gf_setup() ) {
		display_errormsg( gf_get_errormsg() );
	
		goto d_exit;
	}

	/* Start aqistion */
	if( bsv_start() ) {
		display_errormsg( bsv_get_errormsg() );
	
		goto gf_exit;
	}

	/* Start main loop */
	g_mutex_lock(state_mtx_ptr);
	if( state == ALIVE ) {
		state	= RUNNING;
	} else {
		g_mutex_unlock(state_mtx_ptr);
		goto bsv_exit;
	}
	g_mutex_unlock(state_mtx_ptr);

	gdk_threads_enter();
	__change_but_state();
	gdk_threads_leave();

	g_mutex_lock(state_mtx_ptr);
	while( state != DYING ) {
		g_mutex_unlock(state_mtx_ptr);

		/* Get samples */
		if( !(s_ptr = bsv_get_data()) ) {
			/* Timeout or other error */
			display_errormsg( bsv_get_errormsg() );

			g_mutex_lock(state_mtx_ptr);
			state	= DYING;
			continue;
		}

		/* Dispatch samples */
		if( d_set_samples(s_ptr) ) {
			display_errormsg( d_get_errormsg() );

			g_mutex_lock(state_mtx_ptr);
			state	= DYING;
			continue;
		}
		if( gf_set_samples(s_ptr) ) {
			display_errormsg( gf_get_errormsg() );

			g_mutex_lock(state_mtx_ptr);
			state	= DYING;
			continue;
		}

		g_mutex_lock(state_mtx_ptr);
	}
	g_mutex_unlock(state_mtx_ptr);

	gdk_threads_enter();
	__change_but_state();
	gdk_threads_leave();

bsv_exit:
	/* Stop aquistion */
	bsv_stop();
	
	/* Cleanup */
gf_exit:
	gf_cleanup();

d_exit:
	d_cleanup();

exit:
	g_mutex_lock(state_mtx_ptr);
	state	= DEAD;
	g_mutex_unlock(state_mtx_ptr);

	gdk_threads_enter();
	__change_but_state();
	gdk_threads_leave();

	g_thread_exit(NULL);
}


/* Public Functions ***********************************************************/

void da_init(void)
{
	/* init mutex */
	state_mtx_ptr = g_mutex_new();
}

void da_destroy(void)
{
	da_stop();

	/* destroy mutex */
	g_mutex_free(state_mtx_ptr);
}

void da_start(void)
{
	g_mutex_lock(state_mtx_ptr);
	if( state != DEAD ) {
		g_mutex_unlock(state_mtx_ptr);

		/* already started */
		return;
	}

	/* set new state */
	state	= ALIVE;
	g_mutex_unlock(state_mtx_ptr);

	__change_but_state();

	/* start thread */
	reader_ptr = g_thread_create(read_data, NULL, TRUE, NULL);
}

void da_stop(void)
{
	g_mutex_lock(state_mtx_ptr);
	if( state != RUNNING && state != ALIVE ) {
		g_mutex_unlock(state_mtx_ptr);

		/* not running */
		return;
	}

	/* set new state */
	state	= DYING;
	g_mutex_unlock(state_mtx_ptr);

	/* Change but state when thread exited the main loop */
}

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "bsv_module.h"
#include "main.h"

extern GtkWidget*	config_win_ptr;
extern GtkWidget*	file_select_ptr;

gboolean
update_config                          (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	c_update();
	gtk_widget_hide(config_win_ptr);
	return TRUE;
}


void
run_fileselect                         (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_show(file_select_ptr);
}


void
close_file_select                      (GtkDialog       *dialog,
                                        gpointer         user_data)
{
	gtk_widget_hide(file_select_ptr);
}


void
update_file_select                     (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget*	entry;
	gchar*		str_ptr;

	gtk_widget_hide(file_select_ptr);

	entry	= lookup_widget(config_win_ptr, "devnode_entry");
	str_ptr	= (gchar*)gtk_file_selection_get_filename(
										GTK_FILE_SELECTION(file_select_ptr));
	gtk_entry_set_text(GTK_ENTRY(entry), str_ptr);
}


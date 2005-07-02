#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdio.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "bsv_module.h"
#include "pref.h"
#include "dataaq.h"
#include "draw.h"

/* Widgets created in main.c **************************************************/

extern GtkWidget*		bsv_viewer_pref_dlg;
extern GtkWidget*		bsv_fs_dlg;
extern GtkWidget*		bsv_driver_info_dlg;
extern GtkWidget*		bsv_main_win;


/* Callback functions *********************************************************/

void
start_acquistion                       (GtkButton       *button,
                                        gpointer         user_data)
{
	da_start();
 }


void
stop_acquistion                        (GtkButton       *button,
                                        gpointer         user_data)
{
	da_stop();
}


void
vsbar_value_changed                    (GtkRange        *range,
                                        gpointer         user_data)
{
	if( !d_set_offset((int)gtk_range_get_value(range)) ) { 
		d_redraw();
	}
}


void
run_viewer_preference_dlg              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gtk_widget_show(bsv_viewer_pref_dlg);
}


void
run_driver_perferences_dlg             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gtk_widget_show((GtkWidget*)bsv_get_config_win());
}


void
run_driver_info_dlg                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gtk_widget_show( bsv_driver_info_dlg );
}


gboolean
update_viewer_settings                 (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	/* call the real update function */
	update_viewer_settings_but(NULL, NULL);

	return TRUE;
}


gboolean
main_quit                              (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gtk_main_quit();
	return TRUE;
}


void
update_viewer_settings_but             (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkEntry*		entry;
	GtkSpinButton*	spinbut;

	entry	= GTK_ENTRY(lookup_widget(bsv_viewer_pref_dlg, "logfile_entry"));
	p_set_logfile((char*)gtk_entry_get_text(entry));

	entry	= GTK_ENTRY(lookup_widget(bsv_viewer_pref_dlg, "patid_entry"));
	p_set_patid((char*)gtk_entry_get_text(entry));

	entry	= GTK_ENTRY(lookup_widget(bsv_viewer_pref_dlg, "recid_entry"));
	p_set_recid((char*)gtk_entry_get_text(entry));

	spinbut	= GTK_SPIN_BUTTON(lookup_widget(bsv_viewer_pref_dlg, 
											"chanperscreen_spinb"));
	p_set_chanperscreen(gtk_spin_button_get_value_as_int(spinbut));

	spinbut	= GTK_SPIN_BUTTON(lookup_widget(bsv_viewer_pref_dlg,
											"hrange_spinb"));
	p_set_hrange(gtk_spin_button_get_value_as_int(spinbut));

	gtk_widget_hide( bsv_viewer_pref_dlg );
}


void
chansel_combo_changed                  (GtkComboBox     *menuitem,
                                        gpointer         user_data)
{
	d_control_load( gtk_combo_box_get_active(menuitem) );
}


void
apply_but_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkComboBox*	combo1;
	GtkComboBox*	combo2;
	GtkSpinButton*	spin;

	combo1	= (GtkComboBox*)lookup_widget(bsv_main_win, "chansel_combo");
	combo2	= (GtkComboBox*)lookup_widget(bsv_main_win, "combo1");
	spin	= (GtkSpinButton*)lookup_widget(bsv_main_win, "mag_spinbut");

	d_control_apply( gtk_combo_box_get_active(combo1),
					 gtk_spin_button_get_value_as_int(spin),
					 gtk_combo_box_get_active(combo2));
	d_redraw();
}


void
show_fs_dlg                            (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_show(bsv_fs_dlg);
}


void
update_logfile                         (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkEntry*	entry;
	gchar*		str;

	gtk_widget_hide(bsv_fs_dlg);

	entry	= GTK_ENTRY(lookup_widget(bsv_viewer_pref_dlg, "logfile_entry"));
	str		= (gchar*)gtk_file_selection_get_filename(
									GTK_FILE_SELECTION(bsv_fs_dlg));
	gtk_entry_set_text(entry, str);
}


gboolean
resize_area                            (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data)
{
	d_redraw();
	return TRUE;
}


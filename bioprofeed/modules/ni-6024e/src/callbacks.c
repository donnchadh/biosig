#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "bsv_module.h"
#include "main.h"

/* External Variables ***********************************************************/

extern GtkWidget*	config_win_ptr;


/* Private Functions ************************************************************/

static void update_widgets(int channel)
{
	GtkToggleButton*	toggle;
	GtkWidget*			widget;
	char				str[50];
	gboolean			state;

	snprintf(str, 50, "ch%d_chk", channel);
	toggle	= (GtkToggleButton*)lookup_widget(config_win_ptr, str);
	state	= gtk_toggle_button_get_active(toggle);

	snprintf(str, 50, "ch%d_hp_entry", channel);
	widget	= (GtkWidget*)lookup_widget(config_win_ptr, str);
	gtk_widget_set_sensitive(widget, state);

	snprintf(str, 50, "ch%d_tp_entry", channel);
	widget	= (GtkWidget*)lookup_widget(config_win_ptr, str);
	gtk_widget_set_sensitive(widget, state);

	snprintf(str, 50, "ch%d_rng_combo", channel);
	widget	= (GtkWidget*)lookup_widget(config_win_ptr, str);
	gtk_widget_set_sensitive(widget, state);

	snprintf(str, 50, "ch%d_ref_combo", channel);
	widget	= (GtkWidget*)lookup_widget(config_win_ptr, str);
	gtk_widget_set_sensitive(widget, state);
}


/* Public Functions *************************************************************/

gboolean
update_config                          (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gui_update();
	gtk_widget_hide(config_win_ptr);
	return TRUE;
}


void
on_ch1_chk_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	update_widgets(1);
}


void
on_ch2_chk_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	update_widgets(2);
}


void
on_ch3_chk_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	update_widgets(3);
}


void
on_ch4_chk_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	update_widgets(4);
}


void
on_ch5_chk_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	update_widgets(5);
}


void
on_ch6_chk_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	update_widgets(6);
}


void
on_ch7_chk_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	update_widgets(7);
}


void
on_ch8_chk_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	update_widgets(8);
}


void
on_ch9_chk_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	update_widgets(9);
}


void
on_ch10_chk_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	update_widgets(10);
}


void
on_ch11_chk_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	update_widgets(11);
}


void
on_ch12_chk_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	update_widgets(12);
}


void
on_ch13_chk_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	update_widgets(13);
}


void
on_ch14_chk_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	update_widgets(14);
}


void
on_ch15_chk_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	update_widgets(15);
}


void
on_ch16_chk_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	update_widgets(16);
}


#include <gtk/gtk.h>


void
start_acquistion                       (GtkButton       *button,
                                        gpointer         user_data);

void
stop_acquistion                        (GtkButton       *button,
                                        gpointer         user_data);
void
apply_but_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
vsbar_value_changed                    (GtkRange        *range,
                                        gpointer         user_data);

void
run_viewer_preference_dlg              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
run_driver_perferences_dlg             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
run_driver_info_dlg                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
update_viewer_settings                 (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
main_quit                              (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
update_viewer_settings_but             (GtkButton       *button,
                                        gpointer         user_data);

void
chansel_combo_changed                  (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
show_fs_dlg                            (GtkButton       *button,
                                        gpointer         user_data);

void
update_logfile                         (GtkButton       *button,
                                        gpointer         user_data);

gboolean
resize_area                            (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

#include <gtk/gtk.h>


gboolean
update_config                          (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
run_fileselect                         (GtkButton       *button,
                                        gpointer         user_data);

void
close_file_select                      (GtkDialog       *dialog,
                                        gpointer         user_data);

void
update_file_select                     (GtkButton       *button,
                                        gpointer         user_data);

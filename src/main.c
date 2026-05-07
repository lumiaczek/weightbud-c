#include "gui.h"
#include "data_managment.h"
#include <gtk/gtk.h>
#include "form.h"
int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.weightbud", G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);
    g_signal_connect(app,"activate",G_CALLBACK(initialize_data),NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
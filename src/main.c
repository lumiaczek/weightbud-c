#include "data_managment.h"
#include "ui/form/form.h"
#include <gtk/gtk.h>
#include "db.h"

int main(int argc, char **argv) {
    GtkApplication *app;
    AppState *state = g_new0(AppState,1);
    init_memory(state);

    int status;

    app = gtk_application_new("org.gtk.weightbud", G_APPLICATION_DEFAULT_FLAGS);
    
    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), state);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    
    
    cleanup_memory(state);
    g_free(state);
    g_object_unref(app);

    return status;
}
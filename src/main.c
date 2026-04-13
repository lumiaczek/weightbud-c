#include <gtk/gtk.h>

static void print_hello(GtkWidget *widget, gpointer data) {
    g_print("Hello World\n");
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *button;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "WeightBud");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 800);

    button = gtk_button_new_with_label("Przycisk");
    gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(button, GTK_ALIGN_CENTER);

    g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);

    gtk_container_add(GTK_CONTAINER(window), button);

    gtk_widget_show_all(window);
}

int main(int argc,
         char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.weightbud", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
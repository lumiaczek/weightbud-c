#include "gui.h"

static void on_button_clicked(GtkWidget *widget, gpointer data) {
    g_print("Trening zapisany!\n");
}

void on_app_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *button;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Weightbud");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);

    button = gtk_button_new_with_label("Zapisz trening");
    gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(button, GTK_ALIGN_CENTER);

    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), NULL);
    gtk_container_add(GTK_CONTAINER(window), button);

    gtk_widget_show_all(window);
}
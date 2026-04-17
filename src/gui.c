#include "gui.h"
#include <stdbool.h>

static void load_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    gtk_css_provider_load_from_path(provider, "assets/style.css", NULL);

    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(provider);
}

static GtkWidget *create_card(const char *label_text) {
    GtkWidget *card_frame = gtk_frame_new(NULL);

    gtk_widget_set_size_request(card_frame, 160, 180);

    GtkWidget *label = gtk_label_new(label_text);
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(label, GTK_ALIGN_CENTER);

    gtk_container_add(GTK_CONTAINER(card_frame), label);

    gtk_style_context_add_class(gtk_widget_get_style_context(card_frame), "module-card");

    return card_frame;
}

void on_app_activate(GtkApplication *app, gpointer user_data) {
    load_css();

    GtkWidget *window = gtk_application_window_new(app);

    gtk_window_set_title(GTK_WINDOW(window), "WeightBud");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);

    GtkWidget *main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 50);
    gtk_widget_set_halign(main_hbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(main_hbox, GTK_ALIGN_CENTER);

    GtkWidget *left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale("assets/images/hero.png", 900, -1, TRUE, &error);

    GtkWidget *image;

    if (pixbuf != NULL) {
        image = gtk_image_new_from_pixbuf(pixbuf);

        g_object_unref(pixbuf);
    } else {
        g_printerr("Błąd ładowania hero.png: %s\n", error->message);
        g_error_free(error);

        image = gtk_image_new();
    }

    gtk_container_add(GTK_CONTAINER(left_vbox), image);

    GtkWidget *right_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_valign(right_vbox, GTK_ALIGN_CENTER);

    GtkWidget *title_label = gtk_label_new("WEIGHTBUD");
    gtk_style_context_add_class(gtk_widget_get_style_context(title_label), "title-label");

    GtkWidget *subtitle_label = gtk_label_new("Osiągaj swoje cele sportowe\nszybciej i łatwiej z WeightBud.");
    gtk_style_context_add_class(gtk_widget_get_style_context(subtitle_label), "subtitle-label");
    gtk_label_set_justify(GTK_LABEL(subtitle_label), GTK_JUSTIFY_CENTER);

    GtkWidget *cards_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_set_halign(cards_hbox, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(cards_hbox), create_card("Treningi"));
    gtk_container_add(GTK_CONTAINER(cards_hbox), create_card("Waga"));
    gtk_container_add(GTK_CONTAINER(cards_hbox), create_card("Suplementy"));

    GtkWidget *btn_start = gtk_button_new_with_label("Rozpocznij");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_start), "start-button");
    gtk_widget_set_halign(btn_start, GTK_ALIGN_CENTER);

    gtk_container_add(GTK_CONTAINER(right_vbox), title_label);
    gtk_container_add(GTK_CONTAINER(right_vbox), subtitle_label);
    gtk_container_add(GTK_CONTAINER(right_vbox), cards_hbox);

    gtk_box_set_spacing(GTK_BOX(right_vbox), 30);
    gtk_container_add(GTK_CONTAINER(right_vbox), btn_start);

    gtk_container_add(GTK_CONTAINER(main_hbox), left_vbox);
    gtk_container_add(GTK_CONTAINER(main_hbox), right_vbox);
    gtk_container_add(GTK_CONTAINER(window), main_hbox);

    gtk_style_context_add_class(gtk_widget_get_style_context(window), "main-window");

    gtk_widget_show_all(window);
}
#include "gui.h"
#include <fontconfig/fontconfig.h>
#include <gtk/gtk.h>
#include "events.h"



static void load_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    gtk_css_provider_load_from_path(provider, "assets/style.css", NULL);

    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(provider);
}

static void load_custom_fonts(void) {
    const FcChar8 *font_path1 = (const FcChar8 *)"assets/fonts/Montserrat-VariableFont_wght.ttf";

    const FcChar8 *font_path2 = (const FcChar8 *)"assets/fonts/Bungee-Regular.ttf";

    if (!FcConfigAppFontAddFile(FcConfigGetCurrent(), font_path1)) {
        g_printerr("Nie udało się załadować czcionki z: %s\n", font_path1);
    }

    if (!FcConfigAppFontAddFile(FcConfigGetCurrent(), font_path2)) {
        g_printerr("Nie udało się załadować czcionki z: %s\n", font_path2);
    }
}

static void load_svg_icon(GtkBuilder *builder, const char *image_id, const char *filepath, int size) {
    GtkWidget *image_widget = GTK_WIDGET(gtk_builder_get_object(builder, image_id));
    if (!image_widget)
        return;

    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(filepath, size, size, TRUE, &error);

    if (pixbuf != NULL) {
        gtk_image_set_from_pixbuf(GTK_IMAGE(image_widget), pixbuf);
        g_object_unref(pixbuf);
    } else {
        g_printerr("Nie udało się załadować ikony %s: %s\n", filepath, error->message);
        g_error_free(error);
    }
}

void on_app_activate(GtkApplication *app, gpointer user_data) {
    load_custom_fonts();
    load_css();

    GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
    gtk_icon_theme_append_search_path(icon_theme, "assets/icons");

    GtkBuilder *builder = gtk_builder_new_from_file("gui.ui");
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    
    gtk_application_add_window(app, GTK_WINDOW(window));

    GtkWidget *hero_image = GTK_WIDGET(gtk_builder_get_object(builder, "hero_image"));
    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale("assets/images/hero.png", 950, -1, TRUE, &error);

    if (pixbuf != NULL) {
        gtk_image_set_from_pixbuf(GTK_IMAGE(hero_image), pixbuf);
        g_object_unref(pixbuf);
    } else {
        g_printerr("Błąd ładowania hero.png: %s\n", error->message);
        g_error_free(error);
    }

    GtkWidget *stack = GTK_WIDGET(gtk_builder_get_object(builder,"main_stack"));
    GtkWidget *btn_start = GTK_WIDGET(gtk_builder_get_object(builder,"btn_start"));
    
    g_signal_connect(btn_start,"clicked",G_CALLBACK(change_view_on_click),stack);



    g_object_unref(builder);
    gtk_widget_show_all(window);
}
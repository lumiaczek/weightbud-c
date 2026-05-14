#include "ui/form/form.h"
#include "events.h"
#include "ui/dashboard/dashboard.h"
#include <fontconfig/fontconfig.h>
#include <gtk/gtk.h>

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

void load_svg_icon(GtkBuilder *builder, const char *image_id, const char *filepath, int size) {
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

void enforce_numeric_input(GtkEditable *editable, const gchar *new_text, gint new_text_length, gint *position, gpointer user_data) {
    for (int i = 0; i < new_text_length; i++) {
        if (!g_ascii_isdigit(new_text[i]) && new_text[i] != '.' && new_text[i] != ',') {
            g_signal_stop_emission_by_name(G_OBJECT(editable), "insert-text");
            return;
        }
    }
}

static void close_form_and_open_dashboard(GtkButton *btn, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    GtkApplication *app = gtk_window_get_application(GTK_WINDOW(window));
    show_dashboard_window(app);
    gtk_widget_destroy(window);
}

void on_app_activate(GtkApplication *app, gpointer user_data) {
    load_custom_fonts();
    load_css();
    init_memory();

    GtkBuilder *builder = gtk_builder_new_from_file("src/ui/form/form.ui");
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

    GtkWidget *stack = GTK_WIDGET(gtk_builder_get_object(builder, "main_stack"));

    OnboardingWidgets *ob_widgets = g_new(OnboardingWidgets, 1);

    ob_widgets->stack = GTK_STACK(stack);
    ob_widgets->entry_name = GTK_WIDGET(gtk_builder_get_object(builder, "entry_name"));
    ob_widgets->entry_weight = GTK_WIDGET(gtk_builder_get_object(builder, "entry_weight"));
    ob_widgets->entry_height = GTK_WIDGET(gtk_builder_get_object(builder, "entry_height"));
    ob_widgets->entry_age = GTK_WIDGET(gtk_builder_get_object(builder, "entry_age"));

    ob_widgets->entry_target_weight = GTK_WIDGET(gtk_builder_get_object(builder, "entry_target_weight"));
    ob_widgets->entry_target_muscle = GTK_WIDGET(gtk_builder_get_object(builder, "entry_target_muscle"));
    ob_widgets->entry_target_fat = GTK_WIDGET(gtk_builder_get_object(builder, "entry_target_fat"));

    ob_widgets->entry_kcal = GTK_WIDGET(gtk_builder_get_object(builder, "entry_kcal"));
    ob_widgets->entry_protein = GTK_WIDGET(gtk_builder_get_object(builder, "entry_protein"));
    ob_widgets->entry_fat = GTK_WIDGET(gtk_builder_get_object(builder, "entry_fat"));
    ob_widgets->entry_carbs = GTK_WIDGET(gtk_builder_get_object(builder, "entry_carbs"));

    GtkWidget *btn_start = GTK_WIDGET(gtk_builder_get_object(builder, "btn_start"));
    g_signal_connect(btn_start, "clicked", G_CALLBACK(change_view_on_click), stack);

    GtkWidget *btn_form_next = GTK_WIDGET(gtk_builder_get_object(builder, "btn_form_next"));
    g_signal_connect(btn_form_next, "clicked", G_CALLBACK(change_to_form_page_2), stack);

    GtkWidget *btn_finish = GTK_WIDGET(gtk_builder_get_object(builder, "btn_finish"));
    g_signal_connect(btn_finish, "clicked", G_CALLBACK(on_finish_button_clicked), ob_widgets);
    g_signal_connect(btn_finish, "clicked", G_CALLBACK(close_form_and_open_dashboard), window);

    const char *numeric_entries[] = {"entry_weight", "entry_height", "entry_target_weight", "entry_target_muscle", "entry_target_fat"};
    for (int i = 0; i < 5; i++) {
        GtkWidget *entry = GTK_WIDGET(gtk_builder_get_object(builder, numeric_entries[i]));
        if (entry) {
            g_signal_connect(entry, "insert-text", G_CALLBACK(enforce_numeric_input), NULL);
        }
    }

    load_svg_icon(builder, "icon_treningi", "assets/icons/dumbbell.svg", 64);
    load_svg_icon(builder, "icon_waga", "assets/icons/biceps.svg", 64);
    load_svg_icon(builder, "icon_suple", "assets/icons/pill.svg", 64);
    load_svg_icon(builder, "icon_user", "assets/icons/user.svg", 34);
    load_svg_icon(builder, "icon_target", "assets/icons/target.svg", 34);

    load_svg_icon(builder, "icon_male", "assets/icons/mars.svg", 28);
    load_svg_icon(builder, "icon_female", "assets/icons/venus.svg", 28);
    load_svg_icon(builder, "icon_burn", "assets/icons/flame.svg", 28);
    load_svg_icon(builder, "icon_scale", "assets/icons/scale.svg", 28);
    load_svg_icon(builder, "icon_muscle_radio", "assets/icons/biceps.svg", 28);

    load_svg_icon(builder, "icon_next_arrow", "assets/icons/move-right.svg", 28);

    load_svg_icon(builder, "icon_pulse", "assets/icons/activity.svg", 34);
    load_svg_icon(builder, "icon_pill_card", "assets/icons/pill2.svg", 34);
    load_svg_icon(builder, "icon_apple", "assets/icons/apple.svg", 34);

    g_object_unref(builder);
    gtk_widget_show_all(window);
}
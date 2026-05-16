#include "ui/dashboard/dashboard.h"
#include "ui/form/form.h"
#include <gtk/gtk.h>

static void on_nav_button_clicked(GtkButton *btn, gpointer user_data) {
    GtkStack *stack = GTK_STACK(user_data);

    GtkWidget *nav_menu = gtk_widget_get_parent(GTK_WIDGET(btn));

    GList *children = gtk_container_get_children(GTK_CONTAINER(nav_menu));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        GtkWidget *child = GTK_WIDGET(iter->data);
        gtk_style_context_remove_class(gtk_widget_get_style_context(child), "active");
    }
    g_list_free(children);

    gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(btn)), "active");

    const char *page_name = g_object_get_data(G_OBJECT(btn), "target_page");
    if (page_name) {
        gtk_stack_set_visible_child_name(stack, page_name);
    }
}

static void setup_nav_button(GtkBuilder *builder, GtkStack *stack, const char *btn_id, const char *page_name, const char *icon_id, const char *icon_path) {
    GtkWidget *btn = GTK_WIDGET(gtk_builder_get_object(builder, btn_id));
    if (btn) {
        g_object_set_data(G_OBJECT(btn), "target_page", (gpointer)page_name);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_nav_button_clicked), stack);
    }

    if (icon_id && icon_path) {
        load_svg_icon(builder, icon_id, icon_path, 20);
    }
}

void show_dashboard_window(GtkApplication *app) {
    GtkBuilder *builder = gtk_builder_new_from_file("dashboard.ui");
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "dashboard_window"));

    gtk_application_add_window(app, GTK_WINDOW(window));

    GtkStack *stack = GTK_STACK(gtk_builder_get_object(builder, "dashboard_stack"));

    setup_nav_button(builder, stack, "btn_nav_dashboard", "page_dashboard", "icon_nav_dashboard", "assets/icons/home.svg");
    setup_nav_button(builder, stack, "btn_nav_nawyki", "page_nawyki", "icon_nav_nawyki", "assets/icons/activity.svg");
    setup_nav_button(builder, stack, "btn_nav_trening", "page_trening", "icon_nav_trening", "assets/icons/dumbbell.svg");
    setup_nav_button(builder, stack, "btn_nav_waga", "page_waga", "icon_nav_waga", "assets/icons/scale.svg");
    setup_nav_button(builder, stack, "btn_nav_dieta", "page_dieta", "icon_nav_dieta", "assets/icons/apple.svg");
    setup_nav_button(builder, stack, "btn_nav_suple", "page_suple", "icon_nav_suple", "assets/icons/pill.svg");
    setup_nav_button(builder, stack, "btn_nav_narzedzia", "page_narzedzia", "icon_nav_narzedzia", "assets/icons/settings.svg");

    load_svg_icon(builder, "icon_toggle", "assets/icons/sidebar.svg", 24);
    load_svg_icon(builder, "icon_settings", "assets/icons/settings.svg", 24);
    load_svg_icon(builder, "icon_lock", "assets/icons/lock.svg", 24);

    g_object_unref(builder);
    gtk_widget_show_all(window);
}
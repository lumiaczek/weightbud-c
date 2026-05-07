#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>

static void load_css(void);
static void load_custom_fonts(void);
static void load_svg_icon(GtkBuilder *builder, const char *image_id, const char *filepath, int size);
void on_app_activate(GtkApplication *app, gpointer user_data);

#endif
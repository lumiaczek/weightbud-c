#ifndef FORM_H
#define FORM_H

#include <gtk/gtk.h>

void load_svg_icon(GtkBuilder *builder, const char *image_id, const char *filepath, int size);
void on_app_activate(GtkApplication *app, gpointer user_data);

#endif
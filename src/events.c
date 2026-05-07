#include "events.h"
#include <gtk/gtk.h>

void change_view_on_click(GtkButton *button,gpointer user_data, const char * view_name)
{
    GtkStack *stack = GTK_STACK(user_data);

    gtk_stack_set_visible_child_name(stack,"form_view");
}
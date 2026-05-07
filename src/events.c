#include "events.h"
#include <gtk/gtk.h>

void change_view_on_click(GtkButton *button, gpointer user_data) {
    GtkStack *stack = GTK_STACK(user_data);
    gtk_stack_set_visible_child_name(stack, "form_view");
}

void change_to_form_page_2(GtkButton *button, gpointer user_data) {
    GtkStack *stack = GTK_STACK(user_data);
    gtk_stack_set_visible_child_name(stack, "form_view_2");
}
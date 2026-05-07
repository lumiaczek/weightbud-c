#include <gtk/gtk.h>
#include "form.h"
static void print_hello(GtkWidget *widget)
{g_print("Hello World \n");}

static void load_css(void){
    GError *error = NULL;
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    gtk_css_provider_load_from_path(provider,"assets/builder.css",&error);
    if (error) {
        g_printerr("CSS parsing error: %s\n", error->message);
        g_clear_error(&error);
    }
    
    gtk_style_context_add_provider_for_screen(screen,GTK_STYLE_PROVIDER(provider),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(provider);
}


 void build_ui(GtkWidget *window,GtkWidget *stack){
    load_css();
    gtk_window_set_title(GTK_WINDOW(window), "Form");
    GtkWidget *form_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,50);
    
    GtkWidget *test_label = gtk_label_new("Witaj w formularzu!");
    gtk_box_pack_start(GTK_BOX(form_box),test_label,TRUE,TRUE,0);

    
   
   
    gtk_stack_add_named(GTK_STACK(stack), form_box,"form_view");

}
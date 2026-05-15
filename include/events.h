#ifndef EVENTS_H
#define EVENTS_H

#include "data_managment.h"
#include "storage.h"
#include <gtk/gtk.h>

typedef struct {
    GtkWidget *entry_name;
    GtkWidget *entry_weight;
    GtkWidget *entry_height;
    GtkWidget *entry_age;

    GtkWidget *entry_target_weight;
    GtkWidget *entry_target_muscle;
    GtkWidget *entry_target_fat;

    GtkWidget *entry_kcal;
    GtkWidget *entry_protein;
    GtkWidget *entry_fat;
    GtkWidget *entry_carbs;

    GtkStack *stack;
    AppState *state;
} OnboardingWidgets;

void change_view_on_click(GtkButton *button, gpointer user_data);
void change_to_form_page_2(GtkButton *button, gpointer user_data);
void on_finish_button_clicked(GtkButton *button, gpointer user_data);

#endif
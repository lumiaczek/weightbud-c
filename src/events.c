#include "events.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <storage.h>

void change_view_on_click(GtkButton *button, gpointer user_data) {
    GtkStack *stack = GTK_STACK(user_data);
    gtk_stack_set_visible_child_name(stack, "form_view");
}

void change_to_form_page_2(GtkButton *button, gpointer user_data) {
    GtkStack *stack = GTK_STACK(user_data);
    gtk_stack_set_visible_child_name(stack, "form_view_2");
}

void on_finish_button_clicked(GtkButton *button, gpointer user_data) {
    OnboardingWidgets *widgets = (OnboardingWidgets *)user_data;

    const char *name = gtk_entry_get_text(GTK_ENTRY(widgets->entry_name));

    float weight = atof(gtk_entry_get_text(GTK_ENTRY(widgets->entry_weight)));
    int height = atoi(gtk_entry_get_text(GTK_ENTRY(widgets->entry_height)));
    int age = atoi(gtk_entry_get_text(GTK_ENTRY(widgets->entry_age)));

    float t_weight = atof(gtk_entry_get_text(GTK_ENTRY(widgets->entry_target_weight)));
    float t_muscle = atof(gtk_entry_get_text(GTK_ENTRY(widgets->entry_target_muscle)));
    float t_fat = atof(gtk_entry_get_text(GTK_ENTRY(widgets->entry_target_fat)));

    int kcal = atoi(gtk_entry_get_text(GTK_ENTRY(widgets->entry_kcal)));
    int protein = atoi(gtk_entry_get_text(GTK_ENTRY(widgets->entry_protein)));
    int fat = atoi(gtk_entry_get_text(GTK_ENTRY(widgets->entry_fat)));
    int carbs = atoi(gtk_entry_get_text(GTK_ENTRY(widgets->entry_carbs)));

    char empty_habits[MAX_HABITS][32] = {0};
    char empty_supplements[MAX_SUPPLEMENTS][32] = {0};

    UserSettings *new_user = g_new(UserSettings, 1);

    setup_user_settings(
        new_user,
        name,
        age,
        weight,
        height,
        0.0f,
        0.0f,
        t_weight,
        t_fat,
        t_muscle,
        empty_habits,
        0,
        empty_supplements,
        0,
        kcal,
        protein,
        fat,
        carbs);

    g_print("Zapisuję do RAM...\n");
    ram_store_save("user_settings", new_user);

    g_print("\n========================================\n");
    g_print("   PODSUMOWANIE ZAPISANEGO PROFILU\n");
    g_print("========================================\n");
    g_print("DANE PODSTAWOWE:\n");
    g_print("  Imię:           %s\n", new_user->user_name);
    g_print("  Wiek:           %d\n", new_user->age);
    g_print("  Waga (start):   %.1f kg\n", new_user->starting_weight);
    g_print("  Wzrost:         %d cm\n", new_user->height);
    g_print("----------------------------------------\n");
    g_print("CELE WAGOWE/SYLWETKOWE:\n");
    g_print("  Docelowa waga:  %.1f kg\n", new_user->goal_weight);
    g_print("  Docelowa MM:    %.1f %%\n", new_user->goal_mm);
    g_print("  Docelowa BF:    %.1f %%\n", new_user->goal_bf);
    g_print("----------------------------------------\n");
    g_print("MAKROELEMENTY (DIETA):\n");
    g_print("  Kcal:           %d\n", new_user->goal_kcal);
    g_print("  Białko:         %d g\n", new_user->goal_protein);
    g_print("  Tłuszcze:       %d g\n", new_user->goal_fat);
    g_print("  Węglowodany:    %d g\n", new_user->goal_carbs);
    g_print("========================================\n\n");

    // gtk_stack_set_visible_child_name(widgets->stack, "dashboard_view");
}
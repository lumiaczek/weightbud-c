#include "events.h"
#include "ui/dashboard/dashboard.h"
#include <data_managment.h>
#include <db.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <storage.h>

void change_view_on_click(GtkButton *button, gpointer user_data) {
    GtkStack *stack = GTK_STACK(user_data);
    gtk_stack_set_visible_child_name(stack, "form_view");
}

static void clear_error_state(GtkWidget *widget, GtkWidget *error_label) {
    if (widget) {
        GtkStyleContext *ctx = gtk_widget_get_style_context(widget);
        gtk_style_context_remove_class(ctx, "input-error");
    }
    if (error_label) {
        gtk_label_set_text(GTK_LABEL(error_label), "");
        gtk_widget_set_no_show_all(error_label, TRUE);
        gtk_widget_hide(error_label);
    }
}

static void set_error_state(GtkWidget *widget, GtkWidget *error_label, const char *message) {
    if (widget) {
        GtkStyleContext *ctx = gtk_widget_get_style_context(widget);
        gtk_style_context_add_class(ctx, "input-error");
    }
    if (error_label) {
        gtk_label_set_text(GTK_LABEL(error_label), message);
        gtk_widget_show(error_label);
    }
}

static gboolean is_text_empty(GtkEntry *entry) {
    const char *text = gtk_entry_get_text(entry);
    gchar *trimmed = g_strdup(text ? text : "");
    g_strstrip(trimmed);
    gboolean empty = (trimmed[0] == '\0');
    g_free(trimmed);
    return empty;
}

void change_to_form_page_2(GtkButton *button, gpointer user_data) {
    OnboardingWidgets *widgets = (OnboardingWidgets *)user_data;
    gboolean valid = TRUE;

    clear_error_state(widgets->entry_name, widgets->error_name);
    clear_error_state(widgets->radio_male, widgets->error_gender);
    clear_error_state(widgets->radio_female, NULL);
    clear_error_state(widgets->entry_weight, widgets->error_weight);
    clear_error_state(widgets->entry_height, widgets->error_height);
    clear_error_state(widgets->entry_age, widgets->error_age);
    clear_error_state(widgets->entry_target_weight, widgets->error_target_weight);
    clear_error_state(widgets->entry_target_muscle, widgets->error_target_muscle);
    clear_error_state(widgets->entry_target_fat, widgets->error_target_fat);

    if (is_text_empty(GTK_ENTRY(widgets->entry_name))) {
        set_error_state(widgets->entry_name, widgets->error_name, "Uzupełnij imię.");
        valid = FALSE;
    }

    gboolean gender_selected = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets->radio_male)) ||
                               gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets->radio_female));
    if (!gender_selected) {
        set_error_state(widgets->radio_male, widgets->error_gender, "Wybierz płeć.");
        set_error_state(widgets->radio_female, NULL, NULL);
        valid = FALSE;
    }

    if (is_text_empty(GTK_ENTRY(widgets->entry_weight))) {
        set_error_state(widgets->entry_weight, widgets->error_weight, "Uzupełnij wagę.");
        valid = FALSE;
    }
    if (is_text_empty(GTK_ENTRY(widgets->entry_height))) {
        set_error_state(widgets->entry_height, widgets->error_height, "Uzupełnij wzrost.");
        valid = FALSE;
    }
    if (is_text_empty(GTK_ENTRY(widgets->entry_age))) {
        set_error_state(widgets->entry_age, widgets->error_age, "Uzupełnij wiek.");
        valid = FALSE;
    }
    if (is_text_empty(GTK_ENTRY(widgets->entry_target_weight))) {
        set_error_state(widgets->entry_target_weight, widgets->error_target_weight, "Uzupełnij cel wagowy.");
        valid = FALSE;
    }
    if (is_text_empty(GTK_ENTRY(widgets->entry_target_muscle))) {
        set_error_state(widgets->entry_target_muscle, widgets->error_target_muscle, "Uzupełnij docelową masę mięśniową.");
        valid = FALSE;
    }
    if (is_text_empty(GTK_ENTRY(widgets->entry_target_fat))) {
        set_error_state(widgets->entry_target_fat, widgets->error_target_fat, "Uzupełnij docelowy poziom tkanki tłuszczowej.");
        valid = FALSE;
    }

    if (!valid) {
        return;
    }

    gtk_stack_set_visible_child_name(widgets->stack, "form_view_2");
}

static gboolean validate_form_page_2(OnboardingWidgets *widgets) {
    gboolean valid = TRUE;

    clear_error_state(widgets->entry_kcal, widgets->error_kcal);
    clear_error_state(widgets->entry_protein, widgets->error_protein);
    clear_error_state(widgets->entry_fat, widgets->error_fat);
    clear_error_state(widgets->entry_carbs, widgets->error_carbs);

    if (is_text_empty(GTK_ENTRY(widgets->entry_kcal))) {
        set_error_state(widgets->entry_kcal, widgets->error_kcal, "Uzupełnij kalorie.");
        valid = FALSE;
    }
    if (is_text_empty(GTK_ENTRY(widgets->entry_protein))) {
        set_error_state(widgets->entry_protein, widgets->error_protein, "Uzupełnij białko.");
        valid = FALSE;
    }
    if (is_text_empty(GTK_ENTRY(widgets->entry_fat))) {
        set_error_state(widgets->entry_fat, widgets->error_fat, "Uzupełnij tłuszcze.");
        valid = FALSE;
    }
    if (is_text_empty(GTK_ENTRY(widgets->entry_carbs))) {
        set_error_state(widgets->entry_carbs, widgets->error_carbs, "Uzupełnij węglowodany.");
        valid = FALSE;
    }

    return valid;
}

void on_finish_button_clicked(GtkButton *button, gpointer user_data) {
    OnboardingWidgets *widgets = (OnboardingWidgets *)user_data;
    if (!validate_form_page_2(widgets)) {
        return;
    }
    AppState *state = widgets->state;

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
    Diet *diet = g_new(Diet, 1);
    setup_diet(diet,
               kcal,
               protein,
               fat,
               carbs);

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
        0);

    g_print("[Events] Zapisuję do RAM...\n");
    ram_store_save("user_settings", new_user, state);
    ram_store_save("user_diet", diet, state);

    if (sync_user_settings_to_db(state)) {
        g_print("[Events] zapisano użytkownika do bazy \n");

    } else {
        g_printerr("[Events] Błąd: nie udało się zapisać do bazy danyc \n");
    }
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
    g_print("  Kcal:           %d\n", diet->goal_kcal);
    g_print("  Białko:         %d g\n", diet->goal_protein);
    g_print("  Tłuszcze:       %d g\n", diet->goal_fat);
    g_print("  Węglowodany:    %d g\n", diet->goal_carbs);
    g_print("========================================\n\n");

    cleanup_memory(state);

    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    if (GTK_IS_WINDOW(window)) {
        GtkApplication *app = gtk_window_get_application(GTK_WINDOW(window));
        show_dashboard_window(app);
        gtk_widget_destroy(window);
    }
}
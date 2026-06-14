#include "ui/dashboard/dashboard.h"
#include "ui/form/form.h"
#include <data_managment.h>
#include <db.h>
#include <gtk/gtk.h>
#include <stdlib.h>

extern void load_svg_icon(GtkBuilder *builder, const char *image_id, const char *filepath, int size);

typedef struct {
    GtkEntry *entry_1rm_weight;
    GtkEntry *entry_1rm_reps;
    GtkLabel *lbl_1rm_epley;
    GtkLabel *lbl_1rm_brzycki;

    GtkEntry *entry_pace_time;
    GtkEntry *entry_pace_distance;
    GtkLabel *lbl_pace_result;

    GtkEntry *entry_bmi_height;
    GtkEntry *entry_bmi_weight;
    GtkEntry *entry_bmi_age;
    GtkLabel *lbl_bmi_result;
    GtkLabel *lbl_bmr_result;

    GtkEntry *entry_hr_age;
    GtkLabel *lbl_hr_z1;
    GtkLabel *lbl_hr_z2;
    GtkLabel *lbl_hr_z3;
    GtkLabel *lbl_hr_z4;
    GtkLabel *lbl_hr_z5;
} ToolWidgets;

typedef struct {
    AppState *state;
    GtkBuilder *builder;
    GtkBox *habits_box;
    GtkBox *weight_box;
    int loaded_days_habits[30];
    int loaded_days_weight[30];
    int habits_count;
    int weight_count;
} DashboardContext;

static int get_today_date() {
    GDateTime *now = g_date_time_new_now_local();
    int date = g_date_time_get_year(now) * 10000 + g_date_time_get_month(now) * 100 + g_date_time_get_day_of_month(now);
    g_date_time_unref(now);
    return date;
}

static void get_today_date_string(char *buffer, size_t size) {
    GDateTime *now = g_date_time_new_now_local();
    g_snprintf(buffer, size, "%02d.%02d.%04d",
               g_date_time_get_day_of_month(now),
               g_date_time_get_month(now),
               g_date_time_get_year(now));
    g_date_time_unref(now);
}

static gboolean enforce_numeric_input(GtkEditable *editable, const gchar *new_text, gint new_text_length, gint *position, gpointer user_data) {
    for (int i = 0; i < new_text_length; i++) {
        if (!g_ascii_isdigit(new_text[i]) && new_text[i] != '.' && new_text[i] != ',') {
            g_signal_stop_emission_by_name(G_OBJECT(editable), "insert-text");
            return TRUE;
        }
    }
    return FALSE;
}

static gboolean parse_integer_entry(GtkEntry *entry, int *out_value) {
    const char *text = gtk_entry_get_text(entry);
    if (!text || *text == '\0')
        return FALSE;

    char *end;
    long value = g_ascii_strtoll(text, &end, 10);
    if (end == text || *end != '\0' || value < 0)
        return FALSE;

    *out_value = (int)value;
    return TRUE;
}

static gboolean parse_double_entry(GtkEntry *entry, double *out_value) {
    const char *text = gtk_entry_get_text(entry);
    if (!text || *text == '\0')
        return FALSE;

    char *end;
    double value = g_ascii_strtod(text, &end);
    if (end == text || *end != '\0' || value <= 0.0)
        return FALSE;

    *out_value = value;
    return TRUE;
}

static void set_label_text(GtkLabel *label, const char *text) {
    if (label) {
        gtk_label_set_text(label, text);
    }
}

static void on_1rm_input_changed(GtkEditable *editable, gpointer user_data) {
    ToolWidgets *tools = (ToolWidgets *)user_data;
    int weight = 0;
    int reps = 0;

    if (!parse_integer_entry(tools->entry_1rm_weight, &weight) || !parse_integer_entry(tools->entry_1rm_reps, &reps) || reps <= 0 || reps >= 37) {
        set_label_text(tools->lbl_1rm_epley, "---");
        set_label_text(tools->lbl_1rm_brzycki, "---");
        return;
    }

    double epley = weight * (1.0 + reps / 30.0);
    double brzycki = weight * 36.0 / (37.0 - reps);
    char text[64];

    g_snprintf(text, sizeof(text), "%.1f kg", epley);
    set_label_text(tools->lbl_1rm_epley, text);
    g_snprintf(text, sizeof(text), "%.1f kg", brzycki);
    set_label_text(tools->lbl_1rm_brzycki, text);
}

static void on_pace_input_changed(GtkEditable *editable, gpointer user_data) {
    ToolWidgets *tools = (ToolWidgets *)user_data;
    double time_min = 0.0;
    double distance_km = 0.0;

    if (!parse_double_entry(tools->entry_pace_time, &time_min) || !parse_double_entry(tools->entry_pace_distance, &distance_km) || distance_km <= 0.0) {
        set_label_text(tools->lbl_pace_result, "---");
        return;
    }

    double pace = time_min / distance_km;
    int pace_min = (int)pace;
    int pace_sec = (int)((pace - pace_min) * 60.0 + 0.5);
    if (pace_sec >= 60) {
        pace_min += 1;
        pace_sec -= 60;
    }

    char text[32];
    g_snprintf(text, sizeof(text), "%d:%02d /km", pace_min, pace_sec);
    set_label_text(tools->lbl_pace_result, text);
}

static void on_bmi_input_changed(GtkEditable *editable, gpointer user_data) {
    ToolWidgets *tools = (ToolWidgets *)user_data;
    double height_cm = 0.0;
    double weight_kg = 0.0;
    int age = 0;

    if (!parse_double_entry(tools->entry_bmi_height, &height_cm) || !parse_double_entry(tools->entry_bmi_weight, &weight_kg) || !parse_integer_entry(tools->entry_bmi_age, &age) || age <= 0) {
        set_label_text(tools->lbl_bmi_result, "---");
        set_label_text(tools->lbl_bmr_result, "---");
        return;
    }

    double height_m = height_cm / 100.0;
    if (height_m <= 0.0) {
        set_label_text(tools->lbl_bmi_result, "---");
        set_label_text(tools->lbl_bmr_result, "---");
        return;
    }

    double bmi = weight_kg / (height_m * height_m);
    double bmr = 10.0 * weight_kg + 6.25 * height_cm - 5.0 * age + 5.0; // Wzór dla mężczyzny (Mifflin-St Jeor)
    char text[64];

    g_snprintf(text, sizeof(text), "%.1f", bmi);
    set_label_text(tools->lbl_bmi_result, text);
    g_snprintf(text, sizeof(text), "%.0f kcal", bmr);
    set_label_text(tools->lbl_bmr_result, text);
}

static void on_hr_input_changed(GtkEditable *editable, gpointer user_data) {
    ToolWidgets *tools = (ToolWidgets *)user_data;
    int age = 0;

    if (!parse_integer_entry(tools->entry_hr_age, &age) || age <= 0 || age >= 120) {
        set_label_text(tools->lbl_hr_z1, "---");
        set_label_text(tools->lbl_hr_z2, "---");
        set_label_text(tools->lbl_hr_z3, "---");
        set_label_text(tools->lbl_hr_z4, "---");
        set_label_text(tools->lbl_hr_z5, "---");
        return;
    }

    int max_hr = 220 - age;
    int z1_lo = (int)(max_hr * 0.50 + 0.5);
    int z1_hi = (int)(max_hr * 0.60 + 0.5);
    int z2_lo = (int)(max_hr * 0.61 + 0.5);
    int z2_hi = (int)(max_hr * 0.70 + 0.5);
    int z3_lo = (int)(max_hr * 0.71 + 0.5);
    int z3_hi = (int)(max_hr * 0.80 + 0.5);
    int z4_lo = (int)(max_hr * 0.81 + 0.5);
    int z4_hi = (int)(max_hr * 0.90 + 0.5);
    int z5_lo = (int)(max_hr * 0.91 + 0.5);
    char text[64];

    g_snprintf(text, sizeof(text), "%d - %d bpm", z1_lo, z1_hi);
    set_label_text(tools->lbl_hr_z1, text);
    g_snprintf(text, sizeof(text), "%d - %d bpm", z2_lo, z2_hi);
    set_label_text(tools->lbl_hr_z2, text);
    g_snprintf(text, sizeof(text), "%d - %d bpm", z3_lo, z3_hi);
    set_label_text(tools->lbl_hr_z3, text);
    g_snprintf(text, sizeof(text), "%d - %d bpm", z4_lo, z4_hi);
    set_label_text(tools->lbl_hr_z4, text);
    g_snprintf(text, sizeof(text), ">= %d bpm", z5_lo);
    set_label_text(tools->lbl_hr_z5, text);
}

static void connect_tool_callbacks(GtkBuilder *builder, GtkWidget *window) {
    ToolWidgets *tools = g_new0(ToolWidgets, 1);

    tools->entry_1rm_weight = GTK_ENTRY(gtk_builder_get_object(builder, "entry_1rm_weight"));
    tools->entry_1rm_reps = GTK_ENTRY(gtk_builder_get_object(builder, "entry_1rm_reps"));
    tools->lbl_1rm_epley = GTK_LABEL(gtk_builder_get_object(builder, "lbl_1rm_epley"));
    tools->lbl_1rm_brzycki = GTK_LABEL(gtk_builder_get_object(builder, "lbl_1rm_brzycki"));

    tools->entry_pace_time = GTK_ENTRY(gtk_builder_get_object(builder, "entry_pace_time"));
    tools->entry_pace_distance = GTK_ENTRY(gtk_builder_get_object(builder, "entry_pace_distance"));
    tools->lbl_pace_result = GTK_LABEL(gtk_builder_get_object(builder, "lbl_pace_result"));

    tools->entry_bmi_height = GTK_ENTRY(gtk_builder_get_object(builder, "entry_bmi_height"));
    tools->entry_bmi_weight = GTK_ENTRY(gtk_builder_get_object(builder, "entry_bmi_weight"));
    tools->entry_bmi_age = GTK_ENTRY(gtk_builder_get_object(builder, "entry_bmi_age"));
    tools->lbl_bmi_result = GTK_LABEL(gtk_builder_get_object(builder, "lbl_bmi_result"));
    tools->lbl_bmr_result = GTK_LABEL(gtk_builder_get_object(builder, "lbl_bmr_result"));

    tools->entry_hr_age = GTK_ENTRY(gtk_builder_get_object(builder, "entry_hr_age"));
    tools->lbl_hr_z1 = GTK_LABEL(gtk_builder_get_object(builder, "lbl_hr_z1"));
    tools->lbl_hr_z2 = GTK_LABEL(gtk_builder_get_object(builder, "lbl_hr_z2"));
    tools->lbl_hr_z3 = GTK_LABEL(gtk_builder_get_object(builder, "lbl_hr_z3"));
    tools->lbl_hr_z4 = GTK_LABEL(gtk_builder_get_object(builder, "lbl_hr_z4"));
    tools->lbl_hr_z5 = GTK_LABEL(gtk_builder_get_object(builder, "lbl_hr_z5"));

    g_signal_connect(tools->entry_1rm_weight, "changed", G_CALLBACK(on_1rm_input_changed), tools);
    g_signal_connect(tools->entry_1rm_reps, "changed", G_CALLBACK(on_1rm_input_changed), tools);

    g_signal_connect(tools->entry_pace_time, "changed", G_CALLBACK(on_pace_input_changed), tools);
    g_signal_connect(tools->entry_pace_distance, "changed", G_CALLBACK(on_pace_input_changed), tools);

    g_signal_connect(tools->entry_bmi_height, "changed", G_CALLBACK(on_bmi_input_changed), tools);
    g_signal_connect(tools->entry_bmi_weight, "changed", G_CALLBACK(on_bmi_input_changed), tools);
    g_signal_connect(tools->entry_bmi_age, "changed", G_CALLBACK(on_bmi_input_changed), tools);

    g_signal_connect(tools->entry_hr_age, "changed", G_CALLBACK(on_hr_input_changed), tools);

    GtkEntry *numeric_entries[] = {
        tools->entry_1rm_weight,
        tools->entry_1rm_reps,
        tools->entry_pace_time,
        tools->entry_pace_distance,
        tools->entry_bmi_height,
        tools->entry_bmi_weight,
        tools->entry_bmi_age,
        tools->entry_hr_age,
    };

    for (size_t i = 0; i < G_N_ELEMENTS(numeric_entries); i++) {
        if (numeric_entries[i]) {
            g_signal_connect(numeric_entries[i], "insert-text", G_CALLBACK(enforce_numeric_input), NULL);
            g_signal_emit_by_name(numeric_entries[i], "changed");
        }
    }

    g_object_set_data_full(G_OBJECT(window), "tool-widgets", tools, g_free);
}

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

static void refresh_dashboard_stats(DashboardContext *ctx) {
    char buffer[64];
    const char *username = g_hash_table_lookup(ctx->state->memory_collection, "current_user");
    if (!username)
        return;

    double start_w = 0, curr_w = 0, curr_bf = -1, curr_mm = -1, vo2max = 0;
    int workouts_week = 0;

    if (db_get_dashboard_data(ctx->state->db, username, &start_w, &curr_w, &curr_bf, &curr_mm, &vo2max, &workouts_week)) {

        g_snprintf(buffer, sizeof(buffer), "%.1f kg", start_w);
        set_label_text(GTK_LABEL(gtk_builder_get_object(ctx->builder, "lbl_dash_start_weight")), buffer);

        g_snprintf(buffer, sizeof(buffer), "%.2f kg", curr_w);
        set_label_text(GTK_LABEL(gtk_builder_get_object(ctx->builder, "lbl_stat_weight")), buffer);

        if (curr_bf >= 0 && curr_mm >= 0)
            g_snprintf(buffer, sizeof(buffer), "%.1f %% | %.1f %%", curr_bf, curr_mm);
        else
            g_snprintf(buffer, sizeof(buffer), "--- %% | --- %%");
        set_label_text(GTK_LABEL(gtk_builder_get_object(ctx->builder, "lbl_dash_bf_mm")), buffer);

        g_snprintf(buffer, sizeof(buffer), "%d / 5", workouts_week);
        set_label_text(GTK_LABEL(gtk_builder_get_object(ctx->builder, "lbl_stat_workouts")), buffer);

        if (vo2max > 0)
            g_snprintf(buffer, sizeof(buffer), "%.1f ml/kg/min", vo2max);
        else
            g_snprintf(buffer, sizeof(buffer), "Brak danych");
        set_label_text(GTK_LABEL(gtk_builder_get_object(ctx->builder, "lbl_stat_vo2max")), buffer);
    }

    DailyRecord *record = g_hash_table_lookup(ctx->state->memory_collection, "current_daily_record");
    GtkWidget *btn_dash_start = GTK_WIDGET(gtk_builder_get_object(ctx->builder, "btn_dash_start_day"));
    GtkWidget *row_today = GTK_WIDGET(gtk_builder_get_object(ctx->builder, "row_today_habits"));

    if (record) {
        int habits_done = 0;
        for (int i = 0; i < record->habit_count; i++) {
            if (record->habits[i].completed)
                habits_done++;
        }
        g_snprintf(buffer, sizeof(buffer), "%d / %d", habits_done, record->habit_count);
        set_label_text(GTK_LABEL(gtk_builder_get_object(ctx->builder, "lbl_stat_habits")), buffer);

        char date_str[32];
        get_today_date_string(date_str, sizeof(date_str));
        set_label_text(GTK_LABEL(gtk_builder_get_object(ctx->builder, "lbl_dash_action_val")), date_str);
        set_label_text(GTK_LABEL(gtk_builder_get_object(ctx->builder, "lbl_habit_status_title")), "Nawyki · dzień w toku");

        if (row_today) {
            char final_str[64];
            g_snprintf(final_str, sizeof(final_str), "📄 %s (Dziś)", date_str);
            set_label_text(GTK_LABEL(gtk_builder_get_object(ctx->builder, "lbl_today_row_date")), final_str);
            gtk_widget_show(row_today);
        }

        if (btn_dash_start)
            gtk_widget_set_sensitive(btn_dash_start, FALSE);

    } else {
        set_label_text(GTK_LABEL(gtk_builder_get_object(ctx->builder, "lbl_stat_habits")), "0 / 0");
        set_label_text(GTK_LABEL(gtk_builder_get_object(ctx->builder, "lbl_dash_action_val")), "Rozpocznij dzień");
        set_label_text(GTK_LABEL(gtk_builder_get_object(ctx->builder, "lbl_habit_status_title")), "Nawyki · dzień nierozpoczęty");

        if (row_today)
            gtk_widget_hide(row_today);

        if (btn_dash_start)
            gtk_widget_set_sensitive(btn_dash_start, TRUE);
    }
}

static void on_start_day_clicked(GtkButton *btn, gpointer user_data) {
    DashboardContext *ctx = (DashboardContext *)user_data;
    const char *username = g_hash_table_lookup(ctx->state->memory_collection, "current_user");

    UserSettings *user = g_hash_table_lookup(ctx->state->memory_collection, "user_settings");
    if (!user) {
        user = g_new0(UserSettings, 1);
        db_load_user_settings(ctx->state->db, username, user);
        ram_store_save("user_settings", user, ctx->state);
    }

    int today = get_today_date();
    DailyRecord *record = g_new0(DailyRecord, 1);

    init_daily_record(record, today, user);
    ram_store_save("current_daily_record", record, ctx->state);

    db_mark_day_started(ctx->state->db, username, today);

    char date_str[32];
    get_today_date_string(date_str, sizeof(date_str));
    g_print("\n========================================\n");
    g_print("[Dashboard] Prawidłowo rozpoczęto dzień!\n");
    g_print("Data: %s\n", date_str);
    g_print("Użytkownik: %s\n", user->user_name);
    g_print("========================================\n\n");

    gtk_widget_set_sensitive(GTK_WIDGET(btn), FALSE);
    refresh_dashboard_stats(ctx);
}

// Konwertuj datę z formatu YYYYMMDD na string "DD.MM.YYYY"
static void format_date_from_int(int date, char *buffer, size_t size) {
    int year = date / 10000;
    int month = (date / 100) % 100;
    int day = date % 100;
    g_snprintf(buffer, size, "%02d.%02d.%04d", day, month, year);
}

// Załaduj i wyświetl poprzednie dni nawyków
static void load_and_display_previous_days_habits(DashboardContext *ctx) {
    const char *username = g_hash_table_lookup(ctx->state->memory_collection, "current_user");
    if (!username)
        return;

    int dates[30];
    int count = 0;
    if (!db_get_recent_days(ctx->state->db, username, 30, dates, &count)) {
        g_print("[Dashboard] Brak poprzednich dni w bazie\n");
        return;
    }

    // Przechowaj załadowane dni
    ctx->habits_count = count;
    for (int i = 0; i < count; i++) {
        ctx->loaded_days_habits[i] = dates[i];
    }

    // Załaduj i wyświetl dane w UI
    UserSettings *user = g_hash_table_lookup(ctx->state->memory_collection, "user_settings");
    if (!user) {
        user = g_new0(UserSettings, 1);
        db_load_user_settings(ctx->state->db, username, user);
        ram_store_save("user_settings", user, ctx->state);
    }

    // Załaduj ostatni dzień i aktualizuj UI
    if (count > 0) {
        int date = dates[0];
        int today = get_today_date();

        // Jeśli ostatni dzień to nie dzisiaj, wyświetl jego dane
        if (date != today) {
            DailyRecord *record = g_new0(DailyRecord, 1);
            if (db_load_daily_record(ctx->state->db, username, date, record)) {
                char date_str[32];
                format_date_from_int(date, date_str, sizeof(date_str));

                // Przygotuj dane do wyświetlenia w UI
                int habits_done = 0;
                for (int i = 0; i < record->habit_count; i++) {
                    if (record->habits[i].completed)
                        habits_done++;
                }

                g_print("[Dashboard] Ostatni dzień - %s: %d / %d nawyków\n",
                        date_str, habits_done, record->habit_count);
            }
            g_free(record);
        }
    }

    g_print("[Dashboard] Załadowano %d poprzednich dni (nawyki)\n", count);
}

// Załaduj i wyświetl poprzednie dni wagi
static void load_and_display_previous_days_weight(DashboardContext *ctx) {
    const char *username = g_hash_table_lookup(ctx->state->memory_collection, "current_user");
    if (!username)
        return;

    int dates[30];
    int count = 0;
    if (!db_get_recent_days(ctx->state->db, username, 30, dates, &count)) {
        g_print("[Dashboard] Brak poprzednich dni w bazie\n");
        return;
    }

    // Przechowaj załadowane dni
    ctx->weight_count = count;
    for (int i = 0; i < count; i++) {
        ctx->loaded_days_weight[i] = dates[i];
    }

    // Załaduj ostatni dzień i wyświetl dane
    if (count > 0) {
        int date = dates[0];
        int today = get_today_date();

        // Jeśli ostatni dzień to nie dzisiaj, wyświetl jego dane
        if (date != today) {
            DailyRecord *record = g_new0(DailyRecord, 1);
            if (db_load_daily_record(ctx->state->db, username, date, record)) {
                char date_str[32];
                format_date_from_int(date, date_str, sizeof(date_str));

                if (record->body_measure.weight > 0) {
                    g_print("[Dashboard] Ostatni dzień - %s: Waga: %.1f kg, BF: %.1f%%, MM: %.1f%%\n",
                            date_str, record->body_measure.weight,
                            record->body_measure.bodyfat_pct,
                            record->body_measure.musclemass_pct);
                }
            }
            g_free(record);
        }
    }

    g_print("[Dashboard] Załadowano %d poprzednich dni (waga)\n", count);
}

// Callback dla zmiany nawyka
static void on_habit_toggled(GtkToggleButton *btn, gpointer user_data) {
    DashboardContext *ctx = (DashboardContext *)user_data;
    const char *username = g_hash_table_lookup(ctx->state->memory_collection, "current_user");
    if (!username)
        return;

    // Pobierz dane z przycisku
    const char *habit_name = g_object_get_data(G_OBJECT(btn), "habit_name");
    int date = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "date"));

    gboolean completed = gtk_toggle_button_get_active(btn);

    // Zapisz do bazy
    db_save_habit_completion(ctx->state->db, username, date, habit_name, completed);
    // Zaktualizuj aktualny dzień w pamięci
    DailyRecord *record = g_hash_table_lookup(ctx->state->memory_collection, "current_daily_record");
    if (record && record->date == date) {
        for (int i = 0; i < record->habit_count; i++) {
            if (g_strcmp0(record->habits[i].name, habit_name) == 0) {
                record->habits[i].completed = completed;
                break;
            }
        }
        refresh_dashboard_stats(ctx);
    }
    g_print("[Dashboard] Nawyк '%s' na dzień %d: %s\n", habit_name, date, completed ? "zaznaczony" : "niezaznaczony");
}

// Callback dla zmiany wagi
static void on_weight_changed(GtkSpinButton *btn, gpointer user_data) {
    DashboardContext *ctx = (DashboardContext *)user_data;
    const char *username = g_hash_table_lookup(ctx->state->memory_collection, "current_user");
    if (!username)
        return;

    int date = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "date"));
    const char *field = g_object_get_data(G_OBJECT(btn), "field");
    double value = gtk_spin_button_get_value(btn);

    // Pobierz aktualny rekord
    DailyRecord *record = g_new0(DailyRecord, 1);
    if (db_load_daily_record(ctx->state->db, username, date, record)) {
        if (g_strcmp0(field, "weight") == 0) {
            record->body_measure.weight = value;
        } else if (g_strcmp0(field, "bf") == 0) {
            record->body_measure.bodyfat_pct = value;
        } else if (g_strcmp0(field, "mm") == 0) {
            record->body_measure.musclemass_pct = value;
        }
        db_save_daily_record(ctx->state->db, username, record);
    }
    g_free(record);

    g_print("[Dashboard] Zmieniono %s dla dnia %d na %.2f\n", field, date, value);
}

// Podłącz wszystkie checkboxy nawyków i pola wagi do callbacków
static void setup_habit_and_weight_callbacks(GtkBuilder *builder, DashboardContext *ctx) {
    const char *username = g_hash_table_lookup(ctx->state->memory_collection, "current_user");
    if (!username)
        return;

    int today = get_today_date();
    UserSettings *user = g_hash_table_lookup(ctx->state->memory_collection, "user_settings");
    if (!user) {
        user = g_new0(UserSettings, 1);
        db_load_user_settings(ctx->state->db, username, user);
    }

    // Podłącz checkboxy dla dzisiaj
    for (int i = 0; i < user->habit_count && i < 8; i++) {
        char checkbox_id[32];
        g_snprintf(checkbox_id, sizeof(checkbox_id), "chk_habit_%d_today", i);
        GtkCheckButton *chk = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, checkbox_id));
        if (chk) {
            g_object_set_data(G_OBJECT(chk), "habit_name", g_strdup(user->habit_names[i]));
            g_object_set_data(G_OBJECT(chk), "date", GINT_TO_POINTER(today));
            g_signal_connect(chk, "toggled", G_CALLBACK(on_habit_toggled), ctx);
        }
    }

    // Podłącz checkboxy dla poprzednich dni
    for (int day_idx = 0; day_idx < 2 && day_idx + 1 < ctx->habits_count; day_idx++) {
        int date = ctx->loaded_days_habits[day_idx + 1];
        for (int i = 0; i < user->habit_count && i < 8; i++) {
            char checkbox_id[32];
            g_snprintf(checkbox_id, sizeof(checkbox_id), "chk_habit_%d_day%d", i, day_idx);
            GtkCheckButton *chk = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, checkbox_id));
            if (chk) {
                g_object_set_data(G_OBJECT(chk), "habit_name", g_strdup(user->habit_names[i]));
                g_object_set_data(G_OBJECT(chk), "date", GINT_TO_POINTER(date));
                g_signal_connect(chk, "toggled", G_CALLBACK(on_habit_toggled), ctx);
            }
        }
    }

    // Podłącz SpinButton dla wagi - dzisiaj
    GtkSpinButton *spin_weight_today = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spin_weight_today"));
    if (spin_weight_today) {
        g_object_set_data(G_OBJECT(spin_weight_today), "date", GINT_TO_POINTER(today));
        g_object_set_data(G_OBJECT(spin_weight_today), "field", "weight");
        g_signal_connect(spin_weight_today, "value-changed", G_CALLBACK(on_weight_changed), ctx);
    }

    GtkSpinButton *spin_bf_today = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spin_bf_today"));
    if (spin_bf_today) {
        g_object_set_data(G_OBJECT(spin_bf_today), "date", GINT_TO_POINTER(today));
        g_object_set_data(G_OBJECT(spin_bf_today), "field", "bf");
        g_signal_connect(spin_bf_today, "value-changed", G_CALLBACK(on_weight_changed), ctx);
    }

    GtkSpinButton *spin_mm_today = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spin_mm_today"));
    if (spin_mm_today) {
        g_object_set_data(G_OBJECT(spin_mm_today), "date", GINT_TO_POINTER(today));
        g_object_set_data(G_OBJECT(spin_mm_today), "field", "mm");
        g_signal_connect(spin_mm_today, "value-changed", G_CALLBACK(on_weight_changed), ctx);
    }

    // Podłącz SpinButton dla wagi - poprzednie dni
    const char *spin_field_names[] = {"weight", "bf", "mm"};
    const char *spin_prefixes[] = {"spin_weight_day", "spin_bf_day", "spin_mm_day"};

    for (int day_idx = 0; day_idx < 3 && day_idx < ctx->weight_count; day_idx++) {
        int date = ctx->loaded_days_weight[day_idx];
        for (int field = 0; field < 3; field++) {
            char spin_id[32];
            g_snprintf(spin_id, sizeof(spin_id), "%s%d", spin_prefixes[field], day_idx);
            GtkSpinButton *spin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, spin_id));
            if (spin) {
                g_object_set_data(G_OBJECT(spin), "date", GINT_TO_POINTER(date));
                g_object_set_data(G_OBJECT(spin), "field", (gpointer)spin_field_names[field]);
                g_signal_connect(spin, "value-changed", G_CALLBACK(on_weight_changed), ctx);
            }
        }
    }

    g_print("[Dashboard] Podłączono callbacki dla %d checkboksów i spinbuttonów\n", user->habit_count * 3 + 3);
}

void show_dashboard_window(GtkApplication *app, AppState *state) {
    GtkBuilder *builder = gtk_builder_new_from_file("dashboard.ui");
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "dashboard_window"));

    DashboardContext *ctx = g_new0(DashboardContext, 1);
    ctx->state = state;
    ctx->builder = builder;
    g_object_set_data_full(G_OBJECT(window), "dashboard-context", ctx, g_free);

    gtk_application_add_window(app, GTK_WINDOW(window));

    GtkStack *stack = GTK_STACK(gtk_builder_get_object(builder, "dashboard_stack"));

    GtkWidget *btn_start = GTK_WIDGET(gtk_builder_get_object(builder, "btn_dash_start_day"));
    if (btn_start)
        g_signal_connect(btn_start, "clicked", G_CALLBACK(on_start_day_clicked), ctx);

    GtkWidget *btn_habit_start = GTK_WIDGET(gtk_builder_get_object(builder, "btn_habit_start_day"));
    if (btn_habit_start)
        g_signal_connect(btn_habit_start, "clicked", G_CALLBACK(on_start_day_clicked), ctx);

    setup_nav_button(builder, stack, "btn_nav_dashboard", "page_dashboard", "icon_nav_dashboard", "assets/icons/dashboard/home.svg");
    setup_nav_button(builder, stack, "btn_nav_nawyki", "page_nawyki", "icon_nav_nawyki", "assets/icons/dashboard/activity.svg");
    setup_nav_button(builder, stack, "btn_nav_trening", "page_trening", "icon_nav_trening", "assets/icons/dashboard/dumbbell.svg");
    setup_nav_button(builder, stack, "btn_nav_waga", "page_waga", "icon_nav_waga", "assets/icons/dashboard/weight.svg");
    setup_nav_button(builder, stack, "btn_nav_narzedzia", "page_narzedzia", "icon_nav_narzedzia", "assets/icons/dashboard/wrench.svg");

    load_svg_icon(builder, "icon_toggle", "assets/icons/sidebar.svg", 24);
    load_svg_icon(builder, "icon_settings", "assets/icons/settings.svg", 24);
    load_svg_icon(builder, "icon_lock", "assets/icons/lock.svg", 24);

    load_svg_icon(builder, "icon_header_tools", "assets/icons/tools/wrench.svg", 28);
    load_svg_icon(builder, "icon_tool_1rm", "assets/icons/tools/calculator.svg", 20);
    load_svg_icon(builder, "icon_tool_hr", "assets/icons/tools/heart-pulse.svg", 20);
    load_svg_icon(builder, "icon_tool_pace", "assets/icons/tools/timer.svg", 20);
    load_svg_icon(builder, "icon_tool_bmi", "assets/icons/tools/weight.svg", 20);

    load_svg_icon(builder, "icon_dash_calendar", "assets/icons/dashboard/calendar.svg", 20);
    load_svg_icon(builder, "icon_dash_weight", "assets/icons/dashboard/weight_card.svg", 20);
    load_svg_icon(builder, "icon_dash_muscle", "assets/icons/dashboard/muscle.svg", 20);
    load_svg_icon(builder, "icon_stat_trend_blue", "assets/icons/dashboard/trend.svg", 16);
    load_svg_icon(builder, "icon_stat_check_red", "assets/icons/dashboard/check.svg", 16);
    load_svg_icon(builder, "icon_stat_dumbbell_green", "assets/icons/dashboard/sport-shoe.svg", 16);
    load_svg_icon(builder, "icon_stat_pulse_orange", "assets/icons/dashboard/gauge.svg", 16);

    load_svg_icon(builder, "icon_header_habits", "assets/icons/activity_n.svg", 28);
    load_svg_icon(builder, "icon_header_weight_page", "assets/icons/dashboard/weight.svg", 28);

    connect_tool_callbacks(builder, window);

    const char *current_user = (const char *)g_hash_table_lookup(state->memory_collection, "current_user");

    if (current_user) {
        char buffer[64];
        g_snprintf(buffer, sizeof(buffer), "Witaj, %s!", current_user);
        set_label_text(GTK_LABEL(gtk_builder_get_object(builder, "dashboard_user_name")), buffer);

        int today = get_today_date();
        if (db_is_day_started(state->db, current_user, today)) {
            g_print("[Dashboard] Wykryto już rozpoczęty dzień w bazie (Log_Date: %d).\n", today);

            DailyRecord *record = g_new0(DailyRecord, 1);
            UserSettings *user = g_hash_table_lookup(state->memory_collection, "user_settings");
            if (!user) {
                user = g_new0(UserSettings, 1);
                db_load_user_settings(state->db, current_user, user);
                ram_store_save("user_settings", user, state);
            }
            init_daily_record(record, today, user);
            ram_store_save("current_daily_record", record, state);
        }
    }

    refresh_dashboard_stats(ctx);

    // Załaduj poprzednie dni dla dziennika
    load_and_display_previous_days_habits(ctx);
    load_and_display_previous_days_weight(ctx);

    // Podłącz wszystkie callbacki dla nawyków i wagi
    setup_habit_and_weight_callbacks(builder, ctx);

    gtk_widget_show_all(window);
}
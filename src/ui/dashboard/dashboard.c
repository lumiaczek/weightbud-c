#include "ui/dashboard/dashboard.h"
#include "ui/form/form.h"
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

void show_dashboard_window(GtkApplication *app) {
    GtkBuilder *builder = gtk_builder_new_from_file("dashboard.ui");
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "dashboard_window"));

    gtk_application_add_window(app, GTK_WINDOW(window));

    GtkStack *stack = GTK_STACK(gtk_builder_get_object(builder, "dashboard_stack"));

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
    load_svg_icon(builder, "icon_dash_weight", "assets/icons/dashboard/weight.svg", 20);
    load_svg_icon(builder, "icon_dash_muscle", "assets/icons/dashboard/muscle.svg", 20);
    load_svg_icon(builder, "icon_stat_trend_blue", "assets/icons/dashboard/trend.svg", 16);
    load_svg_icon(builder, "icon_stat_check_red", "assets/icons/dashboard/check.svg", 16);
    load_svg_icon(builder, "icon_stat_pill_green", "assets/icons/dashboard/pill.svg", 16);
    load_svg_icon(builder, "icon_stat_fire_orange", "assets/icons/dashboard/fire.svg", 16);

    connect_tool_callbacks(builder, window);

    g_object_unref(builder);
    gtk_widget_show_all(window);
}
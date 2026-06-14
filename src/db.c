#include "db.h"
#include "data_managment.h"
#include "storage.h"
#include <gtk/gtk.h>
#include <sqlite3.h>
#include <stdarg.h>

gboolean db_create_table(AppState *state, const char *table) {
    char *err_msg = 0;
    int rc = sqlite3_exec(state->db, table, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DB] SQL error during table creation %s\n", err_msg);
        err_msg = NULL;
        sqlite3_free(err_msg);
        return FALSE;
    }
    return TRUE;
}

int db_get_user_id(sqlite3 *db, const char *user_name) {
    const char *sql = "SELECT id FROM user_initial WHERE user_name = ?;";
    sqlite3_stmt *stmt;
    int id = -1;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user_name, -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            id = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);

    } else {
        g_printerr("Failed to prepare select for user ID");
    }
    return id;
}
char *db_get_user_name(sqlite3 *db, const char *user_name) {
    const char *sql = "SELECT user_name FROM user_initial WHERE user_name = ?;";
    sqlite3_stmt *stmt;
    char *name = NULL;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user_name, -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char *fetched_name = sqlite3_column_text(stmt, 0);

            if (fetched_name) {
                name = g_strdup((const char *)fetched_name);
            }
        }

        sqlite3_finalize(stmt);
    } else {
        g_printerr("Failed to prepare login statement: %s\n", sqlite3_errmsg(db));
    }
    printf("the user name is %s \n", name);
    return name;
}

gboolean db_init(AppState *state) {

    int rc = sqlite3_open("weightbud.db", &state->db);
    char *err_msg = 0;
    int pragma = sqlite3_exec(state->db, "PRAGMA foreign_keys = ON", 0, 0, &err_msg);

    if (pragma != SQLITE_OK) {
        printf("[DB]: Fail on calling pragma %s\n", err_msg);
        err_msg = NULL;
        sqlite3_free(err_msg);
        return FALSE;
    }
    if (rc != SQLITE_OK) {
        printf("[DB] Błąd przy otwarciu bazy: %s\n", sqlite3_errmsg(state->db));
        sqlite3_close(state->db);
        return FALSE;
    }
    printf("[DB] Succsefully intiialized database");
    db_create_table(state, SQL_USER_INITIAL);
    db_create_table(state, SQL_USER_GOAL);
    db_create_table(state, SQL_USER_HABIT);
    db_create_table(state, SQL_DAILY_HABITS);
    db_create_table(state, SQL_DAILY_MEASUREMENTS);
    db_create_table(state, SQL_WORKOUTS);

    return TRUE;
}

gboolean db_execute_query(sqlite3 *db, const char *sql, const char *format, ...) {

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        g_printerr("[DB] Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return FALSE;
    }
    va_list args;
    va_start(args, format);

    for (int i = 0; format[i] != '\0'; i++) {
        int bind_idx = i + 1;

        switch (format[i]) {
        case 'i':
            sqlite3_bind_int(stmt, bind_idx, va_arg(args, int));
            break;
        case 'd':
            sqlite3_bind_double(stmt, bind_idx, va_arg(args, double));
            break;
        case 's':
            sqlite3_bind_text(stmt, bind_idx, va_arg(args, const char *), -1, SQLITE_TRANSIENT);
            break;
        default:
            g_printerr("[DB] Unknown format character '%c'\n", format[i]);
            sqlite3_finalize(stmt);
            va_end(args);
            return FALSE;
        }
    }
    va_end(args);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        g_printerr("[DB] Execution failed: %s\n", sqlite3_errmsg(db));
        return FALSE;
    }
    return TRUE;
}
gboolean sync_user_settings_to_db(AppState *state) {
    UserSettings *user = (UserSettings *)g_hash_table_lookup(state->memory_collection, "user_settings");

    if (!user)
        return FALSE;

    const char *sql_user =
        "INSERT INTO user_initial (user_name, age, height, starting_weight, starting_mm) "
        "VALUES (?, ?, ?, ?, ?) "
        "ON CONFLICT(user_name) DO UPDATE SET "
        "age = excluded.age, "
        "height = excluded.height, "
        "starting_weight = excluded.starting_weight, "
        "starting_mm = excluded.starting_mm;";

    if (!db_execute_query(state->db, sql_user, "siidd",
                          user->user_name, user->age, user->height,
                          (double)user->starting_weight, (double)user->starting_mm)) {
        return FALSE;
    }
    int current_user_id = db_get_user_id(state->db, user->user_name);
    if (current_user_id == -1) {
        g_printerr("Could not find user ID for %s to link foreign key \n", user->user_name);
        return FALSE;
    }

    const char *sql_goal =
        "INSERT INTO user_goal (user_id, goal_weight, goal_bf, goal_mm) "
        "VALUES (?, ?, ?, ?) "
        "ON CONFLICT(user_id) DO UPDATE SET "
        "goal_weight = excluded.goal_weight, "
        "goal_bf = excluded.goal_bf, "
        "goal_mm = excluded.goal_mm;";

    if (!db_execute_query(state->db, sql_goal, "iddd",
                          current_user_id, (double)user->goal_weight,
                          (double)user->goal_bf, (double)user->goal_mm)) {
        return FALSE;
    }

    g_print("[DB] All user settings synced successfully!\n");
    return TRUE;
}

gboolean db_load_user_settings(sqlite3 *db, const char *username, UserSettings *config) {
    int user_id = db_get_user_id(db, username);
    if (user_id == -1)
        return FALSE;

    memset(config, 0, sizeof(UserSettings));
    strncpy(config->user_name, username, 31);

    const char *sql_user = "SELECT age, height, starting_weight, starting_mm FROM user_initial WHERE id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql_user, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            config->age = sqlite3_column_int(stmt, 0);
            config->height = sqlite3_column_int(stmt, 1);
            config->starting_weight = sqlite3_column_double(stmt, 2);
            config->starting_mm = sqlite3_column_double(stmt, 3);
        }
        sqlite3_finalize(stmt);
    }

    const char *sql_habits = "SELECT name FROM user_habits WHERE user_id = ? LIMIT 10;";
    if (sqlite3_prepare_v2(db, sql_habits, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        int i = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW && i < MAX_HABITS) {
            const unsigned char *h_name = sqlite3_column_text(stmt, 0);
            if (h_name)
                strncpy(config->habit_names[i], (const char *)h_name, 31);
            i++;
        }
        config->habit_count = i;
        sqlite3_finalize(stmt);
    }
    return TRUE;
}

gboolean db_get_dashboard_data(sqlite3 *db, const char *username, double *start_w, double *curr_w, double *curr_bf, double *curr_mm, double *vo2max, int *workouts_week) {
    int user_id = db_get_user_id(db, username);
    if (user_id == -1)
        return FALSE;

    *start_w = 0.0;
    *curr_w = 0.0;
    *curr_bf = -1.0;
    *curr_mm = -1.0;
    *vo2max = 0.0;
    *workouts_week = 0;

    const char *sql_initial = "SELECT starting_weight FROM user_initial WHERE id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql_initial, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            *start_w = sqlite3_column_double(stmt, 0);
            *curr_w = *start_w;
        }
        sqlite3_finalize(stmt);
    }

    const char *sql_daily = "SELECT weight, bf, mm, vo2max FROM daily_measurements WHERE user_id = ? ORDER BY log_date DESC LIMIT 1;";
    if (sqlite3_prepare_v2(db, sql_daily, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            *curr_w = sqlite3_column_double(stmt, 0);
            *curr_bf = sqlite3_column_double(stmt, 1);
            *curr_mm = sqlite3_column_double(stmt, 2);
            *vo2max = sqlite3_column_double(stmt, 3);
        }
        sqlite3_finalize(stmt);
    }

    GDateTime *now = g_date_time_new_now_local();
    GDateTime *week_ago = g_date_time_add_days(now, -7);
    int week_ago_int = g_date_time_get_year(week_ago) * 10000 + g_date_time_get_month(week_ago) * 100 + g_date_time_get_day_of_month(week_ago);
    g_date_time_unref(now);
    g_date_time_unref(week_ago);

    const char *sql_workouts = "SELECT COUNT(*) FROM workouts WHERE user_id = ? AND log_date >= ?;";
    if (sqlite3_prepare_v2(db, sql_workouts, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_int(stmt, 2, week_ago_int);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            *workouts_week = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    return TRUE;
}

gboolean db_is_day_started(sqlite3 *db, const char *username, int date) {
    int user_id = db_get_user_id(db, username);
    if (user_id == -1)
        return FALSE;

    const char *sql = "SELECT COUNT(*) FROM daily_measurements WHERE user_id = ? AND log_date = ?;";
    sqlite3_stmt *stmt;
    gboolean started = FALSE;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_int(stmt, 2, date);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            started = (sqlite3_column_int(stmt, 0) > 0);
        }
        sqlite3_finalize(stmt);
    }
    return started;
}

gboolean db_mark_day_started(sqlite3 *db, const char *username, int date) {
    int user_id = db_get_user_id(db, username);
    if (user_id == -1)
        return FALSE;

    const char *sql = "INSERT OR IGNORE INTO daily_measurements (user_id, log_date) VALUES (?, ?);";
    return db_execute_query(db, sql, "ii", user_id, date);
}

// Załadowanie rekordu dziennego z bazy danych
gboolean db_load_daily_record(sqlite3 *db, const char *username, int date, DailyRecord *record) {
    int user_id = db_get_user_id(db, username);
    if (user_id == -1)
        return FALSE;

    memset(record, 0, sizeof(DailyRecord));
    record->date = date;

    sqlite3_stmt *stmt;

    // Załaduj pomiary wagi
    const char *sql_measurements = "SELECT weight, bf, mm FROM daily_measurements WHERE user_id = ? AND log_date = ?;";
    if (sqlite3_prepare_v2(db, sql_measurements, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_int(stmt, 2, date);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            record->body_measure.weight = sqlite3_column_double(stmt, 0);
            record->body_measure.bodyfat_pct = sqlite3_column_double(stmt, 1);
            record->body_measure.musclemass_pct = sqlite3_column_double(stmt, 2);
        }
        sqlite3_finalize(stmt);
    }

    // Załaduj nawyki
    const char *sql_habits = "SELECT h.name, dh.is_completed FROM user_habits h "
                             "LEFT JOIN daily_habits dh ON h.id = dh.habit_id AND dh.log_date = ? "
                             "WHERE h.user_id = ? LIMIT 10;";
    if (sqlite3_prepare_v2(db, sql_habits, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, date);
        sqlite3_bind_int(stmt, 2, user_id);
        int i = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW && i < MAX_HABITS) {
            const unsigned char *h_name = sqlite3_column_text(stmt, 0);
            if (h_name) {
                strncpy(record->habits[i].name, (const char *)h_name, 31);
                record->habits[i].completed = sqlite3_column_int(stmt, 1);
                i++;
            }
        }
        record->habit_count = i;
        sqlite3_finalize(stmt);
    }

    return TRUE;
}

// Zapisanie rekordu dziennego do bazy danych
gboolean db_save_daily_record(sqlite3 *db, const char *username, const DailyRecord *record) {
    int user_id = db_get_user_id(db, username);
    if (user_id == -1)
        return FALSE;

    // Zapisz pomiary
    if (record->body_measure.weight > 0) {
        db_save_weight_measurement(db, username, record->date,
                                   record->body_measure.weight,
                                   record->body_measure.bodyfat_pct,
                                   record->body_measure.musclemass_pct);
    }

    // Zapisz nawyki
    for (int i = 0; i < record->habit_count; i++) {
        db_save_habit_completion(db, username, record->date,
                                 record->habits[i].name,
                                 record->habits[i].completed);
    }

    return TRUE;
}

// Pobierz listę ostatnich dni
gboolean db_get_recent_days(sqlite3 *db, const char *username, int limit, int *dates, int *count) {
    int user_id = db_get_user_id(db, username);
    if (user_id == -1)
        return FALSE;

    *count = 0;
    const char *sql = "SELECT DISTINCT log_date FROM daily_measurements WHERE user_id = ? "
                      "ORDER BY log_date DESC LIMIT ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_int(stmt, 2, limit);
        int i = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW && i < limit) {
            dates[i] = sqlite3_column_int(stmt, 0);
            i++;
        }
        *count = i;
        sqlite3_finalize(stmt);
    }

    return *count > 0;
}

// Zapisz status nawyka dla konkretnego dnia
gboolean db_save_habit_completion(sqlite3 *db, const char *username, int date, const char *habit_name, gboolean completed) {
    int user_id = db_get_user_id(db, username);
    if (user_id == -1)
        return FALSE;

    // Najpierw znajdź habit_id
    const char *sql_find = "SELECT id FROM user_habits WHERE user_id = ? AND name = ?;";
    sqlite3_stmt *stmt;
    int habit_id = -1;
    if (sqlite3_prepare_v2(db, sql_find, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_text(stmt, 2, habit_name, -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            habit_id = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (habit_id == -1)
        return FALSE;

    // Wstaw lub zaktualizuj
    const char *sql = "INSERT INTO daily_habits (user_id, habit_id, log_date, is_completed) "
                      "VALUES (?, ?, ?, ?) "
                      "ON CONFLICT(user_id, habit_id, log_date) DO UPDATE SET is_completed = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_int(stmt, 2, habit_id);
        sqlite3_bind_int(stmt, 3, date);
        sqlite3_bind_int(stmt, 4, completed ? 1 : 0);
        sqlite3_bind_int(stmt, 5, completed ? 1 : 0);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return (rc == SQLITE_DONE);
    }

    return FALSE;
}

// Zapisz pomiar wagi
gboolean db_save_weight_measurement(sqlite3 *db, const char *username, int date, double weight, double bf, double mm) {
    int user_id = db_get_user_id(db, username);
    if (user_id == -1)
        return FALSE;

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO daily_measurements (user_id, log_date, weight, bf, mm) "
                      "VALUES (?, ?, ?, ?, ?) "
                      "ON CONFLICT(user_id, log_date) DO UPDATE SET weight = ?, bf = ?, mm = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_int(stmt, 2, date);
        sqlite3_bind_double(stmt, 3, weight);
        sqlite3_bind_double(stmt, 4, bf);
        sqlite3_bind_double(stmt, 5, mm);
        sqlite3_bind_double(stmt, 6, weight);
        sqlite3_bind_double(stmt, 7, bf);
        sqlite3_bind_double(stmt, 8, mm);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return (rc == SQLITE_DONE);
    }

    return FALSE;
}
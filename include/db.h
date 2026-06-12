#include <data_managment.h>
#include <glib.h>
#include <sqlite3.h>
#include <storage.h>

typedef struct AppState AppState;

#define SQL_USER_INITIAL                       \
    "CREATE TABLE IF NOT EXISTS user_initial(" \
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"    \
    "user_name VARCHAR(32) UNIQUE,"            \
    "age INTEGER,"                             \
    "height INTEGER,"                          \
    "starting_weight REAL,"                    \
    "starting_bf REAL,"                        \
    "starting_mm REAL);"

#define SQL_USER_GOAL                                                            \
    "CREATE TABLE IF NOT EXISTS user_goal("                                      \
    "user_id INTEGER PRIMARY KEY REFERENCES user_initial(id) ON DELETE CASCADE," \
    "goal_weight REAL,"                                                          \
    "goal_bf REAL,"                                                              \
    "goal_mm REAL);"

#define SQL_USER_HABIT                                               \
    "CREATE TABLE IF NOT EXISTS user_habits("                        \
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"                          \
    "user_id INTEGER REFERENCES user_initial(id) ON DELETE CASCADE," \
    "name VARCHAR(32) NOT NULL);"

#define SQL_DAILY_HABITS                                             \
    "CREATE TABLE IF NOT EXISTS daily_habits("                       \
    "user_id INTEGER REFERENCES user_initial(id) ON DELETE CASCADE," \
    "habit_id INTEGER REFERENCES user_habits(id) ON DELETE CASCADE," \
    "log_date DATE NOT NULL,"                                        \
    "is_completed BOOLEAN DEFAULT FALSE,"                            \
    "PRIMARY KEY (user_id, habit_id, log_date));"

#define SQL_DAILY_MEASUREMENTS                                       \
    "CREATE TABLE IF NOT EXISTS daily_measurements("                 \
    "user_id INTEGER REFERENCES user_initial(id) ON DELETE CASCADE," \
    "log_date DATE NOT NULL,"                                        \
    "weight REAL,"                                                   \
    "bf REAL,"                                                       \
    "mm REAL,"                                                       \
    "PRIMARY KEY (user_id, log_date));"

#define SQL_WORKOUTS                                                 \
    "CREATE TABLE IF NOT EXISTS workouts("                           \
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"                          \
    "user_id INTEGER REFERENCES user_initial(id) ON DELETE CASCADE," \
    "log_date INTEGER NOT NULL,"                                     \
    "type INTEGER,"                                                  \
    "duration_sec INTEGER,"                                          \
    "burned_kcal INTEGER,"                                           \
    "distance REAL);"

gboolean db_create_table(AppState *state, const char *table);

int db_get_user_id(sqlite3 *db, const char *user_name);
char *db_get_user_name(sqlite3 *db, const char *user_name);

gboolean db_init(AppState *state);
gboolean db_execute_query(sqlite3 *db, const char *sql, const char *format, ...);

gboolean sync_user_settings_to_db(AppState *state);

gboolean db_load_user_settings(sqlite3 *db, const char *username, UserSettings *config);
gboolean db_get_dashboard_data(sqlite3 *db, const char *username, double *start_w, double *curr_w, double *curr_bf, double *curr_mm, double *vo2max, int *workouts_week);
gboolean db_is_day_started(sqlite3 *db, const char *username, int date);
gboolean db_mark_day_started(sqlite3 *db, const char *username, int date);
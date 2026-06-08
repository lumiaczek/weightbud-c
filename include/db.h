#include <glib.h>
#include <sqlite3.h>

// Forward declaration to avoid unknown type name errors in this header
typedef struct AppState AppState;

// 1. The Base User Table
#define SQL_USER_INITIAL                       \
    "CREATE TABLE IF NOT EXISTS user_initial(" \
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"    \
    "user_name VARCHAR(32) UNIQUE,"            \
    "age INTEGER,"                             \
    "height INTEGER,"                          \
    "starting_weight REAL,"                    \
    "starting_mm REAL);"

// 2. User Body Goals
#define SQL_USER_GOAL                                                            \
    "CREATE TABLE IF NOT EXISTS user_goal("                                      \
    "user_id INTEGER PRIMARY KEY REFERENCES user_initial(id) ON DELETE CASCADE," \
    "goal_weight REAL,"                                                          \
    "goal_bf REAL,"                                                              \
    "goal_mm REAL);"

// 3. Habit Tracker
#define SQL_USER_HABIT                                               \
    "CREATE TABLE IF NOT EXISTS user_habits("                        \
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"                          \
    "user_id INTEGER REFERENCES user_initial(id) ON DELETE CASCADE," \
    "name VARCHAR(32) NOT NULL);"

// 4. Daily Logs (The "Inner" Tables)
// CRITICAL: We need a 'log_date' so you can track habits day by day!
#define SQL_DAILY_HABITS                                             \
    "CREATE TABLE IF NOT EXISTS daily_habits("                       \
    "user_id INTEGER REFERENCES user_initial(id) ON DELETE CASCADE," \
    "habit_id INTEGER REFERENCES user_habits(id) ON DELETE CASCADE," \
    "log_date DATE NOT NULL,"                                        \
    "is_completed BOOLEAN DEFAULT FALSE,"                            \
    "PRIMARY KEY (user_id, habit_id, log_date));" // Composite key

// create table based on the query given
gboolean db_create_table(AppState *state, const char *table);

// check id to cross reference with foreign keys
int db_get_user_id(sqlite3 *db, const char *user_name);
char *db_get_user_name(sqlite3 *db, const char *user_name);

// initialize (call db_create, add to state, add PRAGMA, check for integrity)
gboolean db_init(AppState *state);

// execute any query
gboolean db_execute_query(sqlite3 *db, const char *sql, const char *format, ...);

// perform Upsert on form tables (user,goal)
gboolean sync_user_settings_to_db(AppState *state);

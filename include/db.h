


// 1. The Base User Table
#define SQL_USER_INITIAL \
    "CREATE TABLE IF NOT EXISTS user_initial(" \
    "id INTEGER PRIMARY KEY AUTOINCREMENT," \
    "user_name VARCHAR(32) UNIQUE," \
    "age INTEGER," \
    "height INTEGER," \
    "starting_weight REAL," \
    "starting_mm REAL);"

    
// 2. User Diet Goals (1-to-1 with user_initial)
#define SQL_USER_DIET \
    "CREATE TABLE IF NOT EXISTS user_diet(" \
    "user_id INTEGER PRIMARY KEY REFERENCES user_initial(id) ON DELETE CASCADE," \
    "goal_kcal INTEGER," \
    "goal_protein INTEGER," \
    "goal_fat INTEGER," \
    "goal_carbs INTEGER);"

// 3. User Body Goals (1-to-1 with user_initial)
#define SQL_USER_GOAL \
    "CREATE TABLE IF NOT EXISTS user_goal(" \
    "user_id INTEGER PRIMARY KEY REFERENCES user_initial(id) ON DELETE CASCADE," \
    "goal_weight REAL," \
    "goal_bg REAL," \
    "goal_mm REAL);"

// 4. Habit & Supplement DEFINITIONS
#define SQL_USER_HABIT \
    "CREATE TABLE IF NOT EXISTS user_habits(" \
    "id INTEGER PRIMARY KEY AUTOINCREMENT," \
    "user_id INTEGER REFERENCES user_initial(id) ON DELETE CASCADE," \
    "name VARCHAR(32) NOT NULL);"

#define SQL_USER_SUPPLEMENTS \
    "CREATE TABLE IF NOT EXISTS user_supplements(" \
    "id INTEGER PRIMARY KEY AUTOINCREMENT," \
    "user_id INTEGER REFERENCES user_initial(id) ON DELETE CASCADE," \
    "name VARCHAR(32) NOT NULL);" // Changed from supplement_names for consistency

// 5. Daily Logs (The "Inner" Tables)
// CRITICAL: We need a 'log_date' so you can track habits day by day!
#define SQL_DAILY_HABITS \ 
    "CREATE TABLE IF NOT EXISTS daily_habits(" \
    "user_id INTEGER REFERENCES user_initial(id) ON DELETE CASCADE," \
    "habit_id INTEGER REFERENCES user_habits(id) ON DELETE CASCADE," \
    "log_date DATE NOT NULL," \
    "is_completed BOOLEAN DEFAULT FALSE," \
    "PRIMARY KEY (user_id, habit_id, log_date));" // Composite key

#define SQL_DAILY_SUPPLEMETS \
    "CREATE TABLE IF NOT EXISTS daily_supplements(" \
    "user_id INTEGER REFERENCES user_initial(id) ON DELETE CASCADE," \
    "supplement_id INTEGER REFERENCES user_supplements(id) ON DELETE CASCADE," \
    "log_date DATE NOT NULL," \
    "amount_taken REAL," \
    "PRIMARY KEY (user_id, supplement_id, log_date));"


gboolean db_create_table(AppState *state, const char *table);
gboolean db_init(AppState *state);
gboolean db_execute_query(sqlite3 *db, const char *sql,const char *format, ... );
gboolean sync_user_settings_to_db(AppState *state);

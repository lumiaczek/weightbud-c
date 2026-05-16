#include "data_managment.h"
#include "storage.h"
#include <gtk/gtk.h>
#include <sqlite3.h>
#include "db.h"
#include <stdarg.h>

static int print_tables_callback(void *NotUsed, int argc, char **argv, char **azColName) {
    for(int i = 0; i < argc; i++) {
        // argv[i] contains the table name
        printf("- %s\n", argv[i] ? argv[i] : "NULL");
    }
    return 0; // Return 0 to tell SQLite to continue processing rows
}

gboolean db_create_table(AppState *state, const char *table){
    char *err_msg = 0;
    int rc = sqlite3_exec(state->db,table,0,0,&err_msg);

    if(rc != SQLITE_OK){
        fprintf(stderr,"[DB] SQL error during table creation %s\n",err_msg);
        err_msg = NULL;
        sqlite3_free(err_msg);
        return FALSE;
    }
    else{
        printf(" \n table created");
        const char *show_tables_sql = "SELECT name FROM sqlite_master WHERE type='table';";
        int tables_rc = sqlite3_exec(state->db,show_tables_sql,print_tables_callback,0,&err_msg);
        if(tables_rc != SQLITE_OK){
            fprintf(stderr, "Failed to show tables: %s\n", err_msg);
            err_msg = NULL;
            sqlite3_free(err_msg);
        }
    }
}

int db_get_user_id(sqlite3 *db, const char *user_name){
    const char *sql = "SELECT id FROM user_initial WHERE user_name = ?;";
    sqlite3_stmt *stmt;
    int id = -1;

    if(sqlite3_prepare_v2(db,sql,-1,&stmt,NULL) == SQLITE_OK){
        sqlite3_bind_text(stmt, 1, user_name, -1, SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) == SQLITE_ROW){
            id = sqlite3_column_int(stmt,0);

        }
        sqlite3_finalize(stmt);
    
    }else{
        g_printerr("Failed to prepare select for user ID");
    }
    return id;



}

gboolean db_init(AppState *state){



    int rc = sqlite3_open("weightbud.db",&state->db);
    char *err_msg = 0;
    int pragma = sqlite3_exec(state->db,"PRAGMA foreign_keys = ON",0,0,&err_msg);
    
 
    
    if (pragma != SQLITE_OK){
        printf("[DB]: Fail on calling pragma %s\n",err_msg);
        err_msg = NULL;
        sqlite3_free(err_msg);
        return FALSE;
    }
    if(rc !=SQLITE_OK){
        printf("[DB] Błąd przy otwarciu bazy: %s\n", sqlite3_errmsg(state->db));
        sqlite3_close(state->db);
        return FALSE;
    }
    printf("[DB] Succsefully intiialized database");
    db_create_table(state,SQL_USER_INITIAL);
    db_create_table(state,SQL_USER_DIET);
    db_create_table(state,SQL_USER_GOAL);
    db_create_table(state,SQL_USER_HABIT);
    db_create_table(state,SQL_USER_SUPPLEMENTS);
    db_create_table(state,SQL_DAILY_HABITS);
    db_create_table(state,SQL_DAILY_SUPPLEMETS);


    return TRUE;
}


gboolean db_execute_query(sqlite3 *db, const char *sql,const char *format, ... ){

    sqlite3_stmt *stmt;

    if(sqlite3_prepare_v2(db,sql, -1,&stmt,NULL) != SQLITE_OK){
        g_printerr("[DB] Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return FALSE;
    }
    va_list args;
    va_start(args,format);

    for(int i =0; format[i] != '\0'; i++){
        int bind_idx = i + 1;

        switch (format[i]) {
            case 'i': // Integer
                sqlite3_bind_int(stmt, bind_idx, va_arg(args, int));
                break;
            case 'd': // Double
                sqlite3_bind_double(stmt, bind_idx, va_arg(args, double));
                break;
            case 's': // String (Text)
                // Using SQLITE_TRANSIENT is safer for strings in case they are freed immediately after
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

 if(rc != SQLITE_DONE){
    g_printerr("[DB] Execution failed: %s\n", sqlite3_errmsg(db));
        return FALSE;
 }
 return TRUE;
}
gboolean sync_user_settings_to_db(AppState *state) {
    UserSettings *user = (UserSettings *)g_hash_table_lookup(state->memory_collection, "user_settings");
    Diet *diet = (Diet *)g_hash_table_lookup(state->memory_collection, "user_diet");

    if (!user || !diet) return FALSE;

    // 1. Sync Base User
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
        return FALSE; // Fail early if the main user insert fails
    }
    int current_user_id = db_get_user_id(state->db,user->user_name);
    if(current_user_id == -1){
        g_printerr("Could not find user ID for %s to link foreign key \n",user->user_name);
        return FALSE;
    }

    // 2. Sync Diet
   const char *sql_diet = 
        "INSERT INTO user_diet (user_id, goal_kcal, goal_protein, goal_fat, goal_carbs) "
        "VALUES (?, ?, ?, ?, ?) "
        "ON CONFLICT(user_id) DO UPDATE SET "
        "goal_kcal = excluded.goal_kcal, "
        "goal_protein = excluded.goal_protein, "
        "goal_fat = excluded.goal_fat, "
        "goal_carbs = excluded.goal_carbs;";

    // Assuming you have the user_id (you might need to fetch it after step 1 if it's a new user)
    
    
    if (!db_execute_query(state->db, sql_diet, "iiiii", 
                          current_user_id, diet->goal_kcal, diet->goal_protein, 
                          diet->goal_fat, diet->goal_carbs)) {
        return FALSE;
    }

    // 3. Sync Goals
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
#include <data_managment.h>
#include <storage.h>
#include <gtk/gtk.h>
#include <sqlite3.h>
#include <db.h>


void ram_store_save(AppState *state,const char *key,gpointer data){
    g_hash_table_insert(state->memory_collection,g_strdup(key),data);
}

gboolean db_create_table(AppState *state,const char *table){
    char *err_msg = 0;
   int rc = sqlite3_exec(state->db,table,0,0,&err_msg);    

    if (rc != SQLITE_OK){
        fprintf(stderr, "[DB] SQL error during table creation %s\n",err_msg);
        sqlite3_free(err_msg);
        return FALSE;
    }else{
       
        return TRUE;
    }
}

gboolean db_init(AppState *state){
    
    int rc = sqlite3_open("weightbud.db",&state->db);
    if(rc != SQLITE_OK){
        printf("[DB] Błąd przy otwarciu bazy: %s\n", sqlite3_errmsg(state->db));
        sqlite3_close(state->db);
        return FALSE;
    }
    
    db_create_table(state,sql_user_initial);
    db_create_table(state,sql_user_goal);
    db_create_table(state,sql_user_habits);
    db_create_table(state,sql_user_supplements);


    
    

   
    return TRUE;
} 
void init_memory(AppState *state){
    state->memory_collection = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
    db_init(state);
    if(!db_init(state)){
        g_printerr("CRITICAL: Failed to initialize database.\n");
    }
    
}

void on_save_button_clicked(GtkWidget *widget,gpointer user_data){
    UserSettings *user = g_new(UserSettings,1);
    AppState *state = (AppState *)user_data;
    g_strlcpy(user->user_name, "Admin", sizeof(user->user_name));
    user->age = 25;
    ram_store_save(state,"user_settings",user);
    


}
gboolean sync_ram_to_db(AppState *state){
UserSettings *user = (UserSettings *)g_hash_table_lookup(state->memory_collection,"user_settings");

if(user == NULL){
    g_print("[DB] No user settings in RAM to save \n");
    return FALSE;
}

const char *sql = 
    "INSERT INTO user_initial (user_name,age,height,starting_weight,starting_mm)"
    "VALUES (?,?,?,?,?)"
    "ON CONFLICT(user_name) DO UPDATE SET "
    "age = excluded.age, height=excluded.height, starting_weight = excluded.starting_weight, starting_mm = excluded.starting_mm;";

    sqlite3_stmt  *stmt;

    int rc = sqlite3_prepare_v2(state->db,sql,-1,&stmt,NULL);
    if(rc != SQLITE_OK){
        g_printerr("[DB] Failed to prepare statment: %s\n", sqlite3_errmsg(state->db));
        return FALSE;
    }
    sqlite3_bind_text(stmt,1,user->user_name, -1,SQLITE_STATIC);
    sqlite3_bind_int(stmt,2,user->age);
    sqlite3_bind_int(stmt,3,user->height);
    sqlite3_bind_double(stmt,3,(double)user->starting_weight);
    sqlite3_bind_double(stmt,5,(double)user->starting_mm);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE){
        g_printerr("[DB] Execution failed %s\n", sqlite3_errmsg(state->db));
        return FALSE;
    }

    sqlite3_finalize(stmt);
   
    sqlite3_close(state->db);
    return TRUE;

}

void initialize_data(){
    AppState app_state;
    init_memory(&app_state);
    on_save_button_clicked(NULL,&app_state);

    gpointer raw_data = g_hash_table_lookup(app_state.memory_collection,"user_settings");
    if (raw_data != NULL){
        UserSettings *saved_user = (UserSettings *)raw_data;
        printf("Retrieved Age: %d\n", saved_user->age);
    }
    sync_ram_to_db(&app_state);

    g_hash_table_destroy(app_state.memory_collection);
    printf("Memory cleared. \n");
    
}

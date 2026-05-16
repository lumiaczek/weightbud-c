#include <data_managment.h>
#include <gtk/gtk.h>
#include <storage.h>
#include "db.h"

void init_memory(AppState *state) {
    state->memory_collection = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,destroy_user_settings);
    
    if(!db_init(state)){
        g_printerr("CRITICAL: Failed to initialize database.\n");
    }
}
void ram_store_save(const char *key, gpointer data,AppState *state) {
    g_hash_table_insert(state->memory_collection, g_strdup(key), data);
    
}

gpointer ram_store_get(const char *key,AppState *state) {
    if (state->memory_collection == NULL)
        return NULL;

    return g_hash_table_lookup(state->memory_collection, key);
}
void destroy_user_settings(gpointer data){
    UserSettings *user = (UserSettings *)data;
    if(user){
        g_free(user->user_name);
        g_free(user);
    }
}
void cleanup_memory(AppState *state) {
    sync_user_settings_to_db(state);

    if (state->memory_collection != NULL) {
        g_hash_table_destroy(state->memory_collection);
       state->memory_collection = NULL;
        printf("Pamięć RAM Store wyczyszczona.\n");
    }
}

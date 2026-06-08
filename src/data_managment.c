#include "db.h"
#include <data_managment.h>
#include <gtk/gtk.h>
#include <storage.h>

void init_memory(AppState *state) {

    state->memory_collection = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);

    if (!db_init(state)) {
        g_printerr("CRITICAL: Failed to initialize database.\n");
    }
}

void ram_store_save(const char *key, gpointer data, AppState *state) {
    g_hash_table_insert(state->memory_collection, g_strdup(key), data);
}

void free_user_data(AppState *state) {
    if (state->memory_collection != NULL) {
        UserSettings *user = g_hash_table_lookup(state->memory_collection, "user_settings");
        if (user) {
            g_free(user);
        }
    }
}

void cleanup_memory(AppState *state) {
    sync_user_settings_to_db(state);
    free_user_data(state);

    g_hash_table_destroy(state->memory_collection);
    state->memory_collection = NULL;

    printf("Pamięć RAM Store wyczyszczona.\n");
}

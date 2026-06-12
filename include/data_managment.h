
#include <gtk/gtk.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <storage.h>
typedef struct AppState {
    GHashTable *memory_collection;
    sqlite3 *db;
} AppState;
void init_memory(AppState *state);

void ram_store_save(const char *key, gpointer data, AppState *state);

void free_user_data(AppState *state);

void cleanup_memory(AppState *state);

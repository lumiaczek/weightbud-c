
#include <gtk/gtk.h>
#include <stdbool.h>
#include <storage.h>
#include <sqlite3.h>
// definicja struktury stanu
 typedef struct AppState {
    GHashTable *memory_collection;
     sqlite3 *db;
}AppState;
// zapis danych do store
void ram_store_save(const char *key, gpointer data, AppState *state);
void init_memory(AppState *state);
gpointer ram_store_get(const char *key, AppState *state);
void cleanup_memory(AppState *state);

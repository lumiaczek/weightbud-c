
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
void init_memory(AppState *state);


void ram_store_save(const char *key, gpointer data, AppState *state);

//called on cleanup

void free_user_data(AppState *state);

// syncs to db and cleans data based on memory collection
void cleanup_memory(AppState *state);


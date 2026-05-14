
#include <gtk/gtk.h>
#include <stdbool.h>
#include <storage.h>
// definicja struktury stanu
typedef struct {
    GHashTable *daily_habits;
    // sqlite3 *db;
} AppState;
// zapis danych do store
void ram_store_save(const char *key, gpointer data);
void init_memory(void);
gpointer ram_store_get(const char *key);
void cleanup_memory(void);

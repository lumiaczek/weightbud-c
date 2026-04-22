
#include <stdbool.h>
#include <gtk/gtk.h>
#include <storage.h>
//definicja struktury stanu
typedef struct {
    GHashTable *daily_habits;
   // sqlite3 *db;
} AppState;
// zapis danych do store
void ram_store_save(const char *key,gpointer data);



void initialize_data();
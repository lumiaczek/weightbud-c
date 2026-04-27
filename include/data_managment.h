
#include <stdbool.h>
#include <gtk/gtk.h>
#include <storage.h>
 #include <sqlite3.h>
//definicja struktury stanu
typedef struct {
    GHashTable *memory_collection;
    sqlite3 *db;
} AppState;
// zapis danych do store
void ram_store_save(AppState *state,const char *key,gpointer data);
void init_memory(AppState *state);
void  on_save_button_clicked(GtkWidget *widget,gpointer user_data);
void initialize_data();

gboolean db_init(AppState *state);
gboolean db_create_table(AppState *state,const char *table);
gboolean sync_ram_to_db(AppState *state);

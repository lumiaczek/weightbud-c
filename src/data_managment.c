#include <data_managment.h>
#include <gtk/gtk.h>
#include <storage.h>

GHashTable *memory_collection = NULL;
void init_memory() {
    memory_collection = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
}
void ram_store_save(const char *key, gpointer data) {
    g_hash_table_insert(memory_collection, g_strdup(key), data);
}

gpointer ram_store_get(const char *key) {
    if (memory_collection == NULL)
        return NULL;

    return g_hash_table_lookup(memory_collection, key);
}

void cleanup_memory() {
    if (memory_collection != NULL) {
        g_hash_table_destroy(memory_collection);
        memory_collection = NULL;
        printf("Pamięć RAM Store wyczyszczona.\n");
    }
}

#include <data_managment.h>
#include <storage.h>
#include <gtk/gtk.h>

GHashTable *memory_collection = NULL;
void init_memory(){
    memory_collection = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
    
}
void ram_store_save(const char *key,gpointer data){
    g_hash_table_insert(memory_collection,g_strdup(key),data);
}

void on_save_button_clicked(){
    UserSettings *user = g_new(UserSettings,1);
    // tu trafia funkcja z UI zajmujaca sie ustawieniem jakies wartosci z formularza czy czegos.
    user->age =  25;


    
    ram_store_save("user_settings", user);
}

void initialize_data(){
    init_memory();
    on_save_button_clicked();
    // pobranie wartości z pointera do pamięci.
    gpointer raw_data = g_hash_table_lookup(memory_collection,"user_settings");
    UserSettings *new_user = (UserSettings *)raw_data;
    printf("Age: %d\n",new_user->age);
    g_hash_table_destroy(memory_collection);
    printf("Memory cleard");
}

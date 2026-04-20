
#include <stdbool.h>
#include <gtk/gtk.h>
#include <storage.h>
//definicja struktury stanu
typedef struct {
    GHashTable *daily_habits;
   // sqlite3 *db;
} AppState;


// definicja Unii w celu obsłużenia różnych typów danych
typedef enum {
    VAL_INT,
    VAL_FLOAT,
    VAL_STRING,
    VAL_BOOL,
    VAL_CUSTOM,
}ValueType;

typedef struct {
    ValueType type;
    union{
        int i_val;
        float f_val;
        char *s_val;
        bool b_val;
        gpointer p_val;
    } data;
} DynamicValue;

void workout_session_update_value(AppState *state, const char *key, DynamicValue);

void on_save_button_clicked();

void initialize_data();
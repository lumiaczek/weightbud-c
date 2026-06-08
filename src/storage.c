/**
 * @file storage.c
 * @author Daniel Borowski (danielborowski05@gmail.com)
 * @brief Plik zawiera funkcje inicjalizujące postawowe struktury danych
 * @version 0.1
 * @date 2026-04-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "storage.h"
#include <string.h>

/**
 * @brief Funckja inicjalizująca ustawienia użytkownika
 *
 */
void setup_user_settings(UserSettings *config, const char *_username,
                         int _age,
                         float _starting_weight,
                         int _height,
                         float _starting_bf,
                         float _starting_mm,
                         float _goal_weight,
                         float _goal_bf,
                         float _goal_mm,
                         char _habit_names[MAX_HABITS][32],
                         int _habit_count)

{
    memset(config, 0, sizeof(UserSettings));

    if (_username != NULL) {
        strncpy(config->user_name, _username, 31);
    }

    config->age = _age;
    config->starting_weight = _starting_weight;
    config->height = _height;
    config->starting_bf = _starting_bf;
    config->starting_mm = _starting_mm;
    config->goal_weight = _goal_weight;
    config->goal_bf = _goal_bf;
    config->goal_mm = _goal_mm;

    for (int i = 0; i < _habit_count; i++) {
        strncpy(config->habit_names[i], _habit_names[i], 31);
    }

    config->habit_count = _habit_count;
}

/**
 * @brief Inicjalizuje nowy dzień na podstawie ustawień użytkownika.
 * @param record Wskaźnik na pustą strukturę dnia do wypełnienia
 * @param target_date Data w formacie YYYYMMDD
 * @param config Globalna konfiguracja użytkownika
 */

void init_daily_record(DailyRecord *record, int target_date, const UserSettings *config) {
    memset(record, 0, sizeof(DailyRecord));

    record->date = target_date;

    record->habit_count = config->habit_count;
    for (int i = 0; i < record->habit_count; i++) {
        strncpy(record->habits[i].name, config->habit_names[i], 31);
        record->habits[i].completed = false;
    }
}
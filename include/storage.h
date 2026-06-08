/**
 * @file storage.h
 * @author Daniel Borowski (daniel.borowski@student.put.poznan.pl)
 * @brief Deklaracje głównych struktur aplikacji Weightbud
 * @version 0.1
 * @date 2026-04-14
 * Plik zawiera deklaracje wszystkich struktur potrzebnych do przechowywania danych:
 * Habit Tracker, Workout Tracker oraz konfigurację użytkownika.
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef STORAGE_H
#define STORAGE_H

#include <stdbool.h>

#define MAX_HABITS 10
#define MAX_WORKOUTS_PER_DAY 3
#define MAX_MEALS 10

typedef enum {
    WORKOUT_NONE = 0,
    WORKOUT_BIKE,
    WORKOUT_RUN,
    WORKOUT_SWIM,
    WORKOUT_STRENGTH
} WorkoutType;

typedef struct {
    WorkoutType type;
    int duration_sec;
    int avg_heart_rate;
    int burned_kcal;
    float distance;
    int pace_sec_per_km;
} Workout;

typedef struct {
    char name[32];
    bool completed;
} Habit;

typedef struct {
    float weight;
    float bodyfat_pct;
    float musclemass_pct;
    float waist_cm;
    float arm_cm;
} BodyMeasurement;

typedef struct {
    char name[32];
    int kcal;
    int protein;
    int fat;
    int carbs;
} Meal;

typedef struct {
    int date;
    int meal_count; // tutaj lepiej
    int workout_count;
    int habit_count;
    Workout workouts[MAX_WORKOUTS_PER_DAY];
    Meal meals[MAX_MEALS];
    Habit habits[MAX_HABITS];

    BodyMeasurement body_measure;
} DailyRecord;

typedef struct {
    char user_name[32];
    int age;

    float starting_weight;
    int height;

    float starting_bf;
    float starting_mm;

    float goal_weight;
    float goal_bf;
    float goal_mm;

    char habit_names[MAX_HABITS][32];
    int habit_count;
} UserSettings;

void init_daily_record(DailyRecord *record, int target_date, const UserSettings *config);

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
                         int _habit_count);

#endif
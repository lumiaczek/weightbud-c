/**
 * @file storage.h
 * @author Daniel Borowski (daniel.borowski@student.put.poznan.pl)
 * @brief Deklaracje głównych struktur aplikacji Weightbud
 * @version 0.1
 * @date 2026-04-14
 * Plik zawiera deklaracje wszystkich struktur potrzebnych do przechowywania danych:
 * Habit Tracker, Workout Tracker, Diet Tracker oraz konfigurację użytkownika.
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef STORAGE_H
#define STORAGE_H

#include <stdbool.h>

#define MAX_HABITS 10
#define MAX_SUPPLEMENTS 10
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
    bool taken_morning;
    bool taken_evening;
} Supplement;

typedef struct {
    char name[32];
    int kcal;
    int protein;
    int fat;
    int carbs;
} Meal;

typedef struct {
    Meal meals[MAX_MEALS];
    int meals_count;
    int goal_kcal;
    int goal_protein;
    int goal_fat;
    int goal_carbs;
} Diet;

typedef struct {
    int date;

    Workout workouts[MAX_WORKOUTS_PER_DAY];
    int workout_count;

    Habit habits[MAX_HABITS];
    int Habit_count;

    BodyMeasurement body_measure;

    Supplement supplementation[MAX_SUPPLEMENTS];
    int supplement_count;

    Diet diet;

} DailyRecord;

typedef struct {
    char user_name[32];
    int age;

    float starting_weight;
    int height;

    float starting_bf;
    float starting_mm;

    float goal_weigth;
    float goal_bf;
    float goal_mm;

    char Habit_names[MAX_HABITS][32];
    int habit_count;

    char supplement_names[MAX_SUPPLEMENTS][32];
    int supplement_count;

    int goal_kcal;
    int goal_protein;
    int goal_fat;
    int goal_carbs;

} UserSettings;

#endif
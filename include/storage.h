#ifndef STORAGE_H
#define STORAGE_H

#include <stdbool.h>

#define MAX_HABITS 10
#define MAX_SUPPLEMENTS 10
#define MAX_WORKOUTS_PER_DAY 3
#define MAX_MEALS 10

/*

    DEFINICJA STRUKTUR APLIKACJI WEIGHTBUD
    ======================================

    PODSTAWOWA STRUKTURA DANYCH
    --------------------------------------

    DailyRecord - struktura przechowuje informacje z wszystkich trackerów, inicjalizowany przy pomocy struktury UserSettings

    STRUKTURY DANYCH MODUŁÓW
    --------------------------------------

    # MODUŁ WORKOUT TRACKER

    Workout - struktura zawiera dane dotyczące treningu (śr. tętno, spalone kcal, czas treningu, dystans, tempo)
    WorkoutType - enumeracja rodzaju treningów  (Triathlon i Siła)

    # MODUŁ HABIT TRACKER

    Habit - struktura przechowująca nawyk

    # MODUŁ WEIGHT TRACKER

    BodyMeasurement - struktura przechowuje pomiary ciała (Waga, BF%, MM%, Talia(CM), Ramie(CM))

    # MODUŁ SUPPLEMENT TRACKER

    Supplement - struktura przechowuje informacje o suplementacji i porze przyjęcia

    # MODUŁ DIET TRACKER

    Meal - struktura pomocniczna przechowująca informacje o ostatnim posiłku (Kcal, protein, fat, carbs)
    Diet - struktura przechowuje informacje o obecnej diecie i spożytych posiłkach

*/

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
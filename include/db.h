// Plik z pomocnymi zmiennymi do bazy danych zeby nie bylo za duzo burdelu w plikach c
//postanowilem podzielic UserSettings na kilka tabel zeby latwiej bylo pisac QUERY i szybciej pszeszukiwano tabele.
const char *sql_user_initial = //informacje podawane przy "logowaniu" 
    "CREATE TABLE IF NOT EXISTS user_initial("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "user_name VARCHAR(32) UNIQUE,"
    "age INT ,"
    "height INT,"
    "starting_weight DOUBLE,"
    "starting_mm DOUBLE);";
    

const char *sql_user_goal = 
    "CREATE TABLE IF NOT EXISTS user_goal("
    "goal_weight DOUBLE,"
    "goal_bg DOUBLE,"
    "goal_mm DOUBLE,"
    "goal_kcal INT,"
    "goal_protein INT,"
    "goal_fat INT,"
    "goal_carbs INT);";

const char *sql_user_habits = 
    "CREATE TABLE IF NOT EXISTS user_habits("
    "habit_names VARCHAR(32),"
    "habit_count INT);";
const char *sql_user_supplements = 
"CREATE TABLE IF NOT EXISTS user_supplements("
"supplement_names VARCHAR(32)," 
"supplement_count INT);";
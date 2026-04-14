# 🏋️‍♂️ Aplikacja Weightbud in C

Aplikacja ta ma za zadanie wspierać użytkownika w jego postępach sportowych i zdrowotnych. Udostępnia kilka modułów służących do systematycznego gromadzenia danych, a następnie ich przejrzystej interpretacji.

## 🧩 Moduły

### 🏃‍♂️ Workout Tracker
Zapisywanie danych o treningach. Obecnie obsługiwane typy aktywności to **triathlon** (pływanie, rower, bieg) oraz **trening siłowy**.

### 📅 Habit Tracker
Śledzenie i odhaczanie codziennych nawyków zdefiniowanych przez użytkownika (maksymalnie **10** nawyków).

### ⚖️ Body Tracker
Śledzenie pomiarów ciała (waga, obwody, BF%, MM%) i wizualizacja postępów w dążeniu do zamierzonego celu sylwetkowego.

### 💊 Supplement Tracker
Monitorowanie dziennego przyjmowania suplementów (maksymalnie **10** preparatów) z podziałem na odpowiednie pory dnia (np. rano/wieczór).

### 🥗 Diet Tracker
Śledzenie spożywanych posiłków. Moduł na bieżąco oblicza przyjęte **kilokalorie oraz makroskładniki** (białko, tłuszcze, węglowodany) i porównuje je z wybranym celem dziennym.

### 🛠️ Tools
Zestaw wbudowanych narzędzi ułatwiających planowanie treningów i analizę postępów (m.in. kalkulator **1RM** - ciężaru maksymalnego, kalkulator **stref tętna** itp.).

---

## 💻 Specyfikacja programistyczna

* **Język:** C
* **Kompilator:** GCC (z wykorzystaniem systemu budowania **CMake**)
* **Interfejs Graficzny (GUI):** GTK3
* **Audyt i zarządzanie pamięcią:** Valgrind

## Autorstwa

Daniel Borowski, Marek Sobków
Studenci Informatyki (Stac) w semestrze II na przedmiot Programowanie niskopoziomowe (PUT)
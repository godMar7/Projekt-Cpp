# FitPlanner

FitPlanner to nowoczesna, w pełni responsywna aplikacja okienkowa (desktopowa) napisana w języku C++ do kompleksowego zarządzania planem treningowym oraz dietetycznym. Projekt wykorzystuje bibliotekę **Dear ImGui** do renderowania interfejsu w czasie rzeczywistym oraz parser **JSON** do trwałego zapisywania postępów użytkownika.

## Główne funkcjonalności

### Moduł Profilu Użytkownika
* Obliczanie wskaźnika **BMI** (Body Mass Index) oraz przyporządkowanie do odpowiedniej kategorii wagowej.
* Wyliczanie zapotrzebowania kalorycznego **BMR** (Basal Metabolic Rate) z uwzględnieniem płci, wieku, wagi, wzrostu i poziomu aktywności fizycznej.
* Przełącznik motywów (Jasny/Ciemny) działający w czasie rzeczywistym.

### Moduł Treningowy
* Podział treningów na poszczególne **dni tygodnia** (od poniedziałku do niedzieli).
* Możliwość przeglądania i przeszukiwania wbudowanej bazy ćwiczeń (wyszukiwarka *case-insensitive* oraz filtrowanie po partiach mięśniowych).
* **Interaktywne podpowiedzi (Tooltips)** wyświetlające porady techniczne po najechaniu kursorem na ćwiczenie.
* Kreator własnych ćwiczeń – dodawanie niestandardowych ruchów do globalnej bazy.

### Moduł Dietetyczny
* Planowanie jadłospisu z podziałem na 5 posiłków (Śniadanie, Drugie Śniadanie, Obiad, Przekąska, Kolacja).
* Dynamiczne podliczanie **kalorii oraz makroskładników** (Białko, Węglowodany, Tłuszcze) w oparciu o wybraną gramaturę w czasie rzeczywistym.
* Pasek postępu (Progress Bar) wizualizujący stopień pokrycia dziennego zapotrzebowania kalorycznego (zmienia kolor na czerwony po przekroczeniu limitu).
* Sortowanie kolumn (rosnąco/malejąco) tabeli produktów spożywczych.
* Kreator własnych produktów dodający je do bazy na 100g.

### Zapis i Eksport
* **Serializacja JSON:** Automatyczny odczyt i zapis profili użytkownika do plików JSON.
* **Eksport TXT:** Możliwość wygenerowania czystego, czytelnego planu tekstowego (`moj_plan.txt`) gotowego do wydrukowania i zabrania na siłownię.

---

## Technologie i Architektura

* **Język:** C++17
* **GUI:** [Dear ImGui](https://github.com/ocornut/imgui) (Immediate Mode GUI)
* **Grafika i okna:** OpenGL3 / GLFW
* **Dane:** [nlohmann/json](https://github.com/nlohmann/json) (Header-only JSON parser)
* **System budowania:** CMake


---

## Struktura katalogów

```text
PROJEKT_CPP/
├── CMakeLists.txt        # Główny plik konfiguracyjny
├── main.cpp              # Pętla renderująca ImGui i logika okna
├── Struktury.h           # Definicje klas i struktur danych
├── ZapisOdczyt.h         # Deklaracje funkcji operujących na plikach
├── ZapisOdczyt.cpp       # Logika parsowania JSON i eksportu TXT
└── data/                 # Bazy danych (generowane automatycznie)
    ├── cwiczenia.json
    ├── produkty.json
    └── profil.json

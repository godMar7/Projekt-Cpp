#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

//klasy
struct Cwiczenie {
    string nazwa;
    string kategoria;
    int trudnosc; 
};

struct ProduktSpozywczy {
    string nazwa;
    float kcal;
    float bialko;
    float weglowodany;
    float tluszcze;
};

// plan treningowy
//klasa pobiera cwiczenie z bazy i bedzie pobierac ustawienia uzytkownika
struct CwiczenieWPlanie {
    Cwiczenie baza;
    int serie;            
    int powtorzenia;      
    float ciezar;         
};

//caly trening z dnia np nazwaPlanu: wtorek - nogi, a w wektorze trzymamy zapisane cwiczenia
struct PlanTreningowy {
    string nazwaPlanu;                  
    vector<CwiczenieWPlanie> cwiczenia; 
};

// dieta
// pobiera wage produktu od uzytkownika i produkt z bazy - pozniej liczenie makro tu
struct SkladnikPosilku {
    ProduktSpozywczy baza; 
    float wagaGramy;       
    // liczenie makro chyba tu np: obliczKcal, obliczBialko, obliczWegle, obliczTluszcze
};

// analogicznie do PlanTreningowy: nazwaPosilku - np obiad i trzyma skladniki w wektorze
struct Posilek {
    string nazwaPosilku; 
    vector<SkladnikPosilku> skladniki;
};

//caly dzien jedzeniowy, trzyma posilki i makro z calego dnia
struct PlanDietetyczny {
    vector<Posilek> posilki;
    float sumaKcal = 0.0f;
    float sumaBialko = 0.0f;
    float sumaWegle = 0.0f;
    float sumaTluszcze = 0.0f;
};

//dane fizyczne osoby korzystajacej z aplikacji
struct ProfilUzytkownika {
    float waga;
    float wzrost;
    int wiek;
    int plec;
    int aktywnosc;

    float wyliczoneBmi;
    float zapotrzebowanieKcal;
    
    //polaczenie z planem treningowym i dieta
    PlanTreningowy aktualnyTrening;
    PlanDietetyczny aktualnaDieta;
};

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "FitPlanner - System Treningowy", NULL, NULL);
    if (window == NULL) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // wczytywanie bazy cwiczen
    vector<Cwiczenie> bazaCwiczen;
    ifstream plikJSON("data/cwiczenia.json");
    if (!plikJSON.is_open()) plikJSON.open("../data/cwiczenia.json");

    if (plikJSON.is_open()) {
        json dane = json::parse(plikJSON);
        for (const auto& item : dane) {
            bazaCwiczen.push_back({
                item["nazwa"], 
                item["kategoria"], 
                item["trudnosc"]
            });
        }
        cout << "[OK] Wczytano " << bazaCwiczen.size() << " cwiczen z bazy JSON." << endl;
    } else {
        cerr << "[ERROR] Nie udalo sie otworzyc pliku data/cwiczenia.json!" << endl;
    }

    // wczytywanie bazy produktow spozywczych
    vector<ProduktSpozywczy> bazaProduktow;
    ifstream plikJSON_Dieta("data/produkty.json");
    if (!plikJSON_Dieta.is_open()) plikJSON_Dieta.open("../data/produkty.json");

    if (plikJSON_Dieta.is_open()) {
        json daneDieta = json::parse(plikJSON_Dieta);
        for (const auto& item : daneDieta) {
            bazaProduktow.push_back({
                item["nazwa"], 
                item["kcal"], 
                item["bialko"],
                item["weglowodany"],
                item["tluszcze"]
            });
        }
        cout << "[OK] Wczytano " << bazaProduktow.size() << " produktow z bazy JSON." << endl;
    } else {
        cerr << "[ERROR] Nie udalo sie otworzyc pliku data/produkty.json!" << endl;
    }

    // zmienne do bmi
    float waga = 70.0f, wzrost = 175.0f, wyliczoneBmi = 0.0f, wyliczoneBmr = 0.0f, zapotrzebowanie = 0.0f;
    int wiek = 25, plec = 0, aktywnosc = 1;
    string kategoriaBmi = "";

    // Glowna petla programu
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | 
                                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                                        ImGuiWindowFlags_NoBringToFrontOnFocus;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGui::Begin("FitPlanner Workspace", nullptr, window_flags);
        ImGui::PopStyleVar(2);

        if (ImGui::BeginTabBar("GlowneZakladki")) {
            
            // ZAKLADKA 1: PROFIL
            if (ImGui::BeginTabItem("Profil i BMI")) {
                ImGui::Text("Uzupelnij dane, aby wyliczyc BMI oraz zapotrzebowanie kaloryczne.");
                ImGui::Separator(); ImGui::Spacing();
                ImGui::Columns(2, "kolumny_profil", false); 

                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Wymiary:");
                ImGui::InputFloat("Waga (kg)", &waga, 1.0f, 5.0f, "%.1f");
                ImGui::InputFloat("Wzrost (cm)", &wzrost, 1.0f, 5.0f, "%.1f");
                ImGui::InputInt("Wiek (lata)", &wiek);
                ImGui::NextColumn();

                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Plec:");
                ImGui::RadioButton("Mezczyzna", &plec, 0); ImGui::SameLine();
                ImGui::RadioButton("Kobieta", &plec, 1);
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Aktywnosc fizyczna:");
                ImGui::RadioButton("Niska", &aktywnosc, 0); ImGui::SameLine();
                ImGui::RadioButton("Srednia", &aktywnosc, 1); ImGui::SameLine();
                ImGui::RadioButton("Wysoka", &aktywnosc, 2);

                ImGui::Columns(1); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                if (ImGui::Button("Oblicz parametry", ImVec2(200, 45))) {
                    if (wzrost > 0 && waga > 0 && wiek > 0) {
                        float wzrostMetry = wzrost / 100.0f;
                        wyliczoneBmi = waga / (wzrostMetry * wzrostMetry);
                        if (wyliczoneBmi < 18.5f) kategoriaBmi = "Niedowaga";
                        else if (wyliczoneBmi < 25.0f) kategoriaBmi = "Waga w normie";
                        else if (wyliczoneBmi < 30.0f) kategoriaBmi = "Nadwaga";
                        else kategoriaBmi = "Otylosc";

                        wyliczoneBmr = (plec == 0) ? ((10.0f * waga) + (6.25f * wzrost) - (5.0f * wiek) + 5.0f) 
                                                   : ((10.0f * waga) + (6.25f * wzrost) - (5.0f * wiek) - 161.0f);
                        float mnoznik = (aktywnosc == 0) ? 1.2f : (aktywnosc == 1 ? 1.55f : 1.725f);
                        zapotrzebowanie = wyliczoneBmr * mnoznik;
                    }
                }

                if (wyliczoneBmi > 0.0f) {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "TWOJE WYNIKI:");
                    ImGui::Text("BMI: %.2f (%s)", wyliczoneBmi, kategoriaBmi.c_str());
                    ImGui::Text("Zapotrzebowanie na utrzymanie wagi: %.0f kcal", zapotrzebowanie);
                }
                ImGui::EndTabItem();
            }

            // ZAKLADKA TRENINGOWA
            if (ImGui::BeginTabItem("Plan Treningowy")) {
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Baza dostepnych cwiczen:");
                ImGui::Text("Wybierz cwiczenia, zeby zbudowac swoj plan.");
                ImGui::Separator();
                ImGui::Spacing();

                // tabele do wyswietlania
                if (ImGui::BeginTable("TabelaCwiczen", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    // naglowki kolumn
                    ImGui::TableSetupColumn("Nazwa Cwiczenia", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Kategoria", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                    ImGui::TableSetupColumn("Poziom Trudnosci", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                    ImGui::TableSetupColumn("Akcja", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableHeadersRow();

                    // pobieranie wierszy z pliku JSON
                    for (const auto& cw : bazaCwiczen) {
                        ImGui::TableNextRow();
                        
                        // Kolumna 1: Nazwa
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%s", cw.nazwa.c_str());

                        // Kolumna 2: Kategoria
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", cw.kategoria.c_str());

                       // Kolumna 3: Rysowanie poziomu trudnosci
                        ImGui::TableSetColumnIndex(2);
                        for (int i = 1; i <= 5; ++i) {
                            if (i <= cw.trudnosc) {
                                // kolor gwiazdek
                                ImVec4 color = (cw.trudnosc <= 2) ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : // zielony
                                               (cw.trudnosc == 3) ? ImVec4(1.0f, 0.8f, 0.0f, 1.0f) : // zołty
                                                                    ImVec4(1.0f, 0.3f, 0.3f, 1.0f);  // czerwony
                                ImGui::TextColored(color, "*"); // swiecaca
                            } else {
                                ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.0f), "*"); // pusta
                            }
                            
                            // Odstęp miedzy gwiadkamii
                            if (i < 5) {
                                ImGui::SameLine();
                                ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
                            }
                        }
                        // Kolumna 4: Przycisk dodawania, jeszcze bez dzialania
                        ImGui::TableSetColumnIndex(3);
                        ImGui::PushID(cw.nazwa.c_str()); // id przycisku
                        if (ImGui::Button("Dodaj", ImVec2(80, 0))) {
                            cout << "Dodano cwiczenie: " << cw.nazwa << endl;
                        }
                        ImGui::PopID();
                    }
                    ImGui::EndTable();
                }

                ImGui::EndTabItem();
            }

            // zakladka dieta i wyswietlanie z jsona
            if (ImGui::BeginTabItem("Dieta i Makro")) {
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Baza dostepnych produktow (wartosci na 100g):");
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::BeginTable("TabelaProduktow", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    // Naglowki
                    ImGui::TableSetupColumn("Nazwa Produktu", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Kcal", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                    ImGui::TableSetupColumn("Bialko (g)", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("Wegle (g)", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("Tluszcze (g)", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                    ImGui::TableSetupColumn("Akcja", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableHeadersRow();

                    for (const auto& prod : bazaProduktow) {
                        ImGui::TableNextRow();
                        
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%s", prod.nazwa.c_str());

                        // wyswietlanie makro i kolorowanie ich
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "%.1f", prod.kcal);

                        ImGui::TableSetColumnIndex(2);
                        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "%.1f", prod.bialko);

                        ImGui::TableSetColumnIndex(3);
                        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%.1f", prod.weglowodany);

                        ImGui::TableSetColumnIndex(4);
                        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%.1f", prod.tluszcze);

                        // Przycisk dodawania
                        ImGui::TableSetColumnIndex(5);
                        ImGui::PushID(prod.nazwa.c_str());
                        if (ImGui::Button("Dodaj", ImVec2(80, 0))) {
                            cout << "Dodano produkt: " << prod.nazwa << endl;
                        }
                        ImGui::PopID();
                    }   
                    ImGui::EndTable();
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
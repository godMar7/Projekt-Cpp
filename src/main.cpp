#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// Funkcja pomocnicza do wyszukiwarki ignorujaca wielkosc liter
bool containsIgnoreCase(const string& str, const string& sub) {
    if (sub.empty()) return true;
    auto it = search(str.begin(), str.end(), sub.begin(), sub.end(),
        [](char ch1, char ch2) { return tolower(ch1) == tolower(ch2); });
    return it != str.end();
}

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

struct CwiczenieWPlanie {
    Cwiczenie baza;       
    int serie;            
    int powtorzenia;      
    float ciezar;         
};

struct PlanTreningowy {
    string nazwaPlanu;                  
    vector<CwiczenieWPlanie> cwiczenia; 
};

struct SkladnikPosilku {
    ProduktSpozywczy baza; 
    float wagaGramy;       
    
    float obliczKcal() const { return (wagaGramy / 100.0f) * baza.kcal; }
    float obliczBialko() const { return (wagaGramy / 100.0f) * baza.bialko; }
    float obliczWegle() const { return (wagaGramy / 100.0f) * baza.weglowodany; }
    float obliczTluszcze() const { return (wagaGramy / 100.0f) * baza.tluszcze; }
};

struct Posilek {
    string nazwaPosilku; 
    vector<SkladnikPosilku> skladniki;
};

struct PlanDietetyczny {
    vector<Posilek> posilki;
    float sumaKcal = 0.0f;
    float sumaBialko = 0.0f;
    float sumaWegle = 0.0f;
    float sumaTluszcze = 0.0f;

    void przeliczSumy() {
        sumaKcal = 0; sumaBialko = 0; sumaWegle = 0; sumaTluszcze = 0;
        for(const auto& posilek : posilki) {
            for(const auto& sk : posilek.skladniki) {
                sumaKcal += sk.obliczKcal();
                sumaBialko += sk.obliczBialko();
                sumaWegle += sk.obliczWegle();
                sumaTluszcze += sk.obliczTluszcze();
            }
        }
    }
};

struct ProfilUzytkownika {
    float waga;
    float wzrost;
    int wiek;
    int plec;
    int aktywnosc;
    float wyliczoneBmi;
    float zapotrzebowanieKcal;
    PlanTreningowy aktualnyTrening;
    PlanDietetyczny aktualnaDieta;
};

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1600, 900, "FitPlanner - Dashboard Treningowy", NULL, NULL);
    if (window == NULL) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    vector<Cwiczenie> bazaCwiczen;
    ifstream plikJSON("data/cwiczenia.json");
    if (!plikJSON.is_open()) plikJSON.open("../data/cwiczenia.json");
    if (plikJSON.is_open()) {
        json dane = json::parse(plikJSON);
        for (const auto& item : dane) bazaCwiczen.push_back({item["nazwa"], item["kategoria"], item["trudnosc"]});
    }

    vector<ProduktSpozywczy> bazaProduktow;
    ifstream plikJSON_Dieta("data/produkty.json");
    if (!plikJSON_Dieta.is_open()) plikJSON_Dieta.open("../data/produkty.json");
    if (plikJSON_Dieta.is_open()) {
        json daneDieta = json::parse(plikJSON_Dieta);
        for (const auto& item : daneDieta) bazaProduktow.push_back({item["nazwa"], item["kcal"], item["bialko"], item["weglowodany"], item["tluszcze"]});
    }

    ProfilUzytkownika user = {70.0f, 175.0f, 25, 0, 1, 0.0f, 2500.0f}; 
    string kategoriaBmi = "";
    PlanTreningowy mojPlan;
    PlanDietetyczny mojaDieta;
    mojaDieta.posilki.push_back({"Caly Dzien", {}});

    // Zmienne do wyszukiwarki i filtrowania
    static char szukajCw[128] = "";
    static int wybranaKategoriaCw = 0;
    const char* kategorieCw[] = {"Wszystkie", "Klatka piersiowa", "Nogi", "Plecy", "Barki", "Biceps", "Triceps", "Brzuch"};
    static char szukajProd[128] = "";

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
        
        ImGui::Begin("FitPlanner Workspace", nullptr, window_flags);

        float calcWidth = ImGui::GetContentRegionAvail().x;
        float calcHeight = ImGui::GetContentRegionAvail().y;

        // MODUL 1: PROFIL (GORNY PANEL)
        ImGui::BeginChild("PanelProfil", ImVec2(calcWidth, 180), true);
        
        // Naglowek
        const char* tytulProfil = "M O J   P R O F I L";
        float windowWidth = ImGui::GetWindowSize().x;
        float textWidth = ImGui::CalcTextSize(tytulProfil).x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", tytulProfil);
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Columns(3, "kolumny_profil", false); 
        
        // Kolumna 1
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Podstawowe wymiary:");
        ImGui::SetNextItemWidth(120); ImGui::InputFloat("Waga (kg)", &user.waga, 1.0f, 5.0f, "%.1f");
        ImGui::SetNextItemWidth(120); ImGui::InputFloat("Wzrost (cm)", &user.wzrost, 1.0f, 5.0f, "%.1f");
        ImGui::SetNextItemWidth(120); ImGui::InputInt("Wiek (lata)", &user.wiek);
        ImGui::NextColumn();

        // Kolumna 2
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Plec i aktywnosc:");
        ImGui::RadioButton("Mezczyzna", &user.plec, 0); ImGui::SameLine();
        ImGui::RadioButton("Kobieta", &user.plec, 1);
        ImGui::Spacing();
        ImGui::RadioButton("Niska aktywnosc", &user.aktywnosc, 0); 
        ImGui::RadioButton("Srednia aktywnosc", &user.aktywnosc, 1); 
        ImGui::RadioButton("Wysoka aktywnosc", &user.aktywnosc, 2);
        ImGui::NextColumn();

        // Kolumna 3
        if (ImGui::Button("Oblicz BMR i BMI", ImVec2(-1, 40))) {
            if (user.wzrost > 0 && user.waga > 0 && user.wiek > 0) {
                float wzrostMetry = user.wzrost / 100.0f;
                user.wyliczoneBmi = user.waga / (wzrostMetry * wzrostMetry);
                if (user.wyliczoneBmi < 18.5f) kategoriaBmi = "Niedowaga";
                else if (user.wyliczoneBmi < 25.0f) kategoriaBmi = "Waga w normie";
                else if (user.wyliczoneBmi < 30.0f) kategoriaBmi = "Nadwaga";
                else kategoriaBmi = "Otylosc";

                float wyliczoneBmr = (user.plec == 0) ? ((10.0f * user.waga) + (6.25f * user.wzrost) - (5.0f * user.wiek) + 5.0f) : ((10.0f * user.waga) + (6.25f * user.wzrost) - (5.0f * user.wiek) - 161.0f);
                float mnoznik = (user.aktywnosc == 0) ? 1.2f : (user.aktywnosc == 1 ? 1.55f : 1.725f);
                user.zapotrzebowanieKcal = wyliczoneBmr * mnoznik;
            }
        }
        ImGui::Spacing();
        if (user.wyliczoneBmi > 0.0f) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Wynik BMI: %.1f (%s)", user.wyliczoneBmi, kategoriaBmi.c_str());
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Zapotrzebowanie: %.0f kcal", user.zapotrzebowanieKcal);
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Uzupelnij dane i kliknij oblicz...");
        }

        ImGui::Columns(1);
        ImGui::EndChild();

        ImGui::Spacing();

        // PODZIAL EKRANU NA POLOWY
        float polowaSzerokosci = (calcWidth / 2.0f) - 4.0f;
        float wysokoscDolnych = ImGui::GetContentRegionAvail().y;

        // MODUL 2: PLAN TRENINGOWY (LEWA STRONA)
        ImGui::BeginChild("PanelTrening", ImVec2(polowaSzerokosci, wysokoscDolnych), true);
        
        const char* tytulTrening = "P L A N   T R E N I N G O W Y";
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(tytulTrening).x) * 0.5f);
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "%s", tytulTrening);
        ImGui::Separator();
        ImGui::Spacing();

        // Wyszukiwarka i filtrowanie cwiczen
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("Szukaj##cw", szukajCw, IM_ARRAYSIZE(szukajCw));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120);
        ImGui::Combo("Partia##cw", &wybranaKategoriaCw, kategorieCw, IM_ARRAYSIZE(kategorieCw));
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Baza cwiczen:");
        if (ImGui::BeginTable("TabelaCwiczen", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 150))) {
            ImGui::TableSetupColumn("Nazwa", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Kat.", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Trud.", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Akcja", ImGuiTableColumnFlags_WidthFixed, 50.0f);
            ImGui::TableHeadersRow();

            for (const auto& cw : bazaCwiczen) {
                // Filtrowanie z menu rozwijanego i pola tekstowego
                if (wybranaKategoriaCw != 0 && cw.kategoria != kategorieCw[wybranaKategoriaCw]) continue;
                if (szukajCw[0] != '\0' && !containsIgnoreCase(cw.nazwa, szukajCw)) continue;

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("%s", cw.nazwa.c_str());
                ImGui::TableSetColumnIndex(1); ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", cw.kategoria.c_str());
                ImGui::TableSetColumnIndex(2);
                for (int i = 1; i <= 5; ++i) {
                    if (i <= cw.trudnosc) ImGui::TextColored((cw.trudnosc <= 2) ? ImVec4(0.2f,1.f,0.2f,1.f) : (cw.trudnosc == 3) ? ImVec4(1.f,0.8f,0.f,1.f) : ImVec4(1.f,0.3f,0.3f,1.f), "*");
                    else ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.0f), "*");
                    if (i < 5) { ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f); }
                }
                ImGui::TableSetColumnIndex(3);
                ImGui::PushID(cw.nazwa.c_str()); 
                if (ImGui::Button("+", ImVec2(50, 0))) mojPlan.cwiczenia.push_back({cw, 3, 10, 0.0f});
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Skonstruowany trening:");
        if (mojPlan.cwiczenia.empty()) {
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "Brak cwiczen w planie.");
        } else {
            if (ImGui::BeginTable("MojPlan", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("Cwiczenie", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Serie", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableSetupColumn("Powt", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableSetupColumn("Kg", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                ImGui::TableHeadersRow();

                for (int i = 0; i < mojPlan.cwiczenia.size(); ++i) {
                    ImGui::TableNextRow();
                    ImGui::PushID(i + 1000); 
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%s", mojPlan.cwiczenia[i].baza.nazwa.c_str());
                    ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(50.0f); ImGui::InputInt("##s", &mojPlan.cwiczenia[i].serie, 0);
                    ImGui::TableSetColumnIndex(2); ImGui::SetNextItemWidth(50.0f); ImGui::InputInt("##p", &mojPlan.cwiczenia[i].powtorzenia, 0);
                    ImGui::TableSetColumnIndex(3); ImGui::SetNextItemWidth(50.0f); ImGui::InputFloat("##kg", &mojPlan.cwiczenia[i].ciezar, 0, 0, "%.1f");
                    ImGui::TableSetColumnIndex(4);
                    if (ImGui::Button("X", ImVec2(40, 0))) { mojPlan.cwiczenia.erase(mojPlan.cwiczenia.begin() + i); i--; }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // MODUL 3: PLAN DIETETYCZNY (PRAWA STRONA)
        ImGui::BeginChild("PanelDieta", ImVec2(0, wysokoscDolnych), true);
        
        mojaDieta.przeliczSumy();

        const char* tytulDieta = "P L A N   D I E T E T Y C Z N Y";
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(tytulDieta).x) * 0.5f);
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "%s", tytulDieta);
        ImGui::Separator();
        ImGui::Spacing();

        // progress bar
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Zjedzone: %.0f kcal / %.0f kcal", mojaDieta.sumaKcal, user.zapotrzebowanieKcal);
        float postepKcal = user.zapotrzebowanieKcal > 0.0f ? (mojaDieta.sumaKcal / user.zapotrzebowanieKcal) : 0.0f;
        if (postepKcal > 1.0f) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.2f, 0.2f, 1.0f)); 
        ImGui::ProgressBar(postepKcal, ImVec2(-1.0f, 15.0f), "");
        if (postepKcal > 1.0f) ImGui::PopStyleColor();

        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "B: %.1f g", mojaDieta.sumaBialko); ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), " | W: %.1f g", mojaDieta.sumaWegle); ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), " | T: %.1f g", mojaDieta.sumaTluszcze);
        ImGui::Spacing();

        // Wyszukiwarka produktow
        ImGui::SetNextItemWidth(180);
        ImGui::InputText("Szukaj##prod", szukajProd, IM_ARRAYSIZE(szukajProd));
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Baza produktow (100g):");
        // Flaga ImGuiTableFlags_Sortable dodaje mozliwosc klikania w naglowki
        if (ImGui::BeginTable("BazaProduktow", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable, ImVec2(0, 150))) {
            ImGui::TableSetupColumn("Produkt", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Kcal", ImGuiTableColumnFlags_WidthFixed, 40.0f);
            ImGui::TableSetupColumn("B", ImGuiTableColumnFlags_WidthFixed, 35.0f);
            ImGui::TableSetupColumn("W", ImGuiTableColumnFlags_WidthFixed, 35.0f);
            ImGui::TableSetupColumn("T", ImGuiTableColumnFlags_WidthFixed, 35.0f);
            ImGui::TableSetupColumn("+", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 40.0f);
            ImGui::TableHeadersRow();

            // Logika wywolywana podczas klikniecia w naglowek
            if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs()) {
                if (sorts_specs->SpecsDirty) {
                    std::sort(bazaProduktow.begin(), bazaProduktow.end(), [sorts_specs](const ProduktSpozywczy& a, const ProduktSpozywczy& b) {
                        const auto& spec = sorts_specs->Specs[0];
                        bool asc = spec.SortDirection == ImGuiSortDirection_Ascending;
                        if (spec.ColumnIndex == 0) return asc ? (a.nazwa < b.nazwa) : (a.nazwa > b.nazwa);
                        if (spec.ColumnIndex == 1) return asc ? (a.kcal < b.kcal) : (a.kcal > b.kcal);
                        if (spec.ColumnIndex == 2) return asc ? (a.bialko < b.bialko) : (a.bialko > b.bialko);
                        if (spec.ColumnIndex == 3) return asc ? (a.weglowodany < b.weglowodany) : (a.weglowodany > b.weglowodany);
                        if (spec.ColumnIndex == 4) return asc ? (a.tluszcze < b.tluszcze) : (a.tluszcze > b.tluszcze);
                        return false;
                    });
                    sorts_specs->SpecsDirty = false;
                }
            }

            for (const auto& prod : bazaProduktow) {
                // Filtrowanie wyszukiwarka
                if (szukajProd[0] != '\0' && !containsIgnoreCase(prod.nazwa, szukajProd)) continue;

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("%s", prod.nazwa.c_str());
                ImGui::TableSetColumnIndex(1); ImGui::TextColored(ImVec4(1.f, 0.8f, 0.f, 1.f), "%.0f", prod.kcal);
                ImGui::TableSetColumnIndex(2); ImGui::TextColored(ImVec4(0.2f, 1.f, 0.2f, 1.f), "%.1f", prod.bialko);
                ImGui::TableSetColumnIndex(3); ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.f, 1.f), "%.1f", prod.weglowodany);
                ImGui::TableSetColumnIndex(4); ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "%.1f", prod.tluszcze);
                ImGui::TableSetColumnIndex(5);
                ImGui::PushID(prod.nazwa.c_str());
                if (ImGui::Button("+", ImVec2(40, 0))) mojaDieta.posilki[0].skladniki.push_back({prod, 100.0f});
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Skonstruowany talerz:");
        if (mojaDieta.posilki[0].skladniki.empty()) {
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "Brak jedzenia na talerzu.");
        } else {
            if (ImGui::BeginTable("MojTalerz", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("Produkt", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Waga(g)", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableSetupColumn("Kcal", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                ImGui::TableSetupColumn("B", ImGuiTableColumnFlags_WidthFixed, 35.0f);
                ImGui::TableSetupColumn("W", ImGuiTableColumnFlags_WidthFixed, 35.0f);
                ImGui::TableSetupColumn("T", ImGuiTableColumnFlags_WidthFixed, 35.0f);
                ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                ImGui::TableHeadersRow();

                for (int i = 0; i < mojaDieta.posilki[0].skladniki.size(); ++i) {
                    ImGui::TableNextRow();
                    auto& sk = mojaDieta.posilki[0].skladniki[i];
                    ImGui::PushID(i + 2000); 

                    ImGui::TableSetColumnIndex(0); ImGui::Text("%s", sk.baza.nazwa.c_str());
                    ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(50.0f); ImGui::InputFloat("##w", &sk.wagaGramy, 0, 0, "%.0f");
                    if (sk.wagaGramy < 0) sk.wagaGramy = 0; 
                    
                    ImGui::TableSetColumnIndex(2); ImGui::TextColored(ImVec4(1.f, 0.8f, 0.f, 1.f), "%.0f", sk.obliczKcal());
                    ImGui::TableSetColumnIndex(3); ImGui::TextColored(ImVec4(0.2f, 1.f, 0.2f, 1.f), "%.1f", sk.obliczBialko());
                    ImGui::TableSetColumnIndex(4); ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.f, 1.f), "%.1f", sk.obliczWegle());
                    ImGui::TableSetColumnIndex(5); ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "%.1f", sk.obliczTluszcze());
                    
                    ImGui::TableSetColumnIndex(6);
                    if (ImGui::Button("X", ImVec2(40, 0))) { mojaDieta.posilki[0].skladniki.erase(mojaDieta.posilki[0].skladniki.begin() + i); i--; }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();

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
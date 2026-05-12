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

// Funkcja pomocnicza do wyszukiwarki
bool containsIgnoreCase(const string& str, const string& sub) {
    if (sub.empty()) return true;
    auto it = search(str.begin(), str.end(), sub.begin(), sub.end(),
        [](char ch1, char ch2) { return tolower(ch1) == tolower(ch2); });
    return it != str.end();
}

// KLASY I STRUKTURY DANYCH

struct Cwiczenie {
    string nazwa;
    string kategoria;
    int trudnosc; 
    string opis; // Podpowiedz jak wykonywac
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

struct DzienTreningowy {
    string nazwaDnia;
    vector<CwiczenieWPlanie> cwiczenia;
};

struct PlanTreningowy {
    vector<DzienTreningowy> dni; 
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
    string kategoriaBmi; 
    float zapotrzebowanieKcal;
    PlanTreningowy aktualnyTrening;
    PlanDietetyczny aktualnaDieta;
};

// FUNKCJE DO ZAPISU I ODCZYTU

bool ZapiszProfil(const ProfilUzytkownika& user) {
    json j;
    j["waga"] = user.waga;
    j["wzrost"] = user.wzrost;
    j["wiek"] = user.wiek;
    j["plec"] = user.plec;
    j["aktywnosc"] = user.aktywnosc;
    j["zapotrzebowanieKcal"] = user.zapotrzebowanieKcal;
    j["wyliczoneBmi"] = user.wyliczoneBmi;
    j["kategoriaBmi"] = user.kategoriaBmi;

    json jTrening = json::array();
    for (const auto& dzien : user.aktualnyTrening.dni) {
        json jDzien;
        jDzien["nazwaDnia"] = dzien.nazwaDnia;
        json jCwiczenia = json::array();
        for (const auto& cw : dzien.cwiczenia) {
            json jCw;
            jCw["baza"]["nazwa"] = cw.baza.nazwa;
            jCw["baza"]["kategoria"] = cw.baza.kategoria;
            jCw["baza"]["trudnosc"] = cw.baza.trudnosc;
            jCw["baza"]["opis"] = cw.baza.opis;
            jCw["serie"] = cw.serie;
            jCw["powtorzenia"] = cw.powtorzenia;
            jCw["ciezar"] = cw.ciezar;
            jCwiczenia.push_back(jCw);
        }
        jDzien["cwiczenia"] = jCwiczenia;
        jTrening.push_back(jDzien);
    }
    j["aktualnyTrening"] = jTrening;

    json jDieta = json::array();
    for (const auto& posilek : user.aktualnaDieta.posilki) {
        json jPosilek;
        jPosilek["nazwaPosilku"] = posilek.nazwaPosilku;
        json jSkladniki = json::array();
        for (const auto& sk : posilek.skladniki) {
            json jSk;
            jSk["baza"]["nazwa"] = sk.baza.nazwa;
            jSk["baza"]["kcal"] = sk.baza.kcal;
            jSk["baza"]["bialko"] = sk.baza.bialko;
            jSk["baza"]["weglowodany"] = sk.baza.weglowodany;
            jSk["baza"]["tluszcze"] = sk.baza.tluszcze;
            jSk["wagaGramy"] = sk.wagaGramy;
            jSkladniki.push_back(jSk);
        }
        jPosilek["skladniki"] = jSkladniki;
        jDieta.push_back(jPosilek);
    }
    j["aktualnaDieta"] = jDieta;

    ofstream file("data/profil.json");
    if (!file.is_open()) file.open("../data/profil.json");
    if (file.is_open()) {
        file << j.dump(4);
        file.close();
        return true;
    }
    return false;
}

bool ZapiszBazeCwiczen(const vector<Cwiczenie>& baza) {
    json j = json::array();
    for(const auto& cw : baza) {
        j.push_back({{"nazwa", cw.nazwa}, {"kategoria", cw.kategoria}, {"trudnosc", cw.trudnosc}, {"opis", cw.opis}});
    }
    ofstream file("data/cwiczenia.json");
    if(!file.is_open()) file.open("../data/cwiczenia.json");
    if(file.is_open()) { file << j.dump(4); return true; }
    return false;
}

bool ZapiszBazeProduktow(const vector<ProduktSpozywczy>& baza) {
    json j = json::array();
    for(const auto& p : baza) {
        j.push_back({{"nazwa", p.nazwa}, {"kcal", p.kcal}, {"bialko", p.bialko}, {"weglowodany", p.weglowodany}, {"tluszcze", p.tluszcze}});
    }
    ofstream file("data/produkty.json");
    if(!file.is_open()) file.open("../data/produkty.json");
    if(file.is_open()) { file << j.dump(4); return true; }
    return false;
}

bool EksportujPlanTXT(const ProfilUzytkownika& user) {
    ofstream file("moj_plan.txt");
    if (!file.is_open()) file.open("../moj_plan.txt");
    if (!file.is_open()) return false;

    file << "========================================\n";
    file << "           FITPLANNER - MOJ PLAN        \n";
    file << "========================================\n\n";
    
    file << "[ PROFIL ]\n";
    file << "Waga: " << user.waga << " kg | Wzrost: " << user.wzrost << " cm | Wiek: " << user.wiek << " lat\n";
    file << "BMI: " << user.wyliczoneBmi << " (" << user.kategoriaBmi << ")\n";
    file << "Zapotrzebowanie kaloryczne: " << user.zapotrzebowanieKcal << " kcal\n\n";

    file << "[ PLAN TRENINGOWY ]\n";
    for(const auto& d : user.aktualnyTrening.dni) {
        file << ">> " << d.nazwaDnia << ":\n";
        if(d.cwiczenia.empty()) {
            file << "   Odpoczynek (Rest day)\n";
        } else {
            for(const auto& cw : d.cwiczenia) {
                file << "   - " << cw.baza.nazwa << " | " << cw.serie << " serie x " << cw.powtorzenia << " powt. | Ciezar: " << cw.ciezar << " kg\n";
            }
        }
        file << "\n";
    }

    file << "[ PLAN DIETETYCZNY ]\n";
    file << "Lacznie: " << user.aktualnaDieta.sumaKcal << " kcal | Bialko: " << user.aktualnaDieta.sumaBialko 
         << "g | Wegle: " << user.aktualnaDieta.sumaWegle << "g | Tluszcze: " << user.aktualnaDieta.sumaTluszcze << "g\n\n";
         
    for(const auto& p : user.aktualnaDieta.posilki) {
        file << ">> " << p.nazwaPosilku << ":\n";
        if(p.skladniki.empty()) {
            file << "   Brak jedzenia\n";
        } else {
            for(const auto& sk : p.skladniki) {
                file << "   - " << sk.baza.nazwa << " (" << sk.wagaGramy << "g) -> " 
                     << sk.obliczKcal() << " kcal (B:" << sk.obliczBialko() << " W:" << sk.obliczWegle() << " T:" << sk.obliczTluszcze() << ")\n";
            }
        }
        file << "\n";
    }

    file.close();
    return true;
}

void WczytajProfil(ProfilUzytkownika& user) {
    ifstream file("data/profil.json");
    if (!file.is_open()) file.open("../data/profil.json");
    if (!file.is_open()) return; 

    try {
        json j;
        file >> j;
        
        user.waga = j.value("waga", 70.0f);
        user.wzrost = j.value("wzrost", 175.0f);
        user.wiek = j.value("wiek", 25);
        user.plec = j.value("plec", 0);
        user.aktywnosc = j.value("aktywnosc", 1);
        user.zapotrzebowanieKcal = j.value("zapotrzebowanieKcal", 2500.0f);
        user.wyliczoneBmi = j.value("wyliczoneBmi", 0.0f);
        user.kategoriaBmi = j.value("kategoriaBmi", "");

        if (j.contains("aktualnyTrening")) {
            user.aktualnyTrening.dni.clear();
            for (const auto& jDzien : j["aktualnyTrening"]) {
                DzienTreningowy dt;
                dt.nazwaDnia = jDzien.value("nazwaDnia", "");
                for (const auto& jCw : jDzien["cwiczenia"]) {
                    CwiczenieWPlanie cw;
                    cw.baza.nazwa = jCw["baza"].value("nazwa", "");
                    cw.baza.kategoria = jCw["baza"].value("kategoria", "");
                    cw.baza.trudnosc = jCw["baza"].value("trudnosc", 1);
                    cw.baza.opis = jCw["baza"].value("opis", "Technika jest najwazniejsza.");
                    cw.serie = jCw.value("serie", 3);
                    cw.powtorzenia = jCw.value("powtorzenia", 10);
                    cw.ciezar = jCw.value("ciezar", 0.0f);
                    dt.cwiczenia.push_back(cw);
                }
                user.aktualnyTrening.dni.push_back(dt);
            }
        }

        if (j.contains("aktualnaDieta")) {
            user.aktualnaDieta.posilki.clear();
            for (const auto& jPosilek : j["aktualnaDieta"]) {
                Posilek p;
                p.nazwaPosilku = jPosilek.value("nazwaPosilku", "");
                for (const auto& jSk : jPosilek["skladniki"]) {
                    SkladnikPosilku sk;
                    sk.baza.nazwa = jSk["baza"].value("nazwa", "");
                    sk.baza.kcal = jSk["baza"].value("kcal", 0.0f);
                    sk.baza.bialko = jSk["baza"].value("bialko", 0.0f);
                    sk.baza.weglowodany = jSk["baza"].value("weglowodany", 0.0f);
                    sk.baza.tluszcze = jSk["baza"].value("tluszcze", 0.0f);
                    sk.wagaGramy = jSk.value("wagaGramy", 100.0f);
                    p.skladniki.push_back(sk);
                }
                user.aktualnaDieta.posilki.push_back(p);
            }
        }
    } catch (...) {
        cout << "Blad podczas wczytywania profilu." << endl;
    }
}

void InicjalizujPusteDniIPosilki(ProfilUzytkownika& user) {
    if (user.aktualnyTrening.dni.empty()) {
        const char* dni[] = {"Poniedzialek", "Wtorek", "Sroda", "Czwartek", "Piatek", "Sobota", "Niedziela"};
        for(int i=0; i<7; i++) user.aktualnyTrening.dni.push_back({dni[i], {}});
    }
    if (user.aktualnaDieta.posilki.empty()) {
        const char* posilki[] = {"Sniadanie", "Drugie Sniadanie", "Obiad", "Przekaska", "Kolacja"};
        for(int i=0; i<5; i++) user.aktualnaDieta.posilki.push_back({posilki[i], {}});
    }
}

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
        for (const auto& item : dane) {
            bazaCwiczen.push_back({item["nazwa"], item["kategoria"], item["trudnosc"], item.value("opis", "Technika jest najwazniejsza. Utrzymuj napiecie.")});
        }
    }

    vector<ProduktSpozywczy> bazaProduktow;
    ifstream plikJSON_Dieta("data/produkty.json");
    if (!plikJSON_Dieta.is_open()) plikJSON_Dieta.open("../data/produkty.json");
    if (plikJSON_Dieta.is_open()) {
        json daneDieta = json::parse(plikJSON_Dieta);
        for (const auto& item : daneDieta) bazaProduktow.push_back({item["nazwa"], item["kcal"], item["bialko"], item["weglowodany"], item["tluszcze"]});
    }

    ProfilUzytkownika user = {70.0f, 175.0f, 25, 0, 1, 0.0f, "", 2500.0f}; 
    
    WczytajProfil(user);
    InicjalizujPusteDniIPosilki(user);

    // Zmienne interfejsu 
    static char szukajCw[128] = "";
    static int wybranaKategoriaCw = 0;
    const char* kategorieCw[] = {"Wszystkie", "Klatka piersiowa", "Nogi", "Plecy", "Barki", "Biceps", "Triceps", "Brzuch"};
    static int wybranyDzienDoDodania = 0;
    const char* nazwyDniUzytkowe[] = {"Poniedzialek", "Wtorek", "Sroda", "Czwartek", "Piatek", "Sobota", "Niedziela"};

    static char szukajProd[128] = "";
    static int wybranyPosilekDoDodania = 0;
    const char* nazwyPosilkowUzytkowe[] = {"Sniadanie", "Drugie Sniadanie", "Obiad", "Przekaska", "Kolacja"};

    static string statusAkcji = "";
    static float czasWyswietlaniaStatusu = 0.0f;

    // Zmienne do wlasnych wpisow
    static char noweCwNazwa[128] = "";
    static int nowaCwKategoria = 0;
    static int noweCwTrudnosc = 3;
    static char noweCwOpis[256] = "";

    static char nowyProdNazwa[128] = "";
    static float nowyProdKcal = 0, nowyProdB = 0, nowyProdW = 0, nowyProdT = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //STAN MOTYWU I DYNAMICZNE KOLORY ---
        static bool jasnyMotyw = false;
        ImVec4 colAkcent = jasnyMotyw ? ImVec4(0.0f, 0.4f, 0.8f, 1.0f) : ImVec4(0.4f, 0.8f, 1.0f, 1.0f);
        ImVec4 colSub = jasnyMotyw ? ImVec4(0.3f, 0.3f, 0.3f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
        ImVec4 colZielony = jasnyMotyw ? ImVec4(0.0f, 0.5f, 0.0f, 1.0f) : ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
        ImVec4 colZolty = jasnyMotyw ? ImVec4(0.8f, 0.5f, 0.0f, 1.0f) : ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
        ImVec4 colCzerwony = jasnyMotyw ? ImVec4(0.8f, 0.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
        ImVec4 colTrening = jasnyMotyw ? ImVec4(0.8f, 0.4f, 0.0f, 1.0f) : ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
        ImVec4 colWegle = jasnyMotyw ? ImVec4(0.0f, 0.4f, 0.8f, 1.0f) : ImVec4(0.4f, 0.8f, 1.0f, 1.0f);
        ImVec4 colPustaGwiazdka = jasnyMotyw ? ImVec4(0.8f, 0.8f, 0.8f, 1.0f) : ImVec4(0.3f, 0.3f, 0.3f, 1.0f);

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | 
                                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                                        ImGuiWindowFlags_NoBringToFrontOnFocus;
        
        ImGui::Begin("FitPlanner Workspace", nullptr, window_flags);

        float calcWidth = ImGui::GetContentRegionAvail().x;
        float calcHeight = ImGui::GetContentRegionAvail().y;

        //profil
        ImGui::BeginChild("PanelProfil", ImVec2(calcWidth, 160), true);
        
        const char* tytulProfil = "M O J   P R O F I L";
        float windowWidth = ImGui::GetWindowSize().x;
        float textWidth = ImGui::CalcTextSize(tytulProfil).x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::TextColored(colAkcent, "%s", tytulProfil);
        
        // PRZELACZNIK MOTYWU
        ImGui::SameLine(windowWidth - 120);
        if (ImGui::Checkbox("Jasny", &jasnyMotyw)) {
            if (jasnyMotyw) ImGui::StyleColorsLight();
            else ImGui::StyleColorsDark();
        }

        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Columns(3, "kolumny_profil", false); 
        
        ImGui::TextColored(colSub, "Podstawowe wymiary:");
        ImGui::SetNextItemWidth(120); ImGui::InputFloat("Waga (kg)", &user.waga, 1.0f, 5.0f, "%.1f");
        ImGui::SetNextItemWidth(120); ImGui::InputFloat("Wzrost (cm)", &user.wzrost, 1.0f, 5.0f, "%.1f");
        ImGui::SetNextItemWidth(120); ImGui::InputInt("Wiek (lata)", &user.wiek);
        ImGui::NextColumn();

        ImGui::TextColored(colSub, "Plec i aktywnosc:");
        ImGui::RadioButton("Mezczyzna", &user.plec, 0); ImGui::SameLine();
        ImGui::RadioButton("Kobieta", &user.plec, 1);
        ImGui::Spacing();
        ImGui::RadioButton("Niska aktywnosc", &user.aktywnosc, 0); 
        ImGui::RadioButton("Srednia aktywnosc", &user.aktywnosc, 1); 
        ImGui::RadioButton("Wysoka aktywnosc", &user.aktywnosc, 2);
        ImGui::NextColumn();

        if (ImGui::Button("Oblicz BMR i BMI", ImVec2(-1, 30))) {
            if (user.wzrost > 0 && user.waga > 0 && user.wiek > 0) {
                float wzrostMetry = user.wzrost / 100.0f;
                user.wyliczoneBmi = user.waga / (wzrostMetry * wzrostMetry);
                if (user.wyliczoneBmi < 18.5f) user.kategoriaBmi = "Niedowaga";
                else if (user.wyliczoneBmi < 25.0f) user.kategoriaBmi = "Waga w normie";
                else if (user.wyliczoneBmi < 30.0f) user.kategoriaBmi = "Nadwaga";
                else user.kategoriaBmi = "Otylosc";

                float wyliczoneBmr = (user.plec == 0) ? ((10.0f * user.waga) + (6.25f * user.wzrost) - (5.0f * user.wiek) + 5.0f) : ((10.0f * user.waga) + (6.25f * user.wzrost) - (5.0f * user.wiek) - 161.0f);
                float mnoznik = (user.aktywnosc == 0) ? 1.2f : (user.aktywnosc == 1 ? 1.55f : 1.725f);
                user.zapotrzebowanieKcal = wyliczoneBmr * mnoznik;
            }
        }
        ImGui::Spacing();
        
        ImGui::PushStyleColor(ImGuiCol_Button, jasnyMotyw ? ImVec4(0.9f, 0.6f, 0.0f, 1.0f) : ImVec4(0.8f, 0.6f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.4f, 0.0f, 1.0f));
        if (ImGui::Button("ZAPISZ DANE", ImVec2(140, 30))) {
            if (ZapiszProfil(user)) statusAkcji = "Zapisano profil pomyslnie!";
            else statusAkcji = "Blad zapisu profilu.";
            czasWyswietlaniaStatusu = 3.0f; 
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, jasnyMotyw ? ImVec4(0.0f, 0.5f, 0.9f, 1.0f) : ImVec4(0.0f, 0.4f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.6f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.3f, 0.6f, 1.0f));
        if (ImGui::Button("EKSPORT TXT", ImVec2(140, 30))) {
            if (EksportujPlanTXT(user)) statusAkcji = "Wyeksportowano do moj_plan.txt!";
            else statusAkcji = "Blad eksportu TXT.";
            czasWyswietlaniaStatusu = 3.0f;
        }
        ImGui::PopStyleColor(3);

        if (user.wyliczoneBmi > 0.0f) {
            ImGui::TextColored(colZielony, "Wynik BMI: %.1f (%s) | Zapotrzebowanie: %.0f kcal", user.wyliczoneBmi, user.kategoriaBmi.c_str(), user.zapotrzebowanieKcal);
        }

        if (czasWyswietlaniaStatusu > 0.0f) {
            czasWyswietlaniaStatusu -= ImGui::GetIO().DeltaTime;
            ImGui::TextColored(colZielony, "%s", statusAkcji.c_str());
        }

        ImGui::Columns(1);
        ImGui::EndChild();

        ImGui::Spacing();
        float polowaSzerokosci = (calcWidth / 2.0f) - 4.0f;
        float wysokoscDolnych = ImGui::GetContentRegionAvail().y;

        //plan treningowy
        ImGui::BeginChild("PanelTrening", ImVec2(polowaSzerokosci, wysokoscDolnych), true);
        
        const char* tytulTrening = "P L A N   T R E N I N G O W Y";
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(tytulTrening).x) * 0.5f);
        ImGui::TextColored(colTrening, "%s", tytulTrening);
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextColored(colSub, "Skonstruowany tydzien treningowy:");
        if (ImGui::BeginTable("MojPlan", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 250))) {
            ImGui::TableSetupColumn("Cwiczenie", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Serie", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Powt", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Kg", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed, 40.0f);
            ImGui::TableHeadersRow();

            for (int d = 0; d < user.aktualnyTrening.dni.size(); ++d) {
                auto& dzien = user.aktualnyTrening.dni[d];
                
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextColored(colZolty, "[ %s ]", dzien.nazwaDnia.c_str());

                if (dzien.cwiczenia.empty()) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
                    ImGui::TextColored(colZielony, "- Odpoczynek (Rest day)");
                } else {
                    for (int i = 0; i < dzien.cwiczenia.size(); ++i) {
                        ImGui::TableNextRow();
                        ImGui::PushID((d * 1000) + i + 5000); 
                        
                        ImGui::TableSetColumnIndex(0); 
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
                        ImGui::Text("- %s", dzien.cwiczenia[i].baza.nazwa.c_str());
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", dzien.cwiczenia[i].baza.opis.c_str());
                        
                        ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(50.0f); ImGui::InputInt("##s", &dzien.cwiczenia[i].serie, 0);
                        ImGui::TableSetColumnIndex(2); ImGui::SetNextItemWidth(50.0f); ImGui::InputInt("##p", &dzien.cwiczenia[i].powtorzenia, 0);
                        ImGui::TableSetColumnIndex(3); ImGui::SetNextItemWidth(50.0f); ImGui::InputFloat("##kg", &dzien.cwiczenia[i].ciezar, 0, 0, "%.1f");
                        
                        ImGui::TableSetColumnIndex(4);
                        if (ImGui::Button("X", ImVec2(40, 0))) { dzien.cwiczenia.erase(dzien.cwiczenia.begin() + i); i--; }
                        ImGui::PopID();
                    }
                }
            }
            ImGui::EndTable();
        }

        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        if (ImGui::CollapsingHeader("Przegladaj Baze Cwiczen i Dodaj do Planu")) {
            ImGui::SetNextItemWidth(150);
            ImGui::InputText("Szukaj##cw", szukajCw, IM_ARRAYSIZE(szukajCw));
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120);
            ImGui::Combo("Partia##cw", &wybranaKategoriaCw, kategorieCw, IM_ARRAYSIZE(kategorieCw));
            
            ImGui::SetNextItemWidth(150);
            ImGui::Combo("Dodaj do##dzien", &wybranyDzienDoDodania, nazwyDniUzytkowe, IM_ARRAYSIZE(nazwyDniUzytkowe));
            ImGui::Spacing();

            if (ImGui::BeginTable("TabelaCwiczen", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 150))) {
                ImGui::TableSetupColumn("Nazwa", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Kat.", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("Trud.", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableSetupColumn("Akcja", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                ImGui::TableHeadersRow();

                for (const auto& cw : bazaCwiczen) {
                    if (wybranaKategoriaCw != 0 && cw.kategoria != kategorieCw[wybranaKategoriaCw]) continue;
                    if (szukajCw[0] != '\0' && !containsIgnoreCase(cw.nazwa, szukajCw)) continue;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::Text("%s", cw.nazwa.c_str());
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", cw.opis.c_str());

                    ImGui::TableSetColumnIndex(1); ImGui::TextColored(colSub, "%s", cw.kategoria.c_str());
                    ImGui::TableSetColumnIndex(2);
                    for (int i = 1; i <= 5; ++i) {
                        if (i <= cw.trudnosc) ImGui::TextColored((cw.trudnosc <= 2) ? colZielony : (cw.trudnosc == 3) ? colZolty : colCzerwony, "*");
                        else ImGui::TextColored(colPustaGwiazdka, "*");
                        if (i < 5) { ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f); }
                    }
                    ImGui::TableSetColumnIndex(3);
                    ImGui::PushID(cw.nazwa.c_str()); 
                    if (ImGui::Button("+", ImVec2(50, 0))) user.aktualnyTrening.dni[wybranyDzienDoDodania].cwiczenia.push_back({cw, 3, 10, 0.0f});
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }

        if (ImGui::CollapsingHeader("Dodaj Wlasne Cwiczenie (Kreator)")) {
            ImGui::Text("Nazwa: "); ImGui::SameLine();
            ImGui::SetNextItemWidth(150); ImGui::InputText("##noweCwNazwa", noweCwNazwa, IM_ARRAYSIZE(noweCwNazwa)); 
            ImGui::SameLine();
            ImGui::Text("Partia: "); ImGui::SameLine();
            ImGui::SetNextItemWidth(120); ImGui::Combo("##nowaCwKategoria", &nowaCwKategoria, &kategorieCw[1], 7); 
            
            ImGui::Text("Porada:"); ImGui::SameLine();
            ImGui::SetNextItemWidth(250); ImGui::InputText("##noweCwOpis", noweCwOpis, IM_ARRAYSIZE(noweCwOpis));
            ImGui::SameLine();
            ImGui::SetNextItemWidth(80);  ImGui::SliderInt("Trudnosc", &noweCwTrudnosc, 1, 5); 
            
            if (ImGui::Button("Zapisz do Bazy##btnCw")) {
                if (strlen(noweCwNazwa) > 0) {
                    string o = (strlen(noweCwOpis) > 0) ? noweCwOpis : "Brak opisu.";
                    bazaCwiczen.push_back({noweCwNazwa, kategorieCw[nowaCwKategoria+1], noweCwTrudnosc, o});
                    ZapiszBazeCwiczen(bazaCwiczen);
                    noweCwNazwa[0] = '\0'; noweCwOpis[0] = '\0';
                }
            }
        }

        ImGui::EndChild();
        ImGui::SameLine();

        //plan dietetyczny
        ImGui::BeginChild("PanelDieta", ImVec2(0, wysokoscDolnych), true);
        
        user.aktualnaDieta.przeliczSumy();

        const char* tytulDieta = "P L A N   D I E T E T Y C Z N Y";
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(tytulDieta).x) * 0.5f);
        ImGui::TextColored(colZielony, "%s", tytulDieta);
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextColored(colSub, "Zjedzone dzisiaj: %.0f kcal / %.0f kcal", user.aktualnaDieta.sumaKcal, user.zapotrzebowanieKcal);
        float postepKcal = user.zapotrzebowanieKcal > 0.0f ? (user.aktualnaDieta.sumaKcal / user.zapotrzebowanieKcal) : 0.0f;
        if (postepKcal > 1.0f) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, colCzerwony); 
        ImGui::ProgressBar(postepKcal, ImVec2(-1.0f, 15.0f), "");
        if (postepKcal > 1.0f) ImGui::PopStyleColor();

        ImGui::TextColored(colZielony, "B: %.1f g", user.aktualnaDieta.sumaBialko); ImGui::SameLine();
        ImGui::TextColored(colWegle, " | W: %.1f g", user.aktualnaDieta.sumaWegle); ImGui::SameLine();
        ImGui::TextColored(colCzerwony, " | T: %.1f g", user.aktualnaDieta.sumaTluszcze);
        ImGui::Spacing();

        ImGui::TextColored(colSub, "Skonstruowany jadlospis:");
        if (ImGui::BeginTable("MojTalerz", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 250))) {
            ImGui::TableSetupColumn("Produkt", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Waga(g)", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Kcal", ImGuiTableColumnFlags_WidthFixed, 40.0f);
            ImGui::TableSetupColumn("B", ImGuiTableColumnFlags_WidthFixed, 35.0f);
            ImGui::TableSetupColumn("W", ImGuiTableColumnFlags_WidthFixed, 35.0f);
            ImGui::TableSetupColumn("T", ImGuiTableColumnFlags_WidthFixed, 35.0f);
            ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed, 40.0f);
            ImGui::TableHeadersRow();

            for (int m = 0; m < user.aktualnaDieta.posilki.size(); ++m) {
                auto& posilek = user.aktualnaDieta.posilki[m];
                
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextColored(colZolty, "[ %s ]", posilek.nazwaPosilku.c_str());

                for (int i = 0; i < posilek.skladniki.size(); ++i) {
                    ImGui::TableNextRow();
                    auto& sk = posilek.skladniki[i];
                    ImGui::PushID((m * 1000) + i); 

                    ImGui::TableSetColumnIndex(0); 
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f); 
                    ImGui::Text("- %s", sk.baza.nazwa.c_str());
                    
                    ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(50.0f); ImGui::InputFloat("##w", &sk.wagaGramy, 0, 0, "%.0f");
                    if (sk.wagaGramy < 0) sk.wagaGramy = 0; 
                    
                    ImGui::TableSetColumnIndex(2); ImGui::TextColored(colZolty, "%.0f", sk.obliczKcal());
                    ImGui::TableSetColumnIndex(3); ImGui::TextColored(colZielony, "%.1f", sk.obliczBialko());
                    ImGui::TableSetColumnIndex(4); ImGui::TextColored(colWegle, "%.1f", sk.obliczWegle());
                    ImGui::TableSetColumnIndex(5); ImGui::TextColored(colCzerwony, "%.1f", sk.obliczTluszcze());
                    
                    ImGui::TableSetColumnIndex(6);
                    if (ImGui::Button("X", ImVec2(40, 0))) { posilek.skladniki.erase(posilek.skladniki.begin() + i); i--; }
                    ImGui::PopID();
                }
            }
            ImGui::EndTable();
        }
        
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        if (ImGui::CollapsingHeader("Przegladaj Baze Produktow i Dodaj do Talerza")) {
            ImGui::SetNextItemWidth(180);
            ImGui::InputText("Szukaj##prod", szukajProd, IM_ARRAYSIZE(szukajProd));
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150);
            ImGui::Combo("Dodaj do##posilek", &wybranyPosilekDoDodania, nazwyPosilkowUzytkowe, IM_ARRAYSIZE(nazwyPosilkowUzytkowe));
            ImGui::Spacing();

            if (ImGui::BeginTable("BazaProduktow", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable, ImVec2(0, 150))) {
                ImGui::TableSetupColumn("Produkt", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Kcal", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                ImGui::TableSetupColumn("B", ImGuiTableColumnFlags_WidthFixed, 35.0f);
                ImGui::TableSetupColumn("W", ImGuiTableColumnFlags_WidthFixed, 35.0f);
                ImGui::TableSetupColumn("T", ImGuiTableColumnFlags_WidthFixed, 35.0f);
                ImGui::TableSetupColumn("+", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 40.0f);
                ImGui::TableHeadersRow();

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
                    if (szukajProd[0] != '\0' && !containsIgnoreCase(prod.nazwa, szukajProd)) continue;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%s", prod.nazwa.c_str());
                    ImGui::TableSetColumnIndex(1); ImGui::TextColored(colZolty, "%.0f", prod.kcal);
                    ImGui::TableSetColumnIndex(2); ImGui::TextColored(colZielony, "%.1f", prod.bialko);
                    ImGui::TableSetColumnIndex(3); ImGui::TextColored(colWegle, "%.1f", prod.weglowodany);
                    ImGui::TableSetColumnIndex(4); ImGui::TextColored(colCzerwony, "%.1f", prod.tluszcze);
                    ImGui::TableSetColumnIndex(5);
                    ImGui::PushID(prod.nazwa.c_str());
                    if (ImGui::Button("+", ImVec2(40, 0))) user.aktualnaDieta.posilki[wybranyPosilekDoDodania].skladniki.push_back({prod, 100.0f});
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }

        if (ImGui::CollapsingHeader("Dodaj Wlasny Produkt (Kreator na 100g)")) {
            ImGui::Text("Nazwa: "); ImGui::SameLine();
            ImGui::SetNextItemWidth(150); ImGui::InputText("##nP", nowyProdNazwa, IM_ARRAYSIZE(nowyProdNazwa)); ImGui::SameLine();
            
            ImGui::Text("Kcal: "); ImGui::SameLine();
            ImGui::SetNextItemWidth(50);  ImGui::InputFloat("##nK", &nowyProdKcal, 0, 0, "%.0f"); ImGui::SameLine();
            
            ImGui::Text("B: "); ImGui::SameLine();
            ImGui::SetNextItemWidth(40);  ImGui::InputFloat("##nB", &nowyProdB, 0, 0, "%.1f"); ImGui::SameLine();
            
            ImGui::Text("W: "); ImGui::SameLine();
            ImGui::SetNextItemWidth(40);  ImGui::InputFloat("##nW", &nowyProdW, 0, 0, "%.1f"); ImGui::SameLine();
            
            ImGui::Text("T: "); ImGui::SameLine();
            ImGui::SetNextItemWidth(40);  ImGui::InputFloat("##nT", &nowyProdT, 0, 0, "%.1f"); ImGui::SameLine();
            
            if (ImGui::Button("Zapisz do Bazy##btnPr")) {
                if (strlen(nowyProdNazwa) > 0) {
                    bazaProduktow.push_back({nowyProdNazwa, nowyProdKcal, nowyProdB, nowyProdW, nowyProdT});
                    ZapiszBazeProduktow(bazaProduktow);
                    nowyProdNazwa[0] = '\0'; nowyProdKcal=0; nowyProdB=0; nowyProdW=0; nowyProdT=0;
                }
            }
        }

        ImGui::EndChild();

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        
        // Dynamiczny kolor tla okna GLFW zaleznie od motywu
        if (jasnyMotyw) glClearColor(0.85f, 0.85f, 0.85f, 1.0f);
        else glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        
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
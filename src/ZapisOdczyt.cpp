#include "ZapisOdczyt.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

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
    for(const auto& cw : baza) j.push_back({{"nazwa", cw.nazwa}, {"kategoria", cw.kategoria}, {"trudnosc", cw.trudnosc}, {"opis", cw.opis}});
    ofstream file("data/cwiczenia.json");
    if(!file.is_open()) file.open("../data/cwiczenia.json");
    if(file.is_open()) { file << j.dump(4); return true; }
    return false;
}

bool ZapiszBazeProduktow(const vector<ProduktSpozywczy>& baza) {
    json j = json::array();
    for(const auto& p : baza) j.push_back({{"nazwa", p.nazwa}, {"kcal", p.kcal}, {"bialko", p.bialko}, {"weglowodany", p.weglowodany}, {"tluszcze", p.tluszcze}});
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
        if(d.cwiczenia.empty()) file << "   Odpoczynek (Rest day)\n";
        else for(const auto& cw : d.cwiczenia) file << "   - " << cw.baza.nazwa << " | " << cw.serie << " serie x " << cw.powtorzenia << " powt. | Ciezar: " << cw.ciezar << " kg\n";
        file << "\n";
    }

    file << "[ PLAN DIETETYCZNY ]\n";
    file << "Lacznie: " << user.aktualnaDieta.sumaKcal << " kcal | Bialko: " << user.aktualnaDieta.sumaBialko 
         << "g | Wegle: " << user.aktualnaDieta.sumaWegle << "g | Tluszcze: " << user.aktualnaDieta.sumaTluszcze << "g\n\n";
         
    for(const auto& p : user.aktualnaDieta.posilki) {
        file << ">> " << p.nazwaPosilku << ":\n";
        if(p.skladniki.empty()) file << "   Brak jedzenia\n";
        else {
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
        json j; file >> j;
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
    } catch (...) { cout << "Blad podczas wczytywania profilu." << endl; }
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
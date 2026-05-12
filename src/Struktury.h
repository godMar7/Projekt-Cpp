#pragma once
#include <string>
#include <vector>

struct Cwiczenie {
    std::string nazwa;
    std::string kategoria;
    int trudnosc; 
    std::string opis; 
};

struct ProduktSpozywczy {
    std::string nazwa;
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
    std::string nazwaDnia;
    std::vector<CwiczenieWPlanie> cwiczenia;
};

struct PlanTreningowy {
    std::vector<DzienTreningowy> dni; 
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
    std::string nazwaPosilku; 
    std::vector<SkladnikPosilku> skladniki;
};

struct PlanDietetyczny {
    std::vector<Posilek> posilki;
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
    std::string kategoriaBmi; 
    float zapotrzebowanieKcal;
    PlanTreningowy aktualnyTrening;
    PlanDietetyczny aktualnaDieta;
};
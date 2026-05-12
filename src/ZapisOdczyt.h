#pragma once
#include "Struktury.h"

// Deklaracje funkcji
bool ZapiszProfil(const ProfilUzytkownika& user);
bool ZapiszBazeCwiczen(const std::vector<Cwiczenie>& baza);
bool ZapiszBazeProduktow(const std::vector<ProduktSpozywczy>& baza);
bool EksportujPlanTXT(const ProfilUzytkownika& user);
void WczytajProfil(ProfilUzytkownika& user);
void InicjalizujPusteDniIPosilki(ProfilUzytkownika& user);
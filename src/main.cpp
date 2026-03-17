#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

using namespace std;

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

    // ZMIENNE APLIKACJI (Stan programu)
    float waga = 70.0f;
    float wzrost = 175.0f;
    int wiek = 25;
    int plec = 0; // 0 = Mezczyzna, 1 = Kobieta
    int aktywnosc = 1; // 0 = Niska, 1 = Srednia, 2 = Wysoka
    
    float wyliczoneBmi = 0.0f;
    float wyliczoneBmr = 0.0f;
    float zapotrzebowanie = 0.0f;
    string kategoriaBmi = "";

    // Glowna petla programu
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // NASZ INTERFEJS
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        // Flagi okna ukrywajace pasek tytulu i blokujace przesuwanie
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | 
                                        ImGuiWindowFlags_NoCollapse | 
                                        ImGuiWindowFlags_NoResize | 
                                        ImGuiWindowFlags_NoMove | 
                                        ImGuiWindowFlags_NoBringToFrontOnFocus;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGui::Begin("FitPlanner Workspace", nullptr, window_flags);
        ImGui::PopStyleVar(2);

        if (ImGui::BeginTabBar("GlowneZakladki")) {
            
            // ZAKLADKA 1: PROFIL I BMI
            if (ImGui::BeginTabItem("Profil i BMI")) {
                ImGui::Text("Uzupelnij dane, aby wyliczyc BMI oraz zapotrzebowanie kaloryczne.");
                ImGui::Separator();
                ImGui::Spacing();

                // Dzielimy ekran na 2 kolumny
                ImGui::Columns(2, "kolumny_profil", false); 

                // Lewa kolumna: Wymiary
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Wymiary:");
                ImGui::InputFloat("Waga (kg)", &waga, 1.0f, 5.0f, "%.1f");
                ImGui::InputFloat("Wzrost (cm)", &wzrost, 1.0f, 5.0f, "%.1f");
                ImGui::InputInt("Wiek (lata)", &wiek);

                ImGui::NextColumn();

                // Prawa kolumna: Plec i Aktywnosc
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Plec:");
                ImGui::RadioButton("Mezczyzna", &plec, 0); ImGui::SameLine();
                ImGui::RadioButton("Kobieta", &plec, 1);

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Aktywnosc fizyczna:");
                ImGui::RadioButton("Niska (praca biurowa, brak treningow)", &aktywnosc, 0);
                ImGui::RadioButton("Srednia (trening 3-5x w tygodniu)", &aktywnosc, 1);
                ImGui::RadioButton("Wysoka (codzienne, ciezkie treningi)", &aktywnosc, 2);

                ImGui::Columns(1); 
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Przycisk i logika obliczen
                if (ImGui::Button("Oblicz parametry", ImVec2(200, 45))) {
                    if (wzrost > 0 && waga > 0 && wiek > 0) {
                        float wzrostMetry = wzrost / 100.0f;
                        wyliczoneBmi = waga / (wzrostMetry * wzrostMetry);

                        if (wyliczoneBmi < 18.5f) kategoriaBmi = "Niedowaga";
                        else if (wyliczoneBmi < 25.0f) kategoriaBmi = "Waga w normie";
                        else if (wyliczoneBmi < 30.0f) kategoriaBmi = "Nadwaga";
                        else kategoriaBmi = "Otylosc";

                        if (plec == 0) { // Mezczyzna
                            wyliczoneBmr = (10.0f * waga) + (6.25f * wzrost) - (5.0f * wiek) + 5.0f;
                        } else { // Kobieta
                            wyliczoneBmr = (10.0f * waga) + (6.25f * wzrost) - (5.0f * wiek) - 161.0f;
                        }

                        float mnoznik = 1.2f; // Niska
                        if (aktywnosc == 1) mnoznik = 1.55f; // Srednia
                        if (aktywnosc == 2) mnoznik = 1.725f; // Wysoka
                        zapotrzebowanie = wyliczoneBmr * mnoznik;
                    }
                }

                // Wyswietlanie wynikow
                if (wyliczoneBmi > 0.0f) {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "TWOJE WYNIKI:");
                    ImGui::Text("BMI: %.2f (%s)", wyliczoneBmi, kategoriaBmi.c_str());
                    ImGui::Text("BMR (Podstawowa przemiana materii): %.0f kcal", wyliczoneBmr);
                    ImGui::Text("Zapotrzebowanie na utrzymanie wagi: %.0f kcal", zapotrzebowanie);
                    
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Cel: Redukcja (ok. -300 kcal): %.0f kcal", zapotrzebowanie - 300);
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Cel: Masa (ok. +300 kcal): %.0f kcal", zapotrzebowanie + 300);
                }

                ImGui::EndTabItem();
            }

            // ZAKLADKA 2: TRENING
            if (ImGui::BeginTabItem("Plan Treningowy")) {
                ImGui::Text("Modul ukladania planu treningowego.");
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(W budowie...)");
                ImGui::EndTabItem();
            }

            // ZAKLADKA 3: DIETA
            if (ImGui::BeginTabItem("Dieta i Makro")) {
                ImGui::Text("Modul ukladania diety i liczenia makroelementow.");
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(W budowie...)");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
        // KONIEC INTERFEJSU

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
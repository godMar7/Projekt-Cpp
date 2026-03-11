cmake_minimum_required(VERSION 3.15)

# Nazwa projektu i wersja języka
project(FitPlanner VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Wskazanie, gdzie są pliki nagłówkowe
include_directories(include)

# Dodanie pliku wykonywalnego
add_executable(FitPlanner src/main.cpp)
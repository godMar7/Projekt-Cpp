cmake_minimum_required(VERSION 3.15)
project(FitPlanner VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Konfiguracja GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw)

# Pliki ImGui
set(IMGUI_DIR libs/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Tworzenie aplikacji
add_executable(FitPlanner src/main.cpp ${IMGUI_SOURCES})

# Ścieżki do nagłówków 
target_include_directories(FitPlanner PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    libs/glfw/include
    libs
)

# Linkowanie bibliotek
target_link_libraries(FitPlanner PRIVATE glfw opengl32)
cmake_minimum_required(VERSION 3.25)

project(RocketSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(BUILD_RAYLIB_CPP_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/raylib)
add_subdirectory(external/raylib-cpp)

add_library(rocket_core STATIC
    src/Aerodynamics.cpp
    src/CfdModule.cpp
    src/DesignLibrary.cpp
    src/Environment.cpp
    src/Forces.cpp
    src/NativeFileDialog.cpp
    src/ProjectIO.cpp
    src/RungeKutta4.cpp
    src/SimulationCore.cpp
    src/Validation.cpp
)

target_include_directories(rocket_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(rocket_sim
    src/MeshGenerator.cpp
    src/RocketApp.cpp
    src/SimulationMonitor.cpp
    src/main.cpp
)

target_link_libraries(rocket_sim
    PRIVATE
        rocket_core
        raylib
        raylib_cpp
)

if (WIN32)
    target_link_libraries(rocket_core PUBLIC Comdlg32)
endif()

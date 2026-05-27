cmake_minimum_required(VERSION 3.25)

project(RocketSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ROCKET_ARTIFACTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/artifacts")

include(CTest)

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
    src/SimulationCaches.cpp
    src/SimulationCore.cpp
    src/SimulationEngine.cpp
    src/Validation.cpp
    src/WeatherProvider.cpp
)

target_include_directories(rocket_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(rocket_sim
    src/DebugTelemetry.cpp
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/rlImGui/rlImGui.cpp
    src/app/RocketAppAeroHelpers.cpp
    src/app/RocketAppImGuiHelpers.cpp
    src/app/RocketAppTrajectory.cpp
    src/MeshGenerator.cpp
    src/RocketApp.cpp
    src/SimulationMonitor.cpp
    src/main.cpp
)

set_target_properties(rocket_sim PROPERTIES
    OUTPUT_NAME "RocketLab"
    RUNTIME_OUTPUT_DIRECTORY "${ROCKET_ARTIFACTS_DIR}/$<CONFIG>"
    PDB_OUTPUT_DIRECTORY "${ROCKET_ARTIFACTS_DIR}/$<CONFIG>"
)

target_include_directories(rocket_sim
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/external/rlImGui
        ${CMAKE_CURRENT_SOURCE_DIR}/external/rlImGui/extras
)

target_link_libraries(rocket_sim
    PRIVATE
        rocket_core
        raylib
        raylib_cpp
)

if (WIN32)
    target_link_libraries(rocket_sim PRIVATE Shell32 Psapi Dxgi)
endif()

if (WIN32 AND MSVC)
    target_link_options(rocket_sim PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
elseif (WIN32)
    target_link_options(rocket_sim PRIVATE "-mwindows")
endif()

if (WIN32)
    target_link_libraries(rocket_core PUBLIC Comdlg32 Winhttp)
endif()

if (BUILD_TESTING)
    add_executable(rocket_core_tests
        tests/rocket_core_tests.cpp
    )

    set_target_properties(rocket_core_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${ROCKET_ARTIFACTS_DIR}/tests/$<CONFIG>"
        PDB_OUTPUT_DIRECTORY "${ROCKET_ARTIFACTS_DIR}/tests/$<CONFIG>"
    )

    target_link_libraries(rocket_core_tests
        PRIVATE
            rocket_core
    )

    add_test(NAME rocket_core_tests COMMAND rocket_core_tests)
endif()

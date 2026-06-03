cmake_minimum_required(VERSION 3.24)
project(helm_ui_gtk4 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(HELM_UI_BUILD_TESTS "Build unit tests" ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)

add_executable(helm-ui
    src/app/main.cpp
    src/app/MainWindow.cpp
    src/shell/HelmShell.cpp
    src/shell/TopStatusBar.cpp
    src/shell/BottomNavBar.cpp
    src/shell/CenterViewport.cpp
    src/shell/SensorTile.cpp
    src/graphics/Theme.cpp
    src/graphics/MarineIcons.cpp
    src/telemetry/DummyTelemetrySource.cpp
    src/telemetry/TelemetryStore.cpp
    src/telemetry/Format.cpp
    src/modules/home/HomeModule.cpp
    src/modules/anchor/AnchorModule.cpp
    src/modules/ais/AisModule.cpp
    src/modules/bilge/BilgeModule.cpp
    src/modules/windlass/WindlassModule.cpp
    src/modules/systems/SystemsModule.cpp
    src/nmea/Nmea0183.cpp
)

target_include_directories(helm-ui PRIVATE src)
target_link_libraries(helm-ui PRIVATE PkgConfig::GTKMM)

if(MSVC)
    target_compile_options(helm-ui PRIVATE /W4 /permissive-)
else()
    target_compile_options(helm-ui PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS helm-ui RUNTIME DESTINATION bin)
install(DIRECTORY assets DESTINATION share/helm-ui)

if(HELM_UI_BUILD_TESTS)
    enable_testing()
    add_executable(nmea-tokenizer-test
        tests/nmea_tokenizer_test.cpp
        src/nmea/Nmea0183.cpp
    )
    target_include_directories(nmea-tokenizer-test PRIVATE src)
    if(MSVC)
        target_compile_options(nmea-tokenizer-test PRIVATE /W4 /permissive-)
    else()
        target_compile_options(nmea-tokenizer-test PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME nmea-tokenizer-test COMMAND nmea-tokenizer-test)
endif()

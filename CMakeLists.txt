cmake_minimum_required(VERSION 3.15)
project(wxTranslate)

set(CMAKE_CXX_STANDARD 20)


# Пути к vcpkg
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

find_package(wxWidgets CONFIG REQUIRED)

add_executable(wxTranslate WIN32 main.cpp "translate.cpp" taskbar.cpp "selected_text.cpp" "selected_text.h" utils.cpp "settings.cpp" "lang_panel.h" "lang_panel.cpp")

# ------------------------------------------------------------
# Windows-specific
# ------------------------------------------------------------
if (WIN32)
    target_sources(wxTranslate PRIVATE
        resources/app.rc
    )

    # GUI app (no console)
    set_target_properties(wxTranslate PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

target_link_libraries(wxTranslate PRIVATE wx::core wx::base wx::net wx::adv)

cmake_minimum_required(VERSION 3.15)
project(wxTranslate)

set(CMAKE_CXX_STANDARD 17)

# Пути к vcpkg
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

find_package(wxWidgets CONFIG REQUIRED)

add_executable(wxTranslate WIN32 main.cpp "lang.cpp" "taskbar.cpp")

target_link_libraries(wxTranslate PRIVATE wx::core wx::base wx::net wx::adv)

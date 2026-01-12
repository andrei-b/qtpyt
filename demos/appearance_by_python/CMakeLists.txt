cmake_minimum_required(VERSION 3.16)
project(simple_timer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

add_executable(appearance_by_python
        main.cpp
        mainwindow.h mainwindow.cpp
        appearanceapi.h appearanceapi.cpp
)

target_include_directories(appearance_by_python PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/../../include
)

target_link_libraries(appearance_by_python PRIVATE Qt6::Widgets qtpyt)

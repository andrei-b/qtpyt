cmake_minimum_required(VERSION 3.16)
project(simple_timer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(simple_timer
        main.cpp
        mainwindow.h
        mainwindow.cpp
)

target_link_libraries(simple_timer PRIVATE Qt6::Widgets)

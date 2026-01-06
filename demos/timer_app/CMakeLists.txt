cmake_minimum_required(VERSION 3.16)
project(simple_timer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(timer_app
        main.cpp
        mainwindow.h
        mainwindow.cpp
        pythonrunner.cpp
        pythonrunner.h
        resources.qrc
)

target_include_directories(timer_app PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/pyextra
        ${CMAKE_SOURCE_DIR}/src/internal
        ${CMAKE_SOURCE_DIR}/../../include
)

target_link_libraries(timer_app PRIVATE Qt6::Widgets qtpyt)

qt_add_resources(timer_app "resources"
        PREFIX "/"
        FILES
        slot.py
)
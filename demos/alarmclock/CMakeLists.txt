cmake_minimum_required(VERSION 3.21)
project(alarmclock_widgets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia)

qt_standard_project_setup()

qt_add_executable(alarmclock
        main.cpp
        alarmengine.h alarmengine.cpp
        mainwindow.h mainwindow.cpp
        alarmengine.cpp
        alarmengine.h
        timepanel.cpp
        timepanel.h
)

target_link_libraries(alarmclock PRIVATE Qt6::Core Qt6::Widgets Qt6::Multimedia qtpyt)

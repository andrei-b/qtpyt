cmake_minimum_required(VERSION 3.28)

# Examples are part of the same build, so no `project()` here.

option(QTPYT_BUILD_DEMOS "Build qtpyt demos" ON)

if(NOT QTPYT_BUILD_DEMOS)
    return()
endif()

# Keep binaries grouped under one folder in the build tree.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Common deps for examples (adjust per example if you want finer-grained deps).
find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Gui Qml Quick Widgets Multimedia)
qt_policy(SET QTP0004 NEW)
qt_policy(SET QTP0001 NEW)
qt_standard_project_setup()

add_subdirectory(timer_app)
add_subdirectory(appearance_by_python)
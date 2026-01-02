cmake_minimum_required(VERSION 3.28)

# Examples are part of the same build, so no `project()` here.

option(QTPYT_BUILD_EXAMPLES "Build qtpyt examples" ON)

if(NOT QTPYT_BUILD_EXAMPLES)
    return()
endif()

# Keep binaries grouped under one folder in the build tree.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Common deps for examples (adjust per example if you want finer-grained deps).
find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Widgets)

# Example \#1: background task app
#add_executable(qtpyt_example_background_task
#        background_task/main.cpp
#)

#target_link_libraries(qtpyt_example_background_task PRIVATE
#        qtpyt::qtpyt
#        Qt6::Core
#        Qt6::Widgets
#)

#target_compile_features(qtpyt_example_background_task PRIVATE cxx_std_20)

#set_target_properties(qtpyt_example_background_task PROPERTIES
#        OUTPUT_NAME "qtpyt-example-background-task"
#)

# Optional: install examples
#include(GNUInstallDirs)
#install(TARGETS qtpyt_example_background_task
#        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#)
cmake_minimum_required(VERSION 3.28)

project(qtpyt_demo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6 (with fallback to Qt5)
find_package(Qt6 COMPONENTS Core Widgets Gui QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

# Demo application sources
set(DEMO_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
)

# Create the executable
add_executable(qtpyt_demo ${DEMO_SOURCES})

# Link against Qt and qtpyt library
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(qtpyt_demo PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        qtpyt
    )
else()
    target_link_libraries(qtpyt_demo PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        qtpyt
    )
endif()

# Set include directories
target_include_directories(qtpyt_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Install the demo executable
install(TARGETS qtpyt_demo
    RUNTIME DESTINATION bin
)

# Install example Python scripts
install(DIRECTORY examples/
    DESTINATION share/qtpyt_demo/examples
    FILES_MATCHING PATTERN "*.py"
)

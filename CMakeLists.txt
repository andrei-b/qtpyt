cmake_minimum_required(VERSION 3.28)
project(qtpyt VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build shared library by default
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Enable testing
enable_testing()

# Add the library subdirectory
add_subdirectory(src)

# Add the tests subdirectory
add_subdirectory(tests)

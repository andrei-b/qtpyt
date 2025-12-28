cmake_minimum_required(VERSION 3.28)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
list(PREPEND CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.10.1/gcc_64")

project(qtpyt VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build shared library by default
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Enable testing
enable_testing()

include(FetchContent)
FetchContent_Declare(pybind11 GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v2.13.5)            # choose a tag


FetchContent_MakeAvailable(pybind11)
pybind11_add_module(qembed src/q_embed_meta_object_py.cpp)

find_package(Python314t REQUIRED)

find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Widgets Gui)

find_package(Qt6 OPTIONAL_COMPONENTS CorePrivate)


# Add the library subdirectory
add_subdirectory(src)

# Add the tests subdirectory
add_subdirectory(tests)


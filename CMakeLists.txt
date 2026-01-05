cmake_minimum_required(VERSION 3.28)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
if(DEFINED ENV{QT_DIR})
    list(PREPEND CMAKE_PREFIX_PATH "$ENV{QT_DIR}")
else()
    message(FATAL_ERROR "Environment variable `QT_DIR` not set; please set `QT_DIR` to your Qt installation path (e.g. $HOME/Qt/6.10.1/gcc_64).")
endif()
project(qtpyt VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build shared library by default
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=1)
set(QT_NO_PRIVATE_MODULE_WARNING ON)
# Enable testing
enable_testing()

include(FetchContent)

find_package(Python314t REQUIRED COMPONENTS Development Embedding)
set(Python3_TARGET_VERSION 3.14t)
set (Python_EXECUTABLE ${Python314t_EXECUTABLE})
set (Python3_EXECUTABLE ${Python314t_EXECUTABLE})
set (Python3_INCLUDE_DIRS ${Python314t_INCLUDE_DIR})
set (Python3_LIBRARIES ${Python314t_LIBRARY})
set(PYBIND11_PYTHON_VERSION 3.14 CACHE STRING "python version for pybind11")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PYBIND11_NOPYTHON OFF)
add_subdirectory(third_party/pybind11)

set (QTPYT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (QTPYT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set (QTPYT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Create module (example)
pybind11_add_module(qembed src/q_embed_meta_object_py.cpp)
# pybind11_add_module wires up include/link flags automatically; you can still add link/compile options:
target_include_directories(qembed PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(qembed PRIVATE ${Python3_LIBRARIES})
message(STATUS "Python314t_EXECUTABLE=${Python314t_EXECUTABLE}")
message(STATUS "Python314t_INCLUDE_DIR=${Python314t_INCLUDE_DIR}")
message(STATUS "Python314t_LIBRARY=${Python314t_LIBRARY}")
message(STATUS "Python3_EXECUTABLE=${Python3_EXECUTABLE}")
message(STATUS "Python3_INCLUDE_DIRS=${Python3_INCLUDE_DIRS}")
message(STATUS "Python3_LIBRARIES=${Python3_LIBRARIES}")
message(STATUS "Python3_TARGET_VERSION=${Python3_TARGET_VERSION}")
message(STATUS "pybind11_DIR=${pybind11_DIR}")
message(STATUS "pybind11_INCLUDE_DIRS=${pybind11_INCLUDE_DIRS}")


find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Widgets Gui Test)

find_package(Qt6 OPTIONAL_COMPONENTS CorePrivate)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(demos)
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
list(PREPEND CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.10.1/gcc_64")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(FetchContent)
FetchContent_Declare(pybind11 GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v2.13.5)            # choose a tag
FetchContent_MakeAvailable(pybind11)
pybind11_add_module(qembed q_embed_meta_object_py.cpp)

find_package(Python314t REQUIRED)

find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Widgets Gui)

find_package(Qt6 OPTIONAL_COMPONENTS CorePrivate)



# Define library sources
set(LIBRARY_SOURCES
        q_embed_meta_object.cpp
        q_embed_meta_object.h
        slot.cpp
        slot.h
        q_embed_meta_object_py.cpp
        q_embed_meta_object_py.h
        pyextra/py_time.cpp
        pyextra/py_time.h
        pyextra/py_date.cpp
        pyextra/py_date.h
        pyextra/py_datetime.cpp
        pyextra/py_datetime.h
        qpymodulebase.cpp
        internal/pycall.h
        internal/annotations.cpp
        internal/annotations.h
        conversions.cpp
        q_py_shared_array.cpp
        internal/q_py_execute_event.cpp
        internal/q_py_execute_event.h
        qpymodule.cpp
        q_py_thread.cpp
        internal/q_py_queue.h
        q_py_thread_pool.cpp
        internal/q_py_sub_interpreter.cpp
        internal/q_py_sub_interpreter.h
        q_py_future.cpp
        internal/q_py_future_impl.cpp
        internal/q_py_future_impl.h
)

# Define library headers
set(LIBRARY_HEADERS
    ../include/qtpyt/qpymodulebase.h
    ../include/qtpyt/qpymodule.h
    ../include/qtpyt/q_py_thread.h
    ../include/qtpyt/q_py_shared_array.h
    ../include/qtpyt/q_py_thread_pool.h
    ../include/qtpyt/q_py_future.h
    ../include/qtpyt/conversions.h
    ../include/qtpyt/pep3118format.h
        ../include/qtpyt/qpyannotation.h
)

# Create the shared library
add_library(qtpyt ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})

target_link_libraries(qtpyt
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::Widgets
        Python314t::Python)

# Set include directories
target_include_directories(qtpyt PRIVATE "${pybind11_SOURCE_DIR}/include")
target_include_directories(qtpyt PRIVATE ${Python3_INCLUDE_DIR})
target_include_directories(qtpyt
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Set library properties
set_target_properties(qtpyt PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${LIBRARY_HEADERS}"
)

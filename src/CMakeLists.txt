set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Define library sources
set(LIBRARY_SOURCES
        q_embed_meta_object.cpp
        q_embed_meta_object.h
        qpyslot.cpp
        ../include/qtpyt/qpyslot.h
        q_embed_meta_object_py.cpp
        q_embed_meta_object_py.h
        pyextra/py_time.cpp
        pyextra/py_time.h
        pyextra/py_date.cpp
        pyextra/py_date.h
        pyextra/py_datetime.cpp
        pyextra/py_datetime.h
        qpymodulebase.cpp
        internal/annotations.cpp
        internal/annotations.h
        conversions.cpp
        qpysharedarray.cpp
        internal/q_py_execute_event.cpp
        internal/q_py_execute_event.h
        qpymodule.cpp
        q_py_thread.cpp
        internal/q_py_queue.h
        qpythreadpool.cpp
        internal/q_py_sub_interpreter.cpp
        internal/q_py_sub_interpreter.h
        qpyfuture.cpp
        internal/q_py_future_impl.cpp
        internal/q_py_future_impl.h
        internal/pycall.cpp
        qpyscript.cpp
        PyRunner.cpp
)

# Define library headers
set(LIBRARY_HEADERS
    ../include/qtpyt/qpymodulebase.h
    ../include/qtpyt/qpymodule.h
    ../include/qtpyt/q_py_thread.h
        ../include/qtpyt/qpysharedarray.h
        ../include/qtpyt/qpythreadpool.h
    ../include/qtpyt/qpyfuture.h
    ../include/qtpyt/conversions.h
    ../include/qtpyt/pep3118format.h
    ../include/qtpyt/qpyannotation.h
    ../include/qtpyt/pycall.h
    ../include/qtpyt/qpyscript.h
    ../include/qtpyt/PyRunner.h
        pymodule.cpp
        pymodule.h
        internal/normalize.cpp
        internal/normalize.h
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

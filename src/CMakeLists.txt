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
        internal/qpymoduleimpl.cpp
        internal/normalize.cpp
        qpymemoryview.cpp
        internal/qpymemoryviewinternal.cpp
)

# Define library headers
set(LIBRARY_HEADERS
    ../include/qtpyt/qpymodulebase.h
    ../include/qtpyt/qpymodule.h
    ../include/qtpyt/q_py_thread.h
        ../include/qtpyt/qpythreadpool.h
    ../include/qtpyt/qpyfuture.h
        conversions.h
        pep3118format.h
    ../include/qtpyt/qpyannotation.h
        internal/pycall.h
    ../include/qtpyt/qpyscript.h
        pymodule.cpp
        pymodule.h
        internal/normalize.h
        internal/pyobjectw.h

        internal/qpymoduleimpl.h
        globalinit.cpp
        ../include/qtpyt/globalinit.h
        internal/stringpool.h
        internal/qpymemoryviewinternal.h
        ../include/qtpyt/qpymemoryview.h
        ../include/qtpyt/qpyarray.h
        qpysequencereference.cpp
        ../include/qtpyt/qpysequencereference.h
        ../include/qtpyt/qpysequencewrapper.h
        ../include/qtpyt/qpysharedbytearray.h
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
target_include_directories(qtpyt PRIVATE ${Python314t_INCLUDE_DIR})
target_include_directories(qtpyt PRIVATE "${pybind11_SOURCE_DIR}/include")
set (pybind11_INCLUDE_DIRS "${pybind11_SOURCE_DIR}/include;${Python314t_INCLUDE_DIR}")
message (STATUS  "!!!!Python314t_INCLUDE_DIR=${Python314t_INCLUDE_DIR}")
message (STATUS "!!!!pybind11_SOURCE_DIR=${pybind11_SOURCE_DIR}")
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

set(QTPYT_TARGET qtpyt)

install(TARGETS ${QTPYT_TARGET}
        EXPORT qtpytTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtpyt
)

install(EXPORT qtpytTargets
        FILE qtpytTargets.cmake
        NAMESPACE qtpyt::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qtpyt
)

configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/qtpytConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/../qtpytConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qtpyt
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/qtpytConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/qtpytConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/qtpytConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qtpyt
)

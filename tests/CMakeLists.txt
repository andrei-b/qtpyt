# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable Google Test framework
include(GoogleTest)

# Create test executable
add_executable(qtpyt_tests
        test_qpymodulebase.cpp
        test_qpyfuture.cpp
        test_qpymodule.cpp
        main.cpp
        test_qpyscript.cpp
        test_signalsslots.cpp
        testobject.h
        ../src/internal/normalize.cpp
        ../src/internal/normalize.h
        test_asyncslots.cpp
        TestObject_2.cpp
        TestObject_2.h
        registertypes.cpp
        registertypes.h
        test_conversions.cpp

)

set_target_properties(qtpyt_tests PROPERTIES AUTOMOC ON)


target_sources(qtpyt_tests PRIVATE
        testobject.h
        ../src/internal/qpymoduleimpl.cpp
        ../src/q_embed_meta_object.cpp
        ../src/qpyslot.cpp
        ../src/q_embed_meta_object_py.cpp
        ../src/pyextra/py_time.cpp
        ../src/pyextra/py_date.cpp
        ../src/pyextra/py_datetime.cpp
        ../src/internal/annotations.cpp
        ../src/qpymodulebase.cpp
        ../src/qpysharedarray.cpp
        ../src/internal/q_py_execute_event.cpp
        ../src/qpymodule.cpp
        ../src/q_py_thread.cpp
        ../src/qpythreadpool.cpp
        ../src/qpyfuture.cpp
        ../src/internal/q_py_future_impl.cpp
        ../src/internal/q_py_sub_interpreter.cpp
        ../src/conversions.cpp
        ../src/internal/pycall.cpp
        ../src/qpyscript.cpp
        ../src/pymodule.cpp
        ../src/qpymemoryview.cpp
        ../src/internal/qpymemoryviewinternal.cpp
        ../src/globalinit.cpp
        ../src/pymodule.h
        ../src/conversions.h
        ../src/qpysequencereference.cpp

)

target_include_directories(qtpyt_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/pyextra
        ${CMAKE_SOURCE_DIR}/src/internal
        ${CMAKE_SOURCE_DIR}/../../include
        "${pybind11_SOURCE_DIR}/include"
        ${Python3_INCLUDE_DIR}
        )


# Link against the library and Google Test
target_link_libraries(qtpyt_tests
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Test
        gtest_main
        gtest
        Qt::Core
        Qt::CorePrivate
        Python314t::Python
)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/tests/registertypes.cpp PROPERTIES SKIP_AUTOMOC ON)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/tests/registertypes.h PROPERTIES SKIP_AUTOMOC ON)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/tests/test_conversions.cpp PROPERTIES SKIP_AUTOMOC ON)


# Set include directories

set (Python_EXECUTABLE, ${Python314t_EXECUTABLE})
#target_link_libraries(qtpyt_tests PRIVATE qtpyt gtest_main)
# ensure pybind11 include is available (pybind11 is already FetchContent earlier)
add_test(NAME conversions COMMAND test_conversions)

# Discover tests
gtest_discover_tests(qtpyt_tests)

add_custom_command(TARGET qtpyt_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/pyfiles
        $<TARGET_FILE_DIR:qtpyt_tests>/pyfiles
)

add_test(NAME qpyscript_tests COMMAND qtpyt_tests)
set_tests_properties(qpyscript_tests PROPERTIES
        WORKING_DIRECTORY $<TARGET_FILE_DIR:qtpyt_tests>
)
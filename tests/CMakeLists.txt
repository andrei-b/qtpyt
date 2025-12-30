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
        test_conversions.cpp
        test_qpymodulebase.cpp
        test_qpyfuture.cpp
        test_qpymodule.cpp
        main.cpp
        test_qpyscript.cpp
        test_signalsslots.cpp
        testobject.h
)

set_target_properties(qtpyt_tests PROPERTIES AUTOMOC ON)


target_sources(qtpyt_tests PRIVATE
        ../src/q_embed_meta_object.cpp
        ../src/qpyslot.cpp
        ../src/q_embed_meta_object_py.cpp
        ../src/pyextra/py_time.cpp
        ../src/pyextra/py_date.cpp
        ../src/pyextra/py_datetime.cpp
        ../src/internal/annotations.cpp
        ../src/qpymodulebase.cpp
        ../src/q_py_shared_array.cpp
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
        testobject.h
)

# Link against the library and Google Test
target_link_libraries(qtpyt_tests
    PRIVATE
        qtpyt
        gtest_main
        gtest
        Qt::Core
        Qt::CorePrivate
        Qt6::Test
        Python314t::Python
)

# Set include directories
target_include_directories(qtpyt_tests PRIVATE "${pybind11_SOURCE_DIR}/include")
target_include_directories(qtpyt_tests PRIVATE ${Python3_INCLUDE_DIR})
target_link_libraries(qtpyt_tests PRIVATE qtpyt gtest_main)
# ensure pybind11 include is available (pybind11 is already FetchContent earlier)
target_include_directories(qtpyt_tests PRIVATE "${pybind11_SOURCE_DIR}/include")
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
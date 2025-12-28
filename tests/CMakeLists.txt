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
        ../src/internal/pycall.cpp
)

target_sources(qtpyt_tests PRIVATE
        ../src/q_embed_meta_object.cpp
        ../src/slot.cpp
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
        ../src/q_py_thread_pool.cpp
        ../src/q_py_future.cpp
        ../src/internal/q_py_future_impl.cpp
        ../src/internal/q_py_sub_interpreter.cpp
        ../src/conversions.cpp          # or internal/conversions.cpp
)

# Link against the library and Google Test
target_link_libraries(qtpyt_tests
    PRIVATE
        qtpyt
        gtest_main
        gtest
        Qt::Core
        Qt::CorePrivate
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

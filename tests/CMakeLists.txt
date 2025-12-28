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
    test_pyrunner.cpp
)

# Link against the library and Google Test
target_link_libraries(qtpyt_tests
    PRIVATE
        qtpyt
        gtest_main
        gtest
)

# Discover tests
gtest_discover_tests(qtpyt_tests)

# FindPython314t.cmake
#
# Usage:
#   find_package(Python314t REQUIRED)
#
# Provides:
#   Python314t::Python
#   Python314t_EXECUTABLE
#   Python314t_INCLUDE_DIR
#   Python314t_LIBRARY
#   Python314t_VERSION

find_program(Python314t_EXECUTABLE
    NAMES python3.14t python3.14
    HINTS
        ENV PATH
        /opt/python-3.14t/bin
)

if (NOT Python314t_EXECUTABLE)
    message(FATAL_ERROR "python3.14t executable not found")
endif()

# Query sysconfig from the actual interpreter
execute_process(
        COMMAND ${Python314t_EXECUTABLE} -c
        "import sysconfig, sys; \
print(sys.version.split()[0]); \
print(sysconfig.get_paths()['include']); \
print(sysconfig.get_config_var('LIBDIR')); \
print(sysconfig.get_config_var('LDLIBRARY'))"
        OUTPUT_VARIABLE _pyinfo
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Turn newline-separated output into a CMake list, then extract items
string(REPLACE "\n" ";" _pyinfo_list "${_pyinfo}")
list(GET _pyinfo_list 0 Python314t_VERSION)
list(GET _pyinfo_list 1 Python314t_INCLUDE_DIR)
list(GET _pyinfo_list 2 _py_libdir)
list(GET _pyinfo_list 3 _py_ldlib)

set(Python314t_LIBRARY "${_py_libdir}/${_py_ldlib}")

if (NOT EXISTS "${Python314t_INCLUDE_DIR}/Python.h")
    message(FATAL_ERROR "Python.h not found in ${Python314t_INCLUDE_DIR}")
endif()

if (NOT EXISTS "${Python314t_LIBRARY}")
    message(FATAL_ERROR "libpython not found: ${Python314t_LIBRARY}")
endif()

# Imported target
add_library(Python314t::Python SHARED IMPORTED)
set_target_properties(Python314t::Python PROPERTIES
    IMPORTED_LOCATION "${Python314t_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${Python314t_INCLUDE_DIR}"
)

message(STATUS "Found Python314t:")
message(STATUS "  Executable: ${Python314t_EXECUTABLE}")
message(STATUS "  Version:    ${Python314t_VERSION}")
message(STATUS "  Include:    ${Python314t_INCLUDE_DIR}")
message(STATUS "  Library:    ${Python314t_LIBRARY}")


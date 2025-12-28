# Define library sources
set(LIBRARY_SOURCES
    PyRunner.cpp
)

# Define library headers
set(LIBRARY_HEADERS
    ../include/qtpyt/PyRunner.h
)

# Create the shared library
add_library(qtpyt ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})

# Set include directories
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

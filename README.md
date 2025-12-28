# qtpyt
Embeds multi-threaded Python into the C++ Qt Application

## Building the Project

This project uses CMake as the build system and includes Google Test for unit testing.

### Prerequisites
- CMake 3.14 or higher
- C++20 compatible compiler
- Git (for fetching Google Test)

### Build Instructions

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Configure the project:
```bash
cmake ..
```

3. Build the shared library and tests:
```bash
cmake --build .
```

### Running Tests

After building, you can run the tests using CTest:
```bash
ctest --output-on-failure
```

Or run the test executable directly:
```bash
./tests/qtpyt_tests
```

## Project Structure

- `include/qtpyt/` - Public header files
- `src/` - Source files for the shared library
- `tests/` - Google Test unit tests
- `build/` - Build directory (created during build)

## Library Output

The build process creates a shared library:
- Linux: `libqtpyt.so`
- Windows: `qtpyt.dll`
- macOS: `libqtpyt.dylib`

The library is versioned (e.g., `libqtpyt.so.1.0.0`).

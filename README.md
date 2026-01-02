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

## Generating Documentation

This project uses Doxygen to generate API documentation from source code comments.

### Prerequisites

Install Doxygen on your system:

**Ubuntu/Debian:**
```bash
sudo apt-get install doxygen
```

**macOS:**
```bash
brew install doxygen
```

**Windows:**
Download and install from [https://www.doxygen.nl/download.html](https://www.doxygen.nl/download.html)

### Generate Documentation

From the project root directory, run:
```bash
doxygen Doxyfile
```

This will generate HTML documentation in the `docs/html/` directory.

### View Documentation

Open the generated documentation in your web browser:
```bash
# On Linux
xdg-open docs/html/index.html

# On macOS
open docs/html/index.html

# On Windows
start docs/html/index.html
```

Or simply navigate to `docs/html/index.html` in your file browser and open it with any web browser.

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

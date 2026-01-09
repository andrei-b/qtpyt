![Python 3.14t](https://img.shields.io/badge/Python-3.14t-blue?logo=python&logoColor=white)
![Qt 6](https://img.shields.io/badge/Qt-6-green?logo=qt&logoColor=white)


# QtPyT
Use multithreaded Python in your C++ Qt Application as plugin or scripting language.

## Overview

qtpyt is a powerful integration library that seamlessly bridges Python and Qt/C++ applications, enabling true multi-threaded Python execution within Qt environments. It leverages Python 3.14's free-threaded interpreter to provide genuine parallel execution of Python code alongside Qt's event loop.

## Key Features

- **Bidirectional Integration**: Call Python from C++ and C++ from Python with automatic type conversion
- **Zero copy data transfer** between the Python and C++ code for many types of vectors and arrays
- **True Multithreading**: Thanks to the new Python Free-Threaded interpreters one can execute multiple Python routines simultaneously using free-threaded Python 3.14+
- **Qt Signal/Slot Support**: Connect Python functions directly to Qt signals as slots
- **Asynchronous Execution**: Run Python code in background threads without blocking the Qt event loop
- **Type Safety**: Automatic mapping between Qt types (QString, QVariant, etc.) and Python types
- **Zero-Overhead Interop**: Direct invocation of Qt methods from Python without additional macros or wrappers
- **Script Execution**: Run Python scripts within your Qt application with full access to Qt APIs

## Use Cases

- Extending Qt applications with Python plugins
- Implementing business logic in Python while keeping UI in C++/Qt
- Running CPU-intensive Python computations in parallel threads
- Rapid prototyping of application features using Python
- Integrating Python data science libraries (NumPy, Pandas) with Qt GUIs

# Capabilities
- Call Python routines from Qt/C++ code. The Qt variable types are automapped into Python types and vice versa.
- Connect Python routines as slots to Qt signals
- Call Qt invokable routines from the Python code without any additional preparations (no macros needed)
- Execute Qt scripts
- Call Python routines, slots and execute scripts asynchronously, in separate threads.
- Run multiple Python routines at the same time using true multithreading

## Building the Project
## Building the Project

This project uses CMake as the build system and includes Google Test for unit testing.

### Prerequisites
- QtPyT is written in C++20
- Qt 6.0+ development libraries are
- Free-threaded Python interpreter and development library v3.14+ (you need to make the free-threaded build)
- PyBind11
- CMake 3.14 or higher


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

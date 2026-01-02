# qtpyt Demo Application

A comprehensive Qt demo application that showcases the qtpyt library's Python integration functionality.

## Overview

The qtpyt demo application provides an interactive GUI for executing Python scripts using the qtpyt library. It demonstrates how to embed Python into Qt applications and provides a user-friendly interface for testing Python code execution.

## Features

- **Python Script Editor**: Full-featured text editor with monospace font for writing Python code
- **Python Interpreter Control**: Initialize the Python interpreter with a single click
- **Script Execution**: Execute Python scripts and see the output in real-time
- **Example Scripts**: Pre-written example scripts demonstrating various Python capabilities
- **Error Handling**: Clear display of error messages and execution status
- **Modern UI**: Clean, responsive interface with proper layouts and styling

## Building the Demo

### Prerequisites

- CMake 3.28 or higher
- C++20 compatible compiler
- Qt6 (or Qt5 as fallback)
- Python 3.14t development libraries
- qtpyt library

### Build Instructions

The demo is built automatically when you build the qtpyt project with the `BUILD_DEMO` option enabled (which is ON by default).

1. Configure the project:
```bash
mkdir build
cd build
cmake ..
```

2. Build the project:
```bash
cmake --build .
```

3. The executable will be created at:
```bash
./demo/qtpyt_demo
```

### Disabling Demo Build

If you want to build qtpyt without the demo:
```bash
cmake -DBUILD_DEMO=OFF ..
```

## Running the Demo

After building, run the demo application:

```bash
cd build
./demo/qtpyt_demo
```

Or from the build directory:
```bash
cmake --build . --target qtpyt_demo
./demo/qtpyt_demo
```

## Using the Application

### Step 1: Initialize Python

Click the "Initialize Python" button to initialize the Python interpreter. This must be done before executing any scripts.

### Step 2: Write or Load a Script

You can either:
- Type Python code directly into the script editor
- Load one of the example scripts from the File menu (File → Load Example)

### Step 3: Execute the Script

Click the "Execute Script" button to run your Python code. The output will appear in the Output area below.

### Additional Features

- **Clear Output**: Clears the output area
- **Clear Script**: Clears the script editor (File → Clear Script or Ctrl+L)
- **Exit**: Close the application (File → Exit or Ctrl+Q)
- **About**: View information about the application (Help → About)

## Example Scripts

The demo includes three example Python scripts:

### 1. Hello World (`hello.py`)
A simple script that demonstrates basic Python execution and prints Python version information.

```python
print("Hello from Python!")
print("This is a simple demonstration of Python embedded in Qt.")
```

### 2. Mathematical Operations (`math_demo.py`)
Demonstrates Python's mathematical capabilities including:
- Basic arithmetic operations
- Mathematical functions (sqrt, sin, cos, log, exp)
- Factorial calculation
- Fibonacci sequence generation

### 3. Data Structures (`data_structures.py`)
Shows various Python data structures:
- Lists and list comprehensions
- Dictionaries
- Sets and set operations
- Tuples
- String operations

## Understanding the Code

### PyRunner API

The demo uses the `PyRunner` class, which provides a simple interface to the qtpyt library:

```cpp
// Create a PyRunner instance
qtpyt::PyRunner runner;

// Initialize the Python interpreter
bool success = runner.initialize();

// Execute a Python script
bool executed = runner.executeScript("print('Hello from Python!')");

// Check initialization status
bool initialized = runner.isInitialized();

// Get error messages
QString error = runner.getLastError();
```

### MainWindow Structure

The main window consists of:
- **Script Editor** (QPlainTextEdit): Where users write Python code
- **Control Buttons** (QPushButton): Initialize, Execute, and Clear buttons
- **Output Area** (QTextEdit): Displays execution results and messages
- **Status Label** (QLabel): Shows initialization status
- **Menu Bar**: File and Help menus

### Signal/Slot Connections

The application uses Qt's signal/slot mechanism:
- Button clicks trigger Python operations
- PyRunner emits signals when initialization changes or scripts execute
- MainWindow slots update the UI accordingly

## Troubleshooting

### Python not initialized
**Problem**: Clicking "Execute Script" does nothing or shows an error.
**Solution**: Make sure to click "Initialize Python" first.

### Cannot find example files
**Problem**: Loading examples shows an error.
**Solution**: Example files are located in the `demo/examples/` directory. Make sure to run the demo from the correct working directory or install the application.

### Build errors
**Problem**: The demo fails to build.
**Solution**: Ensure all dependencies are installed (Qt6, Python 3.14t, qtpyt library).

## Architecture

The demo application is structured as follows:

```
demo/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── main.cpp                # Application entry point
├── MainWindow.h            # Main window class declaration
├── MainWindow.cpp          # Main window implementation
└── examples/               # Example Python scripts
    ├── hello.py
    ├── math_demo.py
    └── data_structures.py
```

## Contributing

This is a demonstration application. For issues or improvements to the qtpyt library itself, please refer to the main repository documentation.

## License

This demo application is part of the qtpyt project and follows the same license.

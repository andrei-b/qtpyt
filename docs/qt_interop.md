/**
@page qt_interop The qt_interop Python Module

@section overview Overview

The `qt_interop` module provides Python bindings for interacting with Qt objects from Python scripts.
It enables dynamic method invocation, property access, and object tree navigation using raw C++ pointers.
This module is a part of the QtPyT library, and is created dynamically at run time.

@section usage Usage

Import the module in your Python script:
@code{.py}
import qt_interop
@endcode

@section functions Functions

@subsection invoke_functions Method Invocation
**invoke_from_variant_list(obj_ptr, method, return_value, args)**

Invokes a Qt method with arguments passed as a list of variants.
 - `obj_ptr` (int): Pointer to the QObject as integer
 - `method` (str): Name of the method to invoke
 - `return_value`: Object to store the return value
 - `args` (list): List of arguments

 **invoke(obj_ptr, method, \*args)**

 Invokes a Qt method and returns the result.
 - `obj_ptr` (int): Pointer to the QObject as integer
 - `method` (str): Name of the method to invoke
 - `*args`: Variable arguments to pass to the method
 - Returns: The method's return value

**invoke_async(obj_ptr, method, \*args)**
Invokes a Qt method that returns void asynchronously.
The call returns immediately, the method is executed later in the Qt event loop.
- `obj_ptr` (int): Pointer to the QObject as integer
- `method` (str): Name of the method to invoke
- `*args`: Variable arguments to pass to the method
- Returns: true if the call was placed successfully


**invoke_ret_void(obj_ptr, method, \*args)**

Invokes a Qt method that returns void.
- `obj_ptr` (int): Pointer to the QObject as integer
- `method` (str): Name of the method to invoke
- `*args`: Variable arguments to pass to the method

**invoke_mt(obj_ptr, method, \*args)**

Thread-safe version of invoke() that can be called when the Python code and the Qt object live in different threads.
- `obj_ptr` (int): Pointer to the QObject as integer
- `method` (str): Name of the method to invoke
- `*args`: Variable arguments to pass to the method
- Returns: The method's return value

@subsection property_functions Property Access

**set_property(obj_ptr, property_name, value)**

Sets a Qt object property.
- `obj_ptr` (int): Pointer to the QObject as integer
- `property_name` (str): Name of the property
- `value`: New value for the property

**set_property_async(obj_ptr, property_name, value)**

Sets a Qt object property asynchronously, when the Python code and the Qt object live in different threads.
The function returns immediately, the property is set later in the Qt event loop.
- `obj_ptr` (int): Pointer to the QObject as integer
- `property_name` (str): Name of the property
- `value`: New value for the property

**get_property(obj_ptr, property_name)**

Gets a Qt object property value.
- `obj_ptr` (int): Pointer to the QObject as integer
- `property_name` (str): Name of the property
- Returns: The property value

**get_property_mt(obj_ptr, property_name)**

Thread-safe version of get_property() that can be called that can be called when the Python code and the Qt object live in different threads.
The function blocks until the property value is retrieved.
- `obj_ptr` (int): Pointer to the QObject as integer
- `property_name` (str): Name of the property
- Returns: The property value

@subsection navigation_functions Object Navigation

**find_object_by_name(root_ptr, name, recursive=True)**

Finds a child object by name in the Qt object tree.
- `root_ptr` (int): Pointer to the root QObject as integer
- `name` (str): Name of the object to find
- `recursive` (bool): Whether to search recursively (default: True)
- Returns: Pointer to the found object as integer, or 0 if not found

@section example Example

@code{.py}
import qt_interop

# Assuming root_obj is available as an integer pointer
button = qt_interop.find_object_by_name(root_obj, "myButton")

# Set property
qt_interop.set_property(button, "text", "Click Me")

# Get property
text = qt_interop.get_property(button, "text")

# Invoke method
result = qt_interop.invoke(button, "click")
@endcode

@section notes Notes

- All object pointers are passed as integers (uintptr_t)
- Functions with `_mt` suffix are thread-safe and can be called from any thread
- The module is automatically initialized when Qt Python bindings are loaded
- The GIL (Global Interpreter Lock) is not used by this module
    */

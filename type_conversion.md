/**
@page type_conversion Type Conversion Rules

@tableofcontents

@section overview Overview

qtpyt provides automatic bidirectional type conversion between Qt/C++ and Python types. This page describes the conversion rules and supported types.

@section cpp_to_python C++ to Python Conversion

| C++ Type                | Python Type             | Notes |
|-------------------------|-------------------------|-------|
| `QPoint, QPointF`       | `tuple`                 | |
| `QQuaternion`           | `tuple`                 | |
| `QRect, QRectF` | `tuple`                 | |
| `QSize, QSizeF` | `tuple`                 | |
| `QString`                          | `str`                   | UTF-8 encoding |
| `QVariant`                         | Appropriate Python type | Automatic unpacking |
| `QVector3D, QVector4D`             | `tuple`                 | |
| `int`, `long`                      | `int`                   | |
| `double`, `float`                  | `float`                 | |
| `bool`                             | `bool`                  | |
| `QList<T>`                         | `list`                  | Element conversion applied recursively |
| `QVector<T>`                       | `list`                  | Element conversion applied recursively |
| `QMap<K,V>`                        | `dict`                  | Key and value conversion applied |
| `QByteArray`                       | `bytes`                 | Zero-copy when possible |

@section python_to_cpp Python to C++ Conversion

| Python Type | C++ Type | Notes |
|-------------|----------|-------|
| `str` | `QString` | UTF-8 encoding |
| `int` | `int`, `long`, `qint64` | Range checked |
| `float` | `double`, `float` | |
| `bool` | `bool` | |
| `list` | `QList<QVariant>` | Elements wrapped in QVariant |
| `tuple` | `QList<QVariant>` | Converted to list |
| `dict` | `QMap<QString, QVariant>` | Keys converted to QString |
| `bytes` | `QByteArray` | Zero-copy when possible |
| `bytearray` | `QByteArray` | |
| `memoryview` | `QByteArray` | For large data transfers |
| `None` | `QVariant()` | Invalid/null QVariant |

@section special_types Special Data Types for Zero-Copy Transfer

qtpyt provides special types for efficient transfer of large data between Python and C++:

@subsection qpymemoryview QPyMemoryView

`QPyMemoryView` creates Python memoryviews backed by C++-allocated buffers, enabling zero-copy data sharing:

```cpp
// C++ side
QPyMemoryView buffer('d', 1000000);  // 1M doubles
// Fill buffer...
pythonFunc(buffer.memoryview());  // Pass to Python without copying

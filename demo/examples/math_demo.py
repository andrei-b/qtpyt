#!/usr/bin/env python3
"""
Mathematical operations example demonstrating Python's computation capabilities.
"""

import math

print("=== Mathematical Operations Demo ===")
print()

# Basic arithmetic
a, b = 10, 3
print(f"Basic Arithmetic:")
print(f"  {a} + {b} = {a + b}")
print(f"  {a} - {b} = {a - b}")
print(f"  {a} * {b} = {a * b}")
print(f"  {a} / {b} = {a / b:.2f}")
print(f"  {a} ** {b} = {a ** b}")
print()

# Math functions
print(f"Math Functions:")
print(f"  sqrt(16) = {math.sqrt(16)}")
print(f"  sin(π/2) = {math.sin(math.pi/2):.4f}")
print(f"  cos(π) = {math.cos(math.pi):.4f}")
print(f"  log(10) = {math.log(10):.4f}")
print(f"  e^2 = {math.exp(2):.4f}")
print()

# Computing factorial
n = 5
factorial = math.factorial(n)
print(f"Factorial: {n}! = {factorial}")
print()

# Computing fibonacci sequence
def fibonacci(n):
    a, b = 0, 1
    result = []
    for _ in range(n):
        result.append(a)
        a, b = b, a + b
    return result

fib_seq = fibonacci(10)
print(f"First 10 Fibonacci numbers: {fib_seq}")

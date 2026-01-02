#!/usr/bin/env python3
"""
Data structures example demonstrating Python lists, dictionaries, and more.
"""

print("=== Python Data Structures Demo ===")
print()

# Lists
print("Lists:")
fruits = ["apple", "banana", "cherry", "date"]
print(f"  Original list: {fruits}")
print(f"  First item: {fruits[0]}")
print(f"  Last item: {fruits[-1]}")
print(f"  Length: {len(fruits)}")
fruits.append("elderberry")
print(f"  After append: {fruits}")
print()

# List comprehension
squares = [x**2 for x in range(1, 6)]
print(f"  Squares (1-5): {squares}")
print()

# Dictionaries
print("Dictionaries:")
person = {
    "name": "Alice",
    "age": 30,
    "city": "New York",
    "profession": "Engineer"
}
print(f"  Person data: {person}")
print(f"  Name: {person['name']}")
print(f"  Keys: {list(person.keys())}")
print(f"  Values: {list(person.values())}")
print()

# Sets
print("Sets:")
set_a = {1, 2, 3, 4, 5}
set_b = {4, 5, 6, 7, 8}
print(f"  Set A: {set_a}")
print(f"  Set B: {set_b}")
print(f"  Union: {set_a | set_b}")
print(f"  Intersection: {set_a & set_b}")
print(f"  Difference (A-B): {set_a - set_b}")
print()

# Tuples
print("Tuples:")
coordinates = (10.5, 20.3, 30.7)
print(f"  Coordinates: {coordinates}")
print(f"  X: {coordinates[0]}, Y: {coordinates[1]}, Z: {coordinates[2]}")
print()

# Working with strings
print("String Operations:")
text = "Python in Qt"
print(f"  Original: '{text}'")
print(f"  Uppercase: '{text.upper()}'")
print(f"  Lowercase: '{text.lower()}'")
print(f"  Split: {text.split()}")
print(f"  Replace: '{text.replace('Python', 'qtpyt')}'")

from array import array
import sys
import time


def func_1(x, y):
    time.sleep(1)
    return x + y


def func_2():
    return 1


def scale_array(arr, factor):
    for i in range(len(arr)):
        arr[i] = arr[i] * factor
    for i in range(len(arr)):
        if arr[i] != i * factor:
            raise RuntimeError("Memory check error")


def make_array(size):
    arr = array('i', (0 for _ in range(size)))
    for i in range(size):
        arr[i] = i
    return arr

def summ_array(arr):
    total = 0
    for value in arr:
        total += value
    return total
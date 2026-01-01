from array import array
import sys

def make_memoryview():
    buf = array('i', [10, 20, 30, 40])
    mv = memoryview(buf)
    return mv

def make_memoryview_2():
    buf = array('d', [1.0, 2.0, 3.0, 4.0])
    mv = memoryview(buf)
    return mv
import qembed

v = qembed.invoke(qembed.root_obj, 'getInts')

print("Returned ints:", v)

if (v != (6, 7, 8, 9, 0)):
    raise RuntimeError("Unexpected return value")
else:
    print("Return value is as expected")


u = qembed.invoke(qembed.root_obj, 'retMap')

print("Returned map:", u)

if (u != {1: 'Apple', 2: 'Orange', 3: 'Banana'}):
    raise RuntimeError("Unexpected return value")
else:
    print("Return value is as expected")
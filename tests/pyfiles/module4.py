import qembed

v = qembed.invoke(qembed.root_obj, 'mulVect', (2, 3), 2)
print("Returned vect:", v)

if (v != (4, 6)):
    raise RuntimeError("Unexpected return value")
else:
    print("Return value is as expected")

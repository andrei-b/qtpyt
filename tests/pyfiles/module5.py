import qt_interop

qt_interop.set_property(root_obj, 'intProperty', 111)

qt_interop.set_property(root_obj, 'value', (24,56))

v = qt_interop.get_property(root_obj, 'value')

print("Got property 'value':", v)

if (v != (24, 56)):
    raise RuntimeError("Unexpected property value")

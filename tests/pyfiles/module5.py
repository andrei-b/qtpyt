import qembed

qembed.set_property(qembed.root_obj, 'intProperty', 111)

qembed.set_property(qembed.root_obj, 'value', (24,56))

v = qembed.get_property(qembed.root_obj, 'value')

print("Got property 'value':", v)

if (v != (24, 56)):
    raise RuntimeError("Unexpected property value")

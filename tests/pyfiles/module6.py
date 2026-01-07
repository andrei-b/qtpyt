import qt_interop

def slot(param):
    qt_interop.set_property(obj, 'intProperty', param[0] + param[1])
    print("Slot called with param:", param)

def slot_2(param):
    qt_interop.set_property(obj, 'value', param)

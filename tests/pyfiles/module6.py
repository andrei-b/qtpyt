import qembed

def slot(param):
    qembed.set_property(obj, 'intProperty', param[0] + param[1])
    print("Slot called with param:", param)
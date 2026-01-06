import qembed
import time
def on_timer_elapsed():
    print("fires")
    qembed.set_property_async(main_window, "styleSheet", "background-color: lightblue;")
    time.sleep(2)
    qembed.set_property_async(main_window, "styleSheet", "background-color: white;")
    time.sleep(2)
    qembed.set_property_async(main_window, "styleSheet", "background-color: lightgreen;")
    time.sleep(2)
    qembed.set_property_async(main_window, "styleSheet", "background-color: gray;")
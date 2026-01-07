import qt_interop
import time
def on_timer_elapsed():
    qt_interop.set_property_async(main_window, "styleSheet", "background-color: lightblue;")
    time.sleep(2)
    qt_interop.set_property_async(main_window, "styleSheet", "background-color: white;")
    time.sleep(2)
    qt_interop.set_property_async(main_window, "styleSheet", "background-color: lightgreen;")
    time.sleep(2)
    qt_interop.set_property_async(main_window, "styleSheet", "background-color: gold;")
    time.sleep(2)
    qt_interop.set_property_async(main_window, "styleSheet", "background-color: gray;")
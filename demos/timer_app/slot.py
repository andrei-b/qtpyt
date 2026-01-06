import qembed
import time
def on_timer_elapsed():
    print("fires")
    qembed.set_property(main_window, "styleSheet", "{background-color: lightblue;}")
    time.sleep(500)
    qembed.set_property(main_window, "styleSheet", "background-color: white;")
    time.sleep(500)
    qembed.set_property(main_window, "styleSheet", "background-color: lightgreen;")
    time.sleep(500)
    qembed.set_property(main_window, "styleSheet", "background-color: gray;")
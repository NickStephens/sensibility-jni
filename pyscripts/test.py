import androidembed
import sensors
import time

androidembed.log("Python saying HELLO!")

while True:
  vals = sensors.poll_event() 
  androidembed.log(str(vals))

androidembed.log("Done gathering sensor data!")

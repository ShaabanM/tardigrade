
import serial
import datetime
from datetime import timezone 
import time

# Set up serial ports
inport = serial.Serial('/dev/ttyUSB0', 115200)
outport = serial.Serial('/dev/ttyS0', 115200)

# Set up frequency of test loop
FREQ = 10 #Hz
CYCLETIME = 1/FREQ *1000 #ms         

while 1:

    start = time.time_ns() / 1000

    # perform the serial loopback
    outport.write(b'0100011101')
    rbytes = inport.read(10)
        
    # write time and bytes to file
    now = time.time_ns() / 1000
    # [time, recieved]
    print(now, rbytes)

    # sleep to ensure total time is correct cadance
    time.sleep((CYCLETIME - (now-start))/1000)


 



import serial
import datetime
from datetime import timezone 
import time
import os
import csv

test_name = "usb2serial"

# Set up serial ports
inport = serial.Serial('/dev/ttyUSB0', 115200)
outport = serial.Serial('/dev/ttyS0', 115200)

# Set up frequency of test loop
FREQ = 10 #Hz
CYCLETIME = 1/FREQ *1000 #ms      
first = True   

while 1:

    start = time.time_ns() / 1000

    # perform the serial loopback
    outport.write(b'0100011101')
    rbytes = inport.read(10)
        
    # write time and bytes to file
    now = time.time_ns() / 1000

    # [time, recieved]
    print(now, rbytes)

    # add data products here
    data = {
        'time': [now], 
        'read_bytes': [rbytes]}

    # write stuff
    keys = sorted(data.keys())
    with open(os.path.join("data", test_name +'.csv'), 'a', newline='') as csv_file:
        writer = csv.writer(csv_file)
        if first:
            writer.writerow(keys)
            first = False
        writer.writerows(zip(*[data[key] for key in keys]))

    # sleep to ensure total time is correct cadance
    time.sleep((CYCLETIME - (now-start))/1000)






 


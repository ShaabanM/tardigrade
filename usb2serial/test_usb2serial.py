#!/home/luvs/anaconda3/bin/python3

import serial
import time
import os
import csv

test_name = "usb2serial"

# Set up serial ports
inport = serial.Serial('/dev/ttyUSB0', 115200)
outport = serial.Serial('/dev/ttyS0', 115200)

# Set up frequency of test loop
FREQ = 100 #Hz
CYCLETIME = 1./FREQ #ms      
first = True   
count = 0
starttime = time.time_ns() / 1.0e9

while 1:

    start = time.time_ns() / 1.0e9

    # perform the serial loopback
    outport.write(b'0100011101')
    rbytes = inport.read(10)
        
    # write time and bytes to file
    now = time.time_ns() / 1.0e9

    # add data products here
    data = {
        'time': [now], 
        'read_bytes': [rbytes]}

    # write stuff
    keys = sorted(data.keys())
    with open(os.path.join("data", test_name + '_'  + str(starttime) + '.csv'), 'a', newline='') as csv_file:
        writer = csv.writer(csv_file)
        if first:
            writer.writerow(keys)
            first = False
        writer.writerows(zip(*[data[key] for key in keys]))

    # sleep to ensure total time is correct cadance
    dt = (now-start)
    if dt < CYCLETIME:
        time.sleep((CYCLETIME - dt))

    if count % FREQ == 0:
        print("Heartbeat ", rbytes, ", ", now)

    count = count + 1






 


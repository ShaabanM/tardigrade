#!/home/luvs/anaconda3/bin/python3
import serial
import time
import os
import csv

test_name = "ard" + str(time.time())

# Set up serial ports
inport = serial.Serial('/dev/ttyACM0', 115200)
first=True
inport.reset_input_buffer()

while 1:
    rbytes = inport.readline()

    # write time and bytes to file
    now = time.time()
    rbytes = rbytes.decode('utf-8')

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

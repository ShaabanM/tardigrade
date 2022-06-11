#!/home/luvs/anaconda3/bin/python3
# -*- coding: utf-8 -*-

'''
test_disks.py

This script is intended to poll disks
and record related data
'''

#############################################################
# IMPORT MODULES
#############################################################
import os
import sys
import csv
import time
import json
import psutil
from datetime import datetime
from pySMART import Device

device_name = "/dev/sdd"
test_name = "evo_pre"

#############################################################
# SUPPORT FUNCTIONS
#############################################################
def get_device_number(name):
    disks = psutil.disk_partitions()
    for i, disk in enumerate(disks):
        if name == disk.device:
            return i
    raise ValueError('Device with the name ' + name + ' was not found') 

def disk_test(data_dirname):

    # load inputs
    with open('data.json') as f:
        data = json.load(f)
    data_save_interval = data['data_save_interval']
    test_cycle_time = data['test_cycle_time']

    # define vars
    ttime = []
    num_detected_disks = []
    disk_space_total = []
    disk_space_used = []
    disk_space_free = []
    disk_space_used_pct = []
    disk_info = []
    temp = []

    print(str(time.time()) + ': starting disk monitor!')

    disks = psutil.disk_partitions()
    old_num_detected_disks = len(disks)
    disk = Device(device_name)

    # probably create list of drives here for data storage

    start = time.time()
    while True:

        time.sleep(test_cycle_time)
        end = time.time()

        ttime += [time.time()]
        disks = psutil.disk_partitions()
        num_detected_disks += [len(disks)]

        # dynamically get device number for given name
        idx = get_device_number(device_name)

        disk_space_total += [psutil.disk_usage(disks[idx][0])[0]]
        disk_space_used += [psutil.disk_usage(disks[idx][0])[1]]
        disk_space_free += [psutil.disk_usage(disks[idx][0])[2]]
        disk_space_used_pct += [psutil.disk_usage(disks[idx][0])[3]]
        disk_info += [disks[idx]]
        temp += [disk.temperature]

        if num_detected_disks[-1] != old_num_detected_disks:
            print('\n\n     NUMBER OF DISKS HAS CHANGED!\n\n')
            print(old_num_detected_disks)
            print(num_detected_disks)
            old_num_detected_disks = num_detected_disks

        if end-start > data_save_interval:
            time1 = time.time()

            # add data products here
            data = {
                'time': ttime, 
                'num_detected_disks': num_detected_disks,
                'disk_space_total': disk_space_total,
                'disk_space_used': disk_space_used,
                'disk_space_free':disk_space_free,
                'temp':temp,
                'info': disk_info}

            now = str(datetime.now())
            now = now.split('.')
            now = now[0]
            now = now.replace(' ', '_')
            now = now.replace(':', '-')

            # write stuff
            keys = sorted(data.keys())
            with open(os.path.join(data_dirname, now+test_name +'_disk_log.csv'), 'a', newline='') as csv_file:
                writer = csv.writer(csv_file)
                writer.writerow(keys)
                writer.writerows(zip(*[data[key] for key in keys]))

            # reset vars
            ttime = []
            num_detected_disks = []
            disk_space_total = []
            disk_space_used = []
            disk_space_free = []
            disk_space_used_pct = []
            disk_info = []
            temp = []

            # reset time
            start = time.time()


#############################################################
# MAIN CODE
#############################################################
if __name__ == '__main__':

    try:
        data_dirname = sys.argv[1]
    except:
        data_dirname = '../data/demo'
    if os.path.exists(os.path.join(data_dirname)):
        pass
    else:
        os.makedirs(os.path.join(data_dirname))
    
    #print(data_dirname)
    disk_test(data_dirname)

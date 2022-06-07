#!/home/luvs/anaconda3/bin/python3
# -*- coding: utf-8 -*-

'''
start_tests.py

This script is intended to trigger and monitor
scripts related to radiation testing
'''

#############################################################
#IMPORT MODULES
#############################################################
import os
import sys
import time
import json
import subprocess
from datetime import datetime


#############################################################
# USER INPUT
#############################################################
ram_pct_to_use     = 0  #%, RAM to be consumed and monitored by test program
test_cycle_time    = 0.1 #seconds, delay between system data checks
data_save_interval = 5   #seconds, length of time of each data file


#############################################################
# MAIN CODE
#############################################################

#create directory for data
data_dir =   'data'
if os.path.exists(data_dir):
    pass
else:
    os.mkdir(data_dir)

init_time = str(datetime.now())
init_time = init_time .split('.')
init_time = init_time [0]
init_time = init_time .replace(' ','_')
init_time = init_time .replace(':','-')
the_dir = os.path.join(data_dir,  init_time )
os.mkdir(the_dir)

#save off parameters that the test scripts use
input_data = {'ram_pct_to_use':ram_pct_to_use, 'test_cycle_time': test_cycle_time, 'data_save_interval':data_save_interval}

with open('data.json', 'w', encoding='utf-8') as f:
    json.dump(input_data, f, ensure_ascii=False, indent=4)

with open(os.path.join(the_dir,'data.json'), 'w', encoding='utf-8') as f:
    json.dump(input_data, f, ensure_ascii=False, indent=4)

#start logging processes
subprocess.Popen([sys.executable,os.path.abspath('test_networks.py'),str(the_dir)], stdin=None, stdout=None, stderr=None)
subprocess.Popen(["bash", "ping_test.sh"],stdin=None, stdout=None, stderr=None)



while True:
    time.sleep(1)
    print("%.2f (heartbeat)" % time.time())#+' (heartbeat)')




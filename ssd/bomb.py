import numpy as np   
import sys

save_path = "test_" # location to save the data dumps (should be on the ssd being beamed)
total_dump = 100 #Total number of kilobytes to save
size_per_dump = 10 # Size of each file dump
n_per_kb = 1000/25 # Conversion from numpy ones to kb 

def explode():
    for i in range(int(total_dump/size_per_dump)):
        np.savetxt(save_path+str(i), np.ones(int(size_per_dump*n_per_kb),dtype=float))

def implode():
    for i in range(int(total_dump/size_per_dump)):
        np.savetxt(save_path+str(i), np.zeros(int(size_per_dump*n_per_kb),dtype=float))


#############################################################
# MAIN CODE
#############################################################
if __name__ == '__main__':

    try:
        n_flush = int(sys.argv[1])
    except:
        n_flush = 2

    explode()

    for i in range(n_flush):
        implode()
        explode()
        print("flushed")
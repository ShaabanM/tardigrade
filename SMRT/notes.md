# SMRT Notes

## Workflow (Repeat Before, During, After Beam)

- This test will run on all beam computers are they are being beamed
- Simply run `sudo python3 start_tests.py` on the intended machine
  - note the code is not identical on all machines due to variblity in OS (Ubuntu 18 vs 20)
- Run an rsync script that will in real time copy over the data from the machine being beamed to the support computer
- If the computer also has IO (e.g. raspberry pi) check relevant computer directory for more tests

## Installing SMRT

- Begin by going to the setup dir and runing the install tool python script
- some parts failed due to not finding `pip3`
- note that the tool will say install complete even if not LOL
- ok the issue was `sudo pip` was for some reason invalid had to change it in the script to be just `pip3` and that worked!
  - note all the dependencies were already on my personal machine since they seem to be all the basic ones, the only new dep is opencv_python
- sas

## start_tests.py

This script starts all of the test scripts and provides a regular, 1 Hz print to the terminal to provide a "heartbeat" to indicate if the computer/interface is still functioning. This script also contains the aforementioned user inputs.

## test_ram.py

This script starts writing 1s to a struct until a user-defined amount of RAM is consumed. Each cycle, the script will check the struct to make sure it's consistent. If it's not, it will log an 'upset' and will print _RAM STATE CHANGE DETECTED_. The RAM will not be overwritten to reset it. Sometimes restarting the script can reset upsets, but sometimes the whole device must be restarted. At other times, especially after the unit under test has been subject to a considerable particle flux, upsets may trigger after restarting even when the particle beam is off. The script also records the percentage of RAM used.

## test_cpu.py

This script records the percentage of CPU used, current frequency in MHz, and CPU temperature in C (temperature is Linux-only).

## test_disks.py

This script records the number of disks/drives detected and will print _NUMBER OF DISKS HAS CHANGED!_ if a change in the number of drives is detected. This script also records some information regarding the drives.

## test_networks.py

This script records the number of network adapters detected and will print _NUMBER OF NETWORK ADAPTERS HAS CHANGED!_ if a change in the number of adapters is detected. This script also records some information regarding the adapters.

## plot_results.py

This script concatenates and loads all data in the most recently created data folder. It then plots the data and, depending on a user-set flag, can also save the plots and pngs.

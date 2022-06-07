# SMRT Notes

## Workflow (Repeat Before, During, After Beam)

- This test will run on all beam computers are they are being beamed
  - Namely: pcm, rpi, ark

### Pre-setup
- Connect computer
  - pcm: 5V power to power supply
  - ark: 12V power to wall plug through adapter
  - rpi: 5V USB power from STU/GSE ARK
- Connect ethernet to Ethernet Switch
  - pcm: Ethernet dongle (5x2 header to the right of the PCI bus)
  - ark: Ethernet port 1 (ARK)
  - rpi: Ethernet port (only 1)
- Login to computer by name
  - All computers have username sst, password sst (should be passwordless login)
  - Name is simple pcm, ark, or rpi (all aliased in hosts from STU/GSE ARK)
- Login to computer by name in another terminal
  - Need at least 2 terminals for test
- If the computer also has IO (e.g. pcm with mesafpga) check relevant directory for more setup.

### Test
- Simply enter the `py_src` directory and run `sudo python3 start_tests.py` on the intended machine
  - note the code is not identical on all machines due to variblity in OS (Ubuntu 18 vs 20)
- In a separate terminal:
  - Run `./rsync.sh [computer_name] > /dev/null 2>&1 &` to start rsync to STU/GSE ARK.
  - This is run in the background, and only as a backup for local data
  - To kill, run `sudo pkill inotifywait`
- If the computer also has IO (e.g. pcm with mesafpga) check relevant directory for more tests.

### Post-test
- Stop test (Ctrl+C)
- Stop any other tests specific to the computer
- Stop background rsync (`sudo pkill inotifywait`)
- Make sure to back up data from `./remotedata/` on STU/GSE ARK to personal laptop or hard drive
  - e.g. `rsync -avP ./remotedata/ user@laptop:/path/to/backup`
  - IMPORTANT: This is done from STU/GSE ARK regardless of computer being tested (due to rsync)

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

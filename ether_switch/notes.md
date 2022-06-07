# Ethernet Switch

## Workflow (Repeat Before, During, After Beam)

### Pre-test setup
- Power Ethernet Switch via 12V power supply
- Connect one port of the Ethernet Switch to the GSE2 port of the STU/GSE ARK
- Connect a 2nd port to the Backup Switch not in the beamline
- Connect the GSE port of the STU/GSE ARK to the Backup Switch
- Route a port from the Backup Switch to the control room.
- Final setup:
  - Control Room <--> Backup Switch <--> GSE Port <--> STU/GSE ARK
  -                         |                               |
  -                   Ethernet Switch <------------->   GSE2 Port
- On personal computer in control room, set the ping IP to the GSE2 port (192.168.1.21)
  - Also set a descriptive `NAME` for the file to be saved.

### Test
- Run the test using `python3 run.py`
  - This will run the SMRT network tracker as well as a ping test
  - In case of Ethernet Switch failure, STU/GSE ARK is still accessible via GSE port on backup

### Post-test
- Stop the run script (Ctrl+C)
- Make sure to back up / rsync data from `./data/` to personal laptop or hard drive
  - e.g. `rsync -avP ./data/ user@laptop:/path/to/backup`

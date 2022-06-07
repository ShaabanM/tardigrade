# Arduino

## Workflow (Repeat Before, During, After Beam)

### Pre-test setup
- Connect the arduino pins to each other in feedback as follows:
  - PWM feedback:   2--3, 4--5, 6--7, 8--9, 10--12
  - PWM ext. read:  11 -- Scope 1+/M1 probe, GND -- Scope 1-/M1 clip
  - Analog reading: A0-A5 -- Scope W1 probe, GND -- Scope W1 clip
- Connect Arduio to STU/GSE ARK via long USB cable
- Load Digilent Scope `arduino_test` waveforms workspace
  - Enable waveform output and scope input

#### Arduino program (if needed, should load by default)
- Launch Arduino IDE: `sudo arduino` on STU/GSE ARK
- Open `test/test.ino`
- Verify (checkmark)
- Upload


### Test
- The main reader script is run on STU/GSE ARK: `sudo python3 reader.py`
  - A new directory in `./data/` will be created; heartbeat in terminal
- Set the scope on the Digilent Scope to record data over the length of the test (plus margin)
- Start scope recording when test has begun


### Post-test
- On Digilent Scope, File->Save Acquisition to save timestream data as .csv
  - Put in the `./data/[test_date]` directory corresponding to the test.
- Stop the reader script (Ctrl+C)
- Make sure to back up / rsync data from `./data/` to personal laptop or hard drive
  - e.g. `rsync -avP ./data/ user@laptop:/path/to/backup`

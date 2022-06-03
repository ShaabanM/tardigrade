# Arduino

## Workflow (Repeat Before, During, After Beam)

- Connect the arduino pins to each other as follows
  - 2--3 4--5 6--7 8--9 10--12
  - 11 -- scope channell 1
- set up the scope to save the the incoming data
- connect pin A0-A5 to the output of the function generator
- connect the device to support ark via long usb cable
- run the reader script with `sudo python3 reader.py`

If need to repogram:

- launch arduino IDE as sudo with `sudo arduino`
- copy the code in `test.ino` into the IDE and upload code

# Serial2USB Tests

## Workflow (Repeat Before, During, After Beam)

### Pre-test
- Connect serial-USB loopback on STU/GSE ARK
  - USB3 <-> USB-serial-converter <-> COM Port (`/dev/ttyS0` i.e. COM 1)
- Check the name of the USB device on STU/GSE ARK (should be `/dev/ttyUSB0`)
  - Set the device names in `test_usb2serial.py`
  - One of them should be `/dev/ttyS0` for COM port
- Set the test name in `test_usb2serial.py`

### Test
- Run the program `python3 test_usb2serial.py` for during test
  - This creates a new folder in `./data/` based on the test_name and date

### Post-test
- Stop the loopback test (Ctrl+C)
- Make sure to back up / rsync data from `./data/` to personal laptop or hard drive
  - e.g. `rsync -avP ./data/ user@laptop:/path/to/backup`

# SSD Tests

## Workflow (Repeat Before, During, After Beam)

- Attach ssd to support ARK via sata<->usb converter
- Find out ssd device name (probably `/dev/sdb`)
- Run smartcal long test `sudo smartctl /dev/[device-name] --test=long`
- Write results of `sudo smartctl /dev/[device-name] -a` to disk
- Set the disk name in `ssd/test_disks.py`
- Run the code continously for time required to dose `sudo python3 run.py`
- Back up date from support computer to personal computer or to git

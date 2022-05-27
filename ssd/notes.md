# SSD Tests

## Workflow (Repeat Before, During, After Beam)

- Run smartcal long test `sudo smartctl /dev/[device-name] --test=long`
- Write results of `sudo smartctl /dev/[device-name] -a` to disk
- Set the disk name in `ssd/test_disks`
- Run the code continously for 15 minutes `sudo python3 run.py`

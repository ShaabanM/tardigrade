# SSD Tests

## Workflow (Repeat Before, During, After Beam)

- Attach ssd to support ARK via sata<->usb converter
- Find out ssd device name (probably `/dev/sdb`)
- Run smartcal long test `sudo smartctl /dev/[device-name] --test=long`
- Write results of `sudo smartctl /dev/[device-name] -a > data/[descriptive_name]` to disk
- Set the disk name in `ssd/test_disks.py`
- Run the code continously for time required to dose `sudo python3 run.py`
    - You may have to mount the disk to /media/[disk_name] for partitions to be recognized (`sudo mount /dev/sdb[#] /media/[disk_name]`)
- Back up date from support computer to personal computer or to git

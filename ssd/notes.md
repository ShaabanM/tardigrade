# SSD Tests

## Workflow (Repeat Before, During, After Beam)

### Pre-test Setup
- Attach SSD to STU/GSE ARK USB3 via SATA->USB converter
  - IMPORTANT: make sure the converter/enclosure is away from beam by using SATA extender
- Find out ssd device name (probably `/dev/sdb`)
  - `dmesg` may have a hint as to recently connected devices
- Set the disk name in `./test_disks.py`
- Mount the disk to a location on the STU/GSE ARK at `/media/[disk_name]`
  - For PCIE, mount block device (e.g. `/dev/sdb`)
  - For all others, mount partition (e.g. `/dev/sdb1')
  - e.g. `sudo mount /dev/sdb[#] /media/[disk_name]`

### Test

#### Baseline test (very long to run)
- Run smartcal long test `sudo smartctl /dev/[device-name] --test=long`
- Write results of `sudo smartctl /dev/[device-name] -a > data/[descriptive_name]` to disk

#### Beam test
- Run the code continously for time required to dose `sudo python3 run.py`
  - If error about not finding device with name, check mounting from Pre-test and restart test
  - This will create a new folder in `./data/` corresponding to test start date

### Post-test
- Stop the smart test (Ctrl+C)
- Make sure to back up / rsync data from `./data/` to personal laptop or hard drive
  - e.g. `rsync -avP ./data/ user@laptop:/path/to/backup`

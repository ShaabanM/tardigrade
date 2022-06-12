# Bassler Camera

## Workflow (Repeat Before, During, After Beam)

### Pre-test setup
- Connect the Basler camera to STU/GSE ARK via long blue USB3 cable
  - This sets up data and power.

### Test
- Test is run by simply running `sudo ./run.sh > data/[name of test.log] 2>&1 &` on STU/GSE ARK
  - This tests at 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000 ms exposure times (2 exposures each)
  - One file per exposure is written to disk in `./data` with exposure time, frame num, date
  - Script only takes a few seconds to run; there is no continuous script during test.
  - Can check on tests with `tail -f data/[name of test].log`
- Stop test with `sudo pkill run_basler.sh`

### Optional monitoring
- During beamline test, if desired, frames can be observed and recorded using `./pylonviewer`
  - Must be SSH'd into STU/GSE ARK with `-YC` (e.g. `ssh luvs@stu -YC`) for GUI access

### Post-test
- Make a new directory in `./data` for the current data: `mkdir data/[descriptive name w/ date]`
- Move all new image files from `./data` to the new directory.
  - e.g. `mv data/*.fits data/[descriptive name w/ date]`
- Make sure to back up / rsync data from `./data/` to personal laptop or hard drive
  - e.g. `rsync -avP ./data/ user@laptop:/path/to/backup`

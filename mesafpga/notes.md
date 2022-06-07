# Mesa Test

## Workflow (Repeat Before, During, After Beam)

### Pre-test Setup
- Connect the mesa board to the PCM computer via PCI stack (should already be connected)
- Connect PCM computer to 5V power supply and to the network using the ethernet dongle
  - Ensure that the PCM ssd (NON BEAMLINE) is safely out of the beam-line (VERY IMPORTANT)
- Note this test cannot be run independently from the PCM SMRT computer test.
  - See `../SMRT/notes.md` for details on setting this up first.
- Recompile (if necessary) by running `make clean all`.
- Test the test program by running `sudo ./loopback ALL_IO.bit`
  - This will program the FPGA and stream .csv data to terminal

### Test
- Run the test redirecting data stream to `./data/[test_name].csv`
  - e.g. `sudo ./loopback ALL_IO.bit > ./data/[test_name].csv1

### Post-test
- Stop the loopback test (Ctrl+C)
- Refer to SMRT for completing tests and backing up data

# PSRB

## Workflow (Repeat Before, During, After Beam)

### Pre-test Setup
- Connect Analog Discovery 2 directory to breakout (no BNC board intermediary)
- Connect the PSRB cable to the breakout
  - PSRB Blue => Breakout 1+ (Orange)
  - PSRB White => Breakout GND (Black)
  - PSRB Green+Purple => Breakout V+ (Red)
  - PSRB Orange => Breakout W1 (Yellow)
  - PSRB Yellow => Breakout W2 (Yellow/White)
  - Breakout GND (Black) => Breakout 1- (Orange/White)
- Connect load and dsub25 to psrb
  - Connect PSRB power input to 48V power supply (2x24 V in series)
- Run `waveforms` and load `psrb_test`

### Test
- Set frequency of W1 and W2 to desired frequency (wnat 1 Hz for one run and 100 mHz for another)
- Turn on voltage source and wave generators
   - This should turn off and on the SSR at 1 Hz
   - Current should be visible on scope depending on load
- Star scope recording when test has begun

### Post-test
- On Digilent Scope, File->Save Acquisition to save timestream data as .csv
  - Put in the `./data/[descriptive_name_with_date]` directory corresponding to the test.
- Make sure to back up / rsync data from `./data/` to personal laptop or hard drive
  - e.g. `rsync -avP ./data/ user@laptop:/path/to/backup`

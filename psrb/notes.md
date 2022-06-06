# PSRB 

## Workflow (Repeat Before, During, After Beam)

- Connect Analog Discovery 2 directoryto breakout (no BNC intermediary)
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
- Set frequency of W1 and W2 to 1 Hz
- Turn on scope, voltage source and wave generators
- Save acquisition for set time to data/[descriptive_name]
- Repeat for slower 100 mHz test
- ensure that the data is been taken, back it up on personal machine or external drive

# Mesa Test

## Workflow (Repeat Before, During, After Beam)

- Note this test cannot be run independently from the PCM computer test
  - connect pcm computer to power 5V and to the network using the ethernet dongle
  - ensure that the ssd is safely out of the beam-line (VERY IMPORTANT)
- Connect the mesa board to the PCM computer via PCI stack (should already be connected)
- Compile by running `make clean all`
- Run the test dumping the prints into a text file using `sudo ./loopback ALL_IO.bit > out.txt`

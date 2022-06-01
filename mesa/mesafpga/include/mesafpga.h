//
//    mesafpga
//
//    Javier Romualdez 2019
//    javier.romualdez@gmail.com
//
//    This program was derived from the mesaflash software for communication
//    with the 4i69-25 FPGA PCI board. Although only the PCI functions for this
//    particular board are relevant at present, all the functionality for
//    multiple boards over a variety of interfaces has been maintained.
//
//    Additionally, the mesaflash software library has been made QNX-compatible
//    with functionality limited to board access, FPGA programming, and basic
//    board I/O. The directory ../mesaflash containes the modified library
//    (also see ../notes.txt for details).

#ifndef MESAFPGA_HEADER_H
#define MESAFPGA_HEADER_H

#include "anyio.h"
#include <stdint.h>

// base addresses for basic I/O
#define MESA_IO_BANK(_p) (((int)_p) / 24)
#define MESA_IO_PORT(_p) (((int)_p) % 24)
#define MESA_READ_WRITE_ADDR(_p) (0x1000 + (MESA_IO_BANK(_p) << 2))
#define MESA_SET_OUTPUT_ADDR(_p) (0x1100 + (MESA_IO_BANK(_p) << 2))
#define MESA_SET_ALTSRC_ADDR(_p) (0x1200 + (MESA_IO_BANK(_p) << 2))
#define MESA_SET_INVERT_ADDR(_p) (0x1400 + (MESA_IO_BANK(_p) << 2))
#define MESA_P1_PIN0 0
#define MESA_P3_PIN0 24
#define MESA_P4_PIN0 48
#define MESA_P1_BANK MESA_IO_BANK(MESA_P1_PIN0)
#define MESA_P3_BANK MESA_IO_BANK(MESA_P3_PIN0)
#define MESA_P4_BANK MESA_IO_BANK(MESA_P4_PIN0)

// base addresses for quadrature encoders
#define Q_ENCODER_BASE 0X3000
#define Q_ENCODER(_n) (Q_ENCODER_BASE + (((uint8_t)_n) * 0x0004))              // nth encoder range 0->7
#define Q_ENCODER_CCR(_n) (Q_ENCODER_BASE + 0x0100 + (((uint8_t)_n) * 0x0004)) // nth encoder range 0->7

// base addresses for SSI I/O
#define MESA_SSI_RW_DATA_ADDR(_ssi) (0x6900 + (((int)_ssi) << 2))
#define MESA_SSI_CONTROL_ADDR(_ssi) (0x6b00 + (((int)_ssi) << 2))
#define MESA_SSI_GLOBAL_TRIG_ADDR 0x6c00

// defines for SSI
#define MESA_SSI_BASE_FREQ 50000000 // [Hz]
#define MESA_SSI_BUSY_MASK 0x0800
#define MESA_SSI_DAV_MASK 0x8000

// the map for SSI is as follows:
// [0-7]   => number of bits to clock from the device
// [8]     => set if triggerable from global trigger
// [11]    => status bit (high is currently reading) [RO]
// [16-31] => clock freqency/65536*MESA_SSI_BASE_FREQ
#define MESA_SSI_CONTROL_VALUE(_freq, _numbits, _globaltrig) ((uint32_t)((((uint64_t)_freq << 16) / MESA_SSI_BASE_FREQ) << 16) | \
                                                              ((_globaltrig > 0) << 8) |                                         \
                                                              (_numbits & 0xff))
// SSI and I/O implementation defines
#define SSI_GLOBAL_TRIGGER_READ 0x01

// base addresses for FSSI I/O
#define MESA_FSSI_RW_DATA_ADDR(_fssi) (0x9100 + (((int)_fssi) << 2))
#define MESA_FSSI_CONTROL_ADDR(_fssi) (0x9300 + (((int)_fssi) << 2))

// defines for FSSI (Fake SSI)
#define MESA_FSSI_BASE_FREQ 50000000 // [Hz]
#define MESA_FSSI_BUSY_MASK 0x0800
#define MESA_FSSI_DAV_MASK 0x8000

// the map for FSSI (Fake SSI) is as follows:
// [0-7]   => number of bits to clock from the device [def: 16]
// [11]    => status bit (high is currently reading) [RO]
// [16-23] => filter width in MESA_SSI_BASE_FREQ Hz samples
// [24-31] => read timeout in MESA_SSI_BASE_FREQ Hz samples
#define MESA_FSSI_CONTROL_VALUE(_numbits, _width, _timeout) ((uint32_t)((_timeout & 0xff) << 24) | \
                                                             ((_width & 0xff) << 16) |             \
                                                             (_numbits & 0xff))

// general IO defines
#define IO_SET_AS_OUTPUT 0x01
#define IO_INVERT_OUTPUT 0x02

board_t *mesafpga_init(char *);
void mesafpga_close(board_t *);
void mesafpga_write_reg(board_t *, uint32_t, uint32_t);
uint32_t mesafpga_read_reg(board_t *, uint32_t);

uint32_t mesafpga_read_raw_ssi(board_t *, uint32_t, unsigned int, int);
void mesafpga_global_trigger_ssi(board_t *);
uint32_t mesafpga_ssi_dav(board_t *, uint32_t);

uint32_t mesafpga_read_raw_fssi(board_t *, uint32_t, unsigned int, int);
uint32_t mesafpga_fssi_dav(board_t *, uint32_t);

void mesafpga_set_io(board_t *, uint32_t, int);
uint8_t mesafpga_read_io(board_t *, uint32_t);
void mesafpga_write_io(board_t *, uint32_t, uint8_t);
void mesafpga_pulse_io(board_t *, uint32_t, uint32_t);
void mesafpga_write_io_bank(board_t *, uint32_t, uint32_t);
void mesafpga_write_io_bank_with_mask(board_t *, uint32_t, uint32_t, uint32_t);

#ifndef MIN
#define MIN(a, b) ((a < b) ? a : b)
#endif

#endif /* MESAFPGA_HEADER_H */

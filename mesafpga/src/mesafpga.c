//
//    mesafpga 
//   
//    Javier Romualdez 2019
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
//
//    Revisions:
//
//    2019-03-01: original working project for programming FPGA and I/O access

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>

#include "mesafpga.h"

static board_access_t board_access = {
  .device_name = "4I69",
  .verbose = 0 
};
static uint32_t io_output_reg[3] = {0};
static uint32_t io_invert_reg[3] = {0};
static uint32_t io_output_mask[3] = {0};
static uint32_t io_function_mask[3] = {0};

int8_t  ssi_clk_enable_pin[8] = {-1, -1, -1, -1, -1, -1, -1, -1};

board_t * mesafpga_init(char * bitfile_name) {
  int ret = 0;
  uint32_t reg = 0;
  board_t * board = NULL;

  if (anyio_init(&board_access) != 0) {     // init library
    return NULL;
  }
  ret = anyio_find_dev(&board_access);      // find board
  if (ret < 0) {
    return NULL;
  }
  board = anyio_get_dev(&board_access, 1);  // if found the get board handle
  if (board == NULL) {
    printf("** MESA: No %s board found\n", board_access.device_name);
    return NULL;
  }

  board->open(board);                 // open board for communication
  if (board_access.verbose) board->print_info(board);           // print what card it is

  // program fpga if a bitfile is given
  if (bitfile_name) {
    // .. and the fpga hasn't been programmed yet
    if (!hm2_read_idrom(&(board->llio.hm2))) {
      ret = anyio_dev_program_fpga(board, bitfile_name);
    }  
  }

  // set all output clocks and alternate source bits for SSI
  //
  // 7I52 configuration:
  //
  // ** If the FPGA configuration changes, this may have to change as well **
  //
  // IO00 = 1 <<  0 = 0x00000001 - ClkEn3 => Output
  // IO10 = 1 << 10 = 0x00000400 - EncMux => Output
  // IO11 = 1 << 11 = 0x00000800 - TX5    => Output/Sec. function
  // IO13 = 1 << 13 = 0x00002000 - TX4    => Output/Sec. function
  // IO15 = 1 << 15 = 0x00008000 - TX3    => Output/Sec. function
  // IO17 = 1 << 17 = 0x00020000 - TX2    => Output/Sec. function
  // IO19 = 1 << 19 = 0x00080000 - TX1    => Output/Sec. function
  // IO21 = 1 << 21 = 0x00200000 - TX0    => Output/Sec. function
  // IO23 = 1 << 23 = 0x00800000 - ClkEn0 => Output

  // set the outputs
  reg = 0x00aaac01; 
  mesafpga_write_reg(board, MESA_SET_OUTPUT_ADDR(MESA_P1_PIN0), reg);
  io_output_mask[MESA_P1_BANK] = reg;

  // clock enable is just setting the pin, so not a secondary function
  reg = 0x002aa800; 
  mesafpga_write_reg(board, MESA_SET_ALTSRC_ADDR(MESA_P1_PIN0), reg);
  ssi_clk_enable_pin[3] = 0; 
  ssi_clk_enable_pin[0] = 23; 

  // invert the output pins to have the correct clock
  mesafpga_write_reg(board, MESA_SET_INVERT_ADDR(MESA_P1_PIN0), reg);
  io_invert_reg[MESA_P1_BANK] = reg;

  // set function mask so that secondary functions can't be tampered with
  io_function_mask[MESA_P1_BANK] = 0x007ffffe; // all 24 io ports on the first header have secondary functions
                                    // don't mask out clk enable/disable pins

  // set all other pins on the other headers (P3/P4) to outputs, non-inverted, no altsrc
  reg = 0x00ffffff;
  mesafpga_write_reg(board, MESA_SET_OUTPUT_ADDR(MESA_P3_PIN0), reg);
  mesafpga_write_reg(board, MESA_SET_OUTPUT_ADDR(MESA_P4_PIN0), reg);
  io_output_mask[MESA_P3_BANK] = reg;
  io_output_mask[MESA_P4_BANK] = reg;

  reg = 0x00000000;
  mesafpga_write_reg(board, MESA_SET_ALTSRC_ADDR(MESA_P3_PIN0), reg);
  mesafpga_write_reg(board, MESA_SET_ALTSRC_ADDR(MESA_P4_PIN0), reg);
  io_invert_reg[MESA_P3_BANK] = reg;
  io_invert_reg[MESA_P4_BANK] = reg;

  reg = 0x00000000;
  mesafpga_write_reg(board, MESA_SET_INVERT_ADDR(MESA_P3_PIN0), reg);
  mesafpga_write_reg(board, MESA_SET_INVERT_ADDR(MESA_P4_PIN0), reg);
  io_function_mask[MESA_P3_BANK] = reg;
  io_function_mask[MESA_P4_BANK] = reg;

  return board;
}

void mesafpga_write_reg(board_t * board, uint32_t addr, uint32_t data) {
  board->llio.write(&board->llio, addr, &data, sizeof(data));
}
uint32_t mesafpga_read_reg(board_t * board, uint32_t addr) {
  uint32_t data = 0;
  board->llio.read(&board->llio, addr, &data, sizeof(data));
  return data;
}

// board, io port number, direction (0 = input, 1 = output)
void mesafpga_set_io(board_t * board, uint32_t io, int flags) {
  uint8_t bank = MESA_IO_BANK(io);
  uint8_t port = MESA_IO_PORT(io);

  if (io_function_mask[bank] & (1 << port)) {
    printf("** MESA: I/O port %d has a protected function\n", io);
    return;
  }

  // set direction
  if (flags & IO_SET_AS_OUTPUT) { // set the bit (output)
    io_output_mask[bank] |= (1 << port); 
  } else { // clear the bit (input)
    io_output_mask[bank] &= 0xffffffff-(1 << port); 
  }

  // set polarity
  if (flags & IO_INVERT_OUTPUT) { // set the bit (inverted)
    io_invert_reg[bank] |= (1 << port); 
  } else { // clear the bit (not inverted)
    io_invert_reg[bank] &= 0xffffffff-(1 << port); 
  }

  // write the output and invert registers
  mesafpga_write_reg(board, MESA_SET_OUTPUT_ADDR(io), io_output_mask[bank]);
  mesafpga_write_reg(board, MESA_SET_INVERT_ADDR(io), io_invert_reg[bank]);
}

uint8_t mesafpga_read_io(board_t * board, uint32_t io) {
  uint8_t bank = MESA_IO_BANK(io);
  uint8_t port = MESA_IO_PORT(io);

  if (io_function_mask[bank] & (1 << port)) {
    printf("** MESA: I/O port %d has a protected function\n", io);
    return 0;
  }

  // report 0 if the port is an output
  if (io_output_mask[bank] & (1 << port)) {
    printf("** MESA: Reading from output port %d\n", io);
    return 0;
  }

  // read the input register and return if pin is set
  return (((mesafpga_read_reg(board, MESA_READ_WRITE_ADDR(io))) & (1 << port)) != 0);
}

void mesafpga_write_io(board_t * board, uint32_t io, uint8_t val) {
  uint8_t bank = MESA_IO_BANK(io);
  uint8_t port = MESA_IO_PORT(io);
  uint32_t reg = 0; 

  if (!(io_output_mask[bank] & (1 << port))) {
    printf("** MESA: Writing to input port %d\n", io);
    return;
  }


  // set output state
  if (val) { // set the bit
    reg = io_output_reg[bank] | (1 << port); 
  } else { // clear the bit
    reg = io_output_reg[bank] & (0xffffffff-(1 << port)); 
  }

  mesafpga_write_io_bank(board, io, reg);
}

void mesafpga_pulse_io(board_t * board, uint32_t io, uint32_t width) {
  mesafpga_write_io(board, io, 1);
  usleep(width);
  mesafpga_write_io(board, io, 0);
}

void mesafpga_write_io_bank(board_t * board, uint32_t io, uint32_t val) {
  uint32_t bank = MESA_IO_BANK(io);

  if (io_function_mask[bank] & val) {
    printf("** MESA: I/O mask 0x%x has a protected function\n", val & io_function_mask[bank]);
    return;
  }

  // write the output registers
  io_output_reg[bank] = val;
  mesafpga_write_reg(board, MESA_READ_WRITE_ADDR(io), io_output_reg[bank]);
}

void mesafpga_write_io_bank_with_mask(board_t * board, uint32_t io, uint32_t val, uint32_t mask) {
  uint32_t bank = MESA_IO_BANK(io);

  // mask
  uint32_t saved = io_output_reg[bank] & (0xffffffff-mask);
  mesafpga_write_io_bank(board, io, (val & mask) | saved);
}

void mesafpga_close(board_t * board) {
  if (board) board->close(board);
  anyio_cleanup(&board_access);
}

// ssi => index, timeout->in us (0 => don't block)
uint32_t mesafpga_read_raw_ssi(board_t * board, uint32_t ssi, unsigned int timeout_us, int flags) {
  int count = 0;  
 
  if (!(flags & SSI_GLOBAL_TRIGGER_READ)) mesafpga_write_reg(board, MESA_SSI_RW_DATA_ADDR(ssi), 0x0);
  if (timeout_us) {
    while (mesafpga_read_reg(board, MESA_SSI_CONTROL_ADDR(ssi)) & MESA_SSI_BUSY_MASK) {
      count++;
      if (count >= timeout_us) {
        printf("** MESA: Read timeout on SSI %d\n", ssi);
        return 0;
      } 
      usleep(1);
    }
  }
  return mesafpga_read_reg(board, MESA_SSI_RW_DATA_ADDR(ssi));
}

// fssi => index, timeout->in us (0 => don't block)
uint32_t mesafpga_read_raw_fssi(board_t * board, uint32_t fssi, unsigned int timeout_us, int flags) {
  int count = 0;  
 
  if (timeout_us) {
    while (mesafpga_read_reg(board, MESA_FSSI_CONTROL_ADDR(fssi)) & MESA_FSSI_BUSY_MASK) {
      count++;
      if (count >= timeout_us) {
        printf("** MESA: Read timeout on FSSI %d\n", fssi);
        return 0;
      } 
      usleep(1);
    }
  }
  return mesafpga_read_reg(board, MESA_FSSI_RW_DATA_ADDR(fssi));
}

void mesafpga_global_trigger_ssi(board_t * board) {
  mesafpga_write_reg(board, MESA_SSI_GLOBAL_TRIG_ADDR, 0x0);
}

uint32_t mesafpga_ssi_dav(board_t * board, uint32_t ssi) {
  return mesafpga_read_reg(board, MESA_SSI_CONTROL_ADDR(ssi)) & MESA_SSI_DAV_MASK;
}

uint32_t mesafpga_fssi_dav(board_t * board, uint32_t fssi) {
  return mesafpga_read_reg(board, MESA_FSSI_CONTROL_ADDR(fssi)) & MESA_FSSI_DAV_MASK;
}


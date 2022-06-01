/*
 *  QNX port of Mesaflash software.
 *
 *  Note:
 *
 *  Although functionality for all board types (except SPI) has
 *  been maintained in the port, only the PCI functions have been
 *  tested and verified (using the 4i69-25 Mesa Tech board).
 *
 *  Javier Romualdez 2019 <javier.romualdez@gmail.com>
 *
 */

#ifndef _QNX_H
#define _QNX_H

#include <hw/inout.h>
#include "boards.h"
#include "libpci/pci.h"

// standard I/O
#define inb(a) in8(a)
#define outb(a,b) out8(b,a)
#define inw(a) in16(a)
#define outw(a,b) out16(b,a)
#define inl(a) in32(a)
#define outl(a,b) out32(b,a)

#endif /* QNX_H */

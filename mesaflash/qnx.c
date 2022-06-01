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

#include <process.h>
#include <hw/pci.h>
#include <sys/mman.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/neutrino.h>

#include "qnx.h"

struct {
  char * name;
  unsigned int vendor_id; 
  unsigned int device_id;
} qnx_pci_supported_boards[] = {
  {NULL, 0, 0}
};

// define all the qnx-equivalent libpci functions and qnx-specific functions
void pci_scan_bus(struct pci_access * pacc)  {
  int i;
	unsigned bus, lastbus, version, hardware, devfunc;

  // get the last bus, version, and hardware number
	int status = pci_present(&lastbus, &version, &hardware);
	if (status != PCI_SUCCESS) {
		printf( "pci_present() failed with error code %d\n", status);
	}

  // loop through all the devices on the bus
  uint32_t class = PCI_CLASS_BRIDGE; // cards are bridge class
  uint32_t subclass = PCI_SUBCLASS_BRIDGE_OTHER;
  struct pci_dev * dev = NULL;
  int index = 0;
  pacc->devices = NULL;
  while (pci_find_class(class | subclass, 
                        index++, // indices start from 0
                        &bus, // get the bus number [0-255]
                        &devfunc // get the device (7-3) and function (2-0) numbers
                        ) == PCI_SUCCESS) { // loop through all devices


    // allocate a new device
    dev = (struct pci_dev *) calloc(sizeof(struct pci_dev), 1);
    dev->next = pacc->devices;

    // set the bus, dev and func
    dev->bus = bus;
    dev->dev = (devfunc & 0xf8) >> 3;
    dev->func = devfunc & 0x07;

    // get the vendor ID
		if ((status=pci_read_config16(bus, devfunc, offsetof(struct _pci_config_regs, Vendor_ID),
																	1, &dev->vendor_id)) != PCI_SUCCESS) {
			printf("Failed to read Vendor_ID with error code %d\n", status);
			return;
		}
    // get the device ID
		if ((status=pci_read_config16(bus, devfunc, offsetof(struct _pci_config_regs, Device_ID),
																	1, &dev->device_id)) != PCI_SUCCESS) {
			printf("Failed to read Device_ID with error code %d\n", status);
			return;
		}
    // get the IRQ/Interrupt line
		if ((status=pci_read_config8(bus, devfunc, offsetof(struct _pci_config_regs, Interrupt_Line),
																	1, &dev->irq)) != PCI_SUCCESS) {
			printf("Failed to read Interrupt_Line with error code %d\n", status);
			return;
		}
    // get the BARs
		if ((status=pci_read_config32(bus, devfunc, offsetof(struct _pci_config_regs, Base_Address_Regs),
																	6, &dev->base_addr[0])) != PCI_SUCCESS) {
			printf("Failed to read Base_Address_Regs with error code %d\n", status);
			return;
		}
    // get the ROM address
		if ((status=pci_read_config32(bus, devfunc, offsetof(struct _pci_config_regs, ROM_Base_Address),
																	1, &dev->rom_base_addr)) != PCI_SUCCESS) {
			printf("Failed to read ROM_Base_Address with error code %d\n", status);
			return;
		}
    dev->access = pacc;
    pacc->devices = dev;
  }
}

struct pci_access * pci_alloc() {
  return (struct pci_access *) calloc(sizeof(struct pci_access), 1);
}
void pci_cleanup(struct pci_access * p) {
  struct pci_dev * dev = p->devices;
  while (dev) {
    struct pci_dev * next = dev->next;
    free(dev);
    dev = next;
  }
  free(p);
}

void pci_init(struct pci_access * p) {
	int handle = pci_attach(0);
  if (handle < 0) {
    printf("pci_attach failed with error code %d\n", handle);
    return;
  }
  if (ThreadCtl(_NTO_TCTL_IO, 0) == -1) {
    printf("Error: ThreadCtl() failed to set I/O privileges.\n");
    return;
  }
}

u8 pci_read_byte(struct pci_dev * dev, int pos) PCI_ABI {
  u8 buff = 0;
	int status = pci_read_config8(dev->bus, // bus number
												 ((dev->dev << 3) & 0xf8) | (dev->func & 0x7), // device function number
												 pos, // position of data
												 1, // number of 8-bit values
												 &buff); // a buffer
  if (status != PCI_SUCCESS) {
    printf("pci_read_config8 failed with error code %d\n", status);
    return 0;
  }
  return buff;
}
u16 pci_read_word(struct pci_dev * dev, int pos) PCI_ABI {
  u16 buff = 0;
	int status = pci_read_config16(dev->bus, // bus number
												 ((dev->dev << 3) & 0xf8) | (dev->func & 0x7), // device function number
												 pos, // position of data (16 bit aligned)
												 1, // number of 16-bit values
												 &buff); // a buffer
  if (status != PCI_SUCCESS) {
    printf("pci_read_config16 failed with error code %d\n", status);
    return 0;
  }
  return buff;
}
u32 pci_read_long(struct pci_dev * dev, int pos) PCI_ABI {
  u32 buff = 0;
	int status = pci_read_config32(dev->bus, // bus number
												 ((dev->dev << 3) & 0xf8) | (dev->func & 0x7), // device function number
												 pos, // position of data (32 bit aligned)
												 1, // number of 32-bit values
												 &buff); // a buffer
  if (status != PCI_SUCCESS) {
    printf("pci_read_config32 failed with error code %d\n", status);
    return 0;
  }
  return buff;
}
int pci_read_block(struct pci_dev * dev, int pos, u8 *buf, int len) PCI_ABI {
	int status = pci_read_config8(dev->bus, // bus number
												 ((dev->dev << 3) & 0xf8) | (dev->func & 0x7), // device function number
												 pos, // position of data (8 bit aligned)
												 len, // number of 8-bit values
												 &buf); // a buffer
  if (status != PCI_SUCCESS) {
    printf("pci_read_config8 failed with error code %d\n", status);
    return 0;
  }
  return len;
}
int pci_write_byte(struct pci_dev * dev, int pos, u8 data) PCI_ABI {
	int status = pci_write_config8(dev->bus, // bus number
												 ((dev->dev << 3) & 0xf8) | (dev->func & 0x7), // device function number
												 pos, // position of data
												 1, // number of 8-bit values
												 &data); // the buffer
  if (status != PCI_SUCCESS) {
    printf("pci_write_config8 failed with error code %d\n", status);
  }
  return status;
}
int pci_write_word(struct pci_dev * dev, int pos, u16 data) PCI_ABI {
	int status = pci_write_config16(dev->bus, // bus number
												 ((dev->dev << 3) & 0xf8) | (dev->func & 0x7), // device function number
												 pos, // position of data (16 bit aligned)
												 1, // number of 16-bit values
												 &data); // the buffer
  if (status != PCI_SUCCESS) {
    printf("pci_write_config16 failed with error code %d\n", status);
  }
  return status;
}
int pci_write_long(struct pci_dev * dev, int pos, u32 data) PCI_ABI {
	int status = pci_write_config32(dev->bus, // bus number
												 ((dev->dev << 3) & 0xf8) | (dev->func & 0x7), // device function number
												 pos, // position of data (32 bit aligned)
												 1, // number of 32-bit values
												 &data); // the buffer
  if (status != PCI_SUCCESS) {
    printf("pci_write_config32 failed with error code %d\n", status);
  }
  return status;
}
int pci_write_block(struct pci_dev * dev, int pos, u8 *buf, int len) PCI_ABI {
	int status = pci_write_config8(dev->bus, // bus number
												 ((dev->dev << 3) & 0xf8) | (dev->func & 0x7), // device function number
												 pos, // position of data
												 len, // number of 8-bit values
												 &buf); // the buffer
  if (status != PCI_SUCCESS) {
    printf("pci_write_config8 failed with error code %d\n", status);
  }
  return status;
}

// other function fill-ins for QNX


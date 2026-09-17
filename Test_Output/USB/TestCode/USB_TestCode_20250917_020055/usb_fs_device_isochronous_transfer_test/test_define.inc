// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: USB_FS_Device_Isochronous_Transfer_test
 * Purpose: Supporting data definitions for USB Full-Speed Device mode Isochronous Transfer
 *          functionality validation testcase.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "usb.h"

#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

#define USB_ISOC_TRB_CTRL     0x00000869U
#define USB_ISOC_XFER_SIZE    1023U
#define USB_ISOC_DEPCMD_FR2   0x00020506U
#define USB_ISOC_DEPCMD_FR4   0x00040506U

#ifndef USB_HANDSHAKE_ADDR_0
#define USB_HANDSHAKE_ADDR_0  0xA0243ffcU
#endif
#ifndef USB_HANDSHAKE_ADDR_1
#define USB_HANDSHAKE_ADDR_1  0xa0243ff4U
#endif
#ifndef USB_HANDSHAKE_ADDR_2
#define USB_HANDSHAKE_ADDR_2  0xa0243ff8U
#endif

static unsigned int buf_data[16];

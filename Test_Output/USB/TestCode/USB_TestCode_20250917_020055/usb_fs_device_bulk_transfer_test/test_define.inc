// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: USB_FS_Device_Bulk_Transfer_test
 * Purpose: Supporting data definitions for USB Full-Speed Device mode Bulk Transfer
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

/*
 * Macros from Meta TestPlan JSON.
 * These macros are expected to be defined by the included headers (e.g., usb.h).
 * They are listed here as reference for the registers and symbols used by this testcase:
 *
 *   Buffer_PointerLO
 *   event_trb_addr
 *   MIZAR_USB_DCTL
 *   MIZAR_USB_GUSB2PHYCFG
 *   MIZAR_USB_GEVNTADRLO
 *   MIZAR_USB_GEVNTADRHI
 *   MIZAR_USB_GEVNTSIZ
 *   MIZAR_USB_GEVNTCOUNT
 *   MIZAR_USB_GCTL
 *   MIZAR_USB_DCFG
 *   MIZAR_USB_DEVTEN
 *   MIZAR_USB_GUCTL
 *   MIZAR_USB_DEPCMDPAR0
 *   MIZAR_USB_DEPCMD
 *   MIZAR_USB_DALEPENA
 *   MIZAR_LSS_SYSREG_INTR_EN0
 *   MIZAR_USB_DSTS
 *   MIZAR_USB_DEPCMDPAR1
 *   MIZAR_LSS_SYSREG_MSK_STS0
 *   MIZAR_LSS_SYSREG_RAW_STCR0
 *   DWORD
 *   Default_Event_Ring_Array
 */

/* Hardcoded memory-mapped handshake addresses from Impacted Registers */
#ifndef USB_HANDSHAKE_ADDR_0
#define USB_HANDSHAKE_ADDR_0  0xA0243ffcU
#endif

#ifndef USB_HANDSHAKE_ADDR_1
#define USB_HANDSHAKE_ADDR_1  0xa0243ff4U
#endif

#ifndef USB_HANDSHAKE_ADDR_2
#define USB_HANDSHAKE_ADDR_2  0xa0243ff8U
#endif

/* Arrays from Meta TestPlan JSON */
static unsigned int buf_data[16];

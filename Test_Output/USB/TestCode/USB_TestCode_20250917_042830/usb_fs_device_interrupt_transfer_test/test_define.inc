// Author - AI Force 2.3. 17-Sep-2025 04:28 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name: USB_FS_Device_Interrupt_Transfer_test
 * Purpose: Test Description was NA. Definition file contains headers, macros,
 *          arrays, and supporting testcase data from the Meta TestPlan JSON.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "usb.h"

/*
 * Macros from Meta TestPlan JSON.
 * Note: Only macro names were provided without values. The actual values are
 * expected to be defined in the included headers (e.g., usb.h) or SoC/IP
 * configuration headers. Macros listed below are preserved as comments for
 * reference. If they are not defined by included headers, they must be
 * provided by the build environment or manually added here.
 */

// MANUAL_REVIEW: The following macro names were listed in the Meta TestPlan JSON
// but no values were provided. They are expected to be defined in usb.h or
// other included headers:
//   Buffer_PointerLO
//   event_trb_addr
//   MIZAR_USB_DCTL
//   MIZAR_USB_GUSB2PHYCFG
//   MIZAR_USB_GEVNTADRLO
//   MIZAR_USB_GEVNTADRHI
//   MIZAR_USB_GEVNTSIZ
//   MIZAR_USB_GEVNTCOUNT
//   MIZAR_USB_GCTL
//   MIZAR_USB_DCFG
//   MIZAR_USB_DEVTEN
//   MIZAR_USB_GUCTL
//   MIZAR_USB_DEPCMDPAR0
//   MIZAR_USB_DEPCMD
//   MIZAR_USB_DALEPENA
//   MIZAR_LSS_SYSREG_INTR_EN0
//   MIZAR_USB_DSTS
//   MIZAR_USB_DEPCMDPAR1
//   MIZAR_LSS_SYSREG_MSK_STS0
//   MIZAR_LSS_SYSREG_RAW_STCR0
//   DWORD
//   Default_Event_Ring_Array
//   Buffer_PointerLO_1

/*
 * Arrays from Meta TestPlan JSON.
 */
static DWORD buf_data[16];

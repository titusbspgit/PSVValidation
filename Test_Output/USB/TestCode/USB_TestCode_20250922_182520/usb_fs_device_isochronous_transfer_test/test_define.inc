// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: USB_FS_Device_Isochronous_Transfer_test
 * Purpose: Defines headers, macros, arrays, and supporting data for USB
 *          Full-Speed Device mode Isochronous Transfer testcase.
 */

#include "test.h"

/* Meta TestPlan Headers */
#include <stdio.h>
#include <stdlib.h>
#include "usb.h"

/*
 * Macros from Meta TestPlan JSON.
 * These register address macros and memory pointers are expected to be
 * defined in the platform/IP headers (e.g., usb.h). If not already defined,
 * uncomment and provide the correct addresses below.
 */

/* USB Controller Register Macros */
// MANUAL_REVIEW: The following macros are listed in the Meta TestPlan JSON Macros field.
// They are expected to be provided by "usb.h" or platform headers.
// If any are missing, define them here with correct addresses.
/*
#ifndef MIZAR_USB_DCTL
#define MIZAR_USB_DCTL          <address>
#endif
#ifndef MIZAR_USB_GUSB2PHYCFG
#define MIZAR_USB_GUSB2PHYCFG   <address>
#endif
#ifndef MIZAR_USB_GEVNTADRLO
#define MIZAR_USB_GEVNTADRLO    <address>
#endif
#ifndef MIZAR_USB_GEVNTADRHI
#define MIZAR_USB_GEVNTADRHI    <address>
#endif
#ifndef MIZAR_USB_GEVNTSIZ
#define MIZAR_USB_GEVNTSIZ      <address>
#endif
#ifndef MIZAR_USB_GEVNTCOUNT
#define MIZAR_USB_GEVNTCOUNT    <address>
#endif
#ifndef MIZAR_USB_GCTL
#define MIZAR_USB_GCTL          <address>
#endif
#ifndef MIZAR_USB_DCFG
#define MIZAR_USB_DCFG          <address>
#endif
#ifndef MIZAR_USB_DEVTEN
#define MIZAR_USB_DEVTEN        <address>
#endif
#ifndef MIZAR_USB_GUCTL
#define MIZAR_USB_GUCTL         <address>
#endif
#ifndef MIZAR_USB_DEPCMDPAR0
#define MIZAR_USB_DEPCMDPAR0    <address>
#endif
#ifndef MIZAR_USB_DEPCMDPAR1
#define MIZAR_USB_DEPCMDPAR1    <address>
#endif
#ifndef MIZAR_USB_DEPCMD
#define MIZAR_USB_DEPCMD        <address>
#endif
#ifndef MIZAR_USB_DALEPENA
#define MIZAR_USB_DALEPENA      <address>
#endif
#ifndef MIZAR_USB_DSTS
#define MIZAR_USB_DSTS          <address>
#endif
#ifndef MIZAR_LSS_SYSREG_INTR_EN0
#define MIZAR_LSS_SYSREG_INTR_EN0   <address>
#endif
#ifndef MIZAR_LSS_SYSREG_MSK_STS0
#define MIZAR_LSS_SYSREG_MSK_STS0   <address>
#endif
#ifndef MIZAR_LSS_SYSREG_RAW_STCR0
#define MIZAR_LSS_SYSREG_RAW_STCR0  <address>
#endif
*/

/* Memory pointer macros */
// MANUAL_REVIEW: Buffer_PointerLO, Buffer_PointerLO_1, event_trb_addr, and
// Default_Event_Ring_Array are RAM-relative addresses whose base (RAM) is not
// resolved in the available headers. Define them with correct platform addresses.
/*
#ifndef Buffer_PointerLO
#define Buffer_PointerLO        <address>
#endif
#ifndef Buffer_PointerLO_1
#define Buffer_PointerLO_1      <address>
#endif
#ifndef event_trb_addr
#define event_trb_addr          <address>
#endif
#ifndef Default_Event_Ring_Array
#define Default_Event_Ring_Array <address>
#endif
*/

/* DWORD size macro */
// MANUAL_REVIEW: DWORD is listed as a macro. Expected to be 4 (bytes).
// If not defined in platform headers, uncomment below.
/*
#ifndef DWORD
#define DWORD 4U
#endif
*/

/* Arrays from Meta TestPlan JSON */
static unsigned int buf_data[16];

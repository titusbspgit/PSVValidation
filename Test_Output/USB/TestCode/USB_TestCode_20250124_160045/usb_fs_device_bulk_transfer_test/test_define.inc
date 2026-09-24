// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: USB_FS_Device_Bulk_Transfer_test
 * Purpose: This testcase validates USB Full-Speed Device mode Bulk Transfer operation.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "usb.h"

#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* Polling timeout constant */
#define USB_POLL_TIMEOUT 10000U

/*
 * Macro names from Meta TestPlan JSON.
 * These macros are expected to be defined in "usb.h" or platform headers.
 * If not already defined, they must be provided by the platform configuration.
 */

#ifndef Buffer_PointerLO
// MANUAL_REVIEW: Buffer_PointerLO macro value not provided in Meta TestPlan JSON.
// #define Buffer_PointerLO <address>
#endif

#ifndef event_trb_addr
// MANUAL_REVIEW: event_trb_addr macro value not provided in Meta TestPlan JSON.
// #define event_trb_addr <address>
#endif

#ifndef MIZAR_USB_DCTL
// MANUAL_REVIEW: MIZAR_USB_DCTL macro value not provided in Meta TestPlan JSON.
// #define MIZAR_USB_DCTL <address>
#endif

#ifndef MIZAR_USB_GUSB2PHYCFG
// MANUAL_REVIEW: MIZAR_USB_GUSB2PHYCFG macro value not provided in Meta TestPlan JSON.
// #define MIZAR_USB_GUSB2PHYCFG <address>
#endif

#ifndef MIZAR_USB_GEVNTADRLO
// MANUAL_REVIEW: MIZAR_USB_GEVNTADRLO macro value not provided in Meta TestPlan JSON.
// #define MIZAR_USB_GEVNTADRLO <address>
#endif

#ifndef MIZAR_USB_GEVNTADRHI
// MANUAL_REVIEW: MIZAR_USB_GEVNTADRHI macro value not provided in Meta TestPlan JSON.
// #define MIZAR_USB_GEVNTADRHI <address>
#endif

#ifndef MIZAR_USB_GEVNTSIZ
// MANUAL_REVIEW: MIZAR_USB_GEVNTSIZ macro value not provided in Meta TestPlan JSON.
// #define MIZAR_USB_GEVNTSIZ <address>
#endif

#ifndef MIZAR_USB_GEVNTCOUNT
// MANUAL_REVIEW: MIZAR_USB_GEVNTCOUNT macro value not provided in Meta TestPlan JSON.
// #define MIZAR_USB_GEVNTCOUNT <address>
#endif

#ifndef MIZAR_USB_GCTL
// MANUAL_REVIEW: MIZAR_USB_GCTL macro value not provided in Meta TestPlan JSON.
// #define MIZAR_USB_GCTL <address>
#endif

#ifndef MIZAR_USB_DCFG
// MANUAL_REVIEW: MIZAR_USB_DCFG macro value not provided in Meta TestPlan JSON.
// #define MIZAR_USB_DCFG <address>
#endif

#ifndef MIZAR_USB_DEVTEN
// MANUAL_REVIEW: MIZAR_USB_DEVTEN macro value not provided in Meta TestPlan JSON.
// #define MIZAR_USB_DEVTEN <address>
#endif

#ifndef MIZAR_USB_GUCTL
// MANUAL_REVIEW: MIZAR_USB_GUCTL macro value not provided in Meta TestPlan JSON.
// #define MIZAR_USB_GUCTL <address>
#endif

#ifndef MIZAR_USB_DEPCMDPAR0
// MANUAL_REVIEW: MIZAR_USB_DEPCMDPAR0 macro value not provided in Meta TestPlan JSON.
// #define MIZAR_USB_DEPCMDPAR0 <address>
#endif

#ifndef MIZAR_USB_DEPCMD
// MANUAL_REVIEW: MIZAR_USB_DEPCMD macro value not provided in Meta TestPlan JSON.
// #define MIZAR_USB_DEPCMD <address>
#endif

#ifndef MIZAR_USB_DALEPENA
// MANUAL_REVIEW: MIZAR_USB_DALEPENA macro value not provided in Meta TestPlan JSON.
// #define MIZAR_USB_DALEPENA <address>
#endif

#ifndef MIZAR_LSS_SYSREG_INTR_EN0
// MANUAL_REVIEW: MIZAR_LSS_SYSREG_INTR_EN0 macro value not provided in Meta TestPlan JSON.
// #define MIZAR_LSS_SYSREG_INTR_EN0 <address>
#endif

#ifndef MIZAR_USB_DSTS
// MANUAL_REVIEW: MIZAR_USB_DSTS macro value not provided in Meta TestPlan JSON.
// #define MIZAR_USB_DSTS <address>
#endif

#ifndef MIZAR_USB_DEPCMDPAR1
// MANUAL_REVIEW: MIZAR_USB_DEPCMDPAR1 macro value not provided in Meta TestPlan JSON.
// #define MIZAR_USB_DEPCMDPAR1 <address>
#endif

#ifndef MIZAR_LSS_SYSREG_MSK_STS0
// MANUAL_REVIEW: MIZAR_LSS_SYSREG_MSK_STS0 macro value not provided in Meta TestPlan JSON.
// #define MIZAR_LSS_SYSREG_MSK_STS0 <address>
#endif

#ifndef MIZAR_LSS_SYSREG_RAW_STCR0
// MANUAL_REVIEW: MIZAR_LSS_SYSREG_RAW_STCR0 macro value not provided in Meta TestPlan JSON.
// #define MIZAR_LSS_SYSREG_RAW_STCR0 <address>
#endif

#ifndef Buffer_PointerLO_1
// MANUAL_REVIEW: Buffer_PointerLO_1 macro value not provided in Meta TestPlan JSON.
// #define Buffer_PointerLO_1 <address>
#endif

#ifndef Default_Event_Ring_Array
// MANUAL_REVIEW: Default_Event_Ring_Array macro value not provided in Meta TestPlan JSON.
// #define Default_Event_Ring_Array <address>
#endif

#ifndef DWORD
#define DWORD uint32_t
#endif

/* Array: buf_data */
static DWORD buf_data[16];

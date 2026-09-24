// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: USB_FS_Device_Bulk_Transfer_test
 * Purpose: This testcase validates USB Full-Speed Device mode Bulk Transfer operation.
 */

#include "test.h"

/* Meta TestPlan Headers */
#include <stdio.h>
#include <stdlib.h>
#include "usb.h"

/* Polling timeout constant */
#define USB_FS_BULK_POLL_TIMEOUT 10000U

/* Number of endpoints to configure */
#define USB_FS_BULK_NUM_ENDPOINTS 8U

/* Buffer data array size */
#define USB_FS_BULK_BUF_DATA_SIZE 16U

/*
 * Macros from Meta TestPlan JSON.
 * These are register address macros and buffer pointers.
 * Actual values are expected to be provided by "usb.h" or platform headers.
 * The #ifndef guards allow override from platform configuration.
 */

#ifndef Buffer_PointerLO
// MANUAL_REVIEW: Buffer_PointerLO value not provided in Meta TestPlan JSON. Define in platform header.
#define Buffer_PointerLO 0UL
#endif

#ifndef event_trb_addr
// MANUAL_REVIEW: event_trb_addr value not provided in Meta TestPlan JSON. Define in platform header.
#define event_trb_addr 0UL
#endif

#ifndef MIZAR_USB_DCTL
// MANUAL_REVIEW: MIZAR_USB_DCTL value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_USB_DCTL 0UL
#endif

#ifndef MIZAR_USB_GUSB2PHYCFG
// MANUAL_REVIEW: MIZAR_USB_GUSB2PHYCFG value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_USB_GUSB2PHYCFG 0UL
#endif

#ifndef MIZAR_USB_GEVNTADRLO
// MANUAL_REVIEW: MIZAR_USB_GEVNTADRLO value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_USB_GEVNTADRLO 0UL
#endif

#ifndef MIZAR_USB_GEVNTADRHI
// MANUAL_REVIEW: MIZAR_USB_GEVNTADRHI value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_USB_GEVNTADRHI 0UL
#endif

#ifndef MIZAR_USB_GEVNTSIZ
// MANUAL_REVIEW: MIZAR_USB_GEVNTSIZ value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_USB_GEVNTSIZ 0UL
#endif

#ifndef MIZAR_USB_GEVNTCOUNT
// MANUAL_REVIEW: MIZAR_USB_GEVNTCOUNT value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_USB_GEVNTCOUNT 0UL
#endif

#ifndef MIZAR_USB_GCTL
// MANUAL_REVIEW: MIZAR_USB_GCTL value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_USB_GCTL 0UL
#endif

#ifndef MIZAR_USB_DCFG
// MANUAL_REVIEW: MIZAR_USB_DCFG value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_USB_DCFG 0UL
#endif

#ifndef MIZAR_USB_DEVTEN
// MANUAL_REVIEW: MIZAR_USB_DEVTEN value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_USB_DEVTEN 0UL
#endif

#ifndef MIZAR_USB_GUCTL
// MANUAL_REVIEW: MIZAR_USB_GUCTL value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_USB_GUCTL 0UL
#endif

#ifndef MIZAR_USB_DEPCMDPAR0
// MANUAL_REVIEW: MIZAR_USB_DEPCMDPAR0 value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_USB_DEPCMDPAR0 0UL
#endif

#ifndef MIZAR_USB_DEPCMD
// MANUAL_REVIEW: MIZAR_USB_DEPCMD value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_USB_DEPCMD 0UL
#endif

#ifndef MIZAR_USB_DALEPENA
// MANUAL_REVIEW: MIZAR_USB_DALEPENA value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_USB_DALEPENA 0UL
#endif

#ifndef MIZAR_LSS_SYSREG_INTR_EN0
// MANUAL_REVIEW: MIZAR_LSS_SYSREG_INTR_EN0 value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_LSS_SYSREG_INTR_EN0 0UL
#endif

#ifndef MIZAR_USB_DSTS
// MANUAL_REVIEW: MIZAR_USB_DSTS value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_USB_DSTS 0UL
#endif

#ifndef MIZAR_USB_DEPCMDPAR1
// MANUAL_REVIEW: MIZAR_USB_DEPCMDPAR1 value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_USB_DEPCMDPAR1 0UL
#endif

#ifndef MIZAR_LSS_SYSREG_MSK_STS0
// MANUAL_REVIEW: MIZAR_LSS_SYSREG_MSK_STS0 value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_LSS_SYSREG_MSK_STS0 0UL
#endif

#ifndef MIZAR_LSS_SYSREG_RAW_STCR0
// MANUAL_REVIEW: MIZAR_LSS_SYSREG_RAW_STCR0 value not provided in Meta TestPlan JSON. Define in platform header.
#define MIZAR_LSS_SYSREG_RAW_STCR0 0UL
#endif

#ifndef Buffer_PointerLO_1
// MANUAL_REVIEW: Buffer_PointerLO_1 value not provided in Meta TestPlan JSON. Define in platform header.
#define Buffer_PointerLO_1 0UL
#endif

#ifndef Default_Event_Ring_Array
// MANUAL_REVIEW: Default_Event_Ring_Array value not provided in Meta TestPlan JSON. Define in platform header.
#define Default_Event_Ring_Array 0UL
#endif

#ifndef DWORD
#define DWORD unsigned int
#endif

/* Array from Meta TestPlan JSON */
static DWORD buf_data[USB_FS_BULK_BUF_DATA_SIZE];

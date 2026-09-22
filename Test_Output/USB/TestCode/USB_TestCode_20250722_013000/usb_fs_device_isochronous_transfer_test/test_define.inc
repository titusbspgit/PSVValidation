// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: USB_FS_Device_Isochronous_Transfer_test
 * Purpose: Defines headers, macros, arrays, and supporting testcase data for
 *          USB Full-Speed Device mode Isochronous Transfer validation on the
 *          DWC USB3 controller.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "usb.h"

/* ------------------------------------------------------------------ */
/* Macros from Meta TestPlan JSON                                     */
/* These macros are expected to be defined by the platform/IP headers. */
/* Guard each with #ifndef so platform headers take precedence.       */
/* ------------------------------------------------------------------ */

#ifndef Buffer_PointerLO
/* MANUAL_REVIEW: Buffer_PointerLO address value must be provided by platform headers */
#endif

#ifndef event_trb_addr
/* MANUAL_REVIEW: event_trb_addr address value must be provided by platform headers */
#endif

#ifndef MIZAR_USB_DCTL
/* MANUAL_REVIEW: MIZAR_USB_DCTL register address must be provided by platform headers */
#endif

#ifndef MIZAR_USB_GUSB2PHYCFG
/* MANUAL_REVIEW: MIZAR_USB_GUSB2PHYCFG register address must be provided by platform headers */
#endif

#ifndef MIZAR_USB_GEVNTADRLO
/* MANUAL_REVIEW: MIZAR_USB_GEVNTADRLO register address must be provided by platform headers */
#endif

#ifndef MIZAR_USB_GEVNTADRHI
/* MANUAL_REVIEW: MIZAR_USB_GEVNTADRHI register address must be provided by platform headers */
#endif

#ifndef MIZAR_USB_GEVNTSIZ
/* MANUAL_REVIEW: MIZAR_USB_GEVNTSIZ register address must be provided by platform headers */
#endif

#ifndef MIZAR_USB_GEVNTCOUNT
/* MANUAL_REVIEW: MIZAR_USB_GEVNTCOUNT register address must be provided by platform headers */
#endif

#ifndef MIZAR_USB_GCTL
/* MANUAL_REVIEW: MIZAR_USB_GCTL register address must be provided by platform headers */
#endif

#ifndef MIZAR_USB_DCFG
/* MANUAL_REVIEW: MIZAR_USB_DCFG register address must be provided by platform headers */
#endif

#ifndef MIZAR_USB_DEVTEN
/* MANUAL_REVIEW: MIZAR_USB_DEVTEN register address must be provided by platform headers */
#endif

#ifndef MIZAR_USB_GUCTL
/* MANUAL_REVIEW: MIZAR_USB_GUCTL register address must be provided by platform headers */
#endif

#ifndef MIZAR_USB_DEPCMDPAR0
/* MANUAL_REVIEW: MIZAR_USB_DEPCMDPAR0 register address must be provided by platform headers */
#endif

#ifndef MIZAR_USB_DEPCMD
/* MANUAL_REVIEW: MIZAR_USB_DEPCMD register address must be provided by platform headers */
#endif

#ifndef MIZAR_USB_DALEPENA
/* MANUAL_REVIEW: MIZAR_USB_DALEPENA register address must be provided by platform headers */
#endif

#ifndef MIZAR_LSS_SYSREG_INTR_EN0
/* MANUAL_REVIEW: MIZAR_LSS_SYSREG_INTR_EN0 register address must be provided by platform headers */
#endif

#ifndef MIZAR_USB_DSTS
/* MANUAL_REVIEW: MIZAR_USB_DSTS register address must be provided by platform headers */
#endif

#ifndef MIZAR_USB_DEPCMDPAR1
/* MANUAL_REVIEW: MIZAR_USB_DEPCMDPAR1 register address must be provided by platform headers */
#endif

#ifndef Buffer_PointerLO_1
/* MANUAL_REVIEW: Buffer_PointerLO_1 address value must be provided by platform headers */
#endif

#ifndef MIZAR_LSS_SYSREG_MSK_STS0
/* MANUAL_REVIEW: MIZAR_LSS_SYSREG_MSK_STS0 register address must be provided by platform headers */
#endif

#ifndef MIZAR_LSS_SYSREG_RAW_STCR0
/* MANUAL_REVIEW: MIZAR_LSS_SYSREG_RAW_STCR0 register address must be provided by platform headers */
#endif

#ifndef DWORD
#define DWORD 4U
#endif

#ifndef Default_Event_Ring_Array
/* MANUAL_REVIEW: Default_Event_Ring_Array value must be provided by platform headers */
#endif

/* ------------------------------------------------------------------ */
/* Arrays from Meta TestPlan JSON                                     */
/* ------------------------------------------------------------------ */
static unsigned char buf_data[16];

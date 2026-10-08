// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#ifndef USB_FS_DEVICE_BULK_TRANSFER_TEST_H
#define USB_FS_DEVICE_BULK_TRANSFER_TEST_H

#include <test.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "usb.h"
#include "log.h"
#include "mmio.h"
#include "reg_access.h"
#include "gic_funcs.h"

/* Meta Macros - USB register and address definitions */
/* MANUAL_REVIEW: The following macros are expected to be provided by usb.h or project headers. */
/* If not defined, they must be supplied by the project configuration. */
#ifndef MIZAR_USB_DEPCMD
/* MANUAL_REVIEW: MIZAR_USB_DEPCMD address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_USB_DEPCMDPAR0
/* MANUAL_REVIEW: MIZAR_USB_DEPCMDPAR0 address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_USB_DEPCMDPAR1
/* MANUAL_REVIEW: MIZAR_USB_DEPCMDPAR1 address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_USB_GCTL
/* MANUAL_REVIEW: MIZAR_USB_GCTL address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_USB_GUCTL
/* MANUAL_REVIEW: MIZAR_USB_GUCTL address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_USB_DEVTEN
/* MANUAL_REVIEW: MIZAR_USB_DEVTEN address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_USB_GEVNTADRLO
/* MANUAL_REVIEW: MIZAR_USB_GEVNTADRLO address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_USB_GEVNTADRHI
/* MANUAL_REVIEW: MIZAR_USB_GEVNTADRHI address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_USB_GEVNTSIZ
/* MANUAL_REVIEW: MIZAR_USB_GEVNTSIZ address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_USB_GEVNTCOUNT
/* MANUAL_REVIEW: MIZAR_USB_GEVNTCOUNT address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_USB_DCFG
/* MANUAL_REVIEW: MIZAR_USB_DCFG address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_USB_DSTS
/* MANUAL_REVIEW: MIZAR_USB_DSTS address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_USB_DCTL
/* MANUAL_REVIEW: MIZAR_USB_DCTL address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_USB_DALEPENA
/* MANUAL_REVIEW: MIZAR_USB_DALEPENA address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_USB_GUSB2PHYCFG
/* MANUAL_REVIEW: MIZAR_USB_GUSB2PHYCFG address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_USB_GFLADJ
/* MANUAL_REVIEW: MIZAR_USB_GFLADJ address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_USB_BASE
/* MANUAL_REVIEW: MIZAR_USB_BASE address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_LSS_SYSREG_MSK_STS0
/* MANUAL_REVIEW: MIZAR_LSS_SYSREG_MSK_STS0 address not resolved - must be supplied by project headers. */
#endif

#ifndef MIZAR_LSS_SYSREG_RAW_STCR0
/* MANUAL_REVIEW: MIZAR_LSS_SYSREG_RAW_STCR0 address not resolved - must be supplied by project headers. */
#endif

#ifndef Buffer_PointerLO
/* MANUAL_REVIEW: Buffer_PointerLO address not resolved - must be supplied by project headers. */
#endif

#ifndef Buffer_PointerLO_1
/* MANUAL_REVIEW: Buffer_PointerLO_1 address not resolved - must be supplied by project headers. */
#endif

#ifndef event_trb_addr
/* MANUAL_REVIEW: event_trb_addr address not resolved - must be supplied by project headers. */
#endif

#ifndef Default_Event_Ring_Array
/* MANUAL_REVIEW: Default_Event_Ring_Array address not resolved - must be supplied by project headers. */
#endif

#ifndef DWORD
/* MANUAL_REVIEW: DWORD stride value not resolved - must be supplied by project headers. */
#endif

/* Meta Arrays */
int buf_data[16];

/* Global variables shared with interrupt handler */
extern volatile int int_pend;
extern int event_counter;

/* IRQ Handler declaration */
void Default_IRQHandler(void);

/* Public FV testcase entry points */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg);
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out);
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem cfg);

#endif /* USB_FS_DEVICE_BULK_TRANSFER_TEST_H */

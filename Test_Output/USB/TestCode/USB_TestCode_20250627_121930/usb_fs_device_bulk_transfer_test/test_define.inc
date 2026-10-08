// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

#include <stdio.h>
#include <stdlib.h>
#include "usb.h"
#include <stdint.h>
#include "log.h"
#include "reg_access.h"
#include "gic_funcs.h"

/* Meta Macros - these are expected to be defined in usb.h or project headers. */
/* If not defined, MANUAL_REVIEW is required for each undefined macro. */
#ifndef MIZAR_USB_DCFG
/* MANUAL_REVIEW: MIZAR_USB_DCFG address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_DEPCMDPAR1
/* MANUAL_REVIEW: MIZAR_USB_DEPCMDPAR1 address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_DEPCMDPAR0
/* MANUAL_REVIEW: MIZAR_USB_DEPCMDPAR0 address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_DEPCMD
/* MANUAL_REVIEW: MIZAR_USB_DEPCMD address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_GEVNTADRLO
/* MANUAL_REVIEW: MIZAR_USB_GEVNTADRLO address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_GEVNTADRHI
/* MANUAL_REVIEW: MIZAR_USB_GEVNTADRHI address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_GEVNTSIZ
/* MANUAL_REVIEW: MIZAR_USB_GEVNTSIZ address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_GEVNTCOUNT
/* MANUAL_REVIEW: MIZAR_USB_GEVNTCOUNT address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_GCTL
/* MANUAL_REVIEW: MIZAR_USB_GCTL address not provided by Meta, expected from usb.h */
#endif

#ifndef Buffer_PointerLO
/* MANUAL_REVIEW: Buffer_PointerLO address not provided by Meta, expected from usb.h */
#endif

#ifndef Default_Event_Ring_Array
/* MANUAL_REVIEW: Default_Event_Ring_Array address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_LSS_SYSREG_MSK_STS0
/* MANUAL_REVIEW: MIZAR_LSS_SYSREG_MSK_STS0 address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_LSS_SYSREG_RAW_STCR0
/* MANUAL_REVIEW: MIZAR_LSS_SYSREG_RAW_STCR0 address not provided by Meta, expected from usb.h */
#endif

#ifndef DWORD
/* MANUAL_REVIEW: DWORD value not provided by Meta, expected from usb.h */
#endif

#ifndef event_trb_addr
/* MANUAL_REVIEW: event_trb_addr address not provided by Meta, expected from usb.h */
#endif

#ifndef Buffer_PointerLO_1
/* MANUAL_REVIEW: Buffer_PointerLO_1 address not provided by Meta, expected from usb.h */
#endif

/* Meta Arrays */
int buf_data[16];

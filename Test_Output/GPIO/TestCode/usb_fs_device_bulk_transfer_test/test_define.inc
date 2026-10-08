// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

#include <stdio.h>
#include <stdlib.h>
#include "usb.h"
#include "log.h"

/* Meta Macros - USB register and address definitions */
/* MANUAL_REVIEW: The following macros are expected to be provided by usb.h or platform headers. */
/* If not defined, uncomment and provide correct values. */
/*
#ifndef MIZAR_USB_DEPCMD
#define MIZAR_USB_DEPCMD <address>
#endif
#ifndef MIZAR_USB_DEPCMDPAR0
#define MIZAR_USB_DEPCMDPAR0 <address>
#endif
#ifndef MIZAR_USB_DEPCMDPAR1
#define MIZAR_USB_DEPCMDPAR1 <address>
#endif
#ifndef MIZAR_USB_GCTL
#define MIZAR_USB_GCTL <address>
#endif
#ifndef MIZAR_USB_GUCTL
#define MIZAR_USB_GUCTL <address>
#endif
#ifndef MIZAR_USB_DEVTEN
#define MIZAR_USB_DEVTEN <address>
#endif
#ifndef MIZAR_USB_GEVNTADRLO
#define MIZAR_USB_GEVNTADRLO <address>
#endif
#ifndef MIZAR_USB_GEVNTADRHI
#define MIZAR_USB_GEVNTADRHI <address>
#endif
#ifndef MIZAR_USB_GEVNTSIZ
#define MIZAR_USB_GEVNTSIZ <address>
#endif
#ifndef MIZAR_USB_GEVNTCOUNT
#define MIZAR_USB_GEVNTCOUNT <address>
#endif
#ifndef MIZAR_USB_DCFG
#define MIZAR_USB_DCFG <address>
#endif
#ifndef MIZAR_USB_DSTS
#define MIZAR_USB_DSTS <address>
#endif
#ifndef MIZAR_USB_DCTL
#define MIZAR_USB_DCTL <address>
#endif
#ifndef MIZAR_USB_DALEPENA
#define MIZAR_USB_DALEPENA <address>
#endif
#ifndef MIZAR_USB_GUSB2PHYCFG
#define MIZAR_USB_GUSB2PHYCFG <address>
#endif
#ifndef MIZAR_USB_GFLADJ
#define MIZAR_USB_GFLADJ <address>
#endif
#ifndef MIZAR_USB_BASE
#define MIZAR_USB_BASE <address>
#endif
#ifndef MIZAR_LSS_SYSREG_MSK_STS0
#define MIZAR_LSS_SYSREG_MSK_STS0 <address>
#endif
#ifndef MIZAR_LSS_SYSREG_RAW_STCR0
#define MIZAR_LSS_SYSREG_RAW_STCR0 <address>
#endif
#ifndef Buffer_PointerLO
#define Buffer_PointerLO <address>
#endif
#ifndef Buffer_PointerLO_1
#define Buffer_PointerLO_1 <address>
#endif
#ifndef event_trb_addr
#define event_trb_addr <address>
#endif
#ifndef Default_Event_Ring_Array
#define Default_Event_Ring_Array <address>
#endif
#ifndef DWORD
#define DWORD 4
#endif
*/

/* Meta Arrays */
int buf_data[16];

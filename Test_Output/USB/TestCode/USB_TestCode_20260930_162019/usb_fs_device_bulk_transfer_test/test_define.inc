// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

#include <stdio.h>
#include <stdlib.h>
#include "usb.h"

/* MANUAL_REVIEW: Confirm DWORD macro value; typically 4 bytes */
#ifndef DWORD
#define DWORD 4U
#endif

/* MANUAL_REVIEW: Confirm Buffer_PointerLO address from project/SoC memory map */
/* MANUAL_REVIEW: Confirm Buffer_PointerLO_1 address from project/SoC memory map */
/* MANUAL_REVIEW: Confirm event_trb_addr address from project/SoC memory map */
/* MANUAL_REVIEW: Confirm Default_Event_Ring_Array address from project/SoC memory map */

/* MANUAL_REVIEW: The following register/address macros must be provided by usb.h or project headers. */
/* If not already defined, uncomment and set correct values: */
/*
#define Buffer_PointerLO        0xXXXXXXXXU
#define Buffer_PointerLO_1      0xXXXXXXXXU
#define event_trb_addr          0xXXXXXXXXU
#define Default_Event_Ring_Array 0xXXXXXXXXU
#define MIZAR_USB_DCTL          0xXXXXXXXXU
#define MIZAR_USB_GUSB2PHYCFG   0xXXXXXXXXU
#define MIZAR_USB_GEVNTADRLO    0xXXXXXXXXU
#define MIZAR_USB_GEVNTADRHI    0xXXXXXXXXU
#define MIZAR_USB_GEVNTSIZ      0xXXXXXXXXU
#define MIZAR_USB_GEVNTCOUNT    0xXXXXXXXXU
#define MIZAR_USB_GCTL          0xXXXXXXXXU
#define MIZAR_USB_DCFG          0xXXXXXXXXU
#define MIZAR_USB_DEVTEN        0xXXXXXXXXU
#define MIZAR_USB_GUCTL         0xXXXXXXXXU
#define MIZAR_USB_DEPCMDPAR0    0xXXXXXXXXU
#define MIZAR_USB_DEPCMDPAR1    0xXXXXXXXXU
#define MIZAR_USB_DEPCMD        0xXXXXXXXXU
#define MIZAR_USB_DALEPENA      0xXXXXXXXXU
#define MIZAR_USB_DSTS          0xXXXXXXXXU
#define MIZAR_LSS_SYSREG_INTR_EN0   0xXXXXXXXXU
#define MIZAR_LSS_SYSREG_MSK_STS0   0xXXXXXXXXU
#define MIZAR_LSS_SYSREG_RAW_STCR0  0xXXXXXXXXU
*/

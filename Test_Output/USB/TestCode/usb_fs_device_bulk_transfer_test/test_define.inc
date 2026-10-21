// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>
#include <stdio.h>
#include <stdlib.h>
#include "usb.h"
#include "log.h"
#include "reg_access.h"
#include "gic_funcs.h"

// Meta Macros - all supplied macros from Meta
// MANUAL_REVIEW: Verify these macro definitions match the platform header values.
// These macros are expected to be defined in usb.h or platform headers.
// If not defined there, uncomment and set correct values below.

// #ifndef Buffer_PointerLO
// #define Buffer_PointerLO / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef Buffer_PointerLO_1
// #define Buffer_PointerLO_1 / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef event_trb_addr
// #define event_trb_addr / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef DWORD
// #define DWORD / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef Default_Event_Ring_Array
// #define Default_Event_Ring_Array / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef MIZAR_USB_GEVNTADRLO
// #define MIZAR_USB_GEVNTADRLO / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef MIZAR_USB_GEVNTADRHI
// #define MIZAR_USB_GEVNTADRHI / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef MIZAR_USB_GEVNTSIZ
// #define MIZAR_USB_GEVNTSIZ / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef MIZAR_USB_GEVNTCOUNT
// #define MIZAR_USB_GEVNTCOUNT / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef MIZAR_USB_GCTL
// #define MIZAR_USB_GCTL / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef MIZAR_USB_DEPCMDPAR1
// #define MIZAR_USB_DEPCMDPAR1 / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef MIZAR_USB_DEPCMDPAR0
// #define MIZAR_USB_DEPCMDPAR0 / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef MIZAR_USB_DEPCMD
// #define MIZAR_USB_DEPCMD / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef MIZAR_USB_DCFG
// #define MIZAR_USB_DCFG / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef MIZAR_USB_DSTS
// #define MIZAR_USB_DSTS / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef MIZAR_USB_DCTL
// #define MIZAR_USB_DCTL / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef MIZAR_USB_DALEPENA
// #define MIZAR_USB_DALEPENA / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef MIZAR_USB_GUSB2PHYCFG
// #define MIZAR_USB_GUSB2PHYCFG / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef MIZAR_LSS_SYSREG_MSK_STS0
// #define MIZAR_LSS_SYSREG_MSK_STS0 / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef MIZAR_LSS_SYSREG_RAW_STCR0
// #define MIZAR_LSS_SYSREG_RAW_STCR0 / MANUAL_REVIEW: set platform-specific value /
// #endif

// #ifndef MIZAR_LSS_SYSREG_INTR_EN0
// #define MIZAR_LSS_SYSREG_INTR_EN0 / MANUAL_REVIEW: set platform-specific value */
// #endif

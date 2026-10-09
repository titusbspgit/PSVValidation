// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>
#include <stdio.h>
#include <stdlib.h>
#include "usb.h"
#include "log.h"
#include "reg_access.h"
#include "gic_funcs.h"

// Meta Macros: supplied by project headers (usb.h or equivalent).
// MIZAR_USB_DCFG, MIZAR_USB_DEPCMDPAR1, MIZAR_USB_DEPCMDPAR0,
// MIZAR_USB_DEPCMD, MIZAR_USB_GEVNTADRLO, MIZAR_USB_GEVNTADRHI,
// MIZAR_USB_GEVNTSIZ, MIZAR_USB_GEVNTCOUNT, MIZAR_USB_GCTL,
// Buffer_PointerLO, Default_Event_Ring_Array,
// MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0,
// DWORD, event_trb_addr, Buffer_PointerLO_1
// These are expected to be provided by usb.h or project headers.

// Volatile int_pend flag for interrupt-driven polling
volatile int int_pend;

// Global event counter updated by IRQ handler
int event_counter;

// Meta Arrays
int buf_data[16];

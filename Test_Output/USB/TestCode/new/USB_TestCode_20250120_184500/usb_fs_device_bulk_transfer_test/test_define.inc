// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>
#include <stdio.h>
#include <stdlib.h>
#include "usb.h"
#include "log.h"
#include "reg_access.h"
#include "gic_funcs.h"

// MANUAL_REVIEW: The following macros are expected to be defined in usb.h.
// If usb.h does not provide them, their definitions must be supplied here.
// Register address macros (expected from usb.h):
// MIZAR_USB_DCTL
// MIZAR_USB_DCFG
// MIZAR_USB_DSTS
// MIZAR_USB_GCTL
// MIZAR_USB_GUCTL
// MIZAR_USB_GUSB2PHYCFG
// MIZAR_USB_GEVNTADRLO
// MIZAR_USB_GEVNTADRHI
// MIZAR_USB_GEVNTSIZ
// MIZAR_USB_GEVNTCOUNT
// MIZAR_USB_DEVTEN
// MIZAR_USB_DALEPENA
// MIZAR_USB_DEPCMD
// MIZAR_USB_DEPCMDPAR0
// MIZAR_USB_DEPCMDPAR1
// MIZAR_LSS_SYSREG_MSK_STS0
// MIZAR_LSS_SYSREG_RAW_STCR0
// MIZAR_LSS_SYSREG_INTR_EN0
// Buffer/address macros (expected from usb.h):
// Buffer_PointerLO
// Buffer_PointerLO_1
// event_trb_addr
// Default_Event_Ring_Array
// DWORD

// Runtime condition 2: Required declarations
extern volatile int int_pend;
extern void nic_programming(void);

// Global event counter - converted to volatile unsigned per runtime conditions
volatile unsigned int event_counter;

// Meta Arrays: buf_data[16] from test_case() local scope
// Preserved but declared at file scope to avoid duplication
// Runtime condition 1: converted to unsigned int
unsigned int buf_data[16];

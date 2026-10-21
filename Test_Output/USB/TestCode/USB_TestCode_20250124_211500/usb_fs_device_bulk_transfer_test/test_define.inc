// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>
#include <stdio.h>
#include "usb.h"
#include "log.h"
#include "reg_access.h"
#include "gic_funcs.h"

// nic_programming prototype (runtime condition 2)
void nic_programming(void);

// Interrupt pending variable (runtime condition 2: volatile int)
volatile int int_pend;

// Event counter (DV file scope)
int event_counter;

// Meta Arrays: buf_data declared in DV local scope, preserved here
int buf_data[16];

// MANUAL_REVIEW: The following macros are presumed defined in usb.h.
// If usb.h does not provide them, they must be defined here:
// MIZAR_LSS_SYSREG_MSK_STS0
// MIZAR_LSS_SYSREG_RAW_STCR0
// MIZAR_LSS_SYSREG_INTR_EN0
// MIZAR_USB_GEVNTCOUNT
// MIZAR_USB_GEVNTADRLO
// MIZAR_USB_GEVNTADRHI
// MIZAR_USB_GEVNTSIZ
// MIZAR_USB_GCTL
// MIZAR_USB_DCFG
// MIZAR_USB_DSTS
// MIZAR_USB_DCTL
// MIZAR_USB_DALEPENA
// MIZAR_USB_GUSB2PHYCFG
// MIZAR_USB_DEPCMDPAR1
// MIZAR_USB_DEPCMDPAR0
// MIZAR_USB_DEPCMD
// MIZAR_USB_DEVTEN
// MIZAR_USB_GUCTL
// Buffer_PointerLO
// Buffer_PointerLO_1
// event_trb_addr
// Default_Event_Ring_Array
// DWORD

// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>
#include <stdio.h>
#include <stdlib.h>
#include "usb.h"
#include "reg_access.h"
#include "mmio.h"
#include "log.h"
// MANUAL_REVIEW: GIC/IRQ APIs (GIC_EnableAllIRQ, GIC_ClearIRQ) are used in this testcase.
// Ensure the correct project header providing these APIs is included.
// The header name could not be determined from Meta and is not invented here.

// MANUAL_REVIEW: nic_programming() is called in the testcase.
// Ensure the correct project header providing this API is included.

// Macros used in testcase - definitions expected from "usb.h" or its transitive includes:
// MIZAR_USB_DCTL
// MIZAR_USB_GUSB2PHYCFG
// MIZAR_USB_GEVNTADRLO
// MIZAR_USB_GEVNTADRHI
// MIZAR_USB_GEVNTSIZ
// MIZAR_USB_GEVNTCOUNT
// MIZAR_USB_GCTL
// MIZAR_USB_DCFG
// MIZAR_USB_DEVTEN
// MIZAR_USB_GUCTL
// MIZAR_USB_DALEPENA
// MIZAR_USB_DSTS
// MIZAR_USB_DEPCMDPAR1
// MIZAR_USB_DEPCMDPAR0
// MIZAR_USB_DEPCMD
// MIZAR_LSS_SYSREG_INTR_EN0
// MIZAR_LSS_SYSREG_MSK_STS0
// MIZAR_LSS_SYSREG_RAW_STCR0
// Buffer_PointerLO
// Buffer_PointerLO_1
// Default_Event_Ring_Array
// event_trb_addr
// DWORD

// Arrays from Meta (declared at local scope in original source)
// MANUAL_REVIEW: buf_data was a local variable in original DV test_case().
// Declared here at file scope for PSV. Verify storage duration is acceptable.
int buf_data[16];

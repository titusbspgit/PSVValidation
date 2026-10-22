// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>
#include <stdio.h>
#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"
#include "gic_funcs.h"
#include "usb.h"

// External function prototype for nic_programming (implementation not supplied)
void nic_programming(void);

// Interrupt pending variable - volatile for ISR synchronization
volatile int int_pend;

// Event counter - updated by ISR, volatile for cross-context visibility
volatile unsigned int event_counter;

// MANUAL_REVIEW: The following macros/symbols are expected to be defined in "usb.h"
// which was not supplied. Verify they are available at compile time:
// MIZAR_USB_DEPCMDPAR1, MIZAR_USB_DEPCMDPAR0, MIZAR_USB_DEPCMD,
// MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0,
// MIZAR_USB_GEVNTCOUNT, MIZAR_USB_GEVNTADRLO, MIZAR_USB_GEVNTADRHI,
// MIZAR_USB_GEVNTSIZ, MIZAR_USB_GCTL, MIZAR_USB_DCFG, MIZAR_USB_DSTS,
// MIZAR_USB_DCTL, MIZAR_USB_DALEPENA, MIZAR_USB_GUSB2PHYCFG,
// MIZAR_USB_DEVTEN, MIZAR_USB_GUCTL, MIZAR_LSS_SYSREG_INTR_EN0,
// Buffer_PointerLO, Buffer_PointerLO_1, event_trb_addr,
// Default_Event_Ring_Array, DWORD

// buf_data array from Meta Arrays (declared locally in DV, preserved here)
// MANUAL_REVIEW: buf_data was declared locally in DV test_case() with no initializer.
// It is not used in the active DV code path. Removed per runtime condition 3 (unused variable cleanup).

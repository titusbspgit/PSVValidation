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

// Meta Macros - all referenced macros are defined in usb.h:
// Buffer_PointerLO
// event_trb_addr
// DWORD
// Default_Event_Ring_Array
// MIZAR_USB_GEVNTADRLO
// MIZAR_USB_GEVNTADRHI
// MIZAR_USB_GEVNTSIZ
// MIZAR_USB_GEVNTCOUNT
// MIZAR_USB_GCTL
// MIZAR_USB_DEPCMDPAR1
// MIZAR_USB_DEPCMDPAR0
// MIZAR_USB_DEPCMD
// MIZAR_USB_DCFG
// MIZAR_USB_DSTS
// MIZAR_USB_DCTL
// MIZAR_USB_DALEPENA
// MIZAR_LSS_SYSREG_MSK_STS0
// MIZAR_LSS_SYSREG_RAW_STCR0
// MIZAR_LSS_SYSREG_INTR_EN0
// Buffer_PointerLO_1

// Global declarations
extern int int_pend;
extern int event_counter;

// MANUAL_REVIEW: nic_programming() is called in init but not defined in Meta - ensure prototype is available from project headers
// Helper function prototypes
void Default_IRQHandler(void);
void set_configuration(int trb_address, int parameter0, int parameter1, int cmd);
void setup_stage(void);
void status_stage(void);
void enumeration(void);

// Public testcase function prototypes
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg);

int usb_fs_device_bulk_transfer_test_run(
 const TestsItem *cfg,
 TestOutput *out
);

int usb_fs_device_bulk_transfer_test_teardown(
 const TestsItem *cfg
);

#endif // USB_FS_DEVICE_BULK_TRANSFER_TEST_H

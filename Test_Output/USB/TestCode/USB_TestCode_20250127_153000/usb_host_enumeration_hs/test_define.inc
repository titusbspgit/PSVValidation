// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

#include <stdio.h>
#include <stdlib.h>
#include "usb.h"
#include "log.h"
#include "reg_access.h"
#include "gic_funcs.h"

// MANUAL_REVIEW: Verify that the following macros are defined by included project headers:
// MIZAR_USB_BASE
// MIZAR_USB_HCSPARAMS1
// MIZAR_USB_HCSPARAMS2
// MIZAR_USB_SUPTPRT2_DW2
// MIZAR_USB_SUPTPRT3_DW2
// MIZAR_USB_PAGESIZE
// MIZAR_USB_CONFIG
// MIZAR_USB_USBCMD
// MIZAR_USB_USBSTS
// MIZAR_USB_DNCTRL
// MIZAR_USB_CRCR_LO
// MIZAR_USB_CRCR_HI
// MIZAR_USB_DCBAAP_LO
// MIZAR_USB_DCBAAP_HI
// MIZAR_USB_PORTSC_20
// MIZAR_USB_IMAN
// MIZAR_USB_IMOD
// MIZAR_USB_ERSTSZ
// MIZAR_USB_ERSTBA_LO
// MIZAR_USB_ERSTBA_HI
// MIZAR_USB_ERDP_LO
// MIZAR_USB_ERDP_HI
// MIZAR_USB_DB
// MIZAR_LSS_SYSREG_INTR_EN0
// MIZAR_LSS_SYSREG_MSK_STS0
// MIZAR_LSS_SYSREG_RAW_STCR0
// DWORD
// Device_Context_Base_Address_Array
// Device_Context_Array
// Default_Command_Ring
// Default_Event_Ring_Array
// Default_Input_Context
// Event_Ring_Segment_Table
// EP0_TR_Dequeue_Pointer

// Meta Arrays
int data_in[512];
int data_out[512];

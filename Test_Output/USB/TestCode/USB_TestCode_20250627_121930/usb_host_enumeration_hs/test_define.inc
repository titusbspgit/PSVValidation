// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

#include <stdio.h>
#include <stdlib.h>
#include "usb.h"
#include <stdint.h>
#include "log.h"
#include "reg_access.h"
#include "gic_funcs.h"

/* Meta Macros - expected to be defined in usb.h or project headers. */
/* If not defined, MANUAL_REVIEW is required for each undefined macro. */

#ifndef MIZAR_USB_BASE
/* MANUAL_REVIEW: MIZAR_USB_BASE address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_HCSPARAMS1
/* MANUAL_REVIEW: MIZAR_USB_HCSPARAMS1 address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_HCSPARAMS2
/* MANUAL_REVIEW: MIZAR_USB_HCSPARAMS2 address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_SUPTPRT2_DW2
/* MANUAL_REVIEW: MIZAR_USB_SUPTPRT2_DW2 address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_SUPTPRT3_DW2
/* MANUAL_REVIEW: MIZAR_USB_SUPTPRT3_DW2 address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_PAGESIZE
/* MANUAL_REVIEW: MIZAR_USB_PAGESIZE address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_CONFIG
/* MANUAL_REVIEW: MIZAR_USB_CONFIG address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_USBCMD
/* MANUAL_REVIEW: MIZAR_USB_USBCMD address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_USBSTS
/* MANUAL_REVIEW: MIZAR_USB_USBSTS address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_DNCTRL
/* MANUAL_REVIEW: MIZAR_USB_DNCTRL address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_CRCR_LO
/* MANUAL_REVIEW: MIZAR_USB_CRCR_LO address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_CRCR_HI
/* MANUAL_REVIEW: MIZAR_USB_CRCR_HI address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_DCBAAP_LO
/* MANUAL_REVIEW: MIZAR_USB_DCBAAP_LO address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_DCBAAP_HI
/* MANUAL_REVIEW: MIZAR_USB_DCBAAP_HI address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_PORTSC_20
/* MANUAL_REVIEW: MIZAR_USB_PORTSC_20 address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_IMAN
/* MANUAL_REVIEW: MIZAR_USB_IMAN address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_IMOD
/* MANUAL_REVIEW: MIZAR_USB_IMOD address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_ERSTSZ
/* MANUAL_REVIEW: MIZAR_USB_ERSTSZ address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_ERSTBA_LO
/* MANUAL_REVIEW: MIZAR_USB_ERSTBA_LO address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_ERSTBA_HI
/* MANUAL_REVIEW: MIZAR_USB_ERSTBA_HI address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_ERDP_LO
/* MANUAL_REVIEW: MIZAR_USB_ERDP_LO address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_ERDP_HI
/* MANUAL_REVIEW: MIZAR_USB_ERDP_HI address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_USB_DB
/* MANUAL_REVIEW: MIZAR_USB_DB address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_LSS_SYSREG_INTR_EN0
/* MANUAL_REVIEW: MIZAR_LSS_SYSREG_INTR_EN0 address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_LSS_SYSREG_MSK_STS0
/* MANUAL_REVIEW: MIZAR_LSS_SYSREG_MSK_STS0 address not provided by Meta, expected from usb.h */
#endif

#ifndef MIZAR_LSS_SYSREG_RAW_STCR0
/* MANUAL_REVIEW: MIZAR_LSS_SYSREG_RAW_STCR0 address not provided by Meta, expected from usb.h */
#endif

#ifndef DWORD
/* MANUAL_REVIEW: DWORD value not provided by Meta, expected from usb.h */
#endif

#ifndef Device_Context_Base_Address_Array
/* MANUAL_REVIEW: Device_Context_Base_Address_Array address not provided by Meta, expected from usb.h */
#endif

#ifndef Device_Context_Array
/* MANUAL_REVIEW: Device_Context_Array address not provided by Meta, expected from usb.h */
#endif

#ifndef Default_Command_Ring
/* MANUAL_REVIEW: Default_Command_Ring address not provided by Meta, expected from usb.h */
#endif

#ifndef Default_Event_Ring_Array
/* MANUAL_REVIEW: Default_Event_Ring_Array address not provided by Meta, expected from usb.h */
#endif

#ifndef Default_Input_Context
/* MANUAL_REVIEW: Default_Input_Context address not provided by Meta, expected from usb.h */
#endif

#ifndef Event_Ring_Segment_Table
/* MANUAL_REVIEW: Event_Ring_Segment_Table address not provided by Meta, expected from usb.h */
#endif

#ifndef EP0_TR_Dequeue_Pointer
/* MANUAL_REVIEW: EP0_TR_Dequeue_Pointer address not provided by Meta, expected from usb.h */
#endif

/* Meta Arrays */
int data_in[512];
int data_out[512];

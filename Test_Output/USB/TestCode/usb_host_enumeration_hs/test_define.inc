// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

#include <stdio.h>
#include "usb.h"

/* Meta Macros - supplied by project/framework headers */
/* MANUAL_REVIEW: Verify that the following macros are defined in usb.h or */
/* other project headers. If not, provide definitions here. */
/* MIZAR_USB_BASE */
/* MIZAR_USB_CONFIG */
/* MIZAR_USB_DCBAAP_LO */
/* MIZAR_USB_DCBAAP_HI */
/* MIZAR_USB_ERSTSZ */
/* MIZAR_USB_USBSTS */
/* MIZAR_USB_IMAN */
/* MIZAR_USB_ERDP_LO */
/* MIZAR_USB_ERDP_HI */
/* MIZAR_USB_ERSTBA_LO */
/* MIZAR_USB_ERSTBA_HI */
/* MIZAR_USB_IMOD */
/* MIZAR_USB_USBCMD */
/* MIZAR_USB_HCSPARAMS1 */
/* MIZAR_USB_SUPTPRT2_DW2 */
/* MIZAR_USB_SUPTPRT3_DW2 */
/* MIZAR_USB_PORTSC_20 */
/* MIZAR_USB_DBOFF */
/* MIZAR_LSS_SYSREG_INTR_EN0 */
/* MIZAR_LSS_SYSREG_MSK_STS0 */
/* MIZAR_LSS_SYSREG_RAW_STCR0 */
/* USB_PORTSC_20_WCE */
/* USB_PORTSC_20_WDE */
/* USB_PORTSC_20_WOE */
/* Device_Context_Base_Address_Array */
/* Default_Event_Ring_Array */
/* Event_Ring_Segment_Table */
/* EP0_TR_Dequeue_Pointer */
/* Default_Command_Ring */
/* Default_Input_Context */
/* MIZAR_USB_DB */

/* Meta Arrays */
int data_in[512];
int data_out[512];

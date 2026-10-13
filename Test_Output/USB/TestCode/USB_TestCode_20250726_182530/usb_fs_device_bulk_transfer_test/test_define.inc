// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>
#include <stdio.h>
#include <stdlib.h>
#include "usb.h"
#include "log.h"
#include "reg_access.h"
#include "gic_funcs.h"
#include "lss_sysreg/lss_sysreg_offset.h"
#include "lss_sysreg/lss_sysreg_def.h"
#include "test_common.h"

/* USB register macros are expected from usb.h and project headers: */
/* MIZAR_USB_DCFG, MIZAR_USB_DEPCMDPAR1, MIZAR_USB_DEPCMDPAR0, */
/* MIZAR_USB_DEPCMD, MIZAR_USB_GEVNTADRLO, MIZAR_USB_GEVNTADRHI, */
/* MIZAR_USB_GEVNTSIZ, MIZAR_USB_GEVNTCOUNT, MIZAR_USB_GCTL, */
/* MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0 */

#if defined(DDR_TRBS)
#define RAM 0x11A0000000
#else
#define RAM 0xa0240000
#endif

#define Default_Command_Ring (RAM + 0x0)
#define Command_Ring_Collection (RAM + 0x400)
#define Default_Event_Ring_Array (RAM + 0xd00)
#define Event_Ring_Segment_Table (RAM + 0x1800)
#define Transfer_Ring_Collection (RAM + 0x1870)
#define EP0_TR_Dequeue_Pointer (RAM + 0x1870)
#define EP2_In_TR_Dequeue_Pointer (RAM + 0x2200)
#define EP2_Out_TR_Dequeue_Pointer (RAM + 0x2600)
#define Default_Input_Context (RAM + 0x3800)
#define Device_Context_Array (RAM + 0x3940)
#define Device_Context_Base_Address_Array (RAM + 0xf00)

#define SCRATCHPAD0 (RAM + 0x400)
#define SCRATCHPAD1 (RAM + 0x600)
#define Scratchpad_Buffer_Array (RAM + 0x800)

#define DWORD 0x4

#define Buffer_PointerLO (RAM + 0x1100)
#define Buffer_PointerLO_1 (RAM + 0x1220)
#define Buffer_PointerLO_2 (RAM + 0x1420)
#define event_trb_addr (RAM + 0x3000)
#define Default_Event_Ring_Array1 (RAM + 0x2000)

/* Meta Arrays */
int buf_data[16];

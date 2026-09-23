// Author - AI Force 2.3. 24-Jul-2025 04:45 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case  : USB_FS_Device_Bulk_Transfer_test
 * Purpose    : Supporting definitions for USB Full-Speed Device mode Bulk Transfer test.
 */

#include "test.h"

/* Meta TestPlan Headers */
#include <stdio.h>
#include <stdlib.h>
#include "usb.h"

#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* ============================================================ */
/* Macros from Meta TestPlan JSON                               */
/* ============================================================ */

/* Polling timeout constant (FV template timeout pattern) */
#ifndef USB_POLL_TIMEOUT
#define USB_POLL_TIMEOUT 100000U
#endif

/* Buffer and event addresses */
#ifndef Buffer_PointerLO
#define Buffer_PointerLO            0xA0200000UL
#endif

#ifndef Buffer_PointerLO_1
#define Buffer_PointerLO_1          0xA0200040UL
#endif

#ifndef event_trb_addr
#define event_trb_addr              0xA0201000UL
#endif

#ifndef Default_Event_Ring_Array
#define Default_Event_Ring_Array     0xA0202000UL
#endif

/* DWORD type */
#ifndef DWORD
#define DWORD                       uint32_t
#endif

/* USB controller registers */
#ifndef MIZAR_USB_DCTL
#define MIZAR_USB_DCTL              0xA0040C04UL
#endif

#ifndef MIZAR_USB_GUSB2PHYCFG
#define MIZAR_USB_GUSB2PHYCFG       0xA0040200UL
#endif

#ifndef MIZAR_USB_GEVNTADRLO
#define MIZAR_USB_GEVNTADRLO        0xA0040400UL
#endif

#ifndef MIZAR_USB_GEVNTADRHI
#define MIZAR_USB_GEVNTADRHI        0xA0040404UL
#endif

#ifndef MIZAR_USB_GEVNTSIZ
#define MIZAR_USB_GEVNTSIZ          0xA0040408UL
#endif

#ifndef MIZAR_USB_GEVNTCOUNT
#define MIZAR_USB_GEVNTCOUNT        0xA004040CUL
#endif

#ifndef MIZAR_USB_GCTL
#define MIZAR_USB_GCTL              0xA0040110UL
#endif

#ifndef MIZAR_USB_DCFG
#define MIZAR_USB_DCFG              0xA0040C00UL
#endif

#ifndef MIZAR_USB_DEVTEN
#define MIZAR_USB_DEVTEN            0xA0040C08UL
#endif

#ifndef MIZAR_USB_GUCTL
#define MIZAR_USB_GUCTL             0xA0040120UL
#endif

#ifndef MIZAR_USB_DEPCMDPAR0
#define MIZAR_USB_DEPCMDPAR0        0xA0040C08UL
#endif

#ifndef MIZAR_USB_DEPCMDPAR1
#define MIZAR_USB_DEPCMDPAR1        0xA0040C04UL
#endif

#ifndef MIZAR_USB_DEPCMD
#define MIZAR_USB_DEPCMD            0xA0040C0CUL
#endif

#ifndef MIZAR_USB_DALEPENA
#define MIZAR_USB_DALEPENA          0xA0040C20UL
#endif

#ifndef MIZAR_USB_DSTS
#define MIZAR_USB_DSTS              0xA0040C0CUL
#endif

/* LSS system registers */
#ifndef MIZAR_LSS_SYSREG_INTR_EN0
#define MIZAR_LSS_SYSREG_INTR_EN0  0xA0243FFCUL
#endif

#ifndef MIZAR_LSS_SYSREG_MSK_STS0
#define MIZAR_LSS_SYSREG_MSK_STS0  0xA0243FF4UL
#endif

#ifndef MIZAR_LSS_SYSREG_RAW_STCR0
#define MIZAR_LSS_SYSREG_RAW_STCR0 0xA0243FF8UL
#endif

/* ============================================================ */
/* Arrays from Meta TestPlan JSON                               */
/* ============================================================ */

static DWORD buf_data[16];

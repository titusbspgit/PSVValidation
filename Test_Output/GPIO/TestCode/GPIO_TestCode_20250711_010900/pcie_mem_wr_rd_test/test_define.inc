// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_mem_wr_rd_test
 * Purpose: PCIe memory write and read test supporting data, macros, and headers.
 *   Validates memory write/read operations through PCIe slave interfaces
 *   for DM0_RC, DM1_RC, DM0_EP, and DM1_EP modes.
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Compile-time mode defines (provided externally via build system) */
#ifndef DM0_RC
/* DM0_RC: compile-time define for DM0 Root Complex mode */
/* #define DM0_RC */
#endif

#ifndef DM1_RC
/* DM1_RC: compile-time define for DM1 Root Complex mode */
/* #define DM1_RC */
#endif

#ifndef DM0_EP
/* DM0_EP: compile-time define for DM0 Endpoint mode */
/* #define DM0_EP */
#endif

#ifndef DM1_EP
/* DM1_EP: compile-time define for DM1 Endpoint mode */
/* #define DM1_EP */
#endif

#ifndef DM0
/* DM0: compile-time define for DM0 controller selection */
/* #define DM0 */
#endif

#ifndef DM1
/* DM1: compile-time define for DM1 controller selection */
/* #define DM1 */
#endif

#ifndef DEBUG_DISPLAY
/* DEBUG_DISPLAY: compile-time define for debug display output */
/* #define DEBUG_DISPLAY */
#endif

/* Synchronization register address */
#define PCIE_SYNC_REG              0xE6004100UL

/* Synchronization readiness value */
#define PCIE_SYNC_READY_VAL        0x11111111U

/* Synchronization completion value */
#define PCIE_SYNC_COMPLETE_VAL     0x12345678U

/* Polling timeout count for bounded polling loops */
#define PCIE_POLL_TIMEOUT          1000U

/* Arrays field: NA - no arrays provided

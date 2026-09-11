// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_device_enumerate_test
 * Purpose: PCIe device enumeration test supporting data, macros, and headers.
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Macros from Meta TestPlan JSON */
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

#ifndef DEBUG_DISPLAY
/* DEBUG_DISPLAY: compile-time define for debug display output */
/* #define DEBUG_DISPLAY */
#endif

/* Synchronization register address */
#define PCIE_SYNC_REG              0xE6004100UL

/* Synchronization completion value */
#define PCIE_SYNC_COMPLETE_VAL     0x12345678U

/* Polling timeout count for bounded polling loops */
#define PCIE_POLL_TIMEOUT          1000U

/* SII link status mask and expected value */
#define PCIE_SII_LINK_STATUS_MASK  0xD1U
#define PCIE_SII_LINK_STATUS_VAL   0xD1U

/* Arrays field: NA - no arrays provided

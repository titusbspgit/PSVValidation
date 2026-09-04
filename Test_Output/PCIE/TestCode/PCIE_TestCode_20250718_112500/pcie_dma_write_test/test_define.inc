// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name: pcie_dma_write_test
 * Purpose: PCIe DMA write and read-back operations across all four DMA channels (0-3)
 * with link training, BAR programming, source memory preload, GIC/interrupt
 * setup, and DMA channel sequencing.
 */

/* Headers */
#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Macros */
/* Note: DM0_RC, DM1_RC, DM0_EP, DM1_EP, DEBUG_DISPLAY are conditional compilation defines. */
/* They should be defined externally via build configuration as needed. */
/* #define DM0_RC */
/* #define DM1_RC */
/* #define DM0_EP */
/* #define DM1_EP */
/* #define DEBUG_DISPLAY */

/* Arrays: NA - No arrays provided for this testcase */

// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_dma_write_test
 * Purpose:   PCIe DMA write and read-back operations across all four DMA
 *            channels (0-3) with interrupt-driven completion, link training,
 *            BAR programming, memory base programming, and GIC setup.
 */

/* Headers */
#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Macros */
/* DM0_RC - Defined externally via build configuration */
/* DM1_RC - Defined externally via build configuration */
/* DM0_EP - Defined externally via build configuration */
/* DM1_EP - Defined externally via build configuration */
/* DEBUG_DISPLAY - Defined externally via build configuration */

/* Arrays */
/* NA - No arrays provided for this testcase */

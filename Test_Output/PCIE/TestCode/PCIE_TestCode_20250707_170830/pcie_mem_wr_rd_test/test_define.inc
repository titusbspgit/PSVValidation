// Author - AI Force 2.3. 07-Jul-2025 17:08 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_mem_wr_rd_test
 * Purpose: This testcase validates PCIe memory write and read operations through
 *          the PCIe slave interfaces. It performs link training, cache programming,
 *          link status polling, BAR programming, memory base programming, cache
 *          disable, memory write/read operations with verification, and final
 *          synchronization polling.
 */

/* Headers from Meta TestPlan */
#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Macros from Meta TestPlan */
/* Note: DM0_RC, DM1_RC, DM0_EP, DM1_EP, DM0, DM1, DEBUG_DISPLAY are compile-time defines */
/* They are expected to be defined via build configuration or command-line flags */

/* Arrays: NA - No arrays provided in Meta TestPlan */

/* Impacted Registers (reference) */
/* 0xE6004100 - Synchronization register */
/* mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF - PCIE0 coherency control */
/* mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF - PCIE1 coherency control */
/* 0xC0 - SII link status register offset */
/* 0x0 - Vendor ID register offset */
/* 0x4 - Command register offset */

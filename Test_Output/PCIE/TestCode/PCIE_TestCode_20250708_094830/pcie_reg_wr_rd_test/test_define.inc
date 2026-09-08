// Author - AI Force 2.3. 08-Jul-2025 09:18 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_reg_wr_rd_test
 * Purpose: Validates register reset default values and write-read integrity
 *          for PCIe DBI DSP controller registers, SII registers, and PHY
 *          registers across both PCIE0 and PCIE1 controllers.
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include <pcie.h>

/* Register count constants */
#define PCIE_DBI_DSP_REG_COUNT 5U
#define PCIE_SII_REG_COUNT     3U
#define PCIE_PHY_REG_COUNT     3U

/* Test pattern counts */
#define PCIE_CHK_VAL_COUNT     6U
#define PCIE_CHK_VAL_PHY_COUNT 3U

/* PHY reset release value */
#define PCIE_PHY_RST_RELEASE_VAL 0x01203000UL

/* PHY 16-bit extraction mask */
#define PCIE_PHY_16BIT_MASK 0x0000FFFFU

/* PHY 13-bit write mask */
#define PCIE_PHY_13BIT_MASK 0x00001FFFU

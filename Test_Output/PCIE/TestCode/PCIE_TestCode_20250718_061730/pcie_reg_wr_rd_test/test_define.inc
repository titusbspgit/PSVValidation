// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: pcie_reg_wr_rd_test
 * Purpose:   PCIe register reset value verification and write-read verification
 *            for DBI DSP controller registers, SII registers, and PHY registers
 *            across both PCIE0 and PCIE1 ports.
 */

/* Headers */
#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include <pcie.h>

/*
 * Macros: No additional compile-time configuration defines required
 * for this testcase beyond standard headers.
 */

/*
 * Arrays: Declared in source file pcie_reg_wr_rd_test.c
 *   rc0_ctl_addr[5]    - PCIE0 DBI DSP register addresses
 *   rc1_ctl_addr[5]    - PCIE1 DBI DSP register addresses
 *   ctl_default[5]     - Expected reset defaults for DBI DSP registers
 *   sii0_addr[3]       - PCIE0 SII register addresses
 *   sii1_addr[3]       - PCIE1 SII register addresses
 *   sii_default[3]     - Expected reset defaults for SII registers
 *   sii0_write_mask[3] - Write masks for PCIE0 SII registers
 *   sii1_write_mask[3] - Write masks for PCIE1 SII registers
 *   phy0_addr[3]       - PCIE0 PHY register addresses
 *   phy1_addr[3]       - PCIE1 PHY register addresses
 *   phy0_default[3]    - Expected reset defaults for PCIE0 PHY registers
 *   phy1_default[3]    - Expected reset defaults for PCIE1 PHY registers
 *   phy0_write_mask[3] - Write masks for PCIE0 PHY registers
 *   phy1_write_mask[3] - Write masks for PCIE1 PHY registers
 *   chk_val[6]         - 32-bit check value patterns for write-read
 *   chk_val_phy[3]     - 16-bit check value patterns for PHY write-read
 */

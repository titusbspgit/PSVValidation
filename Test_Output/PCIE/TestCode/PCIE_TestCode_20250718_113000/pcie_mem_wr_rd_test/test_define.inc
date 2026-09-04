// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/
 * Test Case Name: pcie_mem_wr_rd_test
 * Purpose: PCIe memory write and read-back verification through the PCIe slave
 * interfaces including link training, cache programming, link-up polling,
 * BAR/mem base programming, cache disable, and memory write-read operations.
 /

/ Headers /
#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/ Macros /
/ Note: DM0_RC, DM1_RC, DM0_EP, DM1_EP, DM0, DM1, DEBUG_DISPLAY are conditional /
/ compilation defines. They should be defined externally via build configuration. /
/ #define DM0_RC /
/ #define DM1_RC /
/ #define DM0_EP /
/ #define DM1_EP /
/ #define DM0 /
/ #define DM1 /
/ #define DEBUG_DISPLAY /

/ Arrays: NA - No arrays provided for this testcase */

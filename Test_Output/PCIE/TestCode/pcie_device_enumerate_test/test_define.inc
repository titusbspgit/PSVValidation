// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/
 * Test Case Name: pcie_device_enumerate_test
 * Purpose: PCIe device enumeration test - link training, cache programming,
 * link status polling, Vendor ID verification, BAR sizing/programming,
 * and final synchronization polling.
 /

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/ Macros from Meta TestPlan /
/ DM0_RC, DM1_RC, DM0_EP, DM1_EP, DEBUG_DISPLAY are expected to be defined /
/ externally via build configuration or project defines as needed. /

/ Arrays: NA - no arrays provided */

// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_device_enumerate_test
 * Purpose:   PCIe device enumeration - link training, cache coherency
 *            programming, link-up polling, Vendor ID read, BAR enumeration
 *            and programming, and final synchronization polling.
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

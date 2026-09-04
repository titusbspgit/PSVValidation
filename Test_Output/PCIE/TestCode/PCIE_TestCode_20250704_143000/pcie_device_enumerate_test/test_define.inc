// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*============================================================================
 * Test Case  : pcie_device_enumerate_test
 * Purpose    : PCIe device enumeration - link training, cache coherency
 *              programming, link-up polling, Vendor ID read, bus master enable,
 *              memory base programming, system register configuration, cache
 *              disable, BAR enumeration and programming, synchronization
 *              polling.
 *============================================================================*/

/*============================================================================
 * HEADERS
 *============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/*============================================================================
 * MACROS
 *============================================================================*/
/* Conditional compilation macros - defined externally at build time */
/* DM0_RC  - DM0 Root Complex mode */
/* DM1_RC  - DM1 Root Complex mode */
/* DM0_EP  - DM0 Endpoint mode */
/* DM1_EP  - DM1 Endpoint mode */
/* DEBUG_DISPLAY - Enable verbose debug logging */

/*============================================================================
 * ARRAYS
 *============================================================================*/
/* Arrays: NA - No testcase-specific arrays required */

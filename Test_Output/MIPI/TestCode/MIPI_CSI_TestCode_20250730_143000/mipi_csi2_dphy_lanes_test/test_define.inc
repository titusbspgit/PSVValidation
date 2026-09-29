// Author - AI Force 2.3. 30-Jul-2025 14:30 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name: mipi_csi2_dphy_lanes_test
 * Purpose: Configures and validates MIPI CSI2 DPHY lane operation including
 *          virtual channel, control data, N_LANES, PHY stop state polling,
 *          interrupt status read, and interrupt masking.
 */

#include "test.h"
#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* Headers field from Meta TestPlan JSON was NA - no additional headers */

/* Macros field from Meta TestPlan JSON was NA - no additional macros */

/* Hardcoded addresses from Impacted Registers */
#define MIPI_CSI2_HARDCODED_ADDR_WRITE  0xa0243ffcUL
#define MIPI_CSI2_HARDCODED_ADDR_READ   0xE6001000UL

/* Polling timeout constant */
#define MIPI_CSI2_POLL_TIMEOUT  10000U

/* Arrays field from Meta TestPlan JSON was NA - no additional arrays */

// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * File: test_define.inc
 * Test Case Name: ethernet1_reg_wr_rd_test
 * Purpose: Testcase supporting data for Ethernet1 register default-value
 *          verification and write-read verification of MAC registers.
 */

#include "test.h"

/* Meta TestPlan Headers */
#include <stdio.h>
#include <stdlib.h>
#include <test_common.h>
#include <ethernet1/ethernet1_def.h>
#include <ethernet1/ethernet1_offset.h>

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* ---- Macros ---- */
// MANUAL_REVIEW: SOFT_RST_REG_ADDRESS value was not provided in the Meta TestPlan JSON. Provide the correct value.
#ifndef SOFT_RST_REG_ADDRESS
#define SOFT_RST_REG_ADDRESS  0x00000000U
#endif

// MANUAL_REVIEW: SOFT_RST_REG_DATA value was not provided in the Meta TestPlan JSON. Provide the correct value.
#ifndef SOFT_RST_REG_DATA
#define SOFT_RST_REG_DATA     0x00000000U
#endif

#define CNT  6U

/* ---- Arrays ---- */

/*
 * addr_array: Register addresses under test.
 * Declared with size 434 per source; only 6 entries populated.
 */
static unsigned int addr_array[434] = {
    mizar_ETHERNET1_MAC_CONFIGURATION,
    mizar_ETHERNET1_MAC_EXT_CONFIGURATION,
    mizar_ETHERNET1_MAC_PACKET_FILTER,
    mizar_ETHERNET1_MAC_WD_JB_TIMEOUT,
    mizar_ETHERNET1_MAC_HASH_TABLE_REG0,
    mizar_ETHERNET1_MAC_HASH_TABLE_REG1
};

/*
 * default_value_array: Expected default reset values for each register.
 * MANUAL_REVIEW: Actual default values were not provided in the Meta TestPlan JSON.
 * Populate with correct default reset values for each register.
 */
static unsigned int default_value_array[434] = {0};

/*
 * read_mask_array: Read masks for each register.
 * MANUAL_REVIEW: Actual read mask values were not provided in the Meta TestPlan JSON.
 * Populate with correct read masks for each register.
 */
static unsigned int read_mask_array[434] = {0};

/*
 * write_mask_array: Write masks for each register.
 * MANUAL_REVIEW: Actual write mask values were not provided in the Meta TestPlan JSON.
 * Populate with correct write masks for each register.
 */
static unsigned int write_mask_array[434] = {0};

/*
 * skip_array: Skip flags for write-read verification.
 * MANUAL_REVIEW: Actual skip flags were not provided in the Meta TestPlan JSON.
 * Populate with correct skip flags (1 = skip, 0 = test).
 */
static unsigned int skip_array[434] = {0};

/*
 * skip_rst_array: Skip flags for default reset value verification.
 * MANUAL_REVIEW: Actual skip reset flags were not provided in the Meta TestPlan JSON.
 * Populate with correct skip reset flags (1 = skip, 0 = test).
 */
static unsigned int skip_rst_array[434] = {0};

/*
 * chk_val: Six test data patterns for write-read verification.
 */
static unsigned int chk_val[6] = {
    0xFFFFFFFFU,
    0xAAAAAAAAU,
    0x55555555U,
    0x00000000U,
    0xA5A5A5A5U,
    0xFFFF0000U
};

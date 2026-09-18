// Author - AI Force 2.3. 18-Jul-2025 01:53 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: ethernet0_reg_wr_rd_test
 * Purpose: Register default-value verification and write-read verification
 *   for Ethernet0 MAC registers (mizar_ETHERNET0_MAC_CONFIGURATION,
 *   mizar_ETHERNET0_MAC_EXT_CONFIGURATION, mizar_ETHERNET0_MAC_PACKET_FILTER,
 *   mizar_ETHERNET0_MAC_WD_JB_TIMEOUT, mizar_ETHERNET0_MAC_HASH_TABLE_REG0).
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <test_common.h>
#include <ethernet0/ethernet0_def.h>
#include <ethernet0/ethernet0_offset.h>

/* Macros from Meta TestPlan */
// MANUAL_REVIEW: SOFT_RST_REG_ADDRESS macro name was provided but no value was supplied in the Meta TestPlan JSON.
// Define the correct value for the target SoC.
#ifndef SOFT_RST_REG_ADDRESS
#define SOFT_RST_REG_ADDRESS 0x00000000U
#endif

// MANUAL_REVIEW: SOFT_RST_REG_DATA macro name was provided but no value was supplied in the Meta TestPlan JSON.
// Define the correct value for the target SoC.
#ifndef SOFT_RST_REG_DATA
#define SOFT_RST_REG_DATA 0x00000000U
#endif

// MANUAL_REVIEW: CNT macro name was provided but no value was supplied in the Meta TestPlan JSON.
// The addr_array is declared with size 434 but only 5 register entries are populated.
// Set CNT to the actual number of populated register entries.
#ifndef CNT
#define CNT 5U
#endif

/* Number of test patterns */
#define CHK_VAL_CNT 6U

/*
 * Register address array.
 * MANUAL_REVIEW: addr_array is declared with size 434 but only 5 register
 * entries are populated per the Meta TestPlan. Fill in the actual register
 * addresses from the SoC register map. The 5 known registers are:
 *   mizar_ETHERNET0_MAC_CONFIGURATION
 *   mizar_ETHERNET0_MAC_EXT_CONFIGURATION
 *   mizar_ETHERNET0_MAC_PACKET_FILTER
 *   mizar_ETHERNET0_MAC_WD_JB_TIMEOUT
 *   mizar_ETHERNET0_MAC_HASH_TABLE_REG0
 */
static const uint32_t addr_array[CNT] = {
    mizar_ETHERNET0_MAC_CONFIGURATION,
    mizar_ETHERNET0_MAC_EXT_CONFIGURATION,
    mizar_ETHERNET0_MAC_PACKET_FILTER,
    mizar_ETHERNET0_MAC_WD_JB_TIMEOUT,
    mizar_ETHERNET0_MAC_HASH_TABLE_REG0
};

/*
 * Default value array.
 * MANUAL_REVIEW: Populate with the actual default reset values for each
 * register listed in addr_array. Values must match the hardware reset state.
 */
static const uint32_t default_value_array[CNT] = {
    0x00000000U, /* MANUAL_REVIEW: default for mizar_ETHERNET0_MAC_CONFIGURATION */
    0x00000000U, /* MANUAL_REVIEW: default for mizar_ETHERNET0_MAC_EXT_CONFIGURATION */
    0x00000000U, /* MANUAL_REVIEW: default for mizar_ETHERNET0_MAC_PACKET_FILTER */
    0x00000000U, /* MANUAL_REVIEW: default for mizar_ETHERNET0_MAC_WD_JB_TIMEOUT */
    0x00000000U  /* MANUAL_REVIEW: default for mizar_ETHERNET0_MAC_HASH_TABLE_REG0 */
};

/*
 * Read mask array.
 * MANUAL_REVIEW: Populate with the actual read masks for each register.
 * A value of 0x00000000 means the register is not readable.
 */
static const uint32_t read_mask_array[CNT] = {
    0xFFFFFFFFU, /* MANUAL_REVIEW: read mask for mizar_ETHERNET0_MAC_CONFIGURATION */
    0xFFFFFFFFU, /* MANUAL_REVIEW: read mask for mizar_ETHERNET0_MAC_EXT_CONFIGURATION */
    0xFFFFFFFFU, /* MANUAL_REVIEW: read mask for mizar_ETHERNET0_MAC_PACKET_FILTER */
    0xFFFFFFFFU, /* MANUAL_REVIEW: read mask for mizar_ETHERNET0_MAC_WD_JB_TIMEOUT */
    0xFFFFFFFFU  /* MANUAL_REVIEW: read mask for mizar_ETHERNET0_MAC_HASH_TABLE_REG0 */
};

/*
 * Write mask array.
 * MANUAL_REVIEW: Populate with the actual write masks for each register.
 * A value of 0x00000000 means the register is not writable.
 */
static const uint32_t write_mask_array[CNT] = {
    0xFFFFFFFFU, /* MANUAL_REVIEW: write mask for mizar_ETHERNET0_MAC_CONFIGURATION */
    0xFFFFFFFFU, /* MANUAL_REVIEW: write mask for mizar_ETHERNET0_MAC_EXT_CONFIGURATION */
    0xFFFFFFFFU, /* MANUAL_REVIEW: write mask for mizar_ETHERNET0_MAC_PACKET_FILTER */
    0xFFFFFFFFU, /* MANUAL_REVIEW: write mask for mizar_ETHERNET0_MAC_WD_JB_TIMEOUT */
    0xFFFFFFFFU  /* MANUAL_REVIEW: write mask for mizar_ETHERNET0_MAC_HASH_TABLE_REG0 */
};

/*
 * Skip array for write-read checks.
 * MANUAL_REVIEW: Set to 1 for registers that should be skipped during
 * write-read verification. Set to 0 to include the register.
 */
static const uint32_t skip_array[CNT] = {
    0U, /* mizar_ETHERNET0_MAC_CONFIGURATION */
    0U, /* mizar_ETHERNET0_MAC_EXT_CONFIGURATION */
    0U, /* mizar_ETHERNET0_MAC_PACKET_FILTER */
    0U, /* mizar_ETHERNET0_MAC_WD_JB_TIMEOUT */
    0U  /* mizar_ETHERNET0_MAC_HASH_TABLE_REG0 */
};

/*
 * Skip reset array for default value checks.
 * MANUAL_REVIEW: Set to 1 for registers that should be skipped during
 * default reset value verification. Set to 0 to include the register.
 */
static const uint32_t skip_rst_array[CNT] = {
    0U, /* mizar_ETHERNET0_MAC_CONFIGURATION */
    0U, /* mizar_ETHERNET0_MAC_EXT_CONFIGURATION */
    0U, /* mizar_ETHERNET0_MAC_PACKET_FILTER */
    0U, /* mizar_ETHERNET0_MAC_WD_JB_TIMEOUT */
    0U  /* mizar_ETHERNET0_MAC_HASH_TABLE_REG0 */
};

/* Test data patterns for write-read verification */
static const uint32_t chk_val[CHK_VAL_CNT] = {
    0xFFFFFFFFU,
    0xAAAAAAAAU,
    0x55555555U,
    0x00000000U,
    0xA5A5A5A5U,
    0xFFFF0000U
};

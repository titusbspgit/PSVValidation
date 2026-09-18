// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: ethernet0_reg_wr_rd_test
 * Purpose:   Register default value verification and write-read verification
 *            on Ethernet MAC registers defined in addr_array.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <test_common.h>
#include <ethernet0/ethernet0_def.h>
#include <ethernet0/ethernet0_offset.h>

/*
 * Macro: CNT
 * Description: Total number of entries in the register arrays.
 *              434 entries declared, 5 populated with Ethernet MAC registers.
 */
#define CNT 434U

/*
 * Arrays: Register address, default value, read mask, write mask, skip,
 *         skip-reset, and check-value pattern arrays.
 *
 * The addr_array contains 5 populated Ethernet MAC register addresses.
 * Remaining entries are initialized to 0.
 * Actual register addresses and mask/default values must be populated
 * from the IP-specific header files (ethernet0_def.h, ethernet0_offset.h).
 */

// MANUAL_REVIEW: The Meta TestPlan JSON declares addr_array[434] with 5 populated
// entries (mizar_ETHERNET0_MAC_CONFIGURATION, mizar_ETHERNET0_MAC_EXT_CONFIGURATION,
// mizar_ETHERNET0_MAC_PACKET_FILTER, mizar_ETHERNET0_MAC_WD_JB_TIMEOUT,
// mizar_ETHERNET0_MAC_HASH_TABLE_REG0) but does not provide the exact address values,
// default values, read masks, or write masks. Populate these arrays with the correct
// values from the IP register specification.

static const unsigned int addr_array[CNT] = {
    mizar_ETHERNET0_MAC_CONFIGURATION,
    mizar_ETHERNET0_MAC_EXT_CONFIGURATION,
    mizar_ETHERNET0_MAC_PACKET_FILTER,
    mizar_ETHERNET0_MAC_WD_JB_TIMEOUT,
    mizar_ETHERNET0_MAC_HASH_TABLE_REG0,
    /* Remaining entries are 0 */
};

// MANUAL_REVIEW: Populate default_value_array with the correct reset/default values
// for each register from the IP register specification.
static const unsigned int default_value_array[CNT] = {
    0x00000000U, /* mizar_ETHERNET0_MAC_CONFIGURATION default */
    0x00000000U, /* mizar_ETHERNET0_MAC_EXT_CONFIGURATION default */
    0x00000000U, /* mizar_ETHERNET0_MAC_PACKET_FILTER default */
    0x00000000U, /* mizar_ETHERNET0_MAC_WD_JB_TIMEOUT default */
    0x00000000U, /* mizar_ETHERNET0_MAC_HASH_TABLE_REG0 default */
    /* Remaining entries are 0 */
};

// MANUAL_REVIEW: Populate read_mask_array with the correct read masks
// for each register from the IP register specification.
static const unsigned int read_mask_array[CNT] = {
    0xFFFFFFFFU, /* mizar_ETHERNET0_MAC_CONFIGURATION read mask */
    0xFFFFFFFFU, /* mizar_ETHERNET0_MAC_EXT_CONFIGURATION read mask */
    0xFFFFFFFFU, /* mizar_ETHERNET0_MAC_PACKET_FILTER read mask */
    0xFFFFFFFFU, /* mizar_ETHERNET0_MAC_WD_JB_TIMEOUT read mask */
    0xFFFFFFFFU, /* mizar_ETHERNET0_MAC_HASH_TABLE_REG0 read mask */
    /* Remaining entries are 0 */
};

// MANUAL_REVIEW: Populate write_mask_array with the correct write masks
// for each register from the IP register specification.
static const unsigned int write_mask_array[CNT] = {
    0xFFFFFFFFU, /* mizar_ETHERNET0_MAC_CONFIGURATION write mask */
    0xFFFFFFFFU, /* mizar_ETHERNET0_MAC_EXT_CONFIGURATION write mask */
    0xFFFFFFFFU, /* mizar_ETHERNET0_MAC_PACKET_FILTER write mask */
    0xFFFFFFFFU, /* mizar_ETHERNET0_MAC_WD_JB_TIMEOUT write mask */
    0xFFFFFFFFU, /* mizar_ETHERNET0_MAC_HASH_TABLE_REG0 write mask */
    /* Remaining entries are 0 */
};

// MANUAL_REVIEW: Populate skip_array with the correct skip flags
// for each register. Set to 1 to skip write-read verification.
static const unsigned int skip_array[CNT] = {
    0U, /* mizar_ETHERNET0_MAC_CONFIGURATION */
    0U, /* mizar_ETHERNET0_MAC_EXT_CONFIGURATION */
    0U, /* mizar_ETHERNET0_MAC_PACKET_FILTER */
    0U, /* mizar_ETHERNET0_MAC_WD_JB_TIMEOUT */
    0U, /* mizar_ETHERNET0_MAC_HASH_TABLE_REG0 */
    /* Remaining entries are 0 */
};

// MANUAL_REVIEW: Populate skip_rst_array with the correct reset-skip flags
// for each register. Set to 1 to skip default value verification.
static const unsigned int skip_rst_array[CNT] = {
    0U, /* mizar_ETHERNET0_MAC_CONFIGURATION */
    0U, /* mizar_ETHERNET0_MAC_EXT_CONFIGURATION */
    0U, /* mizar_ETHERNET0_MAC_PACKET_FILTER */
    0U, /* mizar_ETHERNET0_MAC_WD_JB_TIMEOUT */
    0U, /* mizar_ETHERNET0_MAC_HASH_TABLE_REG0 */
    /* Remaining entries are 0 */
};

/* Test data patterns for write-read verification */
static const unsigned int chk_val[6] = {
    0xFFFFFFFFU,
    0xAAAAAAAAU,
    0x55555555U,
    0x00000000U,
    0xA5A5A5A5U,
    0xFFFF0000U
};

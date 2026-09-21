// Author - AI Force 2.3. 18-Jul-2025 17:45 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: ethernet3_reg_wr_rd_test
 * Purpose:   Register default value verification and write-read verification
 *            for Ethernet3 MAC registers.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <test_common.h>
#include <ethernet3/ethernet3_def.h>
#include <ethernet3/ethernet3_offset.h>

/* Macros */
// MANUAL_REVIEW: SOFT_RST_REG_ADDRESS value was not provided in the Meta TestPlan JSON. Populate with the correct SoC address.
#ifndef SOFT_RST_REG_ADDRESS
#define SOFT_RST_REG_ADDRESS 0x00000000U
#endif

// MANUAL_REVIEW: SOFT_RST_REG_DATA value was not provided in the Meta TestPlan JSON. Populate with the correct reset data value.
#ifndef SOFT_RST_REG_DATA
#define SOFT_RST_REG_DATA 0x00000000U
#endif

#define CNT 6U

/*
 * Register arrays
 * The addr_array is declared with size 434 but only 6 register entries are populated.
 * The 6 registers correspond to offsets 0x0000 through 0x0014.
 * Impacted registers:
 *   mizar_ETHERNET3_MAC_CONFIGURATION
 *   mizar_ETHERNET3_MAC_EXT_CONFIGURATION
 *   mizar_ETHERNET3_MAC_PACKET_FILTER
 *   mizar_ETHERNET3_MAC_WD_JB_TIMEOUT
 *   mizar_ETHERNET3_MAC_HASH_TABLE_REG0
 *   mizar_ETHERNET3_MAC_HASH_TABLE_REG1
 */

static const unsigned int addr_array[CNT] = {
    mizar_ETHERNET3_MAC_CONFIGURATION,
    mizar_ETHERNET3_MAC_EXT_CONFIGURATION,
    mizar_ETHERNET3_MAC_PACKET_FILTER,
    mizar_ETHERNET3_MAC_WD_JB_TIMEOUT,
    mizar_ETHERNET3_MAC_HASH_TABLE_REG0,
    mizar_ETHERNET3_MAC_HASH_TABLE_REG1
};

// MANUAL_REVIEW: default_value_array values must be populated with the actual
// reset/default values for each register from the Ethernet3 IP specification.
static const unsigned int default_value_array[CNT] = {
    0x00000000U,  /* mizar_ETHERNET3_MAC_CONFIGURATION default */
    0x00000000U,  /* mizar_ETHERNET3_MAC_EXT_CONFIGURATION default */
    0x00000000U,  /* mizar_ETHERNET3_MAC_PACKET_FILTER default */
    0x00000000U,  /* mizar_ETHERNET3_MAC_WD_JB_TIMEOUT default */
    0x00000000U,  /* mizar_ETHERNET3_MAC_HASH_TABLE_REG0 default */
    0x00000000U   /* mizar_ETHERNET3_MAC_HASH_TABLE_REG1 default */
};

// MANUAL_REVIEW: read_mask_array values must be populated with the actual
// read masks for each register from the Ethernet3 IP specification.
static const unsigned int read_mask_array[CNT] = {
    0xFFFFFFFFU,  /* mizar_ETHERNET3_MAC_CONFIGURATION read mask */
    0xFFFFFFFFU,  /* mizar_ETHERNET3_MAC_EXT_CONFIGURATION read mask */
    0xFFFFFFFFU,  /* mizar_ETHERNET3_MAC_PACKET_FILTER read mask */
    0xFFFFFFFFU,  /* mizar_ETHERNET3_MAC_WD_JB_TIMEOUT read mask */
    0xFFFFFFFFU,  /* mizar_ETHERNET3_MAC_HASH_TABLE_REG0 read mask */
    0xFFFFFFFFU   /* mizar_ETHERNET3_MAC_HASH_TABLE_REG1 read mask */
};

// MANUAL_REVIEW: write_mask_array values must be populated with the actual
// write masks for each register from the Ethernet3 IP specification.
static const unsigned int write_mask_array[CNT] = {
    0xFFFFFFFFU,  /* mizar_ETHERNET3_MAC_CONFIGURATION write mask */
    0xFFFFFFFFU,  /* mizar_ETHERNET3_MAC_EXT_CONFIGURATION write mask */
    0xFFFFFFFFU,  /* mizar_ETHERNET3_MAC_PACKET_FILTER write mask */
    0xFFFFFFFFU,  /* mizar_ETHERNET3_MAC_WD_JB_TIMEOUT write mask */
    0xFFFFFFFFU,  /* mizar_ETHERNET3_MAC_HASH_TABLE_REG0 write mask */
    0xFFFFFFFFU   /* mizar_ETHERNET3_MAC_HASH_TABLE_REG1 write mask */
};

/* Skip arrays - all set to 0, no registers are skipped */
static const unsigned int skip_array[CNT] = {
    0U, 0U, 0U, 0U, 0U, 0U
};

static const unsigned int skip_rst_array[CNT] = {
    0U, 0U, 0U, 0U, 0U, 0U
};

/* Test patterns for write-read verification */
static const unsigned int chk_val[6] = {
    0xFFFFFFFFU,
    0xAAAAAAAAU,
    0x55555555U,
    0x00000000U,
    0xA5A5A5A5U,
    0xFFFF0000U
};

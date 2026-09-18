// Author - AI Force 2.3. 18-Sep-2025 08:17 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: ethernet0_reg_wr_rd_test
 * Purpose:   Register default-value verification and write-read verification
 *            for Ethernet0 MAC registers.
 */

#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <test_common.h>
#include <ethernet0/ethernet0_def.h>
#include <ethernet0/ethernet0_offset.h>

/* Macros */
// MANUAL_REVIEW: SOFT_RST_REG_ADDRESS value was not provided in the Meta TestPlan JSON.
// Define SOFT_RST_REG_ADDRESS with the correct register address for this SoC.
#ifndef SOFT_RST_REG_ADDRESS
#define SOFT_RST_REG_ADDRESS 0x0U
#endif

// MANUAL_REVIEW: SOFT_RST_REG_DATA value was not provided in the Meta TestPlan JSON.
// Define SOFT_RST_REG_DATA with the correct reset data value for this SoC.
#ifndef SOFT_RST_REG_DATA
#define SOFT_RST_REG_DATA 0x0U
#endif

/* CNT: Number of register entries in the arrays */
// MANUAL_REVIEW: The arrays are declared with size 434 but only 5 register entries
// are populated in the original source. Set CNT to the actual populated count.
#ifndef CNT
#define CNT 5U
#endif

/* Register address array */
// MANUAL_REVIEW: addr_array is declared with size 434 in the original source but only
// 5 registers are listed. Populate all required entries for the target SoC.
static const unsigned int addr_array[CNT] = {
    mizar_ETHERNET0_MAC_CONFIGURATION,
    mizar_ETHERNET0_MAC_EXT_CONFIGURATION,
    mizar_ETHERNET0_MAC_PACKET_FILTER,
    mizar_ETHERNET0_MAC_WD_JB_TIMEOUT,
    mizar_ETHERNET0_MAC_HASH_TABLE_REG0
};

/* Default value array */
// MANUAL_REVIEW: Populate default_value_array with the correct reset default values
// for each register listed in addr_array. Values below are placeholders.
static const unsigned int default_value_array[CNT] = {
    0x00000000U,  /* mizar_ETHERNET0_MAC_CONFIGURATION default */
    0x00000000U,  /* mizar_ETHERNET0_MAC_EXT_CONFIGURATION default */
    0x00000000U,  /* mizar_ETHERNET0_MAC_PACKET_FILTER default */
    0x00000000U,  /* mizar_ETHERNET0_MAC_WD_JB_TIMEOUT default */
    0x00000000U   /* mizar_ETHERNET0_MAC_HASH_TABLE_REG0 default */
};

/* Read mask array */
// MANUAL_REVIEW: Populate read_mask_array with the correct read masks for each register.
static const unsigned int read_mask_array[CNT] = {
    0xFFFFFFFFU,  /* mizar_ETHERNET0_MAC_CONFIGURATION read mask */
    0xFFFFFFFFU,  /* mizar_ETHERNET0_MAC_EXT_CONFIGURATION read mask */
    0xFFFFFFFFU,  /* mizar_ETHERNET0_MAC_PACKET_FILTER read mask */
    0xFFFFFFFFU,  /* mizar_ETHERNET0_MAC_WD_JB_TIMEOUT read mask */
    0xFFFFFFFFU   /* mizar_ETHERNET0_MAC_HASH_TABLE_REG0 read mask */
};

/* Write mask array */
// MANUAL_REVIEW: Populate write_mask_array with the correct write masks for each register.
static const unsigned int write_mask_array[CNT] = {
    0xFFFFFFFFU,  /* mizar_ETHERNET0_MAC_CONFIGURATION write mask */
    0xFFFFFFFFU,  /* mizar_ETHERNET0_MAC_EXT_CONFIGURATION write mask */
    0xFFFFFFFFU,  /* mizar_ETHERNET0_MAC_PACKET_FILTER write mask */
    0xFFFFFFFFU,  /* mizar_ETHERNET0_MAC_WD_JB_TIMEOUT write mask */
    0xFFFFFFFFU   /* mizar_ETHERNET0_MAC_HASH_TABLE_REG0 write mask */
};

/* Skip array for write-read check */
// MANUAL_REVIEW: Set skip_array[i] = 1 for registers that should be skipped
// during write-read verification.
static const unsigned int skip_array[CNT] = {
    0U,  /* mizar_ETHERNET0_MAC_CONFIGURATION */
    0U,  /* mizar_ETHERNET0_MAC_EXT_CONFIGURATION */
    0U,  /* mizar_ETHERNET0_MAC_PACKET_FILTER */
    0U,  /* mizar_ETHERNET0_MAC_WD_JB_TIMEOUT */
    0U   /* mizar_ETHERNET0_MAC_HASH_TABLE_REG0 */
};

/* Skip reset array for default value check */
// MANUAL_REVIEW: Set skip_rst_array[i] = 1 for registers that should be skipped
// during default reset value verification.
static const unsigned int skip_rst_array[CNT] = {
    0U,  /* mizar_ETHERNET0_MAC_CONFIGURATION */
    0U,  /* mizar_ETHERNET0_MAC_EXT_CONFIGURATION */
    0U,  /* mizar_ETHERNET0_MAC_PACKET_FILTER */
    0U,  /* mizar_ETHERNET0_MAC_WD_JB_TIMEOUT */
    0U   /* mizar_ETHERNET0_MAC_HASH_TABLE_REG0 */
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

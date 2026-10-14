// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>
#include <stdio.h>
#include <stdlib.h>
#include <test_common.h>
#include <ethernet0/ethernet0_def.h>
#include <ethernet0/ethernet0_offset.h>
#include "log.h"
#include "reg_access.h"

// Meta Step 5: CNT defines the total number of Ethernet0 registers under test
#define CNT 434

// Meta Step 7: Soft reset register address
#define SOFT_RST_REG_ADDRESS 0x00000000

// Meta Step 8: Soft reset register data
#define SOFT_RST_REG_DATA 0x00000000

// MANUAL_REVIEW: The following DEFAULT_VAL, WRITE_MASK, and READ_MASK macros
// are expected to be provided by <ethernet0/ethernet0_def.h> or
// <ethernet0/ethernet0_offset.h>. Verify they are defined in those headers.
// If not, they must be explicitly defined here with their correct values
// from the DV source / register specification.
//
// Default value macros expected:
// ETHERNET0_MAC_CONFIGURATION_DEFAULT_VAL
// ETHERNET0_MAC_EXT_CONFIGURATION_DEFAULT_VAL
// ETHERNET0_MAC_PACKET_FILTER_DEFAULT_VAL
// ETHERNET0_MAC_WD_JB_TIMEOUT_DEFAULT_VAL
// ETHERNET0_MAC_HASH_TABLE_REG0_DEFAULT_VAL
// ETHERNET0_MAC_HASH_TABLE_REG1_DEFAULT_VAL
// ETHERNET0_MAC_VLAN_TAG_CTRL_DEFAULT_VAL
// ETHERNET0_MAC_VLAN_TAG_DATA_DEFAULT_VAL
// ETHERNET0_MAC_VLAN_HASH_TABLE_DEFAULT_VAL
// ETHERNET0_MAC_Q0_TX_FLOW_CTRL_DEFAULT_VAL
//
// Write mask macros expected:
// ETHERNET0_MAC_CONFIGURATION_WRITE_MASK
// ETHERNET0_MAC_EXT_CONFIGURATION_WRITE_MASK
// ETHERNET0_MAC_PACKET_FILTER_WRITE_MASK
// ETHERNET0_MAC_WD_JB_TIMEOUT_WRITE_MASK
// ETHERNET0_MAC_HASH_TABLE_REG0_WRITE_MASK
// ETHERNET0_MAC_HASH_TABLE_REG1_WRITE_MASK
// ETHERNET0_MAC_VLAN_TAG_CTRL_WRITE_MASK
// ETHERNET0_MAC_VLAN_TAG_DATA_WRITE_MASK
// ETHERNET0_MAC_VLAN_HASH_TABLE_WRITE_MASK
// ETHERNET0_MAC_Q0_TX_FLOW_CTRL_WRITE_MASK
//
// Read mask macros expected:
// ETHERNET0_MAC_CONFIGURATION_READ_MASK
// ETHERNET0_MAC_EXT_CONFIGURATION_READ_MASK
// ETHERNET0_MAC_PACKET_FILTER_READ_MASK
// ETHERNET0_MAC_WD_JB_TIMEOUT_READ_MASK
// ETHERNET0_MAC_HASH_TABLE_REG0_READ_MASK
// ETHERNET0_MAC_HASH_TABLE_REG1_READ_MASK
// ETHERNET0_MAC_VLAN_TAG_CTRL_READ_MASK
// ETHERNET0_MAC_VLAN_TAG_DATA_READ_MASK
// ETHERNET0_MAC_VLAN_HASH_TABLE_READ_MASK
// ETHERNET0_MAC_Q0_TX_FLOW_CTRL_READ_MASK
// ETHERNET0_MAC_Q1_TX_FLOW_CTRL_READ_MASK
// ETHERNET0_MAC_Q2_TX_FLOW_CTRL_READ_MASK
// ETHERNET0_MAC_Q3_TX_FLOW_CTRL_READ_MASK
// ETHERNET0_MAC_RX_FLOW_CTRL_READ_MASK

// MANUAL_REVIEW: DEBUG_DISPLAY, ETH_10M, ETH_100M, HALF_DUPLEX are
// conditional compilation macros from Meta. Define them as needed for
// the target build configuration. They are commented out by default.
// #define DEBUG_DISPLAY
// #define ETH_10M
// #define ETH_100M
// #define HALF_DUPLEX

// Meta Step 9: addr_array[434] - register address macros
// MANUAL_REVIEW: Only the first 14 entries are supplied by Meta.
// The remaining 420 entries must be populated from the complete DV source.
// All 434 entries must be present for correct operation.
const int addr_array[434] = {
 mizar_ETHERNET0_MAC_CONFIGURATION,
 mizar_ETHERNET0_MAC_EXT_CONFIGURATION,
 mizar_ETHERNET0_MAC_PACKET_FILTER,
 mizar_ETHERNET0_MAC_WD_JB_TIMEOUT,
 mizar_ETHERNET0_MAC_HASH_TABLE_REG0,
 mizar_ETHERNET0_MAC_HASH_TABLE_REG1,
 mizar_ETHERNET0_MAC_VLAN_TAG_CTRL,
 mizar_ETHERNET0_MAC_VLAN_TAG_DATA,
 mizar_ETHERNET0_MAC_VLAN_HASH_TABLE,
 mizar_ETHERNET0_MAC_Q0_TX_FLOW_CTRL,
 mizar_ETHERNET0_MAC_Q1_TX_FLOW_CTRL,
 mizar_ETHERNET0_MAC_Q2_TX_FLOW_CTRL,
 mizar_ETHERNET0_MAC_Q3_TX_FLOW_CTRL,
 mizar_ETHERNET0_MAC_RX_FLOW_CTRL,
 // MANUAL_REVIEW: Remaining 420 entries must be populated from complete DV source
};

// Meta Step 10: default_value_array[434] - expected default/reset values
// MANUAL_REVIEW: Only the first 10 entries are supplied by Meta.
// The remaining 424 entries must be populated from the complete DV source.
const int default_value_array[434] = {
 ETHERNET0_MAC_CONFIGURATION_DEFAULT_VAL,
 ETHERNET0_MAC_EXT_CONFIGURATION_DEFAULT_VAL,
 ETHERNET0_MAC_PACKET_FILTER_DEFAULT_VAL,
 ETHERNET0_MAC_WD_JB_TIMEOUT_DEFAULT_VAL,
 ETHERNET0_MAC_HASH_TABLE_REG0_DEFAULT_VAL,
 ETHERNET0_MAC_HASH_TABLE_REG1_DEFAULT_VAL,
 ETHERNET0_MAC_VLAN_TAG_CTRL_DEFAULT_VAL,
 ETHERNET0_MAC_VLAN_TAG_DATA_DEFAULT_VAL,
 ETHERNET0_MAC_VLAN_HASH_TABLE_DEFAULT_VAL,
 ETHERNET0_MAC_Q0_TX_FLOW_CTRL_DEFAULT_VAL,
 // MANUAL_REVIEW: Remaining 424 entries must be populated from complete DV source
};

// Meta Step 11: write_mask_array[434] - write masks per register
// MANUAL_REVIEW: Only the first 10 entries are supplied by Meta.
// The remaining 424 entries must be populated from the complete DV source.
const int write_mask_array[434] = {
 ETHERNET0_MAC_CONFIGURATION_WRITE_MASK,
 ETHERNET0_MAC_EXT_CONFIGURATION_WRITE_MASK,
 ETHERNET0_MAC_PACKET_FILTER_WRITE_MASK,
 ETHERNET0_MAC_WD_JB_TIMEOUT_WRITE_MASK,
 ETHERNET0_MAC_HASH_TABLE_REG0_WRITE_MASK,
 ETHERNET0_MAC_HASH_TABLE_REG1_WRITE_MASK,
 ETHERNET0_MAC_VLAN_TAG_CTRL_WRITE_MASK,
 ETHERNET0_MAC_VLAN_TAG_DATA_WRITE_MASK,
 ETHERNET0_MAC_VLAN_HASH_TABLE_WRITE_MASK,
 ETHERNET0_MAC_Q0_TX_FLOW_CTRL_WRITE_MASK,
 // MANUAL_REVIEW: Remaining 424 entries must be populated from complete DV source
};

// Meta Step 12: read_mask_array[434] - read masks per register
// MANUAL_REVIEW: Only the first 12 entries are supplied by Meta.
// The remaining 422 entries must be populated from the complete DV source.
const int read_mask_array[434] = {
 ETHERNET0_MAC_CONFIGURATION_READ_MASK,
 ETHERNET0_MAC_EXT_CONFIGURATION_READ_MASK,
 ETHERNET0_MAC_PACKET_FILTER_READ_MASK,
 ETHERNET0_MAC_WD_JB_TIMEOUT_READ_MASK,
 ETHERNET0_MAC_HASH_TABLE_REG0_READ_MASK,
 ETHERNET0_MAC_HASH_TABLE_REG1_READ_MASK,
 ETHERNET0_MAC_VLAN_TAG_CTRL_READ_MASK,
 ETHERNET0_MAC_VLAN_TAG_DATA_READ_MASK,
 ETHERNET0_MAC_VLAN_HASH_TABLE_READ_MASK,
 ETHERNET0_MAC_Q0_TX_FLOW_CTRL_READ_MASK,
 ETHERNET0_MAC_Q1_TX_FLOW_CTRL_READ_MASK,
 ETHERNET0_MAC_Q2_TX_FLOW_CTRL_READ_MASK,
 // MANUAL_REVIEW: Remaining 422 entries must be populated from complete DV source
};

// Meta Step 13: skip_array[434] - skip flags (1 = skip, 0 = test)
// Indices 6, 7, 9 are set to 1 per Meta (MAC_VLAN_Tag_Ctrl, MAC_VLAN_Tag_Data, etc.)
// MANUAL_REVIEW: Only the first 10 entries are supplied by Meta.
// The remaining 424 entries must be populated from the complete DV source.
const int skip_array[434] = {
 0, 0, 0, 0, 0, 0, 1, 1, 0, 1,
 // MANUAL_REVIEW: Remaining 424 entries must be populated from complete DV source
};

// Meta Step 16: 6 test patterns for write/read verification
int chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000};

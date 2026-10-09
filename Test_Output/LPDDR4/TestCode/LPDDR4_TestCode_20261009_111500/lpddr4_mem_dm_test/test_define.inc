// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "lpddr4.h"
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

// Meta Macros: APS_DRAM, SG2667, SG2133, A53_INITIATOR, MPS_DRAM, AI_INITIATOR, DSP_INITIATOR
// These are conditional compilation macros expected to be defined externally
// (e.g., via build system -D flags). They are not given default values here.

// exp_data_array as per Meta Arrays: unsigned long int exp_data_array[50]
// Used to store expected 64-bit data values for write/read-back comparison
unsigned long int exp_data_array[50];

// Deterministic 64-bit traffic data replacing rand() calls
// 10 elements used for port0 write and port0/port1 read-back verification
static const unsigned long int dm_test_traffic_data[10] = {
 0xA5A5A5A500000000UL,
 0x5A5A5A5A11111111UL,
 0x1234567890ABCDEFUL,
 0xFEDCBA9876543210UL,
 0x0F0F0F0FF0F0F0F0UL,
 0xAAAAAAAA55555555UL,
 0xDEADC0DE0BADF00DUL,
 0xCAFEBABE12345678UL,
 0x0123456789ABCDEFUL,
 0xFFFF0000FFFF0000UL
};

// Global LPDDR4 configuration variables used by lpddr4 training and test flow
extern unsigned long int port0_addr;
extern unsigned long int port1_addr;

// MANUAL_REVIEW: ctl_base, phy_base, bus_width, SG, DBI_EN, DM_EN are
// expected to be declared in lpddr4.h or test_common.h as global variables
// accessible by the LPDDR4 training and configuration infrastructure.
// If not declared there, provide declarations here.
// extern unsigned long int ctl_base;
// extern unsigned long int phy_base;
// extern unsigned int bus_width;
// extern unsigned int SG;
// extern unsigned int DBI_EN;
// extern unsigned int DM_EN;

// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>
#include <stdio.h>
#include <ethernet0/ethernet0_def.h>
#include <test_common.h>
#include <ethernet0/ethernet0_offset.h>
#include <ethernet0/ethernet0_funcs.h>
#include <ethernet0/ethernet0_programming_sequence.h>
#include "reg_access.h"
#include "log.h"

// Meta Macros
// MANUAL_REVIEW: The following macros are compile-time configuration selectors.
// Define them as needed for the target test configuration.
// ETH_10M - Select 10 Mbps speed
// ETH_100M - Select 100 Mbps speed
// HALF_DUPLEX - Select half-duplex mode
// DEBUG_DISPLAY - Enable debug display
// PRELOAD_AGAIN - Enable preload again
// SEL_ENET0 - Select Ethernet interface 0
// SEL_ENET1 - Select Ethernet interface 1
// SEL_ENET2 - Select Ethernet interface 2
// SEL_ENET3 - Select Ethernet interface 3

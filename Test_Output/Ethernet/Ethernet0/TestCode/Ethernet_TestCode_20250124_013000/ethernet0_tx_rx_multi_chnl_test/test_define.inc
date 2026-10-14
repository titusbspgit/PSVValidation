// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ethernet0/ethernet0_def.h>
#include <test_common.h>
#include <ethernet0/ethernet0_offset.h>
#include <ethernet0/ethernet0_funcs.h>
#include <ethernet0/ethernet0_programming_sequence.h>

#include "log.h"
#include "reg_access.h"

// Meta Macros - compile-time configuration flags
// These macros are expected to be defined via build system / compile flags.
// They are NOT defined here to preserve conditional compilation behavior.
//
// ETH_10M - Define to select 10 Mbps speed (sets trns_count=6)
// ETH_100M - Define to select 100 Mbps speed
// HALF_DUPLEX - Define to select half-duplex mode
// DEBUG_DISPLAY - Define to enable debug display
// PRELOAD_AGAIN - Define to enable preload again
// SEL_ENET0 - Define to select Ethernet interface 0
// SEL_ENET1 - Define to select Ethernet interface 1
// SEL_ENET2 - Define to select Ethernet interface 2
// SEL_ENET3 - Define to select Ethernet interface 3

// Meta Arrays: NA - no testcase arrays supplied

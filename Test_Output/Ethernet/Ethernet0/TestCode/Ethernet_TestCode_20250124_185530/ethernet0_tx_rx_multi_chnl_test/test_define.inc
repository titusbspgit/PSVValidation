// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

// Meta Headers (Steps 1-8)
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ethernet0/ethernet0_def.h>
#include <test_common.h>
#include <ethernet0/ethernet0_offset.h>
#include <ethernet0/ethernet0_funcs.h>
#include <ethernet0/ethernet0_programming_sequence.h>

// Register access and logging headers
#include "reg_access.h"
#include "log.h"

// Meta Macros - compile-time configuration flags
// ETH_10M - Define to select 10 Mbps speed
// ETH_100M - Define to select 100 Mbps speed
// HALF_DUPLEX - Define to select half-duplex mode
// DEBUG_DISPLAY - Define to enable debug display
// PRELOAD_AGAIN - Define to enable preload again
// SEL_ENET0 - Define to select Ethernet interface 0
// SEL_ENET1 - Define to select Ethernet interface 1
// SEL_ENET2 - Define to select Ethernet interface 2
// SEL_ENET3 - Define to select Ethernet interface 3

// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <test_common.h>
#include <ethernet0/ethernet0_def.h>
#include <ethernet0/ethernet0_offset.h>
#include <ethernet0/ethernet0_funcs.h>
#include <ethernet0/ethernet0_programming_sequence.h>
#include "log.h"
#include "reg_access.h"

// Meta Macros: compile-time configuration selectors
// These are expected to be defined via build system / compile flags:
// ETH_10M - Select 10 Mbps speed
// ETH_100M - Select 100 Mbps speed
// HALF_DUPLEX - Select half-duplex mode (else full-duplex)
// SEL_ENET0 - Select Ethernet interface 0
// SEL_ENET1 - Select Ethernet interface 1
// SEL_ENET2 - Select Ethernet interface 2
// SEL_ENET3 - Select Ethernet interface 3
// PRELOAD_AGAIN - Enable RX descriptor ring preload on consumption
// DEBUG_DISPLAY - Enable debug display output

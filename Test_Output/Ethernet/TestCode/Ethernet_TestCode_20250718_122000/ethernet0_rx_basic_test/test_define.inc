// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ethernet0/ethernet0_def.h>
#include <test_common.h>
#include <ethernet0/ethernet0_offset.h>
#include <ethernet0/ethernet0_funcs.h>
#include <ethernet0/ethernet0_programming_sequence.h>

#ifndef ETH_10M
#define ETH_10M 0U
#endif
#ifndef ETH_100M
#define ETH_100M 0U
#endif
#ifndef HALF_DUPLEX
#define HALF_DUPLEX 0U
#endif
#ifndef SEL_ENET0
#define SEL_ENET0 0U
#endif
#ifndef SEL_ENET1
#define SEL_ENET1 0U
#endif
#ifndef SEL_ENET2
#define SEL_ENET2 0U
#endif
#ifndef SEL_ENET3
#define SEL_ENET3 0U
#endif
#ifndef DEBUG_DISPLAY
#define DEBUG_DISPLAY 0U
#endif

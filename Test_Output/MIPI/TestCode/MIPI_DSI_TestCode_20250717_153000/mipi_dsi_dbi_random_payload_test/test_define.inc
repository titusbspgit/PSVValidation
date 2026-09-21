// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

#ifndef DM0_RC
/* #define DM0_RC */
#endif

#ifndef DM1_RC
/* #define DM1_RC */
#endif

#ifndef DM0_EP
/* #define DM0_EP */
#endif

#ifndef DM1_EP
/* #define DM1_EP */
#endif

#ifndef DEBUG_DISPLAY
/* #define DEBUG_DISPLAY */
#endif

#define MIPI_DSI_DBI_POLL_TIMEOUT 10000U

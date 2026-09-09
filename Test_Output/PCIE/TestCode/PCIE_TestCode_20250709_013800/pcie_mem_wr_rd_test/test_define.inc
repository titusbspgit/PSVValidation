// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

#define PCIE_SYNC_REG           0xE6004100UL
#define PCIE_SYNC_EXPECTED      0x12345678UL
#define PCIE_SYNC_READY         0x11111111UL
#define PCIE_POLL_TIMEOUT       1000U

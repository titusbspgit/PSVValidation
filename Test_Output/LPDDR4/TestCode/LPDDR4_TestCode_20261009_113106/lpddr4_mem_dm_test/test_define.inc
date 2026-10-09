// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

// Meta Headers
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "lpddr4.h"

// Framework headers for register access and logging
#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

// Meta Macros: conditional compilation macros (defined externally)
// APS_DRAM - controls port address and base address selection
// SG2667 - selects speed grade 2667
// SG2133 - selects speed grade 2133
// A53_INITIATOR - used in XOR-based port0 access guard
// MPS_DRAM - used in XOR-based port0 access guard
// AI_INITIATOR - used in XOR-based port0 access guard
// DSP_INITIATOR - used in XOR-based port0 access guard

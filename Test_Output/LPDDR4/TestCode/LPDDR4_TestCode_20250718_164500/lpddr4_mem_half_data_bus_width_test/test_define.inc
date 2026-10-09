// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "lpddr4.h"
#include "log.h"
#include "reg_access.h"

/ Conditional compilation macros (defined externally as needed): /
/ APS_DRAM - controls port address and base address selection /
/ SG2667 - selects speed grade 2667 /
/ SG2133 - selects speed grade 2133 /
/ A53_INITIATOR - used in XOR-based port0 access guard /
/ MPS_DRAM - used in XOR-based port0 access guard /
/ AI_INITIATOR - used in XOR-based port0 access guard /
/ DSP_INITIATOR - used in XOR-based port0 access guard */

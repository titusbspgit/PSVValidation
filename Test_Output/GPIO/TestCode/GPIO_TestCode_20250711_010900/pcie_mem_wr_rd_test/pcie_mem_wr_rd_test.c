// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_mem_wr_rd_test.h"
#include "test_define.inc"

/*
 * Testcase: pcie_mem_wr_rd_test
 * Description: Validates PCIe memory write and read operations through the
 *   PCIe slave interfaces for DM0_RC, DM1_RC, DM0_EP, and DM1_EP modes.
 */

typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} pcie_mem_wr_rd_test_ctx_t;

static pcie_mem_wr_rd_test_ctx_t g_ctx;

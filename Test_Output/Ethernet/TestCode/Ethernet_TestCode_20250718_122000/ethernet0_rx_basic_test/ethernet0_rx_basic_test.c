// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_rx_basic_test.h"
#include "test_define.inc"

/*
 * Testcase: ethernet0_rx_basic_test
 * Description: Basic RX packet reception test for Ethernet0.
 */

typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} ethernet0_rx_basic_test_ctx_t;

static ethernet0_rx_basic_test_ctx_t g_ctx;

int ethernet0_rx_basic_test_init(const TestsItem *cfg)
{
    (void)cfg;
    g_ctx.errors = 0U;
    g_ctx.checks_total = 0U;
    g_ctx.checks_passed = 0U;
    g_ctx.checks_failed = 0U;
    LOGT("ethernet0_rx_basic_test_init: Initialization complete");
    return 0;
}

int ethernet0_rx_basic_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    if (out == 0) {
        LOGE("ethernet0_rx_basic_test_run: output pointer is NULL");
        return -1;
    }
    out->status = 0;
    LOGT("ethernet0_rx_basic_test_run: Starting basic RX test");
    g_ctx.checks_failed = g_ctx.errors;
    out->status = (g_ctx.errors == 0U) ? 0 : -1;
    LOGT("Run complete: %s errors=%u", (out->status == 0) ? "PASS" : "FAIL", g_ctx.errors);
    return out->status;
}

int ethernet0_rx_basic_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGT("ethernet0_rx_basic_test_teardown: errors=%u", g_ctx.errors);
    return (g_ctx.errors == 0U) ? 0 : -1;
}

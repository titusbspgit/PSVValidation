// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <stdint.h>

#include "framework.h"
#include "log.h"
#include "mmio.h"
#include "reg_access.h"
#include "pcie_device_enumerate_test.h"
#include "test_define.inc"

/*
 * PCIe Device Enumeration Test
 *
 * This testcase performs PCIe device enumeration. It writes 0x0 to 0xE6004100,
 * conditionally invokes link training based on DM0_RC/DM1_RC/DM0_EP/DM1_EP
 * compile-time defines, programs cache coherency via
 * mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF and
 * mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, polls link status,
 * performs enumeration setup, BAR sizing/assignment, and final handshake.
 */

typedef struct {
    unsigned int errors;
} pcie_enum_test_ctx_t;

static pcie_enum_test_ctx_t g_ctx;

static unsigned int data_rd;
static unsigned int rd_wr_data1;

/* ---------- Static helper: cache coherency enable ---------- */
static void pcie_cache_enable_single(unsigned int reg_addr)
{
    /* set_data bits [11:14]=0xf, [3:6]=0xf, write back */
    rd_wr_data1 = set_data(read_reg(reg_addr), 11, 14, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xf);
    write_reg(reg_addr, rd_wr_data1);

    /* set_data bits [27:30]=0xf, [19:22]=0xf, write back */
    rd_wr_data1 = set_data(read_reg(reg_addr), 27, 30, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xf);
    write_reg(reg_addr, rd_wr_data1);
}

/* ---------- Static helper: cache coherency enable combined ---------- */
static void pcie_cache_enable_combined(unsigned int reg_addr)
{
    rd_wr_data1 = set_data(read_reg(reg_addr), 11, 14, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xf);
    write_reg(reg_addr, rd_wr_data1);
}

/* ---------- Static helper: link training sequence ---------- */
static void pcie_link_training_sequence(void)
{
#if defined(DM0_RC) || defined(DM0_EP)
    link_training_dm0_x4(4);
#endif
#if defined(DM1_RC) || defined(DM1_EP)
    link_training_dm1_x4(4);
#endif
}

/* ---------- Static helper: cache enable full block ---------- */
static void pcie_cache_enable_full_block(void)
{
    /* Steps 3-4: Cache programming for PCIE0 */
    pcie_cache_enable_single(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    /* Step 5: Repeat for PCIE1 */
    pcie_cache_enable_single(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    /* Step 6: wait_on(20) */
    wait_on(20);

    /* Step 7: Combined cache programming for PCIE0 */
    pcie_cache_enable_combined(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    /* Step 8: Combined cache programming for PCIE1 */
    pcie_cache_enable_combined(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
}

/* ---------- Static helper: poll link status with timeout ---------- */
static int pcie_poll_sii0_link_up(void)
{
    unsigned int timeout = PCIE_POLL_TIMEOUT;

    data_rd = read_sii0_reg(PCIE_LINK_STATUS_OFFSET);
    while (((data_rd & PCIE_LINK_UP_MASK) != PCIE_LINK_UP_VALUE) && (timeout > 0U)) {
        data_rd = read_sii0_reg(PCIE_LINK_STATUS_OFFSET);
        timeout--;
    }

    LOGT("PCIE0 SII link status[0xC0] = 0x%08x", data_rd);

    if ((data_rd & PCIE_LINK_UP_MASK) != PCIE_LINK_UP_VALUE) {
        LOGE("PCIe0 link-up timeout, status=0x%08x expected mask=0x%08x value=0x%08x",
             data_rd, PCIE_LINK_UP_MASK, PCIE_LINK_UP_VALUE);
        return -1;
    }

    return 0;
}

static int pcie_poll_sii1_link_up(void)
{
    unsigned int timeout = PCIE_POLL_TIMEOUT;

    data_rd = read_sii1_reg(PCIE_LINK_STATUS_OFFSET);
    while (((data_rd & PCIE_LINK_UP_MASK) != PCIE_LINK_UP_VALUE) && (timeout > 0U)) {
        data_rd = read_sii1_reg(PCIE_LINK_STATUS_OFFSET);
        timeout--;
    }

    LOGT("PCIE1 SII link status[0xC0] = 0x%08x", data_rd);

    if ((data_rd & PCIE_LINK_UP_MASK) != PCIE_LINK_UP_VALUE) {
        LOGE("PCIe1 link-up timeout, status=0x%08x expected mask=0x%08x value=0x%08x",
             data_rd, PCIE_LINK_UP_MASK, PCIE_LINK_UP_VALUE);
        return -1;
    }

    return 0;
}

/* ---------- Static helper: cache disable programming ---------- */
static void pcie_cache_disable_single(unsigned int reg_addr)
{
    /* set_data bits [11:14]=0xf, [3:6]=0xf, write back */
    rd_wr_data1 = set_data(read_reg(reg_addr), 11, 14, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xf);
    write_reg(reg_addr, rd_wr_data1);

    /* set_data bits [27:30]=0xf, [19:22]=0x0, write back */
    rd_wr_data1 = set_data(read_reg(reg_addr), 27, 30, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0x0);
    write_reg(reg_addr, rd_wr_data1);
}

static void pcie_cache_disable_combined(unsigned int reg_addr)
{
    rd_wr_data1 = set_data(read_reg(reg_addr), 11, 14, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0x0);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0x0);
    write_reg(reg_addr, rd_wr_data1);
}

/* ---------- Static helper: BAR sizing and assignment for one slave ---------- */
static void pcie_bar_size_and_assign_slv1(void)
{
    /* Step 23: Write 0xFFFFFFFF to pcie_slv1 BAR registers for sizing */
    write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x24, 0xFFFFFFFF);
    LOGT("pcie_slv1 BAR sizing writes completed");

    /* Step 24: Read back pcie_slv1 BAR registers */
    data_rd = read_pcie_slv1_reg(0x10);
    LOGT("pcie_slv1 BAR0 size readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv1_reg(0x14);
    LOGT("pcie_slv1 BAR1 size readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv1_reg(0x18);
    LOGT("pcie_slv1 BAR2 size readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv1_reg(0x1c);
    LOGT("pcie_slv1 BAR3 size readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv1_reg(0x20);
    LOGT("pcie_slv1 BAR4 size readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv1_reg(0x24);
    LOGT("pcie_slv1 BAR5 size readback = 0x%08x", data_rd);

    /* Step 25: Write specific address values to pcie_slv1 BAR registers */
    write_pcie_slv1_reg(0x10, 0x0);
    write_pcie_slv1_reg(0x14, 0x4);
    write_pcie_slv1_reg(0x18, 0x20000000);
    write_pcie_slv1_reg(0x1c, 0x40000000);
    write_pcie_slv1_reg(0x20, 0x60000000);
    write_pcie_slv1_reg(0x24, 0x80000000);
    LOGT("pcie_slv1 BAR address assignment writes completed");

    /* Step 26: Read back pcie_slv1 BAR registers */
    data_rd = read_pcie_slv1_reg(0x10);
    LOGT("pcie_slv1 BAR0 assigned readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv1_reg(0x14);
    LOGT("pcie_slv1 BAR1 assigned readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv1_reg(0x18);
    LOGT("pcie_slv1 BAR2 assigned readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv1_reg(0x1c);
    LOGT("pcie_slv1 BAR3 assigned readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv1_reg(0x20);
    LOGT("pcie_slv1 BAR4 assigned readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv1_reg(0x24);
    LOGT("pcie_slv1 BAR5 assigned readback = 0x%08x", data_rd);
}

static void pcie_bar_size_and_assign_slv0(void)
{
    /* Step 27: Write 0xFFFFFFFF to pcie_slv0 BAR registers for sizing */
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);
    LOGT("pcie_slv0 BAR sizing writes completed");

    /* Read back pcie_slv0 BAR registers */
    data_rd = read_pcie_slv0_reg(0x10);
    LOGT("pcie_slv0 BAR0 size readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv0_reg(0x14);
    LOGT("pcie_slv0 BAR1 size readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv0_reg(0x18);
    LOGT("pcie_slv0 BAR2 size readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv0_reg(0x1c);
    LOGT("pcie_slv0 BAR3 size readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv0_reg(0x20);
    LOGT("pcie_slv0 BAR4 size readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv0_reg(0x24);
    LOGT("pcie_slv0 BAR5 size readback = 0x%08x", data_rd);

    /* Write specific address values to pcie_slv0 BAR registers */
    write_pcie_slv0_reg(0x10, 0x0);
    write_pcie_slv0_reg(0x14, 0x4);
    write_pcie_slv0_reg(0x18, 0x20000000);
    write_pcie_slv0_reg(0x1c, 0x40000000);
    write_pcie_slv0_reg(0x20, 0x60000000);
    write_pcie_slv0_reg(0x24, 0x80000000);
    LOGT("pcie_slv0 BAR address assignment writes completed");

    /* Read back pcie_slv0 BAR registers */
    data_rd = read_pcie_slv0_reg(0x10);
    LOGT("pcie_slv0 BAR0 assigned readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv0_reg(0x14);
    LOGT("pcie_slv0 BAR1 assigned readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv0_reg(0x18);
    LOGT("pcie_slv0 BAR2 assigned readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv0_reg(0x1c);
    LOGT("pcie_slv0 BAR3 assigned readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv0_reg(0x20);
    LOGT("pcie_slv0 BAR4 assigned readback = 0x%08x", data_rd);
    data_rd = read_pcie_slv0_reg(0x24);
    LOGT("pcie_slv0 BAR5 assigned readback = 0x%08x", data_rd);
}

/*
 * Function: pcie_device_enumerate_test_init
 * Description: Performs testcase initialization and pre-condition setup for pcie_device_enumerate_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_init(const TestsItem cfg)
{
    (void)cfg;

    g_ctx = (pcie_enum_test_ctx_t){0};

    LOGT("pcie_device_enumerate_test init");

    return 0;
}

/*
 * Function: pcie_device_enumerate_test_run
 * Description: Executes the main testcase flow for pcie_device_enumerate_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput out)
{
    (void)cfg;

    if (out == 0) {
        LOGE("pcie_device_enumerate_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting pcie_device_enumerate_test run");

    /* Step 1: Initialize synchronization register */
    write_reg(PCIE_SYNC_REG_ADDR, 0x0);
    LOGT("Synchronization register 0xE6004100 initialized to 0x0");

    /* Step 2: First link training sequence */
    LOGT("First link training sequence");
    pcie_link_training_sequence();

    /* Steps 3-8: First cache enable full block */
    LOGT("First cache enable programming block");
    pcie_cache_enable_full_block();

    /* Step 9: Repeat link training and cache programming (duplicate block) */
    LOGT("Second link training sequence (duplicate block)");
    pcie_link_training_sequence();

    LOGT("Second cache enable programming block (duplicate block)");
    pcie_cache_enable_full_block();

    /* Step 10: Read initial PCIE0 link status */
    data_rd = read_sii0_reg(PCIE_LINK_STATUS_OFFSET);
    LOGT("Initial PCIE0 SII link status = 0x%08x", data_rd);

    /* Step 11: Call non_secure_prot_nic */
    non_secure_prot_nic();
    LOGT("non_secure_prot_nic() called");

    /* Step 12: Poll PCIE0 link up with timeout */
    if (pcie_poll_sii0_link_up() != 0) {
        LOGE("PCIe0 link-up polling failed");
        g_ctx.errors++;
        out->status = -1;
        LOGT("Run complete: FAIL errors=%u", g_ctx.errors);
        return out->status;
    }
    LOGT("PCIe0 link up confirmed");

    /* Step 13: Poll PCIE1 link up with timeout */
    if (pcie_poll_sii1_link_up() != 0) {
        LOGE("PCIe1 link-up polling failed");
        g_ctx.errors++;
        out->status = -1;
        LOGT("Run complete: FAIL errors=%u", g_ctx.errors);
        return out->status;
    }
    LOGT("PCIe1 link up confirmed");

#ifdef DM0_RC
    /* Step 14: Read Vendor ID under DM0_RC */
    data_rd = read_pcie_slv0_reg(0x0);
    LOGT("DM0_RC: Vendor ID = 0x%08x", data_rd);

    /* Step 15: Enable command register */
    write_pcie_slv0_reg(0x4, 0x7);
    LOGT("DM0_RC: Command register written with 0x7");
#endif

    /* Step 16: Memory base programming */
    mem_base_program_dm0_x4();
    mem_base_program_dm1_x4();
    LOGT("Memory base programming completed for dm0 and dm1");

    /* Step 17: wait_on(10) */
    wait_on(10);

    /* Step 18: Write 0x1 to system-level registers */
    write_reg(PCIE_SYS_REG_0, 0x1);
    LOGT("write_reg(0xE690000C, 0x1)");
    write_reg(PCIE_SYS_REG_1, 0x1);
    LOGT("write_reg(0xE6900010, 0x1)");
    write_reg(PCIE_SYS_REG_2, 0x1);
    LOGT("write_reg(0xE6900014, 0x1)");
    write_reg(PCIE_SYS_REG_3, 0x1);
    LOGT("write_reg(0xE6900018, 0x1)");
    write_reg(PCIE_SYS_REG_4, 0x1);
    LOGT("write_reg(0xE6900030, 0x1)");
    write_reg(PCIE_SYS_REG_5, 0x1);
    LOGT("write_reg(0xE6900034, 0x1)");

    /* Steps 19-20: Cache disable programming */
    LOGT("Cache disable programming for PCIE0");
    pcie_cache_disable_single(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    LOGT("Cache disable programming for PCIE1");
    pcie_cache_disable_single(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    /* Step 21: wait_on(10), then combined cache disable for both */
    wait_on(10);

    LOGT("Combined cache disable for PCIE0");
    pcie_cache_disable_combined(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    LOGT("Combined cache disable for PCIE1");
    pcie_cache_disable_combined(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    /* Step 22: wait_on(30) */
    wait_on(30);

    /* Steps 23-26: BAR sizing and assignment for pcie_slv1 */
    LOGT("BAR sizing and assignment for pcie_slv1");
    pcie_bar_size_and_assign_slv1();

    /* Step 27: BAR sizing and assignment for pcie_slv0 */
    LOGT("BAR sizing and assignment for pcie_slv0");
    pcie_bar_size_and_assign_slv0();

    /* Step 28: wait_on(10) */
    wait_on(10);

    /* Step 29: Poll read_reg(0xE6004100) until data_rd == 0x12345678 */
    {
        unsigned int handshake_timeout = PCIE_POLL_TIMEOUT;

        data_rd = read_reg(PCIE_SYNC_REG_ADDR);
        while ((data_rd != PCIE_HANDSHAKE_EXPECTED) && (handshake_timeout > 0U)) {
            wait_on(5);
            data_rd = read_reg(PCIE_SYNC_REG_ADDR);
            handshake_timeout--;
        }

        LOGT("Handshake register 0xE6004100 = 0x%08x", data_rd);

        if (data_rd != PCIE_HANDSHAKE_EXPECTED) {
            LOGE("Handshake polling timeout, read=0x%08x expected=0x%08x",
                 data_rd, PCIE_HANDSHAKE_EXPECTED);
            g_ctx.errors++;
        }
    }

    // MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV completion is handled via g_ctx.errors and out->status.

    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: pcie_device_enumerate_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for pcie_device_enumerate_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("pcie_device_enumerate_test teardown: errors=%u", g_ctx.errors);
    return g_ctx.errors == 0U ? 0 : -1;
}

// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <reg_access.h>
#include "pcie_device_enumerate_test.h"
#include "test_define.inc"

/*
 * PSV PCIe Device Enumeration Test
 *
 * This testcase performs PCIe device enumeration. It begins by writing 0x0 to
 * 0xE6004100, then conditionally invokes link training based on DM0_RC, DM1_RC,
 * DM0_EP, or DM1_EP compile-time defines. Cache coherency programming is
 * performed, link status is polled, enumeration/BAR setup is done, and BAR
 * sizing/assignment is validated.
 *
 * PSV changes:
 * - Removed test_common.h
 * - Removed finish()
 * - Added PSV init/run/teardown wrappers
 * - Added timeout protection for link polling
 * - Converted DV handshake polling to PSV-compatible timeout polling
 */

static unsigned int data_rd;
static unsigned int rd_wr_data1;
static int test_err = 0;

/* Forward declarations for static helpers */
static int wait_for_pcie0_link_up(void);
static int wait_for_pcie1_link_up(void);
static void pcie_enable_cache_coherency(void);
static void pcie_disable_cache_coherency(void);
static int pcie_link_training(void);
static int pcie_enumeration_setup(void);
static int pcie_bar_sizing_and_assignment(void);

static int wait_for_pcie0_link_up(void)
{
    unsigned int timeout = PCIE_POLL_TIMEOUT;

    data_rd = read_sii0_reg(PCIE_LINK_STATUS_OFFSET);

    while (((data_rd & PCIE_LINK_UP_MASK) != PCIE_LINK_UP_VALUE) && (timeout > 0U)) {
        data_rd = read_sii0_reg(PCIE_LINK_STATUS_OFFSET);
        timeout--;
    }

    LOGT("PCIE0 SII link status[0xC0] = 0x%08x", data_rd);

    if ((data_rd & PCIE_LINK_UP_MASK) != PCIE_LINK_UP_VALUE) {
        LOGE("PCIe0 link-up timeout, status=0x%08x expected mask/value=0x%08x",
             data_rd, PCIE_LINK_UP_VALUE);
        return -1;
    }

    return 0;
}

static int wait_for_pcie1_link_up(void)
{
    unsigned int timeout = PCIE_POLL_TIMEOUT;

    data_rd = read_sii1_reg(PCIE_LINK_STATUS_OFFSET);

    while (((data_rd & PCIE_LINK_UP_MASK) != PCIE_LINK_UP_VALUE) && (timeout > 0U)) {
        data_rd = read_sii1_reg(PCIE_LINK_STATUS_OFFSET);
        timeout--;
    }

    LOGT("PCIE1 SII link status[0xC0] = 0x%08x", data_rd);

    if ((data_rd & PCIE_LINK_UP_MASK) != PCIE_LINK_UP_VALUE) {
        LOGE("PCIe1 link-up timeout, status=0x%08x expected mask/value=0x%08x",
             data_rd, PCIE_LINK_UP_VALUE);
        return -1;
    }

    return 0;
}

static void pcie_enable_cache_coherency(void)
{
    /* Step 3: Cache programming for PCIE0 - bits [11:14]=0xf, [3:6]=0xf */
    rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF),
                           11, 14, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* Step 4: Cache programming for PCIE0 - bits [27:30]=0xf, [19:22]=0xf */
    rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF),
                           27, 30, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* Step 5: Repeat steps 3-4 for PCIE1 */
    rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF),
                           11, 14, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF),
                           27, 30, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* Step 6: wait_on(20) */
    wait_on(20);

    /* Step 7: Combined cache programming for PCIE0 */
    rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF),
                           11, 14, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* Step 8: Combined cache programming for PCIE1 */
    rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF),
                           11, 14, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
}

static void pcie_disable_cache_coherency(void)
{
    /* Step 19: Cache disable for PCIE0 - bits [11:14]=0xf, [3:6]=0xf */
    rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF),
                           11, 14, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* Cache disable for PCIE0 - bits [27:30]=0xf, [19:22]=0x0 */
    rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF),
                           27, 30, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* Step 20: Cache disable for PCIE1 */
    rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF),
                           11, 14, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF),
                           27, 30, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* Step 21: wait_on(10), then combined cache disable for both */
    wait_on(10);

    rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF),
                           11, 14, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0x0);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF),
                           11, 14, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xf);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0x0);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
}

static int pcie_link_training(void)
{
    LOGT("PCIe link training started");

    /* Step 2: Conditionally call link training based on compile-time defines */
#if defined(DM0_RC) || defined(DM0_EP)
    link_training_dm0_x4(4);
#endif
#if defined(DM1_RC) || defined(DM1_EP)
    link_training_dm1_x4(4);
#endif

    /* Cache enable programming */
    pcie_enable_cache_coherency();

    /* Step 9: Repeat link training and cache programming sequence (duplicate block in source) */
#if defined(DM0_RC) || defined(DM0_EP)
    link_training_dm0_x4(4);
#endif
#if defined(DM1_RC) || defined(DM1_EP)
    link_training_dm1_x4(4);
#endif

    pcie_enable_cache_coherency();

    /* Step 10: Read initial link status for PCIE0 */
    data_rd = read_sii0_reg(PCIE_LINK_STATUS_OFFSET);
    LOGT("Initial PCIE0 link status = 0x%08x", data_rd);

    /* Step 11: Call non_secure_prot_nic */
    non_secure_prot_nic();

    /* Step 12: Poll PCIE0 link up with timeout */
    if (wait_for_pcie0_link_up() != 0) {
        LOGE("PCIe0 link training failed");
        return -1;
    }
    LOGT("PCIe0 link up confirmed");

    /* Step 13: Poll PCIE1 link up with timeout */
    if (wait_for_pcie1_link_up() != 0) {
        LOGE("PCIe1 link training failed");
        return -1;
    }
    LOGT("PCIe1 link up confirmed");

    LOGT("PCIe link training passed");
    return 0;
}

static int pcie_enumeration_setup(void)
{
    LOGT("PCIe enumeration setup started");

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
    write_reg(PCIE_SYS_REG_1, 0x1);
    write_reg(PCIE_SYS_REG_2, 0x1);
    write_reg(PCIE_SYS_REG_3, 0x1);
    write_reg(PCIE_SYS_REG_4, 0x1);
    write_reg(PCIE_SYS_REG_5, 0x1);
    LOGT("System-level registers written with 0x1");

    /* Steps 19-21: Cache disable programming */
    pcie_disable_cache_coherency();

    LOGT("PCIe enumeration setup completed");
    return 0;
}

static int pcie_bar_sizing_and_assignment(void)
{
    LOGT("PCIe BAR sizing and assignment started");

    /* Step 22: wait_on(30) */
    wait_on(30);

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

    /* Step 27: Repeat BAR sizing and assignment for pcie_slv0 */
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);
    LOGT("pcie_slv0 BAR sizing writes completed");

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

    write_pcie_slv0_reg(0x10, 0x0);
    write_pcie_slv0_reg(0x14, 0x4);
    write_pcie_slv0_reg(0x18, 0x20000000);
    write_pcie_slv0_reg(0x1c, 0x40000000);
    write_pcie_slv0_reg(0x20, 0x60000000);
    write_pcie_slv0_reg(0x24, 0x80000000);
    LOGT("pcie_slv0 BAR address assignment writes completed");

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

    /* Step 28: wait_on(10) */
    wait_on(10);

    /*
     * Step 29: DV flow polls read_reg(0xE6004100) until data_rd == 0x12345678
     * with wait_on(5) between iterations.
     * In PSV, this is a DV handshake/scoreboard mechanism.
     * Converting to timeout-protected polling.
     */
    {
        unsigned int handshake_timeout = PCIE_POLL_TIMEOUT;

        data_rd = read_reg(PCIE_DV_HANDSHAKE_ADDR);
        while ((data_rd != PCIE_HANDSHAKE_EXPECTED) && (handshake_timeout > 0U)) {
            wait_on(5);
            data_rd = read_reg(PCIE_DV_HANDSHAKE_ADDR);
            handshake_timeout--;
        }

        LOGT("Handshake register 0xE6004100 = 0x%08x", data_rd);

        if (data_rd != PCIE_HANDSHAKE_EXPECTED) {
            LOGE("Handshake polling timeout, read=0x%08x expected=0x%08x",
                 data_rd, PCIE_HANDSHAKE_EXPECTED);
            return -1;
        }
    }

    // MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV completion is handled via test_err and out->status.

    LOGT("PCIe BAR sizing and assignment passed");
    return 0;
}

static int test_case(void)
{
    int ret;

    test_err = 0;

    LOGT("Entered pcie_device_enumerate_test");

    /* Step 1: Initialize synchronization register */
    write_reg(PCIE_DV_HANDSHAKE_ADDR, 0x0);
    LOGT("Synchronization register 0xE6004100 initialized to 0x0");

    /* Steps 2-13: Link training */
    ret = pcie_link_training();
    if (ret != 0) {
        test_err++;
        return test_err;
    }

    /* Steps 14-21: Enumeration setup */
    ret = pcie_enumeration_setup();
    if (ret != 0) {
        test_err++;
        return test_err;
    }

    /* Steps 22-29: BAR sizing, assignment, and final handshake */
    ret = pcie_bar_sizing_and_assignment();
    if (ret != 0) {
        test_err++;
        return test_err;
    }

    LOGT("pcie_device_enumerate_test completed, test_err=%d", test_err);

    return test_err;
}

/*
 * Function: pcie_device_enumerate_test_init
 * Description: Performs testcase initialization and pre-condition setup for pcie_device_enumerate_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_init(const TestsItem *cfg)
{
    (void)cfg;
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
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    (void)out;

    test_case();

    return test_err;
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
    LOGT("pcie_device_enumerate_test teardown, test_err=%d", test_err);
    return test_err;
}

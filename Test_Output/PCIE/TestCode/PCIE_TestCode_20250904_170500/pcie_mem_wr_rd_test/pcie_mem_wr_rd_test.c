// Author - AI Force 2.3. 04-Sep-2025 17:05 IST
// (EMBENGG-SYSAPPS)

#include "pcie_mem_wr_rd_test.h"
#include "test_define.inc"

/* Global variables for testcase */
unsigned int data_rd, test_err;

/*
 * Function: pcie_mem_wr_rd_test_init
 * Description: Performs testcase initialization for pcie_mem_wr_rd_test.
 *              Initializes synchronization register, performs link training,
 *              programs cache coherency control registers for both PCIE0 and PCIE1.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIE memory write-read test: %s\n", cfg->test_name);

    /* Step 1: Initialize the synchronization register by clearing it */
    write_reg(SYNC_REG_ADDR, 0x0);
    #ifdef DEBUG_DISPLAY
        LOGI("[Init] Sync register 0x%08X cleared\n", SYNC_REG_ADDR);
    #endif

    /* Step 2: Perform PCIe link training for the configured dual-mode controller */
    #ifdef DM0_RC
        link_training_dm0_x4(4);
        #ifdef DEBUG_DISPLAY
            LOGI("[Init] Link training DM0 RC x4 started\n");
        #endif
    #endif
    #ifdef DM1_RC
        link_training_dm1_x4(4);
        #ifdef DEBUG_DISPLAY
            LOGI("[Init] Link training DM1 RC x4 started\n");
        #endif
    #endif
    #ifdef DM0_EP
        link_training_dm0_x4(4);
        #ifdef DEBUG_DISPLAY
            LOGI("[Init] Link training DM0 EP x4 started\n");
        #endif
    #endif
    #ifdef DM1_EP
        link_training_dm1_x4(4);
        #ifdef DEBUG_DISPLAY
            LOGI("[Init] Link training DM1 EP x4 started\n");
        #endif
    #endif

    /* Step 3: CACHE PROGRAMMING - Enable cache coherency for PCIE0 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Enable cache coherency for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 4: Wait for the configuration to take effect */
    wait_on(20);

    /* Re-apply all cache coherency bits to PCIE0 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Re-apply all cache coherency bits to PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    #ifdef DEBUG_DISPLAY
        LOGI("[Init] Cache coherency programming complete for PCIE0 and PCIE1\n");
    #endif

    return 0;
}

/*
 * Function: pcie_mem_wr_rd_test_run
 * Description: Main testcase execution for pcie_mem_wr_rd_test.
 *              Polls link status, reads Vendor ID, enables bus master,
 *              programs BARs and memory base addresses, disables cache coherency,
 *              performs memory write-read verification through PCIe slave ports.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    LOGI("[Test Run] PCIE memory write-read test: %s\n", cfg->test_name);
    test_err = 0;

    /* Step 6: Poll link status on appropriate SII interface until link-up */
    #ifdef DM0
        data_rd = read_sii0_reg(0xC0);
        while ((data_rd & 0xD1) != 0xD1)
        {
            data_rd = read_sii0_reg(0xC0);
            #ifdef DEBUG_DISPLAY
                LOGI("[Run] Polling SII0 link status: data_rd=0x%08X\n", data_rd);
            #endif
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] SII0 link-up confirmed\n");
        #endif
    #endif

    #ifdef DM1
        data_rd = read_sii1_reg(0xC0);
        while ((data_rd & 0xD1) != 0xD1)
        {
            data_rd = read_sii1_reg(0xC0);
            #ifdef DEBUG_DISPLAY
                LOGI("[Run] Polling SII1 link status: data_rd=0x%08X\n", data_rd);
            #endif
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] SII1 link-up confirmed\n");
        #endif
    #endif

    /* Step 7: Read Vendor ID (RC mode) */
    #ifdef DM0_RC
        data_rd = read_pcie_slv0_reg(0x0);
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] Vendor ID from slv0: 0x%08X\n", data_rd);
        #endif
    #endif
    #ifdef DM1_RC
        data_rd = read_pcie_slv1_reg(0x0);
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] Vendor ID from slv1: 0x%08X\n", data_rd);
        #endif
    #endif

    /* Step 8: Enable bus master, memory space, and I/O space access */
    #ifdef DM0_RC
        write_pcie_slv0_reg(0x4, 0x7);
    #endif
    #ifdef DM1_RC
        write_pcie_slv1_reg(0x4, 0x7);
    #endif

    /* Step 9: Program BARs and memory base addresses */
    #ifdef DM0_RC
        mem_base_program_dm0_x4();
        mem_base_program_dm1_x4();
    #endif
    #ifdef DM1_RC
        mem_base_program_dm0_x4();
        mem_base_program_dm1_x4();
    #endif
    #ifdef DM0_EP
        wait_on(30000);
        mem_base_program_dm0_x4();
        mem_base_program_dm1_x4();
    #endif
    #ifdef DM1_EP
        mem_base_program_dm0_x4();
        mem_base_program_dm1_x4();
    #endif

    /* Step 10: Configure non-secure protection via NIC programming */
    non_secure_prot_nic();

    /* Step 11: Sync register handshake */
    write_reg(SYNC_REG_ADDR, SYNC_HANDSHAKE_VAL);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] Sync register written with handshake value 0x%08X\n", SYNC_HANDSHAKE_VAL);
    #endif

    /* Step 12: Disable cache coherency - partial disable */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 13: Wait, then fully clear remaining coherency fields */
    wait_on(10);

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    #ifdef DEBUG_DISPLAY
        LOGI("[Run] Cache coherency disabled for PCIE0 and PCIE1\n");
    #endif

    /* Step 14: Memory write-read verification through PCIe slave ports */
    #ifdef DM0_RC
        pcie_slv0_mem_wr_rd();
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] pcie_slv0_mem_wr_rd completed\n");
        #endif
    #endif
    #ifdef DM1_RC
        pcie_slv1_mem_wr_rd();
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] pcie_slv1_mem_wr_rd completed\n");
        #endif
    #endif
    #ifdef DM0_EP
        pcie_slv0_mem_wr_rd();
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] pcie_slv0_mem_wr_rd completed (EP mode)\n");
        #endif
    #endif
    #ifdef DM1_EP
        pcie_slv1_mem_wr_rd();
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] pcie_slv1_mem_wr_rd completed (EP mode)\n");
        #endif
    #endif

    return out->status = test_err;
}

/*
 * Function: pcie_mem_wr_rd_test_teardown
 * Description: Performs final validation and cleanup for pcie_mem_wr_rd_test.
 *              Polls synchronization register until completion value is received.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIE memory write-read test: %s\n", cfg->test_name);

    /* Step 15: Poll synchronization register until expected completion value */
    data_rd = read_reg(SYNC_REG_ADDR);
    while (data_rd != SYNC_COMPLETE_VAL)
    {
        wait_on(5);
        data_rd = read_reg(SYNC_REG_ADDR);
        #ifdef DEBUG_DISPLAY
            LOGI("[Teardown] Polling sync register: data_rd=0x%08X\n", data_rd);
        #endif
    }
    #ifdef DEBUG_DISPLAY
        LOGI("[Teardown] Sync register matched completion value 0x%08X\n", SYNC_COMPLETE_VAL);
    #endif

    /*
     * Validation / Acceptance Criteria:
     * 1. Link status confirmed on appropriate SII interface.
     * 2. Vendor ID read successfully.
     * 3. Memory write-read verification passed on PCIe slave ports.
     * 4. Synchronization register returned expected completion value.
     * 5. Test passes by calling finish(0).
     */

    /* Step 16: Call finish(0) */
    finish(0);

    return 0;
}

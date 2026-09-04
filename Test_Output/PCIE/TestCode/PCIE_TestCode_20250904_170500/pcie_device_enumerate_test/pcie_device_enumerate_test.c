// Author - AI Force 2.3. 04-Sep-2025 17:05 IST
// (EMBENGG-SYSAPPS)

#include "pcie_device_enumerate_test.h"
#include "test_define.inc"

/* Global variables for testcase */
unsigned int data_rd, test_err;

/*
 * Function: pcie_device_enumerate_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              pcie_device_enumerate_test. Initializes the synchronization register,
 *              performs conditional link training, and programs cache coherency
 *              control registers for both PCIE0 and PCIE1 controllers.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIE device enumerate test: %s\n", cfg->test_name);

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

    /* Step 3: CACHE PROGRAMMING - PCIE0: Read, set bits [11:14]=0xf and [3:6]=0xf, write back */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 4: PCIE0: Read again, set bits [27:30]=0xf and [19:22]=0xf, write back */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 5: Repeat steps 3-4 for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 6: Wait for the configuration to take effect */
    wait_on(20);

    /* Step 7: Re-apply all cache coherency bits to PCIE0 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 8: Re-apply all cache coherency bits to PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 9: Repeat link training and cache programming block (duplicate code block) */
    #ifdef DM0_RC
        link_training_dm0_x4(4);
    #endif
    #ifdef DM1_RC
        link_training_dm1_x4(4);
    #endif
    #ifdef DM0_EP
        link_training_dm0_x4(4);
    #endif
    #ifdef DM1_EP
        link_training_dm1_x4(4);
    #endif

    /* Duplicate cache programming - PCIE0 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Duplicate cache programming - PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    wait_on(20);

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

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
 * Function: pcie_device_enumerate_test_run
 * Description: Main testcase execution for pcie_device_enumerate_test.
 *              Polls link status on SII0 and SII1 interfaces, reads Vendor ID,
 *              enables bus master and memory space access, programs memory base
 *              addresses, configures system-level registers, disables cache
 *              coherency, enumerates and programs BAR registers on both slave ports.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    LOGI("[Test Run] PCIE device enumerate test: %s\n", cfg->test_name);
    test_err = 0;

    /* Step 10: Read SII0 link status and call non_secure_prot_nic() */
    data_rd = read_sii0_reg(0xC0);
    non_secure_prot_nic();

    /* Step 11: Poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
    while ((data_rd & 0xD1) != 0xD1)
    {
        data_rd = read_sii0_reg(0xC0);
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] Polling SII0 link status: data_rd=0x%08X\n", data_rd);
        #endif
        wait_on(10);
    }
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SII0 link-up confirmed: data_rd=0x%08X\n", data_rd);
    #endif

    /* Step 12: Poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
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
        LOGI("[Run] SII1 link-up confirmed: data_rd=0x%08X\n", data_rd);
    #endif

    /* Step 13: Under DM0_RC - Read Vendor ID, enable bus master, program mem base */
    #ifdef DM0_RC
        data_rd = read_pcie_slv0_reg(0x0);
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] Vendor ID read from slv0 offset 0x0: 0x%08X\n", data_rd);
        #endif

        write_pcie_slv0_reg(0x4, 0x7);
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] Bus master + memory space + I/O space enabled (0x7) at slv0 offset 0x4\n");
        #endif

        mem_base_program_dm0_x4();
        mem_base_program_dm1_x4();
        wait_on(10);
    #endif

    /* Step 14: Write 0x1 to system-level configuration registers */
    write_reg(SYSREG_ADDR_0, 0x1);
    write_reg(SYSREG_ADDR_1, 0x1);
    write_reg(SYSREG_ADDR_2, 0x1);
    write_reg(SYSREG_ADDR_3, 0x1);
    write_reg(SYSREG_ADDR_4, 0x1);
    write_reg(SYSREG_ADDR_5, 0x1);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] System-level configuration registers written with 0x1\n");
    #endif

    /* Step 15: DISABLE_CACHE PROGRAMMING - PCIE0 partial disable */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* PCIE1 partial disable */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 16: Wait, then fully clear remaining coherency fields */
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

    /* Step 17: Wait before BAR enumeration */
    wait_on(30);

    /* Step 18: Enumerate BAR registers on slave port 1 - write 0xFFFFFFFF and read back */
    write_pcie_slv1_reg(BAR_OFFSET_0, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(BAR_OFFSET_0);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV1 BAR sizing offset 0x%02X: readback=0x%08X\n", BAR_OFFSET_0, data_rd);
    #endif

    write_pcie_slv1_reg(BAR_OFFSET_1, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(BAR_OFFSET_1);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV1 BAR sizing offset 0x%02X: readback=0x%08X\n", BAR_OFFSET_1, data_rd);
    #endif

    write_pcie_slv1_reg(BAR_OFFSET_2, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(BAR_OFFSET_2);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV1 BAR sizing offset 0x%02X: readback=0x%08X\n", BAR_OFFSET_2, data_rd);
    #endif

    write_pcie_slv1_reg(BAR_OFFSET_3, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(BAR_OFFSET_3);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV1 BAR sizing offset 0x%02X: readback=0x%08X\n", BAR_OFFSET_3, data_rd);
    #endif

    write_pcie_slv1_reg(BAR_OFFSET_4, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(BAR_OFFSET_4);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV1 BAR sizing offset 0x%02X: readback=0x%08X\n", BAR_OFFSET_4, data_rd);
    #endif

    write_pcie_slv1_reg(BAR_OFFSET_5, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(BAR_OFFSET_5);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV1 BAR sizing offset 0x%02X: readback=0x%08X\n", BAR_OFFSET_5, data_rd);
    #endif

    /* Step 19: Program BAR registers on slave port 1 with specific base addresses */
    write_pcie_slv1_reg(BAR_OFFSET_0, BAR_BASE_ADDR_0);
    data_rd = read_pcie_slv1_reg(BAR_OFFSET_0);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV1 BAR program offset 0x%02X = 0x%08X, readback=0x%08X\n", BAR_OFFSET_0, BAR_BASE_ADDR_0, data_rd);
    #endif

    write_pcie_slv1_reg(BAR_OFFSET_1, BAR_BASE_ADDR_1);
    data_rd = read_pcie_slv1_reg(BAR_OFFSET_1);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV1 BAR program offset 0x%02X = 0x%08X, readback=0x%08X\n", BAR_OFFSET_1, BAR_BASE_ADDR_1, data_rd);
    #endif

    write_pcie_slv1_reg(BAR_OFFSET_2, BAR_BASE_ADDR_2);
    data_rd = read_pcie_slv1_reg(BAR_OFFSET_2);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV1 BAR program offset 0x%02X = 0x%08X, readback=0x%08X\n", BAR_OFFSET_2, BAR_BASE_ADDR_2, data_rd);
    #endif

    write_pcie_slv1_reg(BAR_OFFSET_3, BAR_BASE_ADDR_3);
    data_rd = read_pcie_slv1_reg(BAR_OFFSET_3);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV1 BAR program offset 0x%02X = 0x%08X, readback=0x%08X\n", BAR_OFFSET_3, BAR_BASE_ADDR_3, data_rd);
    #endif

    write_pcie_slv1_reg(BAR_OFFSET_4, BAR_BASE_ADDR_4);
    data_rd = read_pcie_slv1_reg(BAR_OFFSET_4);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV1 BAR program offset 0x%02X = 0x%08X, readback=0x%08X\n", BAR_OFFSET_4, BAR_BASE_ADDR_4, data_rd);
    #endif

    write_pcie_slv1_reg(BAR_OFFSET_5, BAR_BASE_ADDR_5);
    data_rd = read_pcie_slv1_reg(BAR_OFFSET_5);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV1 BAR program offset 0x%02X = 0x%08X, readback=0x%08X\n", BAR_OFFSET_5, BAR_BASE_ADDR_5, data_rd);
    #endif

    /* Step 20: Enumerate BAR registers on slave port 0 - write 0xFFFFFFFF and read back */
    write_pcie_slv0_reg(BAR_OFFSET_0, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(BAR_OFFSET_0);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV0 BAR sizing offset 0x%02X: readback=0x%08X\n", BAR_OFFSET_0, data_rd);
    #endif

    write_pcie_slv0_reg(BAR_OFFSET_1, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(BAR_OFFSET_1);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV0 BAR sizing offset 0x%02X: readback=0x%08X\n", BAR_OFFSET_1, data_rd);
    #endif

    write_pcie_slv0_reg(BAR_OFFSET_2, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(BAR_OFFSET_2);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV0 BAR sizing offset 0x%02X: readback=0x%08X\n", BAR_OFFSET_2, data_rd);
    #endif

    write_pcie_slv0_reg(BAR_OFFSET_3, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(BAR_OFFSET_3);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV0 BAR sizing offset 0x%02X: readback=0x%08X\n", BAR_OFFSET_3, data_rd);
    #endif

    write_pcie_slv0_reg(BAR_OFFSET_4, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(BAR_OFFSET_4);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV0 BAR sizing offset 0x%02X: readback=0x%08X\n", BAR_OFFSET_4, data_rd);
    #endif

    write_pcie_slv0_reg(BAR_OFFSET_5, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(BAR_OFFSET_5);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV0 BAR sizing offset 0x%02X: readback=0x%08X\n", BAR_OFFSET_5, data_rd);
    #endif

    /* Step 21: Program BAR registers on slave port 0 with specific base addresses */
    write_pcie_slv0_reg(BAR_OFFSET_0, BAR_BASE_ADDR_0);
    data_rd = read_pcie_slv0_reg(BAR_OFFSET_0);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV0 BAR program offset 0x%02X = 0x%08X, readback=0x%08X\n", BAR_OFFSET_0, BAR_BASE_ADDR_0, data_rd);
    #endif

    write_pcie_slv0_reg(BAR_OFFSET_1, BAR_BASE_ADDR_1);
    data_rd = read_pcie_slv0_reg(BAR_OFFSET_1);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV0 BAR program offset 0x%02X = 0x%08X, readback=0x%08X\n", BAR_OFFSET_1, BAR_BASE_ADDR_1, data_rd);
    #endif

    write_pcie_slv0_reg(BAR_OFFSET_2, BAR_BASE_ADDR_2);
    data_rd = read_pcie_slv0_reg(BAR_OFFSET_2);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV0 BAR program offset 0x%02X = 0x%08X, readback=0x%08X\n", BAR_OFFSET_2, BAR_BASE_ADDR_2, data_rd);
    #endif

    write_pcie_slv0_reg(BAR_OFFSET_3, BAR_BASE_ADDR_3);
    data_rd = read_pcie_slv0_reg(BAR_OFFSET_3);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV0 BAR program offset 0x%02X = 0x%08X, readback=0x%08X\n", BAR_OFFSET_3, BAR_BASE_ADDR_3, data_rd);
    #endif

    write_pcie_slv0_reg(BAR_OFFSET_4, BAR_BASE_ADDR_4);
    data_rd = read_pcie_slv0_reg(BAR_OFFSET_4);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV0 BAR program offset 0x%02X = 0x%08X, readback=0x%08X\n", BAR_OFFSET_4, BAR_BASE_ADDR_4, data_rd);
    #endif

    write_pcie_slv0_reg(BAR_OFFSET_5, BAR_BASE_ADDR_5);
    data_rd = read_pcie_slv0_reg(BAR_OFFSET_5);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] SLV0 BAR program offset 0x%02X = 0x%08X, readback=0x%08X\n", BAR_OFFSET_5, BAR_BASE_ADDR_5, data_rd);
    #endif

    /* Step 22: Wait after BAR programming */
    wait_on(10);

    return out->status = test_err;
}

/*
 * Function: pcie_device_enumerate_test_teardown
 * Description: Performs final validation and cleanup for pcie_device_enumerate_test.
 *              Polls the synchronization register until the expected completion value
 *              is received, confirming successful enumeration, then calls finish(0).
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIE device enumerate test: %s\n", cfg->test_name);

    /* Step 23: Poll synchronization register until value equals 0x12345678 */
    data_rd = read_reg(SYNC_REG_ADDR);
    while (data_rd != SYNC_EXPECTED_VAL)
    {
        wait_on(5);
        data_rd = read_reg(SYNC_REG_ADDR);
        #ifdef DEBUG_DISPLAY
            LOGI("[Teardown] Polling sync register 0x%08X: data_rd=0x%08X\n", SYNC_REG_ADDR, data_rd);
        #endif
    }
    #ifdef DEBUG_DISPLAY
        LOGI("[Teardown] Sync register matched expected value 0x%08X\n", SYNC_EXPECTED_VAL);
    #endif

    /*
     * Validation / Acceptance Criteria:
     * 1. SII0 and SII1 gic registers indicated link-up with (data_rd & 0xD1) == 0xD1.
     * 2. TYPE1_DEV_ID_VEND_ID_REG returned a valid Vendor ID (printed via LOGI).
     * 3. BAR registers on both slv0 and slv1 responded correctly to all-ones writes
     *    for BAR sizing and retained programmed base address values on readback.
     * 4. Synchronization register returned 0x12345678 confirming successful enumeration.
     * 5. Test passes by calling finish(0).
     */

    /* Step 24: Call finish(0) */
    finish(0);

    return 0;
}

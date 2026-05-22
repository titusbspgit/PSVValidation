// Author - AI Force 1.3.2. Date 22-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// -----------------------------------------------------------------------------
// Function: chk_rst_val
// Purpose : Verify default reset values for readable registers, skipping
//           specified addresses per Meta Test Steps.
// -----------------------------------------------------------------------------
static void chk_rst_val(void)
{
    int i;
    unsigned long addr;
    unsigned int data_rd;
    unsigned int exp_val;

    for (i = 0; i < CNT; i++) {
        addr = addr_array[i];

        // Skip when read mask is zero
        if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
            printf("[DBG][RST] Skip idx=%d addr=0x%08lx due to read_mask=0x%08x\n", i, addr, read_mask_array[i]);
#endif
            continue;
        }

        // Skip default comparison for specific registers
        if (addr == mizar_PCIE1_DBI_DSP_CAP_ID_NXT_PTR_REG ||
            addr == mizar_PCIE1_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS ||
            addr == mizar_PCIE1_DBI_DSP_PL_DEBUG1_OFF) {
#ifdef DEBUG_DISPLAY
            printf("[DBG][RST] Skip default-compare idx=%d addr=0x%08lx (excluded)\n", i, addr);
#endif
            continue;
        }

        // Read and compare against expected default value
        data_rd = read_reg(addr);
        exp_val = default_value_array[i];
        if (data_rd != exp_val) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[ERR][RST] Mismatch idx=%d addr=0x%08lx rd=0x%08x exp=0x%08x def_fail_cnt=%d\n",
                   i, addr, data_rd, exp_val, def_fail_cnt);
#endif
        } else {
#ifdef DEBUG_DISPLAY
            printf("[OK ][RST] Match idx=%d addr=0x%08lx val=0x%08x\n", i, addr, data_rd);
#endif
        }
    }
}

// -----------------------------------------------------------------------------
// Function: chk_rd_wr
// Purpose : Perform masked write and read-back verification using patterns.
// -----------------------------------------------------------------------------
static void chk_rd_wr(void)
{
    int i, j;
    unsigned int chk_val[6] = { 0xffffffffU, 0xaaaaaaaaU, 0x55555555U, 0x00000000U, 0xA5A5A5A5U, 0xffff0000U };

    for (j = 0; j < 6; j++) {
        unsigned int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[DBG][WR ] Pattern %d data_wr=0x%08x\n", j, data_wr);
#endif
        // Write phase
        for (i = 0; i < CNT; i++) {
            unsigned long addr = addr_array[i];
            if (skip_array[i] == 1 || write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG][WR ] Skip write idx=%d addr=0x%08lx skip=%d wmask=0x%08x\n",
                       i, addr, skip_array[i], write_mask_array[i]);
#endif
                continue;
            }
            write_reg(addr, data_wr);
#ifdef DEBUG_DISPLAY
            printf("[DBG][WR ] Write idx=%d addr=0x%08lx data=0x%08x\n", i, addr, data_wr);
#endif
        }

        // Read/verify phase
        for (i = 0; i < CNT; i++) {
            unsigned long addr = addr_array[i];
            if (skip_array[i] == 1 || write_mask_array[i] == 0x00000000U || read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG][RD ] Skip read idx=%d addr=0x%08lx skip=%d wmask=0x%08x rmask=0x%08x\n",
                       i, addr, skip_array[i], write_mask_array[i], read_mask_array[i]);
#endif
                continue;
            }

            unsigned int data_rd = read_reg(addr);
            unsigned int wr_n = (write_mask_array[i] ^ 0xffffffffU);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                     (wr_n    & read_mask_array[i] & default_value_array[i]));

            if (data_rd != exp_val) {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[ERR][RD ] Mismatch idx=%d addr=0x%08lx rd=0x%08x exp=0x%08x wr_fail_cnt=%d\n",
                       i, addr, data_rd, exp_val, wr_fail_cnt);
#endif
            } else {
#ifdef DEBUG_DISPLAY
                printf("[OK ][RD ] Match idx=%d addr=0x%08lx val=0x%08x\n", i, addr, data_rd);
#endif
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Function: test_case (Entry Point)
// Purpose : Execute test steps and terminate with finish(0)/finish(1) per
//           acceptance criteria.
// -----------------------------------------------------------------------------
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[INFO] Starting test_case()\n");
#endif

    chk_rst_val();
    chk_rd_wr();

#ifdef DEBUG_DISPLAY
    printf("[INFO] Test counters: def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif

    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        finish(1); // FAIL
        return 1;
    }

    finish(0); // PASS
    return 0;
}

// -----------------------------------------------------------------------------
// Function: main
// Purpose : Invoke the test entry point.
// -----------------------------------------------------------------------------
int main(void)
{
    return test_case();
}

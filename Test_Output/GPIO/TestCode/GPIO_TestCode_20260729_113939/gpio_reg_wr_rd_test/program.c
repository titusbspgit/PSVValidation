// Author - AI Force 1.3.2. Date 29-07-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// -----------------------------------------------------------------------------
// Function: chk_rst_val
// Purpose : Phase-1 reset/default value verification over all registers
// Notes   : Skips indices where skip_rst_array[i]==1 or read_mask_array[i]==0
// -----------------------------------------------------------------------------
static unsigned int def_fail_cnt = 0; // default/reset check failures
static unsigned int wr_fail_cnt  = 0; // write/readback failures

static void chk_rst_val(void)
{
    unsigned int i;
    unsigned int count = (unsigned int)(sizeof(addr_array)/sizeof(addr_array[0]));
    for (i = 0; i < count; i++) {
        unsigned long int addr = addr_array[i];
        if (skip_rst_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
            printf("[DBG] RST-SKIP idx=%u addr=0x%08lx\n", i, addr);
#endif
            continue; // skip reset check for this index
        }
        if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
            printf("[DBG] RST-MASK0 idx=%u addr=0x%08lx\n", i, addr);
#endif
            continue; // nothing readable to validate
        }

        unsigned int data_rd = read_reg(addr);            // raw register read
        unsigned int data    = (data_rd & 0xFFFFFFFEU);   // mask per Meta description

        if (data == default_value_array[i]) {
#ifdef DEBUG_DISPLAY
            printf("[PASS] RST addr=0x%08lx exp=0x%08x rd(masked)=0x%08x raw=0x%08x\n",
                   addr, default_value_array[i], data, data_rd);
#endif
        } else {
            def_fail_cnt++;
            printf("[FAIL] RST addr=0x%08lx exp=0x%08x rd(masked)=0x%08x raw=0x%08x\n",
                   addr, default_value_array[i], data, data_rd);
        }
    }
}

// -----------------------------------------------------------------------------
// Function: chk_rd_wr
// Purpose : Phase-2 write/readback verification for each data pattern in chk_val
// Notes   : Writes masked values when skip==0 and write_mask!=0, then verifies
// -----------------------------------------------------------------------------
static void chk_rd_wr(void)
{
    unsigned int i, j;
    unsigned int count = (unsigned int)(sizeof(addr_array)/sizeof(addr_array[0]));

    for (j = 0; j < 6U; j++) {
        unsigned int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[DBG] PATTERN j=%u data_wr=0x%08x\n", j, data_wr);
#endif
        // Write phase
        for (i = 0; i < count; i++) {
            unsigned long int addr = addr_array[i];
            if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] WR-SKIP idx=%u addr=0x%08lx\n", i, addr);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] WR-MASK0 idx=%u addr=0x%08lx\n", i, addr);
#endif
                continue;
            }
            write_reg(addr, (data_wr & write_mask_array[i]));
#ifdef DEBUG_DISPLAY
            printf("[DBG] WR addr=0x%08lx val=0x%08x (mask=0x%08x)\n", addr, (data_wr & write_mask_array[i]), write_mask_array[i]);
#endif
        }

        // Read/verify phase
        for (i = 0; i < count; i++) {
            unsigned long int addr = addr_array[i];
            if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] RD-SKIP idx=%u addr=0x%08lx\n", i, addr);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] RD-WRMASK0 idx=%u addr=0x%08lx\n", i, addr);
#endif
                continue;
            }
            if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] RD-RDMASK0 idx=%u addr=0x%08lx\n", i, addr);
#endif
                continue;
            }

            unsigned int data_rd = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n    = (write_mask_array[i] ^ 0xFFFFFFFFU);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                    (wr_n & read_mask_array[i] & default_value_array[i]));

            if (data_rd == exp_val) {
#ifdef DEBUG_DISPLAY
                printf("[PASS] RDWR addr=0x%08lx exp=0x%08x rd=0x%08x j=%u\n", addr, exp_val, data_rd, j);
#endif
            } else {
                wr_fail_cnt++;
                printf("[FAIL] RDWR addr=0x%08lx exp=0x%08x rd=0x%08x j=%u wr_mask=0x%08x rd_mask=0x%08x\n",
                       addr, exp_val, data_rd, j, write_mask_array[i], read_mask_array[i]);
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Function: test_case
// Purpose : Entry point - executes both phases and terminates with finish()
// -----------------------------------------------------------------------------
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[INFO] gpio_reg_wr_rd_test: START\n");
#endif
    chk_rst_val();
    chk_rd_wr();

    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U)) {
#ifdef DEBUG_DISPLAY
        printf("[INFO] gpio_reg_wr_rd_test: END - FAIL def=%u wr=%u\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(1);
        return 0; // finish() is the terminal call; return is for signature compliance
    } else {
#ifdef DEBUG_DISPLAY
        printf("[INFO] gpio_reg_wr_rd_test: END - PASS\n");
#endif
        finish(0);
        return 0; // finish() is the terminal call; return is for signature compliance
    }
}

// -----------------------------------------------------------------------------
// Function: soft_reset_chk (disabled)
// Purpose : Example soft reset sequence (not executed)
// -----------------------------------------------------------------------------
#ifdef 0
static void soft_reset_chk(void)
{
    unsigned int i;
    (void)i;

    unsigned int rst_rd = read_reg(SOFT_RST_REG_ADDRESS);
    (void)rst_rd;

    write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA);
    wait_on(10); // platform-provided delay

    // Restore defaults (masked) after reset as a reference pattern
    for (i = 0; i < (unsigned int)(sizeof(addr_array)/sizeof(addr_array[0])); i++) {
        if (write_mask_array[i] != 0U) {
            write_reg(addr_array[i], (default_value_array[i] & write_mask_array[i]));
        }
    }
    wait_on(10);
}
#endif

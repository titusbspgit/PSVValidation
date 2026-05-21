// Author - AI Force 1.3.2. Date 21-05-2026
// (EMBENGG-SYSAPPS)

// Include only test_define.c as per rules
#include "test_define.c"

// -----------------------------------------------------------------------------
// Function: chk_rst_val
// Description:
//   Implements reset value checks as per Meta Test Steps.
//   For each register index in [0 .. CNT-1]:
//     - Skip if skip_rst_array[i] == 1
//     - Skip if read_mask_array[i] == 0x00000000
//     - Read register, mask with 0xFFFFFFFE, compare with default_value_array[i]
//   On mismatch, increment def_fail_cnt and print detailed debug info.
// -----------------------------------------------------------------------------
static unsigned int def_fail_cnt = 0; // default value check failures
static unsigned int wr_fail_cnt  = 0; // write/read check failures

static void chk_rst_val(void)
{
    for (unsigned int i = 0; i < CNT; i++) {
        if (skip_rst_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] RST: Skipping index %u (skip_rst=1)\n", i);
#endif
            continue;
        }

        if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] RST: Skipping index %u (read_mask=0)\n", i);
#endif
            continue;
        }

        unsigned int addr = addr_array[i];
        unsigned int data_rd = read_reg(addr);
        unsigned int data = (data_rd & 0xFFFFFFFEU);

        if (data != default_value_array[i]) {
            def_fail_cnt++;
            printf("[RST_MISMATCH] idx=%u addr=0x%08X exp=0x%08X got_masked=0x%08X got_raw=0x%08X\n",
                   i, addr, default_value_array[i], data, data_rd);
        } else {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] RST: idx=%u addr=0x%08X OK (exp=0x%08X, got=0x%08X)\n",
                   i, addr, default_value_array[i], data);
#endif
        }
    }
}

// -----------------------------------------------------------------------------
// Function: chk_rd_wr
// Description:
//   Implements write/read verification using fixed patterns per Meta Steps.
//   For each pattern in chk_val[]:
//     - Write phase: write (pattern & write_mask) to each non-skipped register
//     - Verify phase: read back, mask with read_mask, and compare to expected
//       value computed using masks and default values.
//   On mismatch, increment wr_fail_cnt and print detailed debug info.
// -----------------------------------------------------------------------------
static void chk_rd_wr(void)
{
    const unsigned int chk_val[6] = {
        0xFFFFFFFFU, 0xAAAAAAAAU, 0x55555555U,
        0xF5F5F5F5U, 0xA5A5A5A5U, 0xFFFF0000U
    };

    for (unsigned int j = 0; j < 6U; j++) {
        unsigned int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] WR: pattern[%u]=0x%08X\n", j, data_wr);
#endif
        // Write phase
        for (unsigned int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] WR: Skipping index %u (skip=1)\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] WR: Skipping index %u (write_mask=0)\n", i);
#endif
                continue;
            }
            unsigned int addr = addr_array[i];
            unsigned int wr_val = (data_wr & write_mask_array[i]);
            write_reg(addr, wr_val);
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] WR: idx=%u addr=0x%08X wr=0x%08X (wmask=0x%08X)\n",
                   i, addr, wr_val, write_mask_array[i]);
#endif
        }

        // Read/verify phase
        for (unsigned int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] RD: Skipping index %u (skip=1)\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] RD: Skipping index %u (write_mask=0)\n", i);
#endif
                continue;
            }
            if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] RD: Skipping index %u (read_mask=0)\n", i);
#endif
                continue;
            }

            unsigned int addr = addr_array[i];
            unsigned int data_rd = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                     (wr_n & read_mask_array[i] & default_value_array[i]));

            if (data_rd != exp_val) {
                wr_fail_cnt++;
                printf("[WRRD_MISMATCH] pat=%u idx=%u addr=0x%08X exp=0x%08X got=0x%08X wmask=0x%08X rmask=0x%08X def=0x%08X\n",
                       j, i, addr, exp_val, data_rd, write_mask_array[i], read_mask_array[i], default_value_array[i]);
            } else {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] RD: idx=%u addr=0x%08X OK (exp=0x%08X, got=0x%08X)\n",
                       i, addr, exp_val, data_rd);
#endif
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Function: test_case
// Description:
//   Entry point. Executes chk_rst_val() followed by chk_rd_wr().
//   Evaluates def_fail_cnt and wr_fail_cnt and terminates via finish().
// -----------------------------------------------------------------------------
void test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] TEST START: gpio_reg_wr_rd_test\n");
#endif
    chk_rst_val();
    chk_rd_wr();

    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U)) {
        printf("[RESULT] FAIL def=%u wr=%u\n", def_fail_cnt, wr_fail_cnt);
        finish(1);
    } else {
        printf("[RESULT] PASS\n");
        finish(0);
    }
}

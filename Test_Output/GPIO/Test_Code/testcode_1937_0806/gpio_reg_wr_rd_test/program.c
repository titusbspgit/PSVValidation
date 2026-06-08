// Author - AI Force 1.3.2. Date 08-06-2026
// (EMBENGG-SYSAPPS)

/* Include only test_define.c as mandated */
#include "test_define.c"

/* External test framework termination hook */
void finish(int status);

/* -------------------------------------------------------------------------
 * Function: log_error_reset
 * Purpose : Log a reset/default mismatch for a register index
 * ------------------------------------------------------------------------- */
static void log_error_reset(int idx, uint32_t got, uint32_t exp)
{
#ifdef DEBUG_DISPLAY
    printf("[RESET-CHK][%s] got=0x%08X exp=0x%08X\n", reg_names[idx], got, exp);
#else
    (void)idx; (void)got; (void)exp;
#endif
}

/* -------------------------------------------------------------------------
 * Function: log_error_wrchk
 * Purpose : Log a masked write-read verification mismatch
 * ------------------------------------------------------------------------- */
static void log_error_wrchk(int idx,
                            uint32_t pattern,
                            uint32_t wr_mask,
                            uint32_t rd_mask,
                            uint32_t rd_val,
                            uint32_t exp_val)
{
#ifdef DEBUG_DISPLAY
    printf("[WR-RD-CHK][%s] pat=0x%08X wr_m=0x%08X rd_m=0x%08X got=0x%08X exp=0x%08X\n",
           reg_names[idx], pattern, wr_mask, rd_mask, rd_val, exp_val);
#else
    (void)idx; (void)pattern; (void)wr_mask; (void)rd_mask; (void)rd_val; (void)exp_val;
#endif
}

/* -------------------------------------------------------------------------
 * Function: soft_reset_chk (disabled as per meta)
 * Purpose : Placeholder for soft reset sequence (not executed)
 * ------------------------------------------------------------------------- */
static void soft_reset_chk(void)
{
#ifdef DEBUG_DISPLAY
    printf("[INFO] soft_reset_chk disabled by meta.\n");
#endif
    /* no-op */
}

/* -------------------------------------------------------------------------
 * Function: test_case (ENTRY POINT)
 * Purpose : Execute steps from Meta Test Steps / Procedure
 * ------------------------------------------------------------------------- */
int test_case(void)
{
    int errors = 0;                 /* error counter */
    uint32_t val = 0u;              /* temporary read value */

#ifdef DEBUG_DISPLAY
    printf("[START] %s\n", TEST_NAME);
#endif

    /* Initialize shadow with reset defaults */
    for (int i = 0; i < (int)REG_COUNT; ++i) {
        reg_shadow[i] = default_reset_val[i];
    }

    /* Step 1: Reset/default verification loop */
    for (int i = 0; i < (int)REG_COUNT; ++i) {
        if (reg_skip[i]) { continue; }
        val = reg_read_idx(i);
        if (val != default_reset_val[i]) {
            log_error_reset(i, val, default_reset_val[i]);
            errors++;
        }
    }

    /* Step 2: Masked write-read verification using provided patterns */
    for (int i = 0; i < (int)REG_COUNT; ++i) {
        if (reg_skip[i]) { continue; }
        const uint32_t wr_m = write_mask[i];
        const uint32_t rd_m = read_mask[i];
        for (unsigned pi = 0u; pi < PATTERN_COUNT; ++pi) {
            const uint32_t p = patterns[pi];
            const uint32_t wr_val = (p & wr_m);           /* apply write mask */
            reg_write_idx(i, wr_val);                     /* write */
            const uint32_t rd_val = reg_read_idx(i);      /* read */
            const uint32_t rd_chk = (rd_val & rd_m);      /* mask read */
            const uint32_t exp_chk = ((p & wr_m) & rd_m); /* expected */
            if (rd_chk != exp_chk) {
                log_error_wrchk(i, p, wr_m, rd_m, rd_chk, exp_chk);
                errors++;
            }
        }
    }

    /* Soft reset check routine exists but is disabled; not executed */
    (void)soft_reset_chk;

#ifdef DEBUG_DISPLAY
    if (errors == 0) {
        printf("[RESULT] PASS\n");
    } else {
        printf("[RESULT] FAIL (errors=%d)\n", errors);
    }
#endif

    if (errors == 0) {
        finish(0); /* PASS */
        return 0;
    } else {
        finish(1); /* FAIL */
        return 1;
    }
}

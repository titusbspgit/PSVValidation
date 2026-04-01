/*
 * Testcase: test_gpio_pedge_all_pads_en
 * High-level objective:
 *   Enable posedge interrupts on all GPIO[8..39] pins. For each iteration:
 *   - Drive a rising edge via pad control at 0xA0243ffc.
 *   - Wait for ISR to acknowledge and clear.
 *   - Verify non-zero group status on entry, clear per-pin RAW for all, and
 *     clear system RAW and re-enable group before next iteration.
 *
 * Notes:
 *   - Uses volatile int_pend for ISR synchronization.
 *   - IRQ ID depends on GPIO0/GPIO1 selection (87/88) via build-time macros.
 */

/* ===== Required includes (unchanged, placed before any other code) ===== */
#include <stdio.h>
#include <lss_sysreg.h>
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

/* ===== Required defines (unchanged) ===== */
#define DEBUG_RW_MSG

/* No register tables or arrays are declared here by design (Stage3 rule).
 * program.c relies solely on symbolic register macros from included headers.
 */

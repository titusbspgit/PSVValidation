/*
 * Testcase: test_gpio_negedge_intr_en
 * High-level objective:
 *   Exercise per-pin negative-edge interrupts for GPIO[8..39]. For each pin:
 *   - Configure pin-level registers for negative-edge detection.
 *   - Enable only that pin's interrupt in the group enable register.
 *   - Generate a falling edge via pad control at 0xA0243ffc.
 *   - Wait for ISR to acknowledge and clear.
 *   - Verify group status bit behavior and clear system/GIC status.
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
#define CNT 49
#define DEBUG_RW_MSG

/* No register tables or arrays are declared here by design (Stage3 rule).
 * program.c relies solely on symbolic register macros from included headers.
 */

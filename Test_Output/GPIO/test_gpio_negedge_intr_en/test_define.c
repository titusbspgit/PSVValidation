#include <stdio.h>
#include <lss_sysreg.h>
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

/* Extracted macro definitions (kept unchanged) */
#define CNT 49

/* Helpers specific to this testcase */
#define GPIO_PAD_COUNT 32           /* Pads 8..39 inclusive */
#define DRIVE_PORT_ADDR 0xA0243ffc  /* External drive port from CSV Memory Start Offset */

/* Global state for test/ISR coordination */
volatile int test_err = 0;
volatile int int_pend = 0;
volatile int cur_idx = -1;   /* 0..31 corresponds to pads 8..39 */

/* Optional per-pad skip control (0 = test, 1 = skip) */
int skip_array[GPIO_PAD_COUNT] = {0};

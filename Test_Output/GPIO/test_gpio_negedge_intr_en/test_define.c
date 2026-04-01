#include <test_common.h>
#include <lss_sysreg.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>
#include <stdint.h>

/*
  Testcase: test_gpio_negedge_intr_en
  High-level: Exercise per-pin negative-edge interrupts for GPIO[8..39].
  Notes from CSV Remarks:
    - Behavior conditional on GPIO0/GPIO1 macros to select IRQ ID (87/88) and sysreg bits.
    - Uses external int_pend flag for ISR synchronization.
    - Drives pad control via 0xA0243ffc to generate edges.
*/

/* Unchanged macros from context */
#define CNT 49

/* Helper: compute per-pin register address for GP0 GPIO[8..39] */
#define GPIO_PIN_ADDR(i) ( (uint32_t)(MIZAR_GPIO_GP0_GPIO_8) + ((uint32_t)(i) * 4u) )

/* Number of GPIO pins covered in this test (8..39 => 32 pins) */
static const uint32_t gpio_pin_count = 32u;

/* Skip map for per-pin operations (no skips by default for this testcase) */
static const unsigned int skip_array[32] = {
  0,0,0,0,  0,0,0,0,
  0,0,0,0,  0,0,0,0,
  0,0,0,0,  0,0,0,0,
  0,0,0,0,  0,0,0,0
};

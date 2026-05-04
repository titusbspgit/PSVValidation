#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

#define GPIO_CNT      32   /* pads 8..39 */
#define EXT_PAD_CTRL  0xA0243ffcu
#define EXT_PAD_STAT  0xA0243ff8u

/* No additional constants required; addresses come from SoC headers. */

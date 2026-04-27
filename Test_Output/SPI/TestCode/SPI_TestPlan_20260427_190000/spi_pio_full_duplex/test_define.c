#ifndef TEST_DEFINE_C
#define TEST_DEFINE_C

#include <stdio.h>
#include <stdlib.h>
#include<test_common.h>
#include "../common/spi_parameter_def.h"
#include<spi.h>

/* No testcase-specific macros provided in context; keep section intentionally empty per rules */

/* Impacted registers array (use exactly the impacted register macros) */
static const unsigned long impacted_regs[] = {
    MIZAR_SPI_DATA_REG,
    MIZAR_SPI_MIS
};

/* Skip registers: none provided. Keep count as 0 and provide a placeholder array. */
#define NUM_SKIP_REGS 0u
static const unsigned long skip_regs[1] = { 0u }; /* placeholder, count is NUM_SKIP_REGS */

#endif /* TEST_DEFINE_C */

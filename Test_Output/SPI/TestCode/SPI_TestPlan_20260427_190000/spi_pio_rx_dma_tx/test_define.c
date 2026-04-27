#ifndef TEST_DEFINE_C
#define TEST_DEFINE_C

#include <stdio.h>
#include <stdlib.h>
#include<test_common.h>
#include "../common/spi_parameter_def.h"
#include<spi.h>

/* Provided testcase macros (unchanged) */
#define SPI_INTR_MASK 0x0
#define SPI_TX_FIFO_THLD  0x1

/* Impacted registers array */
static const unsigned long impacted_regs[] = {
    MIZAR_SPI_DATA_REG,
    MIZAR_SPI_IMSC,
    MIZAR_SPI_MIS
};

/* Skip registers: none provided. Keep count as 0 and provide a placeholder array. */
#define NUM_SKIP_REGS 0u
static const unsigned long skip_regs[1] = { 0u }; /* placeholder, count is NUM_SKIP_REGS */

#endif /* TEST_DEFINE_C */

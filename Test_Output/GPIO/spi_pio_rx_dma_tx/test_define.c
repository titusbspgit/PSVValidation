#include <stdio.h>
#include <stdlib.h>
#include<test_common.h>
#include "../common/spi_parameter_def.h"
#include<spi.h>

#define SPI_INTR_MASK 0x0
#define SPI_TX_FIFO_THLD  0x1

/* Arrays listing impacted and skipped registers (order preserved as metadata) */
const unsigned long int addr_array[3] = {
    MIZAR_SPI_DATA_REG,
    MIZAR_SPI_IMSC,
    MIZAR_SPI_MIS
};

/* No explicit skip list provided; default to not skipping any impacted registers */
const unsigned int skip_array[3] = { 0U, 0U, 0U };

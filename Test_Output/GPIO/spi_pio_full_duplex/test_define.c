#include <stdio.h>
#include <stdlib.h>
#include<test_common.h>
#include "../common/spi_parameter_def.h"
#include<spi.h>

/* Arrays listing impacted and skipped registers (order preserved as metadata) */
const unsigned long int addr_array[2] = {
    MIZAR_SPI_DATA_REG,
    MIZAR_SPI_MIS
};

/* No explicit skip list provided; default to not skipping any impacted registers */
const unsigned int skip_array[2] = { 0U, 0U };

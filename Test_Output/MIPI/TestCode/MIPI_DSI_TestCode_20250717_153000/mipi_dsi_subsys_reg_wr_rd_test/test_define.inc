// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

#define MIPI_DSI_SUBSYS_POLL_TIMEOUT 10000U
#define MIPI_DSI_SUBSYS_REG_COUNT 5U

#ifndef CTRL_REG_ADDR
#define CTRL_REG_ADDR 0xE6004100UL
#endif

#ifndef SYNC_HANDSHAKE_VALUE
#define SYNC_HANDSHAKE_VALUE 0x12345678U
#endif

static const uint32_t addr_array[MIPI_DSI_SUBSYS_REG_COUNT] = {
    MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL,
    MIZAR_MIPI_DSI_SUBSYS_LOW_PWR,
    MIZAR_MIPI_DSI_SUBSYS_DBITE,
    MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV,
    MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW
};

static const uint32_t skip_array[MIPI_DSI_SUBSYS_REG_COUNT] = {
    0U, 0U, 0U, 0U, 1U
};

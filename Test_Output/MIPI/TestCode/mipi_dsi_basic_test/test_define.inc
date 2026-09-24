// Author - AI Force 2.3. 25-Jul-2025 04:30 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_dsi_basic_test
 * Purpose: Supporting definitions for basic MIPI DSI DBI write-memory-start
 *   operation using the integrated GDMA (DMA330).
 */

#include "test.h"

#include <stdint.h>

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* Headers field from Meta TestPlan JSON was NA - no additional headers */

/* DMA interrupt enable mask for channels */
#define DMA_INTEN_CH_MASK       0x3U

/* DMA source address register channel identifier */
#define DMA_SAR                 0x00U

/* DMA destination address register channel identifier */
#define DMA_DAR                 0x01U

/* RAM base address for DMA source */
#define RAM_BASE                0x00100000UL

/* DSI write memory start command address */
#define DSI_WRITE_MEMORY_START  0x2CU

/* DSI interrupt number */
#define DSI_INTR_NO             32U

/* DMA330 instruction opcodes */
#define DMAMOV_SAR_OPCODE       0x00BCU
#define DMAMOV_DAR_OPCODE       0x02BCU
#define DMALP_OPCODE            0x20U
#define DMALD_OPCODE            0x04U
#define DMAST_OPCODE            0x08U
#define DMALPEND_OPCODE         0x28U
#define DMASEV_OPCODE           0x34U
#define DMAEND_OPCODE           0x00U

/* Macros field from Meta TestPlan JSON was NA - no additional macros */

/* Arrays field from Meta TestPlan JSON was NA - no additional arrays */

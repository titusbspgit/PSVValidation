// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case Name: mipi_dsi_subsys_reg_wr_rd_test
 * Purpose: Supporting definitions for MIPI DSI subsystem register write-read verification testcase.
 */

#include "test.h"

#include <stdint.h>

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* ============================================================ */
/* Headers from Meta TestPlan JSON                              */
/* ============================================================ */
// Headers field was NA in the Meta TestPlan JSON. No additional headers to emit.

/* ============================================================ */
/* Macros from Meta TestPlan JSON                               */
/* ============================================================ */
// Macros field was NA in the Meta TestPlan JSON. No additional macros to emit.

/* ============================================================ */
/* Arrays from Meta TestPlan JSON                               */
/* ============================================================ */
// Arrays field was NA in the Meta TestPlan JSON. No additional arrays to emit.

/* ============================================================ */
/* Impacted Registers (5 registers)                             */
/* Addresses are not supplied in the Meta TestPlan JSON.         */
/* Each register is preserved as a MANUAL_REVIEW placeholder.   */
/* ============================================================ */

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL
// #define MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_SUBSYS_LOW_PWR was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_SUBSYS_LOW_PWR
// #define MIZAR_MIPI_DSI_SUBSYS_LOW_PWR 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_SUBSYS_DBITE was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_SUBSYS_DBITE
// #define MIZAR_MIPI_DSI_SUBSYS_DBITE 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV
// #define MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW
// #define MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

/* ============================================================ */
/* Register Default Value Macros (5 registers)                  */
/* Values are not supplied in the Meta TestPlan JSON.            */
/* Each macro is preserved as a MANUAL_REVIEW placeholder.      */
/* ============================================================ */

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_DEFAULT_VAL was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_DEFAULT_VAL
// #define MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_DEFAULT_VAL 0x0UL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_LOW_PWR_DEFAULT_VAL was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_LOW_PWR_DEFAULT_VAL
// #define MIPI_DSI_SUBSYS_LOW_PWR_DEFAULT_VAL 0x0UL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_DBITE_DEFAULT_VAL was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_DBITE_DEFAULT_VAL
// #define MIPI_DSI_SUBSYS_DBITE_DEFAULT_VAL 0x0UL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_DBI_FDIV_DEFAULT_VAL was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_DBI_FDIV_DEFAULT_VAL
// #define MIPI_DSI_SUBSYS_DBI_FDIV_DEFAULT_VAL 0x0UL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_INTERRUPT_RAW_DEFAULT_VAL was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_INTERRUPT_RAW_DEFAULT_VAL
// #define MIPI_DSI_SUBSYS_INTERRUPT_RAW_DEFAULT_VAL 0x0UL /* MANUAL_REVIEW: value not supplied */
#endif

/* ============================================================ */
/* Register Read Mask Macros (5 registers)                      */
/* Values are not supplied in the Meta TestPlan JSON.            */
/* Each macro is preserved as a MANUAL_REVIEW placeholder.      */
/* ============================================================ */

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_READ_MASK was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_READ_MASK
// #define MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_READ_MASK 0xFFFFFFFFUL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_LOW_PWR_READ_MASK was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_LOW_PWR_READ_MASK
// #define MIPI_DSI_SUBSYS_LOW_PWR_READ_MASK 0xFFFFFFFFUL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_DBITE_READ_MASK was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_DBITE_READ_MASK
// #define MIPI_DSI_SUBSYS_DBITE_READ_MASK 0xFFFFFFFFUL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_DBI_FDIV_READ_MASK was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_DBI_FDIV_READ_MASK
// #define MIPI_DSI_SUBSYS_DBI_FDIV_READ_MASK 0xFFFFFFFFUL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_INTERRUPT_RAW_READ_MASK was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_INTERRUPT_RAW_READ_MASK
// #define MIPI_DSI_SUBSYS_INTERRUPT_RAW_READ_MASK 0xFFFFFFFFUL /* MANUAL_REVIEW: value not supplied */
#endif

/* ============================================================ */
/* Register Write Mask Macros (5 registers)                     */
/* Values are not supplied in the Meta TestPlan JSON.            */
/* Each macro is preserved as a MANUAL_REVIEW placeholder.      */
/* ============================================================ */

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_WRITE_MASK was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_WRITE_MASK
// #define MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_WRITE_MASK 0xFFFFFFFFUL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_LOW_PWR_WRITE_MASK was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_LOW_PWR_WRITE_MASK
// #define MIPI_DSI_SUBSYS_LOW_PWR_WRITE_MASK 0xFFFFFFFFUL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_DBITE_WRITE_MASK was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_DBITE_WRITE_MASK
// #define MIPI_DSI_SUBSYS_DBITE_WRITE_MASK 0xFFFFFFFFUL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_DBI_FDIV_WRITE_MASK was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_DBI_FDIV_WRITE_MASK
// #define MIPI_DSI_SUBSYS_DBI_FDIV_WRITE_MASK 0xFFFFFFFFUL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_INTERRUPT_RAW_WRITE_MASK was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_INTERRUPT_RAW_WRITE_MASK
// #define MIPI_DSI_SUBSYS_INTERRUPT_RAW_WRITE_MASK 0xFFFFFFFFUL /* MANUAL_REVIEW: value not supplied */
#endif

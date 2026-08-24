/*
 // Author - AI Force 1.3.2. Date 24-01-2025
 // (EMBENGG-SYSAPPS)
*/

/*
 * Test Case Name: pcie_device_enumerate_test
 *
 * Purpose: Definitions, macros, register addresses, and declarations for
 * PCIe device enumeration testcase covering PCIE0 and PCIE1 controllers.
 */

/* Headers: NA - no specific headers were provided in the input */
/* Include platform/project-specific headers as needed by the build environment */

/* Debug logging support */
#ifdef DEBUG_DISPLAY
#define DEBUG_PRINT(msg) debug_print(msg)
#else
#define DEBUG_PRINT(msg)
#endif

/* ============================================================ */
/* Macros: NA - no specific macros were provided in the input.   */
/* The following are derived from Impacted Registers and Test    */
/* Steps to support the testcase implementation.                 */
/* ============================================================ */

/* Control register address */
#define CTRL_REG_ADDR 0xE6004100

/* SII link status register offset */
#define SII_LINK_STATUS_OFFSET 0xC0

/* Link-up status mask */
#define LINK_UP_MASK 0xD1

/* Expected poll value for completion */
#define POLL_EXPECTED_VALUE 0x12345678

/* System-level register addresses */
#define SYS_REG_0C 0xE690000C
#define SYS_REG_10 0xE6900010
#define SYS_REG_14 0xE6900014
#define SYS_REG_18 0xE6900018
#define SYS_REG_30 0xE6900030
#define SYS_REG_34 0xE6900034

/* ============================================================ */
/* Arrays: NA - no arrays were provided in the input.            */
/* ============================================================ */

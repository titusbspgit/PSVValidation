/*
 // Author - AI Force 1.3.2. Date 25-06-2025
 // (EMBENGG-SYSAPPS)
*/

/*
 * Test Case Name : pcie_dma_write_test
 * Purpose        : Definitions, macros, and register addresses for PCIe
 *                  DMA write and read-back testcase across all four DMA
 *                  channels. Covers DMA doorbell, interrupt mask, status,
 *                  and clear registers for both PCIE0 and PCIE1.
 */

/* ---------------------------------------------------------------- */
/* Debug logging support                                            */
/* ---------------------------------------------------------------- */
#ifdef DEBUG_DISPLAY
#include <stdio.h>
#define debug_print(...) printf(__VA_ARGS__)
#else
#define debug_print(...) do {} while(0)
#endif

#ifndef LOGI
#define LOGI(...) printf(__VA_ARGS__)
#endif

/* ---------------------------------------------------------------- */
/* Control / Synchronization Register                               */
/* ---------------------------------------------------------------- */
#define CTRL_REG_ADDR                                       0xE6004100U
#define SYNC_HANDSHAKE_VALUE                                0x12345678U

/* ---------------------------------------------------------------- */
/* SII Link Status Registers                                       */
/* ---------------------------------------------------------------- */
#define SII_LINK_STATUS_OFFSET                              0xC0U
#define SII_LINK_UP_MASK                                    0xD1U

#ifndef SII0_LINK_STATUS_REG
extern volatile unsigned int SII0_LINK_STATUS_REG;
#endif

#ifndef SII1_LINK_STATUS_REG
extern volatile unsigned int SII1_LINK_STATUS_REG;
#endif

/* ---------------------------------------------------------------- */
/* PCIe Slave Port Register Offsets                                 */
/*   TYPE1_DEV_ID_VEND_ID_REG  -> offset 0x0                       */
/*   TYPE1_STATUS_COMMAND_REG  -> offset 0x4                        */
/* ---------------------------------------------------------------- */
#define PCIE_SLV_VENDOR_ID_OFFSET                           0x00U
#define PCIE_SLV_CMD_STATUS_OFFSET                          0x04U
#define PCIE_CMD_MEM_IO_BUSMASTER                           0x7U

/* ---------------------------------------------------------------- */
/* DMA Interrupt Channel Mask (lower 4 bits)                       */
/* ---------------------------------------------------------------- */
#define DMA_INT_CHANNEL_MASK                                0x0000000FU

/* ---------------------------------------------------------------- */
/* GIC IRQ Numbers                                                  */
/* ---------------------------------------------------------------- */
#define GIC_IRQ_PCIE0                                       0x20U
#define GIC_IRQ_PCIE1                                       0x23U

/* ---------------------------------------------------------------- */
/* DMA Transfer Parameters                                         */
/* ---------------------------------------------------------------- */
#define DMA_TRANSFER_LEN                                    0x40U
#define SRC_ADDR0                                           0xE6000000U
#define DST_ADDR0                                           0xE6001000U
#define DST_ADDR1                                           0xE6020000U
#define DST_ADDR2                                           0xE6020000U
#define DST_ADDR3                                           0xE6020000U

/* Source data patterns */
#define SRC_PATTERN_0                                       0xC0DEBEEDU
#define SRC_PATTERN_1                                       0xF00DDEAFU

/* ---------------------------------------------------------------- */
/* DM0_RC DMA Write/Read Addresses (0xA7xxxxxx region)             */
/* Note: Exact addresses not fully specified; placeholders based on */
/*       the described 0xA7xxxxxx region for DM0_RC.               */
/* ---------------------------------------------------------------- */
#define DM0_WR_ADDR0                                        0xA7000000U
#define DM0_WR_ADDR1                                        0xA7010000U
#define DM0_WR_ADDR2                                        0xA7020000U
#define DM0_WR_ADDR3                                        0xA7030000U
#define DM0_RD_ADDR0                                        0xA7000000U
#define DM0_RD_ADDR1                                        0xA7010000U
#define DM0_RD_ADDR2                                        0xA7020000U
#define DM0_RD_ADDR3                                        0xA7030000U

/* ---------------------------------------------------------------- */
/* DM1_RC DMA Write/Read Addresses (0xC7xxxxxx region)             */
/* Note: Exact addresses not fully specified; placeholders based on */
/*       the described 0xC7xxxxxx region for DM1_RC.               */
/* ---------------------------------------------------------------- */
#define DM1_WR_ADDR0                                        0xC7000000U
#define DM1_WR_ADDR1                                        0xC7010000U
#define DM1_WR_ADDR2                                        0xC7020000U
#define DM1_WR_ADDR3                                        0xC7030000U
#define DM1_RD_ADDR0                                        0xC7000000U
#define DM1_RD_ADDR1                                        0xC7010000U
#define DM1_RD_ADDR2                                        0xC7020000U
#define DM1_RD_ADDR3                                        0xC7030000U

/* ---------------------------------------------------------------- */
/* PCIE0 DMA Registers (DMA_WRITE)                                 */
/* ---------------------------------------------------------------- */
#ifndef mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF
extern volatile unsigned int mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF;
#endif

#ifndef mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF
extern volatile unsigned int mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF;
#endif

#ifndef mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF
extern volatile unsigned int mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF;
#endif

#ifndef mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF
extern volatile unsigned int mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF;
#endif

/* ---------------------------------------------------------------- */
/* PCIE0 DMA Registers (DMA_READ)                                  */
/* ---------------------------------------------------------------- */
#ifndef mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF
extern volatile unsigned int mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF;
#endif

#ifndef mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF
extern volatile unsigned int mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF;
#endif

#ifndef mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF
extern volatile unsigned int mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF;
#endif

#ifndef mizar_PCIE0_DBI_DSP_DMA_READ_INT_CLEAR_OFF
extern volatile unsigned int mizar_PCIE0_DBI_DSP_DMA_READ_INT_CLEAR_OFF;
#endif

/* ---------------------------------------------------------------- */
/* PCIE1 DMA Registers (DMA_WRITE)                                 */
/* ---------------------------------------------------------------- */
#ifndef mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF
extern volatile unsigned int mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF;
#endif

#ifndef mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF
extern volatile unsigned int mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF;
#endif

#ifndef mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF
extern volatile unsigned int mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF;
#endif

#ifndef mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF
extern volatile unsigned int mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF;
#endif

/* ---------------------------------------------------------------- */
/* PCIE1 DMA Registers (DMA_READ)                                  */
/* ---------------------------------------------------------------- */
#ifndef mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF
extern volatile unsigned int mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF;
#endif

#ifndef mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF
extern volatile unsigned int mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF;
#endif

#ifndef mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF
extern volatile unsigned int mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF;
#endif

#ifndef mizar_PCIE1_DBI_DSP_DMA_READ_INT_CLEAR_OFF
extern volatile unsigned int mizar_PCIE1_DBI_DSP_DMA_READ_INT_CLEAR_OFF;
#endif

/* ---------------------------------------------------------------- */
/* FV Framework Type Declarations                                   */
/* ---------------------------------------------------------------- */
#ifndef TESTS_ITEM_DEFINED
#define TESTS_ITEM_DEFINED
typedef struct {
    const char *test_name;
} TestsItem;
#endif

#ifndef TEST_OUTPUT_DEFINED
#define TEST_OUTPUT_DEFINED
typedef struct {
    int status;
} TestOutput;
#endif

/* ---------------------------------------------------------------- */
/* Extern Function Declarations                                     */
/* ---------------------------------------------------------------- */
extern void link_training_dm0_x4(int width);
extern void link_training_dm1_x4(int width);
extern void bar_program_dm0_x4(void);
extern void bar_program_dm1_x4(void);
extern void mem_base_program_dm0_x4(void);
extern void mem_base_program_dm1_x4(void);
extern void non_secure_prot_nic(void);
extern void wait_on(int count);
extern void finish(int status);
extern unsigned int read_pcie_slv0_reg(unsigned int offset);
extern void write_pcie_slv0_reg(unsigned int offset, unsigned int value);
extern unsigned int read_pcie_slv1_reg(unsigned int offset);
extern void write_pcie_slv1_reg(unsigned int offset, unsigned int value);

/* GIC functions */
extern void GIC_Set(void);
extern void GIC_EnableAllIRQ(void);
extern void GIC_ClearIRQ(unsigned int irq_num);

/* DMA channel programming functions - PCIE0 (DM0) */
extern void program_dma_wch0(unsigned int hi_src, unsigned int lo_src, unsigned int hi_dst, unsigned int lo_dst, unsigned int hi_len, unsigned int lo_len);
extern void program_dma_wch1(unsigned int hi_src, unsigned int lo_src, unsigned int hi_dst, unsigned int lo_dst, unsigned int hi_len, unsigned int lo_len);
extern void program_dma_wch2(unsigned int hi_src, unsigned int lo_src, unsigned int hi_dst, unsigned int lo_dst, unsigned int hi_len, unsigned int lo_len);
extern void program_dma_wch3(unsigned int hi_src, unsigned int lo_src, unsigned int hi_dst, unsigned int lo_dst, unsigned int hi_len, unsigned int lo_len);
extern void program_dma_rch0(unsigned int hi_src, unsigned int lo_src, unsigned int hi_dst, unsigned int lo_dst, unsigned int hi_len, unsigned int lo_len);
extern void program_dma_rch1(unsigned int hi_src, unsigned int lo_src, unsigned int hi_dst, unsigned int lo_dst, unsigned int hi_len, unsigned int lo_len);
extern void program_dma_rch2(unsigned int hi_src, unsigned int lo_src, unsigned int hi_dst, unsigned int lo_dst, unsigned int hi_len, unsigned int lo_len);
extern void program_dma_rch3(unsigned int hi_src, unsigned int lo_src, unsigned int hi_dst, unsigned int lo_dst, unsigned int hi_len, unsigned int lo_len);

/* DMA channel programming functions - PCIE1 (DM1) */
extern void program_dma1_wch0(unsigned int hi_src, unsigned int lo_src, unsigned int hi_dst, unsigned int lo_dst, unsigned int hi_len, unsigned int lo_len);
extern void program_dma1_wch1(unsigned int hi_src, unsigned int lo_src, unsigned int hi_dst, unsigned int lo_dst, unsigned int hi_len, unsigned int lo_len);
extern void program_dma1_wch2(unsigned int hi_src, unsigned int lo_src, unsigned int hi_dst, unsigned int lo_dst, unsigned int hi_len, unsigned int lo_len);
extern void program_dma1_wch3(unsigned int hi_src, unsigned int lo_src, unsigned int hi_dst, unsigned int lo_dst, unsigned int hi_len, unsigned int lo_len);
extern void program_dma1_rch0(unsigned int hi_src, unsigned int lo_src, unsigned int hi_dst, unsigned int lo_dst, unsigned int hi_len, unsigned int lo_len);
extern void program_dma1_rch1(unsigned int hi_src, unsigned int lo_src, unsigned int hi_dst, unsigned int lo_dst, unsigned int hi_len, unsigned int lo_len);
extern void program_dma1_rch2(unsigned int hi_src, unsigned int lo_src, unsigned int hi_dst, unsigned int lo_dst, unsigned int hi_len, unsigned int lo_len);
extern void program_dma1_rch3(unsigned int hi_src, unsigned int lo_src, unsigned int hi_dst, unsigned int lo_dst, unsigned int hi_len, unsigned int lo_len);

/* ---------------------------------------------------------------- */
/* Macros - No specific macros were supplied in the input.          */
/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
/* Arrays - No specific arrays were supplied in the input.          */
/* ---------------------------------------------------------------- */

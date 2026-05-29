#include <stdio.h>
#include "hal_uart.h"
#include "Uart_RX_Fifo_Enable.h"
#include "gic_funcs.h"
#include <uart/uart_def.h>
#include <uart/uart_offset.h>
#include <uart/uart_defines.h>
#include <uart/uart_functions.h>


unsigned int i, j;
static volatile unsigned int err1;
static volatile unsigned int rx_count;
static volatile uint8_t rx_buf[16];

int Uart_RX_Fifo_Enable_init(const TestsItem *cfg)
{
    (void)cfg;
    //printf("[Test Init] GPIO test: %s\n", cfg->test_name);
    LOGI("[Test Init]  UART test: %s\n", cfg->test_name);
    
     return 0;
}
        
static void uart_irq_route_enable(void)
{
#ifdef UART0
    write_reg(0xA000001C, 0x00004000U);
    GIC_EnableIRQ(66);
#endif
#ifdef UART1
    write_reg(0xA000001C, 0x00008000U);
    GIC_EnableIRQ(67);
#endif
#ifdef UART2
    write_reg(0xA000001C, 0x00010000U);
    GIC_EnableIRQ(68);
#endif
#ifdef UART3
    write_reg(0xA000001C, 0x00020000U);
    GIC_EnableIRQ(69);
#endif
#ifdef UART4
    write_reg(0xA000001C, 0x00040000U);
    GIC_EnableIRQ(70);
#endif
#ifdef UART5
    write_reg(0xA000001C, 0x00080000U);
    GIC_EnableIRQ(71);
#endif
#ifdef UART6
    write_reg(0xA000001C, 0x00100000U);
    GIC_EnableIRQ(72);
#endif
#ifdef UART7
    write_reg(0xA000001C, 0x00200000U);
    GIC_EnableIRQ(73);
#endif
#ifdef UART8
    write_reg(0xA000001C, 0x00400000U);
    GIC_EnableIRQ(74);
#endif
#ifdef UART9
    write_reg(0xA000001C, 0x00800000U);
    GIC_EnableIRQ(75);
#endif
}

static void uart_irq_route_clear(void)
{
#ifdef UART0
    write_reg(0xA0000018, 0x00004000U);
    GIC_ClearIRQ(66);
#endif
#ifdef UART1
    write_reg(0xA0000018, 0x00008000U);
    GIC_ClearIRQ(67);
#endif
#ifdef UART2
    write_reg(0xA0000018, 0x00010000U);
    GIC_ClearIRQ(68);
#endif
#ifdef UART3
    write_reg(0xA0000018, 0x00020000U);
    GIC_ClearIRQ(69);
#endif
#ifdef UART4
    write_reg(0xA0000018, 0x00040000U);
    GIC_ClearIRQ(70);
#endif
#ifdef UART5
    write_reg(0xA0000018, 0x00080000U);
    GIC_ClearIRQ(71);
#endif
#ifdef UART6
    write_reg(0xA0000018, 0x00100000U);
    GIC_ClearIRQ(72);
#endif
#ifdef UART7
    write_reg(0xA0000018, 0x00200000U);
    GIC_ClearIRQ(73);
#endif
#ifdef UART8
    write_reg(0xA0000018, 0x00400000U);
    GIC_ClearIRQ(74);
#endif
#ifdef UART9
    write_reg(0xA0000018, 0x00800000U);
    GIC_ClearIRQ(75);
#endif
}

int Uart_RX_Fifo_Enable_run(const TestsItem *cfg, TestOutput *out)
{
    static const uint8_t tx_bytes[] = {
        0xAAU, 0xBBU, 0xCCU, 0xDDU, 0xEEU,
        0x33U, 0xA1U, 0x43U, 0x45U, 0x66U
    };
    const unsigned int transfer_count =
        (unsigned int)(sizeof(tx_bytes) / sizeof(tx_bytes[0]));
    const unsigned int rx_wait_limit = 1000U;


    hal_uart_cfg_t uart_cfg;
    unsigned int i;

    if ((cfg == NULL) || (out == NULL)) {
        return -1;
    }

    /*
 *      Note:
 *      If LOGI()/printf() use the same UART instance under test,
 *      avoid extra prints after counters are reset, because they can
 *      affect TX state and interrupt counts.
 *                          */
    LOGI("[Test Run] UART test: %s", cfg->test_name);
    printf("Started the FIFO Enable test\n");

    err1 = 0U;
    rx_count = 0U;
 

    uart_cfg.input_clk_hz = 24000000U;
    uart_cfg.baudrate     = 115200U;
    uart_cfg.data_bits    = 8U;
    uart_cfg.stop_bits    = 1U;
    uart_cfg.parity       = HAL_UART_PARITY_NONE;
    uart_cfg.fifo_enable  = true;

    if (!hal_uart_init(&uart_cfg)) {
        LOGE("hal_uart_init failed");
        out->status = 1U;
        return out->status;
    }

      uart_irq_route_enable();



     /* Enable RX FIFO + reset FIFOs + set trigger */
    write_reg(MIZAR_UART_FIFO_CONTROL_REGISTER,0x1);

    /*  Enable RX interrupt (RX data + RX timeout) */
    write_reg(MIZAR_UART_INTERRUPT_ENABLE_REGISTER, UART_IER_RX_DATA);

    /*  Transmit all bytes (FIFO style  no waiting) */
    for (i = 0U; i < transfer_count; i++) {
        write_reg(MIZAR_UART_THR, (uint32_t)tx_bytes[i]);
    }

    /*  * Wait for RX completion  */
    {
        unsigned int timeout = rx_wait_limit;

        while ((rx_count < transfer_count) && timeout--) {
            wait_on(5U);
        }

        if (timeout == 0U) {
            printf("Timeout waiting for RX data (%u/%u)\n",
                   rx_count, transfer_count);
            err1 = 1U;
        }
    }

    /* Validate RX byte count    */
    if ((err1 == 0U) && (rx_count != transfer_count)) {
        printf("RX count mismatch: expected %u, got %u\n",
               transfer_count, rx_count);
        err1 = 1U;
    }

    /*  Data integrity check*/
    for (i = 0U; (err1 == 0U) && (i < transfer_count); i++) {
        if (rx_buf[i] != tx_bytes[i]) {
            printf("Data mismatch at %u: TX=0x%02X RX=0x%02X\n",
                   i, tx_bytes[i], rx_buf[i]);
            err1 = 1U;
            break;
        }
    }

    /* Disable interrupts */
    write_reg(MIZAR_UART_INTERRUPT_ENABLE_REGISTER, 0U);

    out->status = err1;
    return out->status;
}
                                   
void Default_IRQHandler(void)
{
    unsigned int rd_data;

    /* Extract interrupt bits */
    rd_data = read_reg(MIZAR_UART_INTERRUPT_STATUS_REGISTER) & 0x0FU;

    /*
 *      * RX FIFO ENABLE:
 *      * 0x04 = RX data available (trigger level)
 *      * 0x0C = RX timeout (FIFO has data but stalled)
 *      * BOTH mean RX FIFO contains data
 *      */
    if ((rd_data == 0x04U) || (rd_data == 0x0CU)) {

        /* Drain RX FIFO completely */
        while (read_reg(MIZAR_UART_LINE_STATUS_REGISTER) & 0x1U) {

            if (rx_count < sizeof(rx_buf)) {
                rx_buf[rx_count++] = (uint8_t)read_reg(MIZAR_UART_RHR);
            } else {
                err1 = 1U; /* RX overflow */
                break;
            }
        }
    } else {
        /* Unexpected interrupt source */
        err1 = 1U;
    }

    uart_irq_route_clear();
}

int Uart_RX_Fifo_Enable_teardown(const TestsItem *cfg)
{
    (void)cfg;
    //printf("[DONE] GPIO teardown: %s\n", cfg->test_name);
    LOGI("[TEARDOWN] UART teardown: %s\n", cfg->test_name);
    
    return 0;
}


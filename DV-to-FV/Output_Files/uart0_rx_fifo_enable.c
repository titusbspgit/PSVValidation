/*
    Program: UART0 RX FIFO Enable Test - FV Structured Version
    Agent: Ag-FV-DV-Transition Agent
*/
#include <stdio.h>
#include <stdlib.h>
#include <test_common.h>
#include <uart/uart_def.h>
#include <uart/uart_offset.h>
#include "../common/uart_defines.h"
#include "../common/uart_functions.h"

extern int int_pend;
unsigned int i;
unsigned int j;
unsigned int err1;

volatile unsigned int uart0_isr_mis_addr_last;
volatile unsigned int uart0_isr_rd_data_last;
volatile unsigned int uart0_isr_rx_data_last;
volatile unsigned int uart0_isr_iteration_index_last;
volatile unsigned int uart0_isr_event_counter;

/*
    Function: uart0_rx_fifo_enable_init
    Phase: Initialization
    Details: Moves setup and configuration from DV test_case to FV init phase.
*/
int uart0_rx_fifo_enable_init(void)
{
    int rcnt;
    int arcnt;
    int nbytes[10] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x33, 0xa1, 0x43, 0x45, 0x66};
    err1 = 0;
#ifdef RAND
    rand_inst = uart_if_rand();
#endif
#ifdef UART0
    write_reg(0xA000001C, 0x00004000);
    GIC_EnableIRQ(66);
#endif
#ifdef UART1
    write_reg(0xA000001C, 0x00008000);
    GIC_EnableIRQ(67);
#endif
#ifdef UART2
    write_reg(0xA000001C, 0x00010000);
    GIC_EnableIRQ(68);
#endif
#ifdef UART3
    write_reg(0xA000001C, 0x00020000);
    GIC_EnableIRQ(69);
#endif
#ifdef UART4
    write_reg(0xA000001C, 0x00040000);
    GIC_EnableIRQ(70);
#endif
#ifdef UART5
    write_reg(0xA000001C, 0x00080000);
    GIC_EnableIRQ(71);
#endif
#ifdef UART6
    write_reg(0xA000001C, 0x00100000);
    GIC_EnableIRQ(72);
#endif
#ifdef UART7
    write_reg(0xA000001C, 0x00200000);
    GIC_EnableIRQ(73);
#endif
#ifdef UART8
    write_reg(0xA000001C, 0x00400000);
    GIC_EnableIRQ(74);
#endif
#ifdef UART9
    write_reg(0xA000001C, 0x00800000);
    GIC_EnableIRQ(75);
#endif
    write_reg(MIZAR_UART_FIFO_CONTROL_REGISTER, 0x1);
    uart_config();
    write_reg(MIZAR_UART_INTERRUPT_ENABLE_REGISTER, UART_IER_RX_DATA);
    write_reg(0xA0243FFC, 0x963);
    uart0_isr_event_counter = 0;
    uart0_isr_mis_addr_last = 0;
    uart0_isr_rd_data_last = 0;
    uart0_isr_rx_data_last = 0;
    uart0_isr_iteration_index_last = 0;
    return 0;
}

/*
    Function: uart0_rx_fifo_enable_run
    Phase: Execution
    Details: Core test loop, waits for RX interrupts per transfer. No validation or prints here.
*/
int uart0_rx_fifo_enable_run(void)
{
    for (i = 0; i < NO_OF_TRANSFERS; i++)
    {
        int_pend = 1;
        while (int_pend)
        {
            wait_on(5);
        }
    }
    return 0;
}

/*
    Function: uart0_rx_fifo_enable_teardown
    Phase: Output / Teardown
    Details: All printing and validation consolidated here.
*/
int uart0_rx_fifo_enable_teardown(void)
{
    printf("Started the RX FIFO Enable test\n");
    if (uart0_isr_rd_data_last & 0x4 != 0x4)
    {
        printf("RX-%d: RX line is not ready: %d\n", i, uart0_isr_rd_data_last);
        err1 = 1;
    }
    else
    {
#ifdef DEBUG_DISPLAY
        printf("Scratchpad register transfer%d = %x\n", i, read_reg(MIZAR_UART_SRACTH_PAD_REGISTER));
#endif
#ifdef DEBUG_DISPLAY
        printf("RX: Recieved byte%d = %x\n", i, uart0_isr_rx_data_last);
        printf("Scratchpad register afetr transfer%d read = %x\n", i, read_reg(MIZAR_UART_SRACTH_PAD_REGISTER));
#endif
    }
    err1 = uart_vip_scbd_status();
    finish(err1);
    return err1;
}

/*
    Function: Default_IRQHandler
    Phase: Runtime ISR (no validation or prints; they are deferred to teardown)
*/
void Default_IRQHandler(void)
{
    unsigned int rd_data;
    unsigned int mis_addr;
    unsigned int rx_data;
    int_pend = 0;
    mis_addr = MIZAR_UART_INTERRUPT_STATUS_REGISTER;
    rd_data = read_reg(mis_addr);
    uart0_isr_mis_addr_last = mis_addr;
    uart0_isr_rd_data_last = rd_data;
    uart0_isr_iteration_index_last = i;
    uart0_isr_event_counter = uart0_isr_event_counter + 1;
    if (rd_data & 0x4 != 0x4)
    {
        /* Validation moved to teardown */
    }
    else
    {
#ifdef DEBUG_DISPLAY
        /* Prints moved to teardown */
#endif
        rx_data = read_reg(MIZAR_UART_RHR);
        uart0_isr_rx_data_last = rx_data;
#ifdef DEBUG_DISPLAY
        /* Prints moved to teardown */
#endif
#ifdef RAND
        rand_gic_disable(rand_inst);
#endif
#ifdef UART0
        write_reg(0xA0000018, 0x00004000);
        GIC_ClearIRQ(66);
#endif
#ifdef UART1
        write_reg(0xA0000018, 0x00008000);
        GIC_ClearIRQ(67);
#endif
#ifdef UART2
        write_reg(0xA0000018, 0x00010000);
        GIC_ClearIRQ(68);
#endif
#ifdef UART3
        write_reg(0xA0000018, 0x00020000);
        GIC_ClearIRQ(69);
#endif
#ifdef UART4
        write_reg(0xA0000018, 0x00040000);
        GIC_ClearIRQ(70);
#endif
#ifdef UART5
        write_reg(0xA0000018, 0x00080000);
        GIC_ClearIRQ(71);
#endif
#ifdef UART6
        write_reg(0xA0000018, 0x00100000);
        GIC_ClearIRQ(72);
#endif
#ifdef UART7
        write_reg(0xA0000018, 0x00200000);
        GIC_ClearIRQ(73);
#endif
#ifdef UART8
        write_reg(0xA0000018, 0x00400000);
        GIC_ClearIRQ(74);
#endif
#ifdef UART9
        write_reg(0xA0000018, 0x00800000);
        GIC_ClearIRQ(75);
#endif
    }
}

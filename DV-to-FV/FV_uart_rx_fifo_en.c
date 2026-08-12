/*
 // Author - AI Force 1.3.2. Date 29-05-2026
 // (EMBENGG-SYSAPPS)
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

/********************************************************************
 * Function Name  : uart_rx_fifo_en_init
 * Description    : Initialize UART RX FIFO enable test configuration and interrupts
 * Parameters     : const TestsItem *cfg
 * Return Value   : int (0 on success)
 ********************************************************************/
int uart_rx_fifo_en_init(const TestsItem *cfg)
{
    (void)cfg;
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
    return 0;
}

/********************************************************************
 * Function Name  : uart_rx_fifo_en_run
 * Description    : Execute UART RX FIFO enable test core logic and wait for interrupts
 * Parameters     : const TestsItem *cfg, TestOutput *out
 * Return Value   : int (test status stored in out->status)
 ********************************************************************/
int uart_rx_fifo_en_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    write_reg(0xA0243FFC, 0x963);
    for (i = 0; i < NO_OF_TRANSFERS; i++)
    {
        int_pend = 1;
        while (int_pend)
        {
            wait_on(5);
        }
    }
/*
    printf("SCR: No. of received bytes count in scratch pad were %d\n",rcnt);
    for(j=0;j<NO_OF_TRANSFERS;j++){
        rx_data = read_reg(MIZAR_UART_RHR);
        printf("RX: Recieved byte%d = %x\n",i,rx_data);
    }
    arcnt = read_reg(MIZAR_UART_SRACTH_PAD_REGISTER);
    printf("SCR: No. of received bytes in fifo after read of RHR %d\n",arcnt);
    if(rcnt != i){
    printf("No. of transfres %d is not equal to received %d",i,rcnt);
    err1 = 1;
    }*/
    err1 = uart_vip_scbd_status();
    return out->status = err1;
}

/********************************************************************
 * Function Name  : Default_IRQHandler
 * Description    : UART default interrupt handler for RX FIFO enable test
 * Parameters     : None
 * Return Value   : None
 ********************************************************************/
void Default_IRQHandler()
{
    unsigned int rd_data;
    unsigned int mis_addr;
    unsigned int rx_data;
    int_pend = 0;
    mis_addr = MIZAR_UART_INTERRUPT_STATUS_REGISTER;
    rd_data = read_reg(mis_addr);
    if (rd_data & 0x4 != 0x4)
    {
        printf("RX-%d: RX line is not ready: %d\n", i, rd_data);
        err1 = 1;
    }
    else
    {
#ifdef DEBUG_DISPLAY
        printf("Scratchpad register transfer%d = %x\n", i, read_reg(MIZAR_UART_SRACTH_PAD_REGISTER));
#endif
        rx_data = read_reg(MIZAR_UART_RHR);
#ifdef DEBUG_DISPLAY
        printf("RX: Recieved byte%d = %x\n", i, rx_data);
        printf("Scratchpad register afetr transfer%d read = %x\n", i, read_reg(MIZAR_UART_SRACTH_PAD_REGISTER));
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

/********************************************************************
 * Function Name  : uart_rx_fifo_en_teardown
 * Description    : Output and teardown for UART RX FIFO enable test
 * Parameters     : const TestsItem *cfg
 * Return Value   : int (0 on success)
 ********************************************************************/
int uart_rx_fifo_en_teardown(const TestsItem *cfg)
{
    (void)cfg;
    printf("Started the RX FIFO Enable test\n");
    return 0;
}

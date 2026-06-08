/*****************************************************************************
 *
 *  Program     : gpio_pedge.c
 *  Description : FV Structured Test - GPIO Positive Edge Interrupt Test
 *                Converted from DV-style test_case() into FV-style
 *                gpio_pedge_init / gpio_pedge_run / gpio_pedge_teardown
 *
 *****************************************************************************/

#include <lss_sysreg.h>
#include <stdio.h>
#include <test_define.c>
#include <test_common.h>
#include "gpio_pedge.h"
#include "gic_funcs.h"

/*****************************************************************************
 *  Global / Static Variables
 *****************************************************************************/
unsigned int gpio_number;
unsigned int i;
static volatile unsigned int test_err;
extern volatile int int_pend;

/*****************************************************************************
 *
 *  Function    : gpio_pedge_init
 *  Description : Initialization phase - log test start
 *
 *****************************************************************************/
int gpio_pedge_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] GPIO Positive Edge test: %s\n", cfg->test_name);

    return 0;
}

/*****************************************************************************
 *
 *  Function    : gpio_irq_route_enable (static helper)
 *  Description : Enable sysreg interrupt routing and GIC IRQ for GPIO
 *
 *****************************************************************************/
static void gpio_irq_route_enable(void)
{
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
    GIC_EnableIRQ(87);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
    GIC_EnableIRQ(88);
#endif
}

/*****************************************************************************
 *
 *  Function    : gpio_irq_route_clear (static helper)
 *  Description : Clear sysreg interrupt status and GIC IRQ for GPIO
 *
 *****************************************************************************/
static void gpio_irq_route_clear(void)
{
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    GIC_ClearIRQ(88);
#endif
}

/*****************************************************************************
 *
 *  Function    : gpio_pedge_run
 *  Description : Execution phase - configure GPIO positive edge interrupts,
 *                drive edges, and wait for ISR acknowledgement per pin
 *
 *****************************************************************************/
int gpio_pedge_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int rdata;
    unsigned int wr_val;

    if ((cfg == NULL) || (out == NULL))
    {
        return -1;
    }

    LOGI("[Test Run] GPIO Positive Edge test: %s\n", cfg->test_name);

    test_err = 0U;

    /* Enable GIC and sysreg interrupt routing */
    gpio_irq_route_enable();

    /* Enable posedge interrupt (bit17=1) for each pin */
    for (i = 0U; i < 32U; i++)
    {
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4U), 0x00020000U);
    }

    wait_on(10);

    /* Put GPIOs 8-39 in input mode (doe=1) */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFU);

    wait_on(10);

    /* Enable group interrupt */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFU);

    /* Drive positive edges and wait for ISR per iteration */
    for (i = 0U; i < 32U; i++)
    {
        /* Prepare known level low */
        write_reg(0xA0243ffc, 0x00000000U);
        wait_on(10);

        /* Arm before the edge */
        int_pend = 1;

        /* Rising edge */
        write_reg(0xA0243ffc, 0xFFFFFFFFU);

        /* Wait with timeout to avoid infinite hangs */
        {
            int timeout = 2000;

            while ((int_pend == 1) && (--timeout > 0))
            {
                wait_on(10);
            }

            if (timeout == 0)
            {
                printf("ERROR: Timeout waiting for GPIO IRQ at i=%u\n", i);
                test_err++;
                break;
            }
        }

        /* Drive low again to prep for next iteration */
        write_reg(0xA0243ffc, 0x00000000U);
        wait_on(10);
    }

    out->status = test_err;
    return out->status;
}

/*****************************************************************************
 *
 *  Function    : Default_IRQHandler
 *  Description : ISR - handles GPIO positive edge group interrupt,
 *                validates status, clears per-pin and group interrupts
 *
 *****************************************************************************/
void Default_IRQHandler(void)
{
    unsigned int j;
    unsigned int rdata;
    unsigned int rdata_grp;
    unsigned int wr_val;

    wr_val = 1U << i;
    int_pend = 0;

#ifdef DEBUG_DISPLAY
    printf("\nEntered into default IRQ Handler!! with pad value = %d\n", i);
#endif

    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);

    /* Mask group during service */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000U);

    if ((rdata_grp & (0xFFFFFFFFU)) != 0U)
    {
#ifdef DEBUG_DISPLAY
        printf("SUCCESS: GPIO_NUM = %0d  status = %0x Default_IRQHandler:: group Interrupt raised\n", i, rdata_grp);
#endif
    }
    else
    {
        printf("ERROR: Group Interrupt not occured\n");
        test_err = test_err + 1U;
    }

    /* Clear per-pin raw status: write 1 to iclr (bit16) */
    for (j = 0U; j < 32U; j++)
    {
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j * 4U), 0x00010000U);
    }

    wait_on(2);

    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);

    if (rdata_grp == 0x0U)
    {
#ifdef DEBUG_DISPLAY
        printf("SUCCESS : Group Interrupt cleared successfully\n");
#endif
    }
    else
    {
        printf("ERROR : Group Interrupt clear failed: Interrupt value:%x\n", rdata_grp);
        test_err = test_err + 1U;
    }

#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0U)
    {
        printf("sysreg status not cleared : %0x\n", MIZAR_LSS_SYSREG_RAW_STCR1);
        test_err++;
    }
#endif

#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0U)
    {
        printf("sysreg status not cleared : %0x\n", MIZAR_LSS_SYSREG_RAW_STCR1);
        test_err++;
    }
#endif

    /* Re-enable group interrupt output for next iteration */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFU);

#ifdef GPIO0
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    GIC_ClearIRQ(88);
#endif
}

/*****************************************************************************
 *
 *  Function    : gpio_pedge_teardown
 *  Description : Teardown phase - final validation reporting and cleanup
 *
 *****************************************************************************/
int gpio_pedge_teardown(const TestsItem *cfg)
{
    (void)cfg;

    if (test_err != 0U)
    {
        LOGI("[TEARDOWN] GPIO Positive Edge test FAILED with %u errors: %s\n", test_err, cfg->test_name);
    }
    else
    {
        LOGI("[TEARDOWN] GPIO Positive Edge test PASSED: %s\n", cfg->test_name);
    }

    return 0;
}

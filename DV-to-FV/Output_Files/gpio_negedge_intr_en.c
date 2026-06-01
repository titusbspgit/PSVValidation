/*
 * Program: GPIO Negative Edge Interrupt Enable - FV Structured
 * Agent: Ag-FV-DV-Transition Agent
 */
#include <stdio.h>
#include "hal_gpio.h"
#include "gic_funcs.h"
#include <lss_sysreg.h>
#include <gpio_negedge_intr_en.h>

static volatile unsigned int test_err;
static volatile unsigned int cap_rdata[32];
static volatile unsigned int cap_group_sts_before[32];
static volatile unsigned int cap_group_sts_after[32];
static volatile unsigned int cap_clear_rdata[32];
static volatile unsigned int cap_irq_seen[32];

unsigned int gpio_number;
unsigned int rdata;
unsigned int wr_val;
unsigned int i;
unsigned int addr1;
volatile int int_pend;

/*
 * Function: gpio_negedge_intr_en_init
 * Phase: Initialization
 */
int gpio_negedge_intr_en_init(const TestsItem *cfg)
{
    (void)cfg;
    test_err = 0U;
#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
#endif
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif
    write_reg(0xA0243FFCU, 0xFFFFFFFFU);
    return 0;
}

/*
 * Function: gpio_negedge_intr_en_run
 * Phase: Execution
 */
int gpio_negedge_intr_en_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    (void)out;
    for (i = 0U; i < 32U; i++)
    {
        addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4U);
        write_reg(addr1, 0x00140000U);
        wait_on(50U);
        wr_val = (1U << i);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10U);
        write_reg(0xA0243FFCU, 0xFFFFFFFFU);
        wait_on(30U);
        write_reg(0xA0243FFCU, ~(wr_val));
        int_pend = 1;
        while (int_pend)
        {
            wait_on(10U);
        }
    }
    return 0;
}

/*
 * ISR: Default_IRQHandler
 * Responsibility: Minimal handling, capture state, clear interrupt
 */
void Default_IRQHandler(void)
{
    unsigned int rdata_grp;
    unsigned int raddr;
    unsigned int raddr2;
    int_pend = 0;
    write_reg(0xA0243FFCU, 0xFFFFFFFFU);
    raddr = MIZAR_GPIO_GP0_GPIO_8 + (i * 4U);
    rdata = read_reg(raddr);
    cap_rdata[i] = rdata;
    if ((rdata & 0x2U) != 0x0U)
    {
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        cap_group_sts_before[i] = rdata_grp;
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1, wr_val);
        raddr2 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4U);
        write_reg(raddr2, 0x00110001U);
        cap_clear_rdata[i] = read_reg(raddr2);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000U);
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        cap_group_sts_after[i] = rdata_grp;
#ifdef GPIO0
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        GIC_ClearIRQ(88);
#endif
        cap_irq_seen[i] = 1U;
    }
    else
    {
        cap_irq_seen[i] = 0U;
    }
}

/*
 * Function: gpio_negedge_intr_en_teardown
 * Phase: Output / Teardown (all prints and validation here)
 */
int gpio_negedge_intr_en_teardown(const TestsItem *cfg)
{
    (void)cfg;
    printf("[TEARDOWN] GPIO teardown: %s\n", cfg->test_name);
    for (i = 0U; i < 32U; i++)
    {
        unsigned int observed_rdata;
        unsigned int observed_grp_before;
        unsigned int observed_grp_after;
        unsigned int observed_clear_rdata;
        unsigned int expected_mask;
        observed_rdata = cap_rdata[i];
        observed_grp_before = cap_group_sts_before[i];
        observed_grp_after = cap_group_sts_after[i];
        observed_clear_rdata = cap_clear_rdata[i];
        expected_mask = (1U << i);
#ifdef DEBUG_DISPLAY
        printf("Entered into default IRQ Handler!! with pad value = %u\n", i);
#endif
        if ((observed_rdata & 0x1U) != 0x0U)
        {
#ifdef DEBUG_DISPLAY
            printf("SUCCESS: GPIO_NUM = %0u Default_IRQHandler:: DIN value matches with the Pad_value ..read data = %0x\n", i, observed_rdata);
#endif
        }
        else
        {
            printf("ERROR: GPIO_NUM = %0u Default_IRQHandler:: DIN value does not match with the Pad_value read_data = %0x\n", i, observed_rdata);
            test_err = test_err + 1U;
        }
        if ((observed_rdata & 0x2U) != 0x0U)
        {
#ifdef DEBUG_DISPLAY
            printf("SUCCESS: GPIO_NUM = %0u  status = %0x Default_IRQHandler:: Raw Interrupt raised at negedge\n", i, observed_rdata);
#endif
            if ((observed_grp_before & expected_mask) != 0U)
            {
#ifdef DEBUG_DISPLAY
                printf("SUCCESS: GPIO_NUM = %0u  status = %0x Default_IRQHandler:: group Interrupt raised\n", i, observed_grp_before);
#endif
            }
            else
            {
                printf("ERROR: Group Interrupt not occured\n");
                test_err = test_err + 1U;
            }
            if (observed_clear_rdata == 0x00100001U)
            {
#ifdef DEBUG_DISPLAY
                printf("SUCCESS : Interrupt cleared successfully  rdata = %0x\n", observed_clear_rdata);
#endif
            }
            else
            {
                printf("ERROR : Interrupt clear failed : Interrupt value = %x\n", observed_clear_rdata);
                test_err = test_err + 1U;
            }
            if (observed_grp_after == 0x00000000U)
            {
#ifdef DEBUG_DISPLAY
                printf("SUCCESS : Group Interrupt cleared successfully\n");
#endif
            }
            else
            {
                printf("ERROR : Group Interrupt clear failed: Interrupt value:%x\n", observed_grp_after);
                test_err = test_err + 1U;
            }
        }
        else
        {
            printf("Interrupt Not occured\n");
            test_err = test_err + 1U;
        }
    }
    return 0;
}

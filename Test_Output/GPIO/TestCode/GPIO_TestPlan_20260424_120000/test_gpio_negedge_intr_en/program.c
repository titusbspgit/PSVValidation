// Author - AI Force 1.3.2. Date 24-04-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
High-level description (AS-IS from Hidden_Test_Description):
program.c sets up interrupts and negedge behavior. test_case(): ifdef GPIO0 GIC_EnableIRQ(87); ifdef GPIO1 GIC_EnableIRQ(88); write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIOx_INTR). write_reg(0xA0243ffc, 0xffffffff). For i=0..31: addr1=MIZAR_GPIO_GP0_GPIO_8+(i*4); write_reg(addr1, (1<<20)|(1<<18)|(1<<16)); wait_on(10). Then for i=0..31: wr_val=1<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); wait_on(10); int_pend=1; write_reg(0xA0243ffc,0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val); timeout=5000; while(int_pend && timeout--) wait_on(10); if(timeout==0) {printf timeout; test_err++;}. finish(test_err). Default_IRQHandler(): local_wr=1<<i; int_pend=0; write_reg(0xA0243ffc,0xffffffff); raddr=MIZAR_GPIO_GP0_GPIO_8+(i*4); rdata=read_reg(raddr); if ((rdata & 0x1)!=0) test_err++; if ((rdata & 0x2)!=0x0) { rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr)==0) test_err++; raddr2=MIZAR_GPIO_GP0_GPIO_8+(i*4); write_reg(raddr2,(1<<20)|(1<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp!=0x0) test_err++; #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87); #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88); #endif } else { test_err++; }
*/

/* Globals to coordinate between main flow and ISR */
static volatile unsigned int int_pend = 0;
static volatile unsigned int wr_val_global = 0;
static volatile unsigned int test_err = 0;

/*
Function: Default_IRQHandler
Purpose : Handle GPIO interrupt: validate per-pin status and clear as per procedure.
Note    : Only GPIO impacted registers are used per authoring rules. Upstream LSS_SYSREG accesses are intentionally omitted.
*/
void Default_IRQHandler(void)
{
    unsigned int local_wr = wr_val_global; // 1<<i of the currently enabled pin
    int_pend = 0; // indicate ISR observed
#ifdef DEBUG_DISPLAY
    printf("[ISR] Enter local_wr=0x%08x\n", local_wr);
#endif
    // Read back per-pin register matching the bit set in local_wr
    for (unsigned int i = 0; i < 32; i++) {
        if ((local_wr & (1u<<i)) == 0) continue;
        unsigned long raddr = addr_array[i];
        unsigned int rdata = read_reg(raddr);
#ifdef DEBUG_DISPLAY
        printf("[ISR] idx=%u raddr=0x%08lx rdata=0x%08x\n", i, raddr, rdata);
#endif
        if ((rdata & 0x1u) != 0u) { // DATA_IN should be 0 per negative edge assumption
            test_err++;
        }
        if ((rdata & 0x2u) != 0x0u) { // INTR_RAW_STS should be set before clear
            unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
            if ((rdata_grp & local_wr) == 0u) {
                test_err++;
            }
            // Re-arm and clear: IO_CTRL | INTR_CLR
            write_reg(raddr, (1u<<20)|(1u<<16));
            write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);
            rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
            if (rdata_grp != 0x0u) {
                test_err++;
            }
#ifdef GPIO0
            GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
            GIC_ClearIRQ(88);
#endif
        } else {
            test_err++;
        }
        break;
    }
}

/*
Function: test_case
Purpose : Configure negedge interrupt per pin, enable group interrupt, wait for ISR, and finalize pass/fail.
Note    : External stimulus register 0xA0243ffc and LSS_SYSREG programming are omitted per impacted-register rule.
*/
int test_case(void)
{
#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
#endif

    // Configure per-pin: IO_CTRL | NEDGE_INTR_EN | INTR_CLR
    for (unsigned int i = 0; i < 32; i++) {
        unsigned long addr1 = addr_array[i];
        write_reg(addr1, (1u<<20)|(1u<<18)|(1u<<16));
        wait_on(10);
    }

    // For each pin: enable raw clear + intr enable, set int_pend and wait (stimulus external, may timeout)
    for (unsigned int i = 0; i < 32; i++) {
        unsigned int wr_val = (1u<<i);
        wr_val_global = wr_val;
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);
        int_pend = 1;
        unsigned int timeout = 5000;
        while (int_pend && timeout--) {
            wait_on(10);
        }
        if (timeout == 0u) {
#ifdef DEBUG_DISPLAY
            printf("[TIMEOUT] i=%u wr_val=0x%08x\n", i, wr_val);
#endif
            test_err++;
        }
    }

    if (test_err == 0u) {
        finish(0); // PASS
    } else {
        finish(1); // FAIL
    }
    return 0;
}

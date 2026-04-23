// Author - AI Force 1.3.2. Date 23-04-2026
// (EMBENGG-SYSAPPS)

#include<test_define.c>
#include<test_common.h>
#include<stdio.h>

/*
Purpose: test_gpio_pedge_all_pads_en/
program.c enables GIC (87/88) and routes the interrupt via MIZAR_LSS_SYSREG_INTR_EN1. It writes MIZAR_GPIO_GP0_GPIO_8+(i*4) = 0x00020000 (peie) for i=0..31, sets MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4 = 0x000000FF (input), enables MIZAR_GPIO_GP0_INTR1_INTR_EN1 = 0xFFFFFFFF, then loops i=0..31: write 0xA0243ffc=0x0, wait, int_pend=1, write 0xA0243ffc=0xFFFFFFFF; poll int_pend with timeout; drive low again. Default_IRQHandler(): wr_val=(1<<i); int_pend=0; rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); if ((rdata_grp & 0xffffffff) != 0) success else error; clear per-pin raw by writing 0x00010000 to each MIZAR_GPIO_GP0_GPIO_8+(j*4); verify group status cleared; clear MIZAR_LSS_SYSREG_RAW_STCR1 (GPIO0/1) and confirm readback bit clears; re-enable MIZAR_GPIO_GP0_INTR1_INTR_EN1; clear GIC.
*/

static inline unsigned int read_reg(volatile unsigned int addr){return *((volatile unsigned int*)addr);} 
static inline void write_reg(volatile unsigned int addr, unsigned int val){*((volatile unsigned int*)addr)=val;}

static volatile int int_pend = 0; // set to 0 in ISR upon interrupt
static int test_err = 0;

// Minimal wait function stub; replace with platform delay as needed
static void wait_on(unsigned int loops){ volatile unsigned int x=loops; while(x--){ __asm__ __volatile__("nop"); } }

// ISR prototype (platform integrates actual vector to call this)
void Default_IRQHandler(void){
    // ISR logic per metadata
    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u); // disable group during service
    if ((rdata_grp & 0xFFFFFFFFu) != 0u){
        // success path
    } else {
#ifdef DEBUG_DISPLAY
        printf("[ISR] Group STS empty\n");
#endif
        test_err++;
    }

    // Clear per-pin raw by writing 0x00010000 to each GPIO pin register (bits: INTR_CLR)
    for (int j=0;j<32;j++){
        volatile unsigned int reg = (unsigned int)(MIZAR_GPIO_GP0_GPIO_8 + (j*4));
        write_reg(reg, 0x00010000u);
        wait_on(2);
    }

    // Verify group status cleared
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x0u){
#ifdef DEBUG_DISPLAY
        printf("[ISR] Group STS not cleared: 0x%08X\n", rdata_grp);
#endif
        test_err++;
    }

    // Clear system controller raw status for GPIO0/1 as applicable
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    unsigned int rdata_sys = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rdata_sys & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0u){
#ifdef DEBUG_DISPLAY
        printf("[ISR] RAW_STCR1 GPIO0 bit not cleared\n");
#endif
        test_err++;
    }

    // Re-enable and mark done
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
    int_pend = 0;
}

void test_case(void){
#ifdef DEBUG_DISPLAY
    printf("[test_gpio_pedge_all_pads_en] Start\n");
#endif

    // Route interrupt via LSS_SYSREG and enable NVIC/GIC (symbolic per metadata)
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);

    // Program per-pin PEDGE enable and input mode
    for (int i=0;i<32;i++){
        volatile unsigned int preg = (unsigned int)(MIZAR_GPIO_GP0_GPIO_8 + (i*4));
        write_reg(preg, 0x00020000u); // PEDGE_INTR_EN bit
    }
    wait_on(10);

    // Set groups as input
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10);

    // Enable group interrupt
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    // Stimulate each pad with rising edge via external stim register
    for (int i=0;i<32;i++){
        write_reg(GPIO_PAD_STIM_REG, 0x00000000u);
        wait_on(10);
        int_pend = 1;
        write_reg(GPIO_PAD_STIM_REG, 0xFFFFFFFFu);
        int timeout = 2000;
        while ((int_pend==1) && (--timeout>0)){
            wait_on(10);
        }
        if (timeout==0){
#ifdef DEBUG_DISPLAY
            printf("Timeout waiting for ISR for pad %d\n", i);
#endif
            test_err++;
            break;
        }
        write_reg(GPIO_PAD_STIM_REG, 0x00000000u);
        wait_on(10);
    }

    if (test_err==0){
#ifdef DEBUG_DISPLAY
        printf("[test_gpio_pedge_all_pads_en] PASS\n");
#endif
        finish(0);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[test_gpio_pedge_all_pads_en] FAIL err=%d\n", test_err);
#endif
        finish(1);
    }
}

#include <stdio.h>
#include <lss_sysreg.h>
#include "test_define.c"
#include <test_common.h>

unsigned int gpio_number, test_err, rdata, wr_val, i, addr1;
extern int int_pend;

int test_case()
{
    test_err = 0;

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

    write_reg(0xA0243ffc, 0xffffffff);

    for (i = 0; i < 32; i++) {
        addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4);
        write_reg(addr1, (1u << 20) | (1u << 18) | (1u << 16));
        wait_on(10);
    }

    for (i = 0; i < 32; i++) {
        wr_val = 1u << i;

        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);

    
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);

        int_pend = 1;

        write_reg(0xA0243ffc, 0xffffffff);
        wait_on(30);
        write_reg(0xA0243ffc, ~wr_val);

        unsigned int timeout = 5000;
        while (int_pend && timeout--) {
            wait_on(10);
        }
        if (timeout == 0) {
            printf("ERROR: Timeout waiting for GPIO%u negedge interrupt\n", (unsigned)(i + 8));
            test_err++;
            
        }
    }

    finish(test_err);
}

void Default_IRQHandler()
{
    unsigned int rdata_grp, raddr, raddr2;
    
    unsigned int local_wr = 1u << i;

    int_pend = 0;

    
    write_reg(0xA0243ffc, 0xffffffff);

    raddr = MIZAR_GPIO_GP0_GPIO_8 + (i * 4);
    rdata = read_reg(raddr);

#ifdef DEBUG_DISPLAY
    // printf("Entered into default IRQ Handler!! with pad value = %d", i);
#endif

    
    if ((rdata & 0x1) != 0) {
        test_err++;
    }

    if ((rdata & 0x2) != 0x0) {
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); 

        
        if ((rdata_grp & local_wr) == 0) {
            test_err++;
        }

        
        raddr2 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4);
        write_reg(raddr2, (1u << 20) | (1u << 16));

        
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);

        
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0) {
            test_err++;
        }

#ifdef GPIO0
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        GIC_ClearIRQ(87);
#endif

#ifdef GPIO1
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        GIC_ClearIRQ(88);
#endif

    } else {
        
        test_err++;
    }
}
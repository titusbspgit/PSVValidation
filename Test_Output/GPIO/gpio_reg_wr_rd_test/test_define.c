#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include<gpio/gpio_def.h>
#include<gpio/gpio_offset.h>

/* Macros collected from testcase context (unchanged) */
#define SOFT_RST_REG_ADDRESS 0x00000000
#define SOFT_RST_REG_DATA    0x00000000
#define CNT                  49
#define DEBUG_RW_MSG

/*
   NOTE:
   - Register maps, address tables, masks, and skip lists are intentionally NOT defined here.
   - These are expected to be provided by the platform/integration layer.
   - This file only declares the external arrays used by program.c.
*/
extern unsigned int addr_array[CNT];
extern unsigned int default_value_array[CNT];
extern unsigned int read_mask_array[CNT];
extern unsigned int write_mask_array[CNT];
extern unsigned char skip_array[CNT];
extern unsigned char skip_rst_array[CNT];

/* External platform helpers provided via test_common.h */
/*
   unsigned int read_reg(unsigned int addr);
   void write_reg(unsigned int addr, unsigned int data);
   void wait_on(unsigned int cycles);
   void finish(int status);
   DEBUG_DISPLAY(...)  // logging macro
*/

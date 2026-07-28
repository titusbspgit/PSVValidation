#include <stdio.h>
#include <stdlib.h>
#include<usb/usb_def.h>
#include<usb/usb_offsets.h>

int test_case () {
	printf("[C-Programme] Hello world\n");

        int rd_data = 0;
	rd_data= read_reg(0xA0000000);
	printf("THE READ DATA is %X\n",rd_data);
	rd_data= read_reg(0xa001706c);
	printf("THE READ DATA1 is %X\n",rd_data);
        write_reg(0xA0240000,0xdeadbeef); 
        rd_data = read_reg(0xA0240000); 
	printf("THE READ DATA2 is %X\n",rd_data);

        finish(0);
}

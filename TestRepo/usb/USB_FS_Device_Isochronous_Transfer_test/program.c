#include <stdio.h>
#include <stdlib.h>
#include "usb.h"

extern int int_pend;
int event_counter;

int test_case() {
	int rd_data,wr_data,port_count,db_offset;
	int buf_data[16],i,bulk,intr,event_comletion,j;
	int hand_shake;

	nic_programming();
	GIC_EnableAllIRQ();
	for(j=0;j< 20;j++){
	write_reg(Buffer_PointerLO + j*DWORD ,0x0);
	write_reg(event_trb_addr + j*DWORD ,0x0);

	}
	wr_data = 0x40f00000; //soft reset
	write_reg(MIZAR_USB_DCTL,wr_data);
	rd_data = read_reg(MIZAR_USB_DCTL);
	while(rd_data != 0xf00000) {
		wait_on(100);
		rd_data = read_reg(MIZAR_USB_DCTL);
	}
        printf("Soft Rst is done\n");

	wr_data = 0x40002407;
	write_reg(MIZAR_USB_GUSB2PHYCFG,wr_data);

	write_reg(MIZAR_USB_GEVNTADRLO,Default_Event_Ring_Array);
	write_reg(MIZAR_USB_GEVNTADRHI,0x0);
	write_reg(MIZAR_USB_GEVNTSIZ,0x30);
	write_reg(MIZAR_USB_GEVNTCOUNT,0x0);
	rd_data = read_reg(MIZAR_USB_GCTL);

	//wr_data = 0x30c12234; //port direction
	wr_data = 0x30c12214; //port direction
	write_reg(MIZAR_USB_GCTL,wr_data);
	rd_data = read_reg(MIZAR_USB_DCFG);
	write_reg(MIZAR_USB_DCFG,0x480801);
	write_reg( MIZAR_USB_DEVTEN,0x1f);
	rd_data = read_reg(MIZAR_USB_GUCTL);
	write_reg(MIZAR_USB_GUCTL,0xa400010);

	 
	set_configuration(0,0,0,0x409); //START NEW CONFIGURATION

        set_configuration(0,0x200,0x700,0x401);
        set_configuration(0x10,0x200,0x2000700,0x401);
        set_configuration(0x20,0x206,0x4000700,0x401);
        set_configuration(0x30,0x20206,0x6000700,0x401);
        set_configuration(0x40,0x204,0x8000700,0x401);
        set_configuration(0x50,0x40204,0xa000700,0x401);
        set_configuration(0x60,0x1ffa,0xc000700,0x401);
        set_configuration(0x70,0x61ffa,0xe000700,0x401);

	//start endpoint tx resource configuration
	for(i =0;i<8;i++) {
		write_reg(MIZAR_USB_DEPCMDPAR0+(i*0x10),0x1);
		write_reg(MIZAR_USB_DEPCMD+(i*0x10),0x402);
		rd_data = read_reg(MIZAR_USB_DEPCMD+(i*0x10));
		while(rd_data == 0x402) {
			wait_on(10);
			rd_data = read_reg(MIZAR_USB_DEPCMD+(i*0x10));
		}
	}


	write_reg(MIZAR_USB_DALEPENA,0x3); //enabling physical ep 0,1;
	write_reg(MIZAR_USB_DCTL,0x80f00000);
	write_reg(MIZAR_LSS_SYSREG_INTR_EN0,0x80000000); //INterrupt enable at sysreg

        //link state connect reset events 
	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}

	if(event_counter <= 0x4){
		int_pend = 1;
		while(int_pend) {
			wait_on(100);
		}
        }	
	write_reg(MIZAR_USB_DCFG,0x480801);
	if(event_counter <= 0x4){
		int_pend = 1;
		while(int_pend) {
			wait_on(100);
		}
        }

	//Enumeration
	write_reg(0xA0243ffc,0xdeadbee0);
	
	rd_data = read_reg(MIZAR_USB_DCFG);
	rd_data = read_reg(MIZAR_USB_DSTS);
	write_reg(MIZAR_USB_DCFG,0x480801);
	write_reg(MIZAR_USB_DCTL,0x80f00a00);
	write_reg(MIZAR_USB_DALEPENA,0xff);
	printf("Buffer_PointerLO is %x\n",Buffer_PointerLO);
	wait_on(5000);

	setup_stage();
	write_reg(MIZAR_USB_GUSB2PHYCFG,0x40002547);
	if(event_counter <= 0x4){
		int_pend = 1;
		while(int_pend) {
			wait_on(100);
		}
        }
	if(event_counter <= 0x4){
		int_pend = 1;
		while(int_pend) {
			wait_on(100);
		}
        }

//	int_pend = 1;
//	while(int_pend) {
//		wait_on(100);
//	}
	//SET ADDRESS
	write_reg(MIZAR_USB_DCFG,0x480809);
	write_reg(event_trb_addr,Buffer_PointerLO);
	write_reg(event_trb_addr+0x8,0x0); 
	write_reg(event_trb_addr+0xc,0x853);
	write_reg(MIZAR_USB_DEPCMDPAR1+0x10,event_trb_addr);
	write_reg(MIZAR_USB_DEPCMDPAR0+0x10,0x0);

	write_reg(MIZAR_USB_DEPCMD+0x10,0x506);
	rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);
	while(rd_data == 0x506){
		wait_on(10);
		rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);
	}

//		if(event_counter <= 0x4){
//		int_pend = 1;
//		while(int_pend) {
//			wait_on(100);
//		}
//        }
	if(event_counter <= 0x4){
		int_pend = 1;
		while(int_pend) {
			wait_on(100);
		}
        }

//		if(event_counter <= 0x4){
//		int_pend = 1;
//		while(int_pend) {
//			wait_on(100);
//		}
//        }
//	if(event_counter <= 0x4){
		int_pend = 1;
		while(int_pend) {
			wait_on(100);
		}
//        }
       hand_shake = read_reg(0xa0243ff4);
         while(hand_shake == 0x0) {
	 	hand_shake = read_reg(0xa0243ff4);
	  }
	/*  #ifndef PROC_MODE
	int_pend = 1;
		while(int_pend) {
			wait_on(100);
		}
	  #endif */
	enumeration();
	 hand_shake = read_reg(0xa0243ff8);
         while(hand_shake == 0x0) {
	 	hand_shake = read_reg(0xa0243ff8);
	  }
	  
	rd_data = read_reg(MIZAR_USB_DSTS);

        //Wait for frame number1
	while((rd_data & 0x00000FF8) == 0x0 ){
		wait_on(100);
		rd_data = read_reg(MIZAR_USB_DSTS);
	}

	write_reg(0xA0243ffc,0xdeadbee6);
/*	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}
*/
	//ISOCHRONUS OUT
	
	write_reg(event_trb_addr,Buffer_PointerLO_1);
	write_reg(event_trb_addr+0x8,0x3ff); 
	write_reg(event_trb_addr+0xc,0x869);
	write_reg(MIZAR_USB_DEPCMDPAR1+0x60,event_trb_addr);
	write_reg(MIZAR_USB_DEPCMDPAR0+0x60,0x0);
	write_reg(MIZAR_USB_DEPCMD+0x60,0x20506);
	rd_data = read_reg(MIZAR_USB_DEPCMD+0x60);
	while(rd_data == 0x20506){
		wait_on(10);
		rd_data = read_reg(MIZAR_USB_DEPCMD+0x60);
	}

	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}

	rd_data = read_reg(MIZAR_USB_DSTS);
	//wait for the frame number2
	while(((rd_data & 0x00000038) >> 3) != 0x2 ){
		wait_on(100);
		rd_data = read_reg(MIZAR_USB_DSTS);
	}

	write_reg(0xA0243ffc,0xdeadbee7);
	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}
	//wait for the frame number3

	while(((rd_data & 0x00000038) >> 3) != 0x3 ){
		wait_on(100);
		rd_data = read_reg(MIZAR_USB_DSTS);
	}

	write_reg(0xA0243ffc,0xdeadbee8);

	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}


	write_reg(event_trb_addr,Buffer_PointerLO_1);
	write_reg(event_trb_addr+0x8,0x3ff); 
	write_reg(event_trb_addr+0xc,0x869);
	write_reg(MIZAR_USB_DEPCMDPAR1+0x70,event_trb_addr);
	write_reg(MIZAR_USB_DEPCMDPAR0+0x70,0x0);

	write_reg(MIZAR_USB_DEPCMD+0x70,0x40506);
	rd_data = read_reg(MIZAR_USB_DEPCMD+0x70);
	while(rd_data == 0x40506){
		wait_on(10);
		rd_data = read_reg(MIZAR_USB_DEPCMD+0x70);
	}
	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}
	//wait for the frame number4

	while(((rd_data & 0x00000038) >> 3) != 0x4 ){
		wait_on(100);
		rd_data = read_reg(MIZAR_USB_DSTS);
	}

	write_reg(0xA0243ffc,0xdeadbee9); 
	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}
	finish(0);
}

void Default_IRQHandler()
{
	int rd_data,sysreg_rd_data,event_count;
	int_pend = 0;
	rd_data =read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
	rd_data =read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);

	event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
	event_counter = event_count;
	write_reg(MIZAR_USB_GEVNTCOUNT,event_count);

	if( rd_data && 0x80000000){
		write_reg(MIZAR_LSS_SYSREG_RAW_STCR0,0x80000000);
	}
	GIC_ClearIRQ(84);
}


void setup_stage() {
	int rd_data;
	write_reg(event_trb_addr,Buffer_PointerLO);
	write_reg(event_trb_addr+0x8,0x8); 
	write_reg(event_trb_addr+0xc,0x823);
	write_reg(MIZAR_USB_DEPCMDPAR1,event_trb_addr);
	write_reg(MIZAR_USB_DEPCMDPAR0,0x0);

	write_reg(MIZAR_USB_DEPCMD,0x506);
	rd_data = read_reg(MIZAR_USB_DEPCMD);
	while(rd_data == 0x506){
		wait_on(10);
		rd_data = read_reg(MIZAR_USB_DEPCMD);
	}
	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}

}


void status_stage() {
	int rd_data;
	write_reg(event_trb_addr,Buffer_PointerLO);
	write_reg(event_trb_addr+0x8,0x0); 
	write_reg(event_trb_addr+0xc,0x843);
	write_reg(MIZAR_USB_DEPCMDPAR1,event_trb_addr);
	write_reg(MIZAR_USB_DEPCMDPAR0,0x0);

	write_reg(MIZAR_USB_DEPCMD,0x506);
	rd_data = read_reg(MIZAR_USB_DEPCMD);
	while(rd_data == 0x506){
		wait_on(10);
		rd_data = read_reg(MIZAR_USB_DEPCMD);
	}
	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}

	wait_on(100);

	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}
}


void enumeration()
{
 
       int rd_data; 

	setup_stage();
	write_reg(0xA0243ffc,0xdeadbee1);
	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}

	write_reg(event_trb_addr,Buffer_PointerLO);
	write_reg(event_trb_addr+0x8,0x12); 
	write_reg(event_trb_addr+0xc,0x853);
	//data
	write_reg(Buffer_PointerLO,0x02000012);
	write_reg(Buffer_PointerLO+0x4,0x40000000);
	write_reg(Buffer_PointerLO+0x8,0x00000000);
	write_reg(Buffer_PointerLO+0xc,0x00000000);
	write_reg(Buffer_PointerLO+0x10,0x00000100);
	write_reg(MIZAR_USB_DEPCMDPAR1+0x10,event_trb_addr);
	write_reg(MIZAR_USB_DEPCMDPAR0+0x10,0x0);

	write_reg(MIZAR_USB_DEPCMD+0x10,0x506);
	rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);
	while(rd_data == 0x506){
		wait_on(10);
		rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);
	}

	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}

	wait_on(100);

	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}
	//status
	//ADDED to check the xfernotready evnet 
	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}

        
	status_stage();

//	wait_on(1000);
        //write_reg(0xA0243ffc,0xdeadbee1);
//	int_pend = 1;
//	while(int_pend) {
//		wait_on(100);
//	}
//	int_pend = 1;
//	while(int_pend) {
//		wait_on(100);
//	}
	//DEVICE QUALIFIER
//	setup_stage();
//	//data
//	write_reg(0xA0243ffc,0xdeadbee2);
//
//	int_pend = 1;
//	while(int_pend) {
//		wait_on(100);
//	}
//
//	write_reg(event_trb_addr,Buffer_PointerLO);
//	write_reg(event_trb_addr+0x8,0xa); 
//	write_reg(event_trb_addr+0xc,0x853);
//	write_reg(Buffer_PointerLO,0x0000060a);
//	write_reg(Buffer_PointerLO+0x4,0x00000000);
//	write_reg(Buffer_PointerLO+0xc,0x00000000);
//	write_reg(MIZAR_USB_DEPCMDPAR1+0x10,event_trb_addr);
//	write_reg(MIZAR_USB_DEPCMDPAR0+0x10,0x0);
//
//	write_reg(MIZAR_USB_DEPCMD+0x10,0x506);
//	rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);
//	while(rd_data == 0x506){
//		wait_on(10);
//		rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);
//	}
//
//	int_pend = 1;
//	while(int_pend) {
//		wait_on(100);
//	}
//
//	wait_on(100);
//
//	int_pend = 1;
//	while(int_pend) {
//		wait_on(100);
//	}
//	//status
//	status_stage();
//
	//GET DESCRIPTOR USB CONFIGURATION
	setup_stage();
	//data
	write_reg(0xA0243ffc,0xdeadbee3);
	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}
//	09 02 3c 00 01 01 00 e0 32
	write_reg(event_trb_addr,Buffer_PointerLO);
	write_reg(event_trb_addr+0x8,0x9); 
	write_reg(event_trb_addr+0xc,0x853);
	write_reg(Buffer_PointerLO,0x003c0209);
	write_reg(Buffer_PointerLO+0x4,0xe0000101);
	write_reg(Buffer_PointerLO+0x8,0x00000032);
	write_reg(Buffer_PointerLO+0xc,0x00000000);
	write_reg(MIZAR_USB_DEPCMDPAR1+0x10,event_trb_addr);
	write_reg(MIZAR_USB_DEPCMDPAR0+0x10,0x0);

	write_reg(MIZAR_USB_DEPCMD+0x10,0x506);
	rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);
	while(rd_data == 0x506){
		wait_on(10);
		rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);
	}

	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}

	wait_on(100);

	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}

	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}
	//status
	status_stage();
	//USB_SET_CONFIGURATION_OR_RESET_TT
	setup_stage();
	//status data stage
	write_reg(0xA0243ffc,0xdeadbee4);

	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}
	write_reg(event_trb_addr,Buffer_PointerLO);
	write_reg(event_trb_addr+0x8,0x0); 
	write_reg(event_trb_addr+0xc,0x853);
	write_reg(Buffer_PointerLO,0x00);
	write_reg(MIZAR_USB_DEPCMDPAR1+0x10,event_trb_addr);
	write_reg(MIZAR_USB_DEPCMDPAR0+0x10,0x0);

	write_reg(MIZAR_USB_DEPCMD+0x10,0x506);
	rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);
	while(rd_data == 0x506){
		wait_on(10);
		rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);
	}

	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}

	wait_on(100);

	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}        
	//GET DESCRIPTOR USB CONFIGURATION
	setup_stage();
	write_reg(0xA0243ffc,0xdeadbee5);
	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}
	//data stage
	//09023c00 010100e0 32090400 00060101 00000755 01034000 01070502 024000ff 07050301 ff030107 05810340 00010705 82024000 ff070583 01ff0301
	//09023c00 010100e0 32090400 00060101 00000705 01034000 01070502 024000ff 07050301 ff030107 05810340 00010705 82024000 ff070583 01ff0301
	write_reg(event_trb_addr,Buffer_PointerLO);
	write_reg(event_trb_addr+0x8,0x3c); 
	write_reg(event_trb_addr+0xc,0x853);
	write_reg(Buffer_PointerLO,     0x003c0209);
	write_reg(Buffer_PointerLO+0x4, 0xe0000101);
	write_reg(Buffer_PointerLO+0x8, 0x00040932);
	write_reg(Buffer_PointerLO+0xc, 0x01010600);
	write_reg(Buffer_PointerLO+0x10,0x05070000);
	write_reg(Buffer_PointerLO+0x14,0x00400301);
	write_reg(Buffer_PointerLO+0x18,0x02050701);
	write_reg(Buffer_PointerLO+0x1c,0xff004002);
	write_reg(Buffer_PointerLO+0x20,0x01030507);
	write_reg(Buffer_PointerLO+0x24,0x070103ff);
	write_reg(Buffer_PointerLO+0x28,0x40038105);
	write_reg(Buffer_PointerLO+0x2c,0x05070100);
	write_reg(Buffer_PointerLO+0x30,0x00400282);
	write_reg(Buffer_PointerLO+0x34,0x830507ff);
	write_reg(Buffer_PointerLO+0x38,0x0103ff01);
//	write_reg(Buffer_PointerLO+0x3c,0x01830507);        
//	write_reg(Buffer_PointerLO+0x40,0x07000400);
//	write_reg(Buffer_PointerLO+0x44,0x00010305);       
//	write_reg(Buffer_PointerLO+0x48,0x00000004);	
	write_reg(MIZAR_USB_DEPCMDPAR1+0x10,event_trb_addr);
	write_reg(MIZAR_USB_DEPCMDPAR0+0x10,0x0);

	write_reg(MIZAR_USB_DEPCMD+0x10,0x506);
	rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);
	while(rd_data == 0x506){
		wait_on(10);
		rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);
	}
	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}

	wait_on(100);

	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}


	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}
	status_stage();

}
void set_configuration(int trb_address,int parameter0,int parameter1,int cmd)
{
int read_data;
        
	write_reg(MIZAR_USB_DEPCMDPAR1+trb_address ,parameter1);
	write_reg(MIZAR_USB_DEPCMDPAR0+trb_address ,parameter0);
	write_reg(MIZAR_USB_DEPCMD+trb_address ,cmd);
        read_data = read_reg(MIZAR_USB_DEPCMD+trb_address);

	while(read_data == cmd){
		wait_on(30);
		read_data = read_reg(MIZAR_USB_DEPCMD+trb_address);
        }   
}

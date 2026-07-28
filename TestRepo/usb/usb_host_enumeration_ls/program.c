#include <stdio.h>
#include <stdlib.h>
#include "usb.h"

extern int int_pend;
int test_case() {

	int rd_data,port_count,db_offset;
	int event_completion,usb_status,port_status;
	int input_context_address ;
	int  data_in[512],j,data_out[512],i;
	int count =0;

	nic_programming();
//	GIC_EnableIRQ(84);
        GIC_EnableAllIRQ();

	write_reg(MIZAR_USB_GCTL,set_data(read_reg(MIZAR_USB_GCTL),0xFFFFFFFF,0x30c11234));
	write_reg(MIZAR_USB_GFLADJ,set_data(read_reg(MIZAR_USB_GFLADJ),0xFFFFFFFF,0xa87f000));
	write_reg(MIZAR_USB_GUCTL,set_data(read_reg(MIZAR_USB_GUCTL),0xFFFFFFFF,0x2000010));

	//PIPE CONTROL REG  MIZAR_USB_GUSB3PIPECTL 0xc100
	rd_data=read_reg(MIZAR_USB_BASE+0xc2c0);
	write_reg(MIZAR_USB_BASE+0xc2c0,0x10c0002);

	//PHY CONTROL REG
	rd_data = read_reg(MIZAR_USB_BASE+0xc200);
	write_reg(MIZAR_USB_BASE+0xc200,0x102407);

	port_count = read_reg(MIZAR_USB_HCSPARAMS1);
	rd_data=read_reg(MIZAR_USB_SUPTPRT2_DW2);
	rd_data=read_reg(MIZAR_USB_SUPTPRT3_DW2);
	rd_data=read_reg(MIZAR_USB_PORTSC_20); //PORTSC REG

	write_reg(MIZAR_USB_PORTSC_20,set_data(read_reg(MIZAR_USB_PORTSC_20),USB_PORTSC_20_WCE,1));
	write_reg(MIZAR_USB_PORTSC_20,set_data(read_reg(MIZAR_USB_PORTSC_20),USB_PORTSC_20_WDE,1));
	write_reg(MIZAR_USB_PORTSC_20,set_data(read_reg(MIZAR_USB_PORTSC_20),USB_PORTSC_20_WOE,1));
	write_reg(MIZAR_USB_PORTSC_20,0xe0002a0);

	db_offset=read_reg(MIZAR_USB_DBOFF); //DBOFF register

	write_reg(Event_Ring_Segment_Table,Default_Event_Ring_Array); //EVENT RING SEGMENT table
	write_reg(Event_Ring_Segment_Table + DWORD,0x0);
	write_reg(Event_Ring_Segment_Table + 2*(DWORD),0x30); //EVENT RING SEGMENT size

	rd_data= read_reg(MIZAR_USB_HCSPARAMS2); // HCSPARAMS2 returns 1400_00f1  
	rd_data =read_reg(MIZAR_USB_PAGESIZE);// PAGESIZE returns 1 (refer 19549 line no. of synopsys log)

	write_reg(Scratchpad_Buffer_Array,SCRATCHPAD0);
	write_reg(Scratchpad_Buffer_Array+ DWORD,0x0);
	write_reg(Scratchpad_Buffer_Array+ 2*DWORD,SCRATCHPAD1);
	write_reg(Scratchpad_Buffer_Array+ 3*DWORD,0x0);

	//loading Device_Context_Base_Address_Array
	write_reg(Device_Context_Base_Address_Array,Scratchpad_Buffer_Array);
	write_reg(Device_Context_Base_Address_Array + DWORD ,0x0);

	write_reg(Device_Context_Base_Address_Array + 2*DWORD ,Device_Context_Array+0x100); 
	write_reg(Device_Context_Base_Address_Array + 3*DWORD ,0x0);

	write_reg(Device_Context_Base_Address_Array + 4*DWORD ,Device_Context_Array+0x0d00);
	write_reg(Device_Context_Base_Address_Array + 5*DWORD ,0x0);

	write_reg(MIZAR_USB_CRCR_LO,Default_Command_Ring+0x1);
	write_reg(MIZAR_USB_CRCR_HI,0x0); 

	write_reg(MIZAR_USB_CONFIG,0x10);  // CONFIG MaxSlotsEn = 'h10

	rd_data = read_reg(MIZAR_USB_CONFIG);
	write_reg(MIZAR_USB_CONFIG,0x110); //CONFIG CIE When set to '1', the software shall initialize the Configuration Value, Interface Number, and Alternate Settingfields in the Input Control Context when it is associated with a Configure Endpoint Command.

	write_reg(MIZAR_USB_DCBAAP_LO,Device_Context_Base_Address_Array); 
	write_reg(MIZAR_USB_DCBAAP_HI,0x0);
	write_reg(MIZAR_USB_ERSTSZ,0x1); //EVENT RING SEGMENT SIZE 

	write_reg(MIZAR_USB_ERDP_LO,Default_Event_Ring_Array); //ERDP DWORD0 defines the high order bits of the 64-bit address of the current Event Ring Dequeue Pointer
	write_reg(MIZAR_USB_ERDP_HI,0x0); //ERDP DWORD1
	write_reg(MIZAR_USB_ERSTBA_LO,Event_Ring_Segment_Table);
	write_reg(MIZAR_USB_ERSTBA_HI,0x0);
	write_reg(MIZAR_USB_IMOD,0x0);
	write_reg(MIZAR_USB_IMAN,0x2);
	write_reg(MIZAR_USB_USBCMD,0x4); //usbcmd INNTERRUPT enable, enabling interrupts grenerated by interrupters

	write_reg(MIZAR_LSS_SYSREG_INTR_EN0,0x80000000); //INterrupt enable at sysreg
	write_reg(MIZAR_USB_USBCMD,0x5); //usbcmd run/stop bit is 1 , run
	int_pend = 1;

	while(int_pend) {
		wait_on(100);
	}

	usb_status=read_reg(MIZAR_USB_USBSTS);
	write_reg(MIZAR_USB_USBSTS,0x8);
	write_reg(MIZAR_USB_IMAN,0x2);
	write_reg(MIZAR_USB_ERDP_HI,0x0); //READ
	port_status=read_reg(MIZAR_USB_PORTSC_20);//port connect status should be high

	write_reg(MIZAR_USB_ERDP_LO,Default_Event_Ring_Array+0x18); //32 bytes increment --->d20
	write_reg(MIZAR_USB_PORTSC_20,0xe0006f1); //port reset

	write_reg(MIZAR_USB_USBSTS,0x8);
	write_reg(MIZAR_USB_IMAN,0x2);

	write_reg(MIZAR_USB_PORTSC_20,0xe220200);

	port_status=read_reg(MIZAR_USB_PORTSC_20);

	int_pend =1;
	while(int_pend) {
		wait_on(100);
	}

	//slot command TRB
	write_reg(Default_Command_Ring+0x0,0x0);
	write_reg(Default_Command_Ring+0x4,0x0);
	write_reg(Default_Command_Ring+0x8,0x0);
	write_reg(Default_Command_Ring+0xc,0x00002401);

	usb_status=read_reg(MIZAR_USB_USBSTS);
	write_reg(MIZAR_USB_USBSTS,0x8);
	write_reg(MIZAR_USB_IMAN,0x2);
	write_reg(MIZAR_USB_ERDP_HI,0x0); 

	write_reg(MIZAR_USB_ERDP_LO,Default_Event_Ring_Array+0x28); //32 bytes increment --->d40

	write_reg(MIZAR_USB_USBSTS,0x8);
	write_reg(MIZAR_USB_IMAN,0x2);
	write_reg(MIZAR_USB_PORTSC_20,0xe200e01);
	write_reg(MIZAR_USB_DB,0x0);  
	int_pend =1;

	while(int_pend) {
		wait_on(100);
	}  
	write_reg(Default_Input_Context,0x0);
	write_reg(Default_Input_Context+DWORD,0x3);

	write_reg(Default_Input_Context+0x40,0x08200000);
	write_reg(Default_Input_Context+0x44,0x00010000);
	write_reg(Default_Input_Context+0x80,0x00); //EP0
	write_reg(Default_Input_Context+0x84,0x00080020); 
	//write_reg(Default_Input_Context+0x88	,0x200001); //transfer_ring
	write_reg(Default_Input_Context+0x88	,EP0_TR_Dequeue_Pointer | 0x1); //transfer_ring

	write_reg(Default_Input_Context+0x90	,0x08);

	//Address command

	// input_context_address =(Default_Input_Context << 4)+ 0x0;
	input_context_address =(Default_Input_Context)+ 0x0;

	write_reg(Default_Command_Ring+0x10,input_context_address);
	write_reg(Default_Command_Ring+0x1c,0x01002e01);

	write_reg(MIZAR_USB_USBSTS,0x8);
	write_reg(MIZAR_USB_IMAN,0x2);
	write_reg(MIZAR_USB_ERDP_HI,0x0); 
	write_reg(MIZAR_USB_ERDP_LO,Default_Event_Ring_Array+0x38); //32 bytes increment --->d40
	write_reg(MIZAR_USB_DB,0x0);  

	int_pend =1;
	while(int_pend) {
		wait_on(100);
	}	
	event_completion=read_reg(Default_Event_Ring_Array+0x30);
	//BSR ==1
	write_reg(Default_Input_Context+DWORD,0x3);

	write_reg(Default_Input_Context+0x40,0x08200000);
	write_reg(Default_Input_Context+0x44,0x00010000);
	write_reg(Default_Input_Context+0x80,0x00);
	write_reg(Default_Input_Context+0x84,0x00080020); 
	//write_reg(Default_Input_Context+0x88	,0x200001); //transfer_ring
	write_reg(Default_Input_Context+0x88	,EP0_TR_Dequeue_Pointer | 0x1); //transfer_ring
	write_reg(Default_Input_Context+0x90	,0x08);

	//Address command
	write_reg(Default_Command_Ring+0x20,input_context_address);
	write_reg(Default_Command_Ring+0x2c,0x01002c01);

	write_reg(MIZAR_USB_USBSTS,0x8);
	write_reg(MIZAR_USB_IMAN,0x2);
	write_reg(MIZAR_USB_ERDP_HI,0x0); 
	write_reg(MIZAR_USB_ERDP_LO,Default_Event_Ring_Array+0x48); //32 bytes increment --->d40
	write_reg(MIZAR_USB_DB,0x0); 

	int_pend =1;
	while(int_pend) {
		wait_on(100);
	}
	event_completion= read_reg(Default_Event_Ring_Array+0x40);
	enumeration();
	printf("after enumeration\n");
/*	//SET CONFIGURATION
	write_reg(Default_Input_Context+DWORD,0x0d); //A5configured
	write_reg(Default_Input_Context+0x40,0x18200000);
	write_reg(Default_Input_Context+0x44,0x00010000);
	write_reg(Default_Input_Context+0x4c,0x01);

	write_reg(Default_Input_Context+0x80,0x00);
	write_reg(Default_Input_Context+0x84,0x00080020); 
	//  write_reg(Default_Input_Context+0x88	,0x200001); //transfer_ring
	write_reg(Default_Input_Context+0x88	,EP0_TR_Dequeue_Pointer | 0x1); //transfer_ring
	write_reg(Default_Input_Context+0x90	,0x08);

	//EP1 introut
        write_reg(Default_Input_Context+0xc0,0x30000); //changed
        write_reg(Default_Input_Context+0xc4,0x00080018); //changed
        write_reg(Default_Input_Context+0xc8,EP2_Out_TR_Dequeue_Pointer | 0x1); //transfer ring 28000           
	write_reg(Default_Input_Context+0xd0,0x00080008); //changed

	 //EP1 intrin
         write_reg(Default_Input_Context+0x100,0x30000);
         write_reg(Default_Input_Context+0x104,0x00080038);
         write_reg(Default_Input_Context+0x108,EP2_In_TR_Dequeue_Pointer | 0x1); //transfer ring 2a00           
	 write_reg(Default_Input_Context+0x110,0x0080008); //changed

	//EP command
	write_reg(Default_Command_Ring+0x30,input_context_address);
	write_reg(Default_Command_Ring+0x34,0x0);
	write_reg(Default_Command_Ring+0x38,0x0);
	write_reg(Default_Command_Ring+0x3c,0x1003001);


	write_reg(MIZAR_USB_USBSTS,0x8);
	write_reg(MIZAR_USB_IMAN,0x2);
	write_reg(MIZAR_USB_ERDP_HI,0x0); 
	write_reg(MIZAR_USB_ERDP_LO,Default_Event_Ring_Array+0x58); //32 bytes increment --->d40
	write_reg(MIZAR_USB_DB,0x0);

	event_completion=  read_reg(Default_Event_Ring_Array+0x50);
	int_pend =1;
	while(int_pend) {
		wait_on(100);
	} 
	event_completion=  read_reg(Default_Event_Ring_Array+0x50);

         //enumeration();

	for(j=0;j<2;j=j+1) {
		data_in[j] = j;
		write_reg((EP2_Out_TR_Dequeue_Pointer+0x100+j*4),data_in[j]);

	}    
	write_reg(EP2_Out_TR_Dequeue_Pointer,EP2_Out_TR_Dequeue_Pointer+0x100);
	write_reg(EP2_Out_TR_Dequeue_Pointer+0x8,0x8); 
	write_reg(EP2_Out_TR_Dequeue_Pointer+0xc,0x425);

	write_reg(MIZAR_USB_USBSTS,0x8);
	write_reg(MIZAR_USB_IMAN,0x2);
	write_reg(MIZAR_USB_ERDP_HI,0x0); 
	write_reg(MIZAR_USB_ERDP_LO,Default_Event_Ring_Array+0x68); //32 bytes increment --->d40   
	write_reg(MIZAR_USB_BASE+0x484,0x2);

	int_pend =1;
	while(int_pend) {
		wait_on(100);
	}
	event_completion=read_reg(Default_Event_Ring_Array+0x60);

        if(count > 0x0){
		finish(1);
	} else {*/
		finish(0);

}

void Default_IRQHandler()
{
	int rd_data,sysreg_rd_data;
	int_pend = 0;
	rd_data =read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
	rd_data =read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
	if( rd_data && 0x80000000){
		write_reg(MIZAR_USB_IMAN,0x1);
		write_reg(MIZAR_LSS_SYSREG_RAW_STCR0,0x80000000);

	}
	GIC_ClearIRQ(84);
}

void enumeration()
{
	int event_completion;
	//GET configuration get device
	write_reg( EP0_TR_Dequeue_Pointer,0x01000680);
	write_reg( EP0_TR_Dequeue_Pointer+0x4,0x00120000);
	write_reg( EP0_TR_Dequeue_Pointer+0x8,0x08);
	write_reg( EP0_TR_Dequeue_Pointer+0xc,0x00030861);
	//DATA
	write_reg( EP0_TR_Dequeue_Pointer+0x10,EP0_TR_Dequeue_Pointer+0x200);
	write_reg( EP0_TR_Dequeue_Pointer+0x18,0x12); 
	write_reg( EP0_TR_Dequeue_Pointer+0x1c,0x00010c27);
	//Status stage
	write_reg( EP0_TR_Dequeue_Pointer+0x2c,0x00001023);


	/*//GET QUALIFIER
	write_reg( EP0_TR_Dequeue_Pointer+0x30,0x06000680);
	write_reg(EP0_TR_Dequeue_Pointer+0x34,0x000a0000);
	write_reg(EP0_TR_Dequeue_Pointer+0x38,0x08);
	write_reg(EP0_TR_Dequeue_Pointer+0x3c,0x00030861);
	//DATA
	write_reg(EP0_TR_Dequeue_Pointer+0x40,EP0_TR_Dequeue_Pointer+0x210);
	write_reg(EP0_TR_Dequeue_Pointer+0x48,0xa);
	write_reg(EP0_TR_Dequeue_Pointer+0x4c,0x00010c27);
	//Status stage
	write_reg(EP0_TR_Dequeue_Pointer+0x5c,0x00001023);*/

	//GET configuration stage
	write_reg(EP0_TR_Dequeue_Pointer+0x30,0x02000680);
	write_reg(EP0_TR_Dequeue_Pointer+0x34,0x00090000);
	write_reg(EP0_TR_Dequeue_Pointer+0x38,0x08);
	write_reg(EP0_TR_Dequeue_Pointer+0x3c,0x00030861);
	//DATA
	write_reg(EP0_TR_Dequeue_Pointer+0x40,EP0_TR_Dequeue_Pointer+0x240);
	write_reg(EP0_TR_Dequeue_Pointer+0x48,0x9);
	write_reg(EP0_TR_Dequeue_Pointer+0x4c,0x00010c25);
	//Status stage
	write_reg(EP0_TR_Dequeue_Pointer+0x5c,0x00001023);

	//SET configuration
	//SETUP stage
	write_reg(EP0_TR_Dequeue_Pointer+0x60,0x00010900);
	write_reg(EP0_TR_Dequeue_Pointer+0x64,0x00000000);
	write_reg(EP0_TR_Dequeue_Pointer+0x68,0x08);
	write_reg(EP0_TR_Dequeue_Pointer+0x6c,0x00000841);
	//Status stage
	write_reg(EP0_TR_Dequeue_Pointer+0x7c,0x00011023);


	//GET configuration2
	//GET configuration stage
	write_reg(EP0_TR_Dequeue_Pointer+0x80,0x02000680);
	write_reg(EP0_TR_Dequeue_Pointer+0x84,0x00090000);
	write_reg(EP0_TR_Dequeue_Pointer+0x88,0x08);
	write_reg(EP0_TR_Dequeue_Pointer+0x8c,0x00030861);
	//DATA
	write_reg(EP0_TR_Dequeue_Pointer+0x90,EP0_TR_Dequeue_Pointer+0x500);
	write_reg(EP0_TR_Dequeue_Pointer+0x98,0x8);
	write_reg(EP0_TR_Dequeue_Pointer+0x9c,0x00010c27);
	//Status stage
	write_reg(EP0_TR_Dequeue_Pointer+0xac,0x00001023);
/*	write_reg(MIZAR_USB_BASE+0x484,0x1);
	event_completion=read_reg(Default_Event_Ring_Array+0xe0);
	while(event_completion == 0) {
		event_completion=read_reg(Default_Event_Ring_Array+0xe0);
	}
*/
	//GET configuration3
	write_reg(EP0_TR_Dequeue_Pointer+0xb0,0x02000680);
	write_reg(EP0_TR_Dequeue_Pointer+0xb4,0x00180000);
	write_reg(EP0_TR_Dequeue_Pointer+0xb8,0x08);
	write_reg(EP0_TR_Dequeue_Pointer+0xbc,0x00030861);
	//DATA
	write_reg(EP0_TR_Dequeue_Pointer+0xc0,EP0_TR_Dequeue_Pointer+0x560);
	write_reg(EP0_TR_Dequeue_Pointer+0xc8,0x18);
	write_reg(EP0_TR_Dequeue_Pointer+0xcc,0x00010c25);
	//Status stage
	write_reg(EP0_TR_Dequeue_Pointer+0xdc,0x00001023);
	write_reg(MIZAR_USB_BASE+0x484,0x1);
/*	int_pend = 1;
	while(int_pend) {
		wait_on(100);
	}*/
//	printf("event_completion is %x\n",event_completion); 
	//event_completion=read_reg(Default_Event_Ring_Array+0x100);
	printf("event_completion is %x Default_Event_Ring_Array %x\n",event_completion,Default_Event_Ring_Array); 
	event_completion=read_reg(Default_Event_Ring_Array+0x110);

	while(event_completion ==0 ) {
		wait_on(4);
		event_completion=read_reg(Default_Event_Ring_Array+0x110);
	} 

}    

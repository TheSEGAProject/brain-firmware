

/**************************  SDLOD.C  ******************************************
*
* SDLOD routines here
*
* needs change!
*
* V1.00 11/02/2005 wzr
*		Started
*
*
******************************************************************************/

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* function not referenced */
/*lint -e716 */		/* while(1) ... */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */
/*lint -e768 */		/* global struct member not referenced */



#include "DIAG.h"			//diagnostic defines
#include <msp430x54x.h>		//processor register description
#include "std.h"			//common definitions
#include "config.h" 		//system configuration definitions
//#include "main.h"			//mains defines
//#include "MISC.H"			//misc functions
#include "DELAY.h"  		//delay routines
#include "SERIAL.H" 		//serial port IO pkg
#include "SDLOD.h"			//timer routines
//#include "MISC.h"			//misc utility routines
//#include "BIGSUB.h"		//big subtract routines
//#include "L2FRAM.h"		//level 2 Ferro ram
//#include "OPMODE.h"		//opmode msg routines
//#include "ACTION.h"		//action routines
//#include "REPORT.h"		//msg reporting routines
//#include "MSG.h"			//general msg routines
#include "SDCTL.h"			//SD control module
#include "BUZ.h"			//sound module
#include "BUTTON.h"			//button handler routines

#ifdef ESPORT_ENABLED				//defined in diag.h
 #include "ESPORT.h"			//external serial port
#endif


/****************************  EXTERNS  ************************************/


extern volatile uchar ucaMSG_BUFF[ 70 ];	//overflows into ucaX0FLD





/***************************  DECLARATIONS  **********************************/

void vSDLOD_do_basic_burn_cmds(
		uint uiCFGS_value  //CMD_BCF_EECON1_CFGS = flash, CMD_BSF_EECON1_CFGS = config
		);







/*****************************  CODE  ***************************************/



/**********************  vSDLOD_init() ****************************************
*
* Init the SDLOD timer
*
*****************************************************************************/

void vSDLOD_init(
		void
		)
	{

//	vSERIAL_setBinaryMode(BINARY_MODE_ON);

	return;

	}/* END: vSDLOD_init() */





/**********************  vSDLOD_quit() ****************************************
*
* Init the SDLOD timer
*
*****************************************************************************/

void vSDLOD_quit(
		void
		)
	{

//	vSERIAL_setBinaryMode(BINARY_MODE_OFF);

	return;

	}/* END: vSDLOD_quit() */







/**********************  vSDLOD_enter_LVP_mode()  ******************************
*
* Enter Low Voltage In-Curcuit Serial Program Mode
*
******************************************************************************/

void vSDLOD_enter_LVP_mode(
		void
		)
	{
/*
	vSDCTL_set_SD_power_bit(SD_POWER_ON);	//make sure SD is powered
	vDELAY_wait100usTic(300);				//30ms for SD power up
	vSDCTL_set_SD_reset_bit(SD_RESET_MODE);

	PORTF &= ~0b11110000;		//MCLR=0,  DATA=0,  CLK=0,  PGM=0
	TRISF &= ~0b11110000;		//MCLR=out,DATA=out,CLK=out,PGM=out

	vDELAY_wait100usTic(300);				//30ms for SD halt

	PORTFbits.SD_IN_CIRCUIT_ENA_BIT = 1;	//raise PGM bit

	vDELAY_wait4us();						//delay 2us mininum

	PORTFbits.SD_RESET_BIT = 1;				//raise reset (start running)

	vDELAY_wait100usTic(300);				//30ms for SD to come active
*/
	return;

	}/* END: vSDLOD_enter_LVP_mode() */








/**********************  vSDLOD_leave_LVP_mode()  ****************************
*
* Exit Low Voltage In-Curcuit Serial Program Mode
*
******************************************************************************/

void vSDLOD_leave_LVP_mode(
		void
		)
	{
/*
	PORTFbits.SD_IN_CIRCUIT_ENA_BIT = 0;	//lower PGM bit
	vDELAY_wait4us();						//delay 2us mininum

	PORTFbits.SD_RESET_BIT = 0;				//Pull reset low (RESET)
	vDELAY_wait100usTic(300);				//30ms for SD to die

	PORTFbits.SD_RESET_BIT = 1;				//Raise Reset (RUN)
	vDELAY_wait100usTic(300);				//30ms for SD to startup

	TRISF = TRISF_INIT_VAL;					//set bit dirs back to init vals
*/
	return;

	}/* END: vSDLOD_leave_LVP_mode() */






/**********************  ucSDLOD_bin()  **************************************
*
* This routine clocks in 8 bits.
*
*Ret:	readback data
*
******************************************************************************/
uchar ucSDLOD_bin(
		void
		)
	{
/*
	uchar ucc;
	uchar ucInData;

	PORTFbits.SD_IN_CIRCUIT_DATA_BIT = 0;	//clr the bit
	TRISFbits.SD_IN_CIRCUIT_DATA_BIT = 1;	//set dir to in
	_asm NOP _endasm;


	// CLOCK THE DATA IN 
	ucInData = 0;
	for(ucc=0;  ucc<8;  ucc++)
		{
		PORTFbits.SD_IN_CIRCUIT_CLK_BIT = 1;	//raise the clk (send xmt data)

		_asm NOP _endasm;

		PORTFbits.SD_IN_CIRCUIT_CLK_BIT = 0;	//lower clk (latch read data)

		_asm NOP _endasm;

		ucInData >>= 1;
		ucInData |= (PORTFbits.SD_IN_CIRCUIT_DATA_BIT <<7); //read the data

		}// END: for() 

	TRISFbits.SD_IN_CIRCUIT_DATA_BIT = 0;	//set dir to out

	return(ucInData);
*/
return(0);
	}// END: uiSDLOD_bin() 






/**********************  vSDLOD_generic_bout()  **************************************
*
* This routine clocks out 8 bits of data.
*
******************************************************************************/
void vSDLOD_generic_bout(
		uchar ucOutData,
		uchar ucBitCount	//number of bits to send (0 - 8)
		)
	{
/*
	uchar ucc;

	#if 0
	vSERIAL_rom_sout("ClkingOutVal= ");
	vSERIAL_HB8out(ucOutData);
	vSERIAL_crlf();
	#endif


//	PORTFbits.SD_IN_CIRCUIT_DATA_BIT = 0;	//clr the bit to avoid driving
//	TRISFbits.SD_IN_CIRCUIT_DATA_BIT = 0;	//switch direction to out
	_asm NOP _endasm;


	// CLOCK THE DATA OUT //
	for(ucc=0;  ucc<ucBitCount;  ucc++)
		{
		// SET THE DATA BIT TO A WRITE //
		PORTFbits.SD_IN_CIRCUIT_DATA_BIT = ucOutData & 0b00000001;
		ucOutData >>= 1;

		_asm NOP _endasm;

		PORTFbits.SD_IN_CIRCUIT_CLK_BIT = 1;	//raise the clk (send xmt data)

		_asm NOP _endasm;

		PORTFbits.SD_IN_CIRCUIT_CLK_BIT = 0;	//lower clk

		_asm NOP _endasm;

		}// END: for() //
*/
	return;

	}/* END: vSDLOD_generic_bout() */





/**********************  vSDLOD_bout()  **************************************
*
* This routine clocks out 8 bits of data.
*
******************************************************************************/
void vSDLOD_bout(
		uchar ucOutData
		)
	{

//	vSDLOD_generic_bout(ucOutData,8);

	return;

	}/* END: ucSDLOD_bout() */






/*******************  vSDLOD_nibble_out()  ***********************************
*
* Send the 4bit Cmd nibble
*
******************************************************************************/
void vSDLOD_nibble_out(
		uchar uc4BitCmd
		)
	{

//	vSDLOD_generic_bout(uc4BitCmd,4);

	return;

	}/* END: vSDLOD_nibble_out() */





/*******************  vSDLOD_XCT_single_write_cmd()  **************************
*
* Send a 20 bit command packet to the SD board
*
******************************************************************************/

void vSDLOD_XCT_single_write_cmd(
		uchar ucFourBitCmd,
		uint uiDataPayload
		)
	{
/*
	uchar ucVal;

	vSDLOD_nibble_out(ucFourBitCmd);			//send the cmd

	ucVal = (uchar)uiDataPayload;				//LO byte
	vSDLOD_bout(ucVal);

	ucVal = (uchar)(uiDataPayload>>8);			//HI byte
	vSDLOD_bout(ucVal);
*/
	return;

	}/* END: vSDLOD_XCT_single_write_cmd() */





/*******************  ucSDLOD_XCT_single_read_cmd()  **************************
*
* Send a 20 bit read command packet to the SD board and ret the result
*
* RET: 8bit read value
*
******************************************************************************/

uchar ucSDLOD_XCT_single_read_cmd(
		uchar ucReadCmd4Bit
		)
	{
/*
	uchar ucDat;

	vSDLOD_nibble_out(ucReadCmd4Bit);

	vSDLOD_bout(0x00);

	ucDat = ucSDLOD_bin();

	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_NOP);	//erata padding

	return(ucDat);
*/
return(0);
	}/* END: ucSDLOD_XCT_single_read_cmd() */





/*********************  vSDLOD_load_tblptr()  ********************************
*
*
*
*****************************************************************************/

void vSDLOD_load_tblptr(
		usl uslAddr
		)
	{
/*
	uint uiByteVal;

	// SET TBL PTR TO ADDR //
	uiByteVal = (uint)((uslAddr >> 16)& 0xFF);
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_MOVLW_I | uiByteVal);
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_MOVWF_TBLPTRU);

	uiByteVal = (uint)((uslAddr >> 8)& 0xFF);
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_MOVLW_I | uiByteVal);
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_MOVWF_TBLPTRH);

	uiByteVal = (uint)(uslAddr & 0xFF);
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_MOVLW_I | uiByteVal);
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_MOVWF_TBLPTRL);
*/
	return;

	}/* END: vSDLOD_load_tblptr() */





/*****************  ucSDLOD_read_mem_byte()  ********************************
*
* RET: byte value at address specified
*
*****************************************************************************/

uchar ucSDLOD_read_mem_byte(
		usl uslAddr
		)
	{
/*
	uchar ucVal;

	vSDLOD_load_tblptr(uslAddr);
	ucVal = ucSDLOD_XCT_single_read_cmd(PROG_TBL_RD);

	return(ucVal);
*/
return(0);
	}/* END: ucSDLOD_read_mem_byte() */





/*****************  ucSDLOD_chk_for_SD_readable()  **************************
*
*
*
*****************************************************************************/

uchar ucSDLOD_chk_for_SD_readable(
		void
		)
	{
/*
	uchar ucVal;

	ucVal = ucSDLOD_read_mem_byte(0x3FFFFE);

	if((ucVal != 0) && (ucVal != 0xFF)) return(1);
*/
	return(0);

	}/* END: ucSDLOD_chk_for_SD_readable() */





/********************* vSDLOD_write_config_reg()  ***************************
*
*
*
*****************************************************************************/

void vSDLOD_burn_config_reg(
		usl uslAddr,
		uchar ucData
		)
	{
/*
	uint uiData;

	uiData = ucData;				//assume even byte
	if(uslAddr % 2) uiData <<= 8;	//odd byte

	// ENABLE SINGLE PANEL WRITES //
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_BSF_EECON1_EEPGD);//flash
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_BSF_EECON1_CFGS); //config area
	vSDLOD_load_tblptr(0x3C0006);
	vSDLOD_XCT_single_write_cmd(PROG_TBL_WR, 0x0000);

	// WRITE THE CONFIG REGISTER //
	vSDLOD_load_tblptr(uslAddr);
	vSDLOD_XCT_single_write_cmd(PROG_TBL_WR, uiData);
	vSDLOD_do_basic_burn_cmds(CMD_BSF_EECON1_CFGS);	//config
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_BCF_EECON1_WREN); //write disable
*/
	return;

	}/* END: vSDLOD_burn_config_reg() */







/*********************  vSDLOD_setup_452_config_regs()  **********************
*
*
*
******************************************************************************/

void vSDLOD_setup_452_config_regs(
		void
		)
	{
/*
	vSDLOD_burn_config_reg(0x300001, 0x22);	//Clk
	vSDLOD_burn_config_reg(0x300002, 0x0C);	//Brown out
	vSDLOD_burn_config_reg(0x300003, 0x0E);	//Watch dog
	vSDLOD_burn_config_reg(0x300005, 0x01);	//CCP2
	vSDLOD_burn_config_reg(0x300006, 0x84);	//LVP
*/
	return;

	}/* END: vSDLOD_setup_452_config_regs() */








/*********************  vSDLOD_do_basic_burn_cmds()  *************************
*
* This routine assumes the tblptr is already setup
*
*****************************************************************************/

void vSDLOD_do_basic_burn_cmds(
		uint uiCFGS_value //CMD_BCF_EECON1_CFGS= flash, CMD_BSF_EECON1_CFGS= config
		)
	{
/*
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_BSF_EECON1_EEPGD);//select flash mem
	vSDLOD_XCT_single_write_cmd(PROG_CORE, uiCFGS_value);		 //flash/config
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_BSF_EECON1_WREN); //write enable flash

	// DO MAGICAL INCANTATION FOR WRITE //
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_MOVLW_I + 0x55);
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_MOVWF_EECON2);
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_MOVLW_I + 0xAA);
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_MOVWF_EECON2);

	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_BSF_EECON1_WR);	//start burn
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_NOP);

	// DO THE BURN WAIT //
	vDELAY_wait100usTic(50);			//5ms
	vDELAY_wait4us();					//5us
	vDELAY_wait4us();					//extra
*/
	return;

	}/* END: vSDLOD_do_basic_burn_cmds() */






/*********************  vSDLOD_erase_64_byte_blk()  *************************
*
* Erase a single row of 64 bytes.
*
*****************************************************************************/
void vSDLOD_erase_64_byte_blk(
		usl uslAddr	//base addr of blk to be erased.
		)
	{
/*
	vSDLOD_load_tblptr(uslAddr);	//set blk base addr
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_BSF_EECON1_FREE); //erase bit
	vSDLOD_do_basic_burn_cmds(CMD_BCF_EECON1_CFGS);	//do the erase
*/
	return;

	}/* END: vSDLOD_erase_64_byte_blk() */






/*********************  vSDLOD_write_64_byte_blk() ***************************
*
* NOTE: erase is expected to have been done already
*
*****************************************************************************/

void vSDLOD_write_64_byte_blk(
		usl uslAddr,
		uchar ucErase_flag	//YES_ERASE_64BLK  or  NO_ERASE_64BLK
		)
	{
/*
	uchar ucii;
	uchar ucjj;
	uint uiData;

	// ERASE THE BLK WE ARE ABOUT TO WRITE //
	if(ucErase_flag == YES)
		vSDLOD_erase_64_byte_blk(uslAddr);

	// NOW BURN THE DATA IN 8 BYTE BLKS //
	for(ucii=0;  ucii<8;  ucii++)
		{
		vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_BSF_EECON1_EEPGD);	//flash mem
		vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_BCF_EECON1_WREN);

		vSDLOD_load_tblptr(uslAddr + (8 * ucii));
		for(ucjj=0;  ucjj<8;  ucjj+=2)
			{

			uiData = ucaMSG_BUFF[5 + (8 * ucii)+ ucjj];		//load Hi byte
			uiData <<= 8;									//put it first
			uiData |= ucaMSG_BUFF[4 + (8 * ucii)+ ucjj];	//load Hi byte

			vSDLOD_XCT_single_write_cmd(PROG_TBL_WR_POST_INC_2, uiData);

			}// END: for(usljj) //

		vSDLOD_load_tblptr(uslAddr + (8 * ucii));
		vSDLOD_do_basic_burn_cmds(CMD_BCF_EECON1_CFGS);

		}// END: for() //


	// DISABLE THE WRITES //
	vSDLOD_XCT_single_write_cmd(PROG_CORE, CMD_BCF_EECON1_WREN);
*/
	return;

 	}/* END: vSDLOD_write_64_byte_blk() */








/*****************  ucSDLOD_burn_single_msg_pkt_on_SD()  **********************
*
* Send the data to the SD board
*
*
*	;The message pkt below is the end result buffer layout of the ram image.
*	;---------------------------;
*	;MSG_BUFF+0 = MSG TYPE		;
*	;---------------------------;
*	;MSG_BUFF+1 = DEST ADDR LO	; <------------
*	;MSG_BUFF+2 = DEST ADDR MD	;			   |
*	;MSG_BUFF+3 = DEST ADDR HI	;			   |
*	;---------------------------;			   |
*	;MSG_BUFF+4 = DATA0 VAL LO  ; Data start   |
*	;MSG_BUFF+5 = DATA0 VAL HI	;			   |----- ChkSum Calculation
*	;    .						;			   |      Start and End
*	;    .						;			   |
*	;    .						;			   |
*	;MSG_BUFF+66 = DATA31 VAL LO;			   |
*	;MSG_BUFF+67 = DATA31 VAL HI; <------------
*	;---------------------------;
*	;MSG_BUFF+68 = CHKSUM		;
*	;---------------------------;
*
*
* RET:	CPROT_WR_BAD		if burn didn't burn right
*		CPROT_WR_OK			if burn was perfect
*		CPROT_WR_SAME		if no burn was needed
*
*
******************************************************************************/

uchar ucSDLOD_burn_single_msg_pkt_on_SD( //RET=CPROT_WR_OK, CPROT_WR_BAD, CPROT_WR_SAME
		void
		)
	{
/*
	uchar ucci;
	uchar uccj;
	usl uslAddr;


	// LOAD ADDRESS FROM MSG BUFFER //
	uslAddr = ucaMSG_BUFF[3];
	uslAddr <<= 8;
	uslAddr |= ucaMSG_BUFF[2];
	uslAddr <<= 8;
	uslAddr |= ucaMSG_BUFF[1];

	// COMPARE THE SD BLK TO CURRENT DATA TO SEE IF A BURN IS NEEDED //
	vSDLOD_load_tblptr(uslAddr);
	for(uccj=0;  uccj<64;  uccj++)
		{
		if(ucaMSG_BUFF[uccj+4] != ucSDLOD_XCT_single_read_cmd(PROG_TBL_RD_POST_INC))
 			break;
		}// END: for() //
	if(uccj==64) return(CPROT_WR_SAME);

	
	// TRY TO BURN THE ROM 3 TIMES //
	for(ucci=0;  ucci<3;  ucci++)
		{
		// BURN THE DATA //
		vSDLOD_write_64_byte_blk(uslAddr, YES_ERASE_64BLK);

		// COMPARE THE BURN DATA TO THE SENT DATA //
		vSDLOD_load_tblptr(uslAddr);
		for(uccj=0;  uccj<64;  uccj++)
			{
			if(ucaMSG_BUFF[uccj+4] != ucSDLOD_XCT_single_read_cmd(PROG_TBL_RD_POST_INC))
 				break;
			}// END: for() //
		if(uccj==64) return(CPROT_WR_OK);

		}// END: for(ucci) //

	return(CPROT_WR_BAD);
*/
return(CPROT_WR_OK);

	}/* END: SDLOD_burn_single_msg_pkt_on_SD() */






/**************************  ucSDLOD_run_loader()  ****************************
*
* Run the SD board loader
*
*
*	;The message pkt below is the end result buffer layout of the ram image.
*	;---------------------------;
*	;MSG_BUFF+0 = MSG TYPE		;
*	;---------------------------;
*	;MSG_BUFF+1 = DEST ADDR LO	; <------------
*	;MSG_BUFF+2 = DEST ADDR MD	;			   |
*	;MSG_BUFF+3 = DEST ADDR HI	;			   |
*	;---------------------------;			   |
*	;MSG_BUFF+4 = DATA0 VAL LO  ; Data start   |
*	;MSG_BUFF+5 = DATA0 VAL HI	;			   |----- ChkSum Calculation
*	;    .						;			   |      Start and End
*	;    .						;			   |
*	;    .						;			   |
*	;MSG_BUFF+66 = DATA31 VAL LO;			   |
*	;MSG_BUFF+67 = DATA31 VAL HI; <------------
*	;---------------------------;
*	;MSG_BUFF+68 = CHKSUM		;
*	;---------------------------;
*
*
* RET:	= 0 ok
*		= 1 ERR: spurious character during transmission
*		= 2 ERR: Timeout on data packet 
*
*
*****************************************************************************/

uchar ucSDLOD_run_loader(
		void
		)
	{
/*
	uint  uiChar;
	uchar ucChkSum;
	uchar ucBuffIdx;
	uchar ucRetVal;
	uchar ucBurnVal;
	uchar ucReadableFlg;

	ucRetVal = 0;						// assume an OK return //
	
	// DO ANY SETUP NECESSARY //
	vSDLOD_init();						// setup the SDLOD bits //
 	vSDLOD_enter_LVP_mode();			// ENTER LVP MODE //

	// SETUP THE CONFIG BITS //
	vSDLOD_setup_452_config_regs();

	// CHECK THE SD TO SEE IF ITS READABLE //
	ucReadableFlg = ucSDLOD_chk_for_SD_readable();

	// WAIT FOR IDENTIFY FROM PC //
	vBUTTON_reset();
	while(TRUE)		//lint !e774
		{
		if(ucSERIAL_kbhit())
			{
			if(ucSERIAL_bin() == CPROT_IDENT) break;	//wait for identifier

			// IF WE GET AN UNRECOGNIZEABLE CHAR -- BEEP //
			vBUZ_blink_buzzer(1);
			}

		if(ucBUTTON_isButtonFlgSet())	//button push can exit SDLOD
			{
			ucRetVal = 0;
			goto RLoader_xit;
			}

		} // END: while() //

	vSERIAL_bout(CPROT_ID_ACK);				//reply to identify


	// DO THE MAIN PROTOCOL LOOP HERE //
	while(TRUE)		//lint !e774
		{
		uiChar = uiSERIAL_timedBin(2000);	//read a char

		if(uiChar == CPROT_IDENT)
			{
			vSERIAL_bout(CPROT_ID_ACK);
			continue;
			}

		if(uiChar == CPROT_DONE)
			{
			vSERIAL_bout(CPROT_WR_OK);
			break;
			}
		
		if(uiChar != CPROT_WRITE)		// chk for a spurious char //
			{
			ucRetVal = 1;
			break;
			}

		// CHAR WAS A CPROT_WRITE //

		//  START WRITE PROTOCOL //

		// GET THE MAIN BODY OF DATA FROM PC //
		ucChkSum = 0;
		for(ucBuffIdx=1;  ucBuffIdx<=67;  ucBuffIdx++)
			{
			uiChar = uiSERIAL_timedBin(10000);
			if(uiChar > 255)
				{
				ucRetVal = 2;
				goto RLoader_xit;
				}
			ucaMSG_BUFF[ucBuffIdx] = (uchar)uiChar;
			ucChkSum += (uchar)uiChar;

			}// END: while() //

		// NOW GET THE CHKSUM //
		uiChar = uiSERIAL_timedBin(2000);		

		// CHECK IF CHKSUM WAS BAD //
		if(ucChkSum != uiChar)
			{
			vSERIAL_bout(CPROT_CHKSUM_BAD);
			continue;
			}
		vSERIAL_bout(CPROT_CHKSUM_OK);		//indicate chksum was OK

		// IF THE DEVICE IS LOCKED THE RETURN A BAD BURN IMMEDIATELY //
		if(!ucReadableFlg)
			{
			vSERIAL_bout(CPROT_WR_NOT_POSSIBLE);
			break;
			}


		// PUMP THE DATA TO THE SD BOARD //
		ucBurnVal = ucSDLOD_burn_single_msg_pkt_on_SD();

		// SEND THE REPLY CHAR //
		vSERIAL_bout(ucBurnVal);

		// EXIT IF WE HAVE A PROBLEM //
		if((ucBurnVal != CPROT_WR_OK) && (ucBurnVal != CPROT_WR_SAME)) break;

		}// END: while() //

RLoader_xit:

	// CLOSE UP THE SHOP //
	vSDLOD_leave_LVP_mode();			// EXIT LVP MODE //
	vSDLOD_quit();

	return(ucRetVal);
*/
return(0);

	}// END: ucSDLOD_run_loader() //



/* --------------------------  END of MODULE  ------------------------------- */

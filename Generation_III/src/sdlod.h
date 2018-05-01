
/***************************  SDLOD.H  ****************************************
*
* Header for SDLOD routine pkg
*
*
* Will likely need change! to make room for new mini SD's
*
* 
*
* V1.00 07/21/2006 wzr
*	started
*
******************************************************************************/


#ifndef SDLOD_H_INCLUDED
  #define SDLOD_H_INCLUDED




#define YES_ERASE_64BLK 	YES
#define  NO_ERASE_64BLK		NO





/* --------------  COMMUNICATION PROTOCOL DEFINITIONS  --------------------- */

#define BOOT2			1



#define CPROT_WRITE				0xE0 //(from PC) write command

#ifdef BOOT2
  #define CPROT_IDENT			0xEA //(from PC) identify cmd to get Boot 2 going
#else
  #define CPROT_IDENT			0xE1 //(from PC) (DEFAULT) identify cmd for Boot 1 start
#endif

#define CPROT_DONE				0xE2 //(from PC) done command
#define CPROT_READ				0xE3 //not used

#define CPROT_WR_OK				0xE4 //(from pic) good write ACK
#define CPROT_WR_BAD			0xE5 //(from pic) bad write ACK
#define CPROT_WR_SAME			0xEB //(from pic) there is an exact match between
									 //the blk send & the blk already burned
#define CPROT_WR_NOT_POSSIBLE	0xEC //(from pic) Device write protected

#define CPROT_CHKSUM_OK			0xE6 //(from pic) on good chksum
#define CPROT_CHKSUM_BAD		0xE7 //(from pic) on bad chksum

#define CPROT_ID_ACK			0xE8 //(from pic) IDENTIFY received ok

#define CPROT_READ_ACK			0xE9 //(from pic) not used




/*******************  LVP COMMAND VALUES  ************************************/
#define PROG_CORE						0x0		//0000
#define PROG_SHFT_OUT_TABLAT_REG		0x2		//0010

#define PROG_TBL_RD						0x8		//1000
#define PROG_TBL_RD_POST_INC			0x9		//1001
#define PROG_TBL_RD_POST_DEC			0xA		//1010
#define PROG_TBL_RD_PRE_INC				0xB		//1011

#define PROG_TBL_WR						0xC		//1100
#define PROG_TBL_WR_POST_INC_2			0xD		//1101
#define PROG_TBL_WR_POST_DEC_2			0xE		//1110
#define PROG_TBL_WR_ST_PRGMING			0xF		//1111




/*************************   COMMANDS  **************************************/
#define CMD_BSF_EECON1_EEPGD			0x8EA6
#define CMD_BCF_EECON1_EEPGD			0x9EA6

#define CMD_BSF_EECON1_CFGS				0x8CA6
#define CMD_BCF_EECON1_CFGS				0x9CA6

#define CMD_BSF_EECON1_WREN				0x84A6
#define CMD_BCF_EECON1_WREN				0x94A6

#define CMD_BSF_EECON1_FREE				0x88A6
#define CMD_BSF_EECON1_WR				0x82A6

#define CMD_MOVLW_I						0x0E00	//add + amt to this value

#define CMD_MOVWF_TBLPTRU				0x6EF8
#define CMD_MOVWF_TBLPTRH				0x6EF7
#define CMD_MOVWF_TBLPTRL				0x6EF6

#define CMD_MOVWF_TABLAT				0x6EF5

#define CMD_MOVWF_EECON2				0x6EA7

#define CMD_NOP							0x0000







void vSDLOD_init(
		void
		);

void vSDLOD_quit(
		void
		);

void vSDLOD_enter_LVP_mode(
		void
		);

void vSDLOD_leave_LVP_mode(
		void
		);

uchar ucSDLOD_bin(
		void
		);

void vSDLOD_bout(
		uchar ucOutData
		);

void vSDLOD_load_tblptr(
		usl uslAddr
		);

uchar ucSDLOD_XCT_single_read_cmd(
		uchar ucReadCmd4Bit
		);

void vSDLOD_XCT_single_write_cmd(
		uchar ucFourBitCmd,
		uint uiDataPayload
		);

void vSDLOD_burn_config_reg(
		usl uslAddr,
		uchar ucData
		);

void vSDLOD_write_64_byte_blk(
		usl uslAddr,
		uchar ucErase_flag	//YES_ERASE_64BLK  or  NO_ERASE_64BLK
		);

uchar ucSDLOD_burn_single_msg_pkt_on_SD( //RET=CPROT_WR_OK, CPROT_WR_BAD, CPROT_WR_SAME
		void
		);

uchar ucSDLOD_run_loader( //0= OK, >0 = ERR
		void
		);



#endif /* SDLOD_H_INCLUDED */


/* --------------------------  END of MODULE  ------------------------------- */


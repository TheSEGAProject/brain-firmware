

/**************************  MSG.C  *****************************************
*
* Routines to perform MSGs during events
*
*
* V1.00 10/04/2003 wzr
*		Started
*
******************************************************************************/

#include "DIAG.h"			//Diagnostic package

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e716 */		/* while(1)... */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e752 */		/* local declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


#include <msp430x54x.h>		//processor reg description */
#include "STD.h"			//standard defines
#include "CONFIG.h" 		//system configuration description file
//#include "MAIN.H"			//
#include "RADIO.h"			//radio routines
#include "MISC.H"			//homeless functions
//#include "READING.h"		//sensor reading module
//#include "AD.h" 			//AD module
#include "CRC.h"			//CRC calculation module
#include "L2SRAM.h"  		//disk storage module
#include "SERIAL.h"			//serial IO port stuff
#include "L2FRAM.h" 		//Level 2 Fram routines
//#include "SRAM.H"			//static ram routines
#include "MSG.h"    		//event MSG module
//#include "SDCTL.h" 		//SD board control routines
//#include "SDCOM.h" 		//SD communication package
//#include "L2FLASH.h"		//level 2 flash routines
#include "TIME.h"			//Time routines
//#include "ACTION.h" 		//Event action routines
//#include "DELAY.h"  		//Delay routines
//#include "RTS.h"			//Real time Scheduler
#include "GID.h"			//group ID routines
#include "SENSOR.h"			//sensor name routines

extern volatile uchar ucaMSG_BUFF[ MAX_RESERVED_MSG_SIZE ];

extern uchar ucGLOB_curMsgSeqNum;

extern unsigned int uiGLOB_curTripleOffset;


extern volatile union							//ucFLAG1_BYTE
		{
		uchar byte;

		struct
			{
			unsigned FLG1_X_DONE_BIT:		1;	//bit 0
			unsigned FLG1_X_LAST_BIT_BIT:	1;	//bit 1
			unsigned FLG1_X_FLAG_BIT:		1;	//bit 2 ;1=XMIT, 0=RECEIVE
			unsigned FLG1_R_HAVE_MSG_BIT:	1;	//bit 3	;1=REC has a msg, 0=no msg
			unsigned FLG1_R_CODE_PHASE_BIT: 1;	//bit 4 ;1=MSG PHASE, 0=BARKER PHASE
			unsigned FLG1_R_ABORT_BIT:		1;	//bit 5
			unsigned FLG1_X_NXT_LEVEL_BIT:	1;	//bit 6
			unsigned FLG1_R_SAMPLE_BIT: 	1;	//bit 7
			}FLAG1_STRUCT;

		}ucFLAG1_BYTE;


extern volatile union							//ucFLAG2_BYTE
		{
		uchar byte;

		struct
		 {
	  	 unsigned FLG2_T3_ALARM_MCH_BIT:1;		//bit 0 ;1=Alarm, 0=no alarm
		 unsigned FLG2_T1_ALARM_MCH_BIT:1;		//bit 1
		 unsigned FLG2_BUTTON_INT_BIT:1;		//bit 2 ;1=XMIT, 0=RECEIVE
		 unsigned FLG2_CLK_INT_BIT:1;			//bit 3	;1=clk ticked, 0=no tic
		 unsigned FLG2_X_FROM_MSG_BUFF_BIT:1;	//bit 4
		 unsigned FLG2_R_BUSY_BIT:1;			//bit 5 ;int: 1=REC BUSY, 0=IDLE
		 unsigned FLG2_R_BARKER_ODD_EVEN_BIT:1;	//bit 6 ;int: 1=odd, 0=even
		 unsigned FLG2_R_BITVAL_BIT:1;			//bit 7 ;int: 
		 }FLAG2_STRUCT;

		}ucFLAG2_BYTE;






const char *cpaMsgName[MSG_TYPE_MAX_COUNT] =
	{
	"NONE",			//0
	"STUP",			//1
	"DC1",			//2
	"DC2",			//3
	"DC3",			//4
	"OM1",			//5
	"OM2",			//6
	"OM3",			//7
	"DC4",			//8
	"DC5",			//9
	"DC6",			//10
	"TST1",			//11
	"TST2"			//12
	};





/*****************************  CODE STARTS HERE  ****************************/


/*********************** ucMSG_chkMsgIntegrity()  *************************
*
* This routine checks the message header in this order:
*		1. CRC
*		2. Message Type
*		3. Group ID
*		4. Dest SN
*		5. Src SN
*
*
*
* CheckByteBits:
*
*    7        6        5        4        3         2       1        0
*ÚÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄ¿
*³chk crc ³chk msg ³  chk   ³  chk   ³  chk   ³  chk   ³ unused ³ unused ³
*³        ³  type  ³ group  ³ group  ³dest SN ³src  SN ³        ³        ³
*³        ³        ³selector³  ID    ³        ³        ³        ³        ³
*ÀÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÙ
*
* ReportByteBits:
*
*    7        6        5        4        3         2       1        0
*ÚÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄ¿
*³ report ³ report ³ report ³ report ³ report ³ report ³ unused ³ unused ³
*³  CRC   ³msg type³grp  sel³group ID³dest SN ³src  SN ³        ³        ³
*³  err   ³  err   ³  err   ³  err   ³  err   ³  err   ³        ³        ³
*ÀÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÙ
*
* RET value:
*
*    7        6        5        4        3         2       1        0
*ÚÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄ¿
*³ found  ³ found  ³ found  ³ found  ³ found  ³ found  ³ unused ³ unused ³
*³  CRC   ³msg type³grp  sel³group ID³dest SN ³src  SN ³        ³        ³
*³  err   ³  err   ³  err   ³  err   ³  err   ³  err   ³        ³        ³
*ÀÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÙ
*
*
*****************************************************************************/

uchar ucMSG_chkMsgIntegrity(  //RET: BitMask if BAD,  0 if OK
		uchar ucChkByteBits,
		uchar ucReportByteBits,
		uchar ucMsgType,
		uint uiExpectedSrcSN,
		uint uiExpectedDestSN
		)
	{
	uchar ucErrRetVal;
	uint uiMsgDestSN;
	uint uiMsgSrcSN;

	ucErrRetVal = 0;								//assume no errors

	/* CHECK THE CRC -- IF ITS BAD -- LOOP BACK */
	if((ucChkByteBits & CHKBIT_CRC) &&
	   (!ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_REC)))
	   	{
	   	if(ucReportByteBits & CHKBIT_CRC)
	   		{
	   		vSERIAL_rom_sout("MSG:BdCRC\r\n"); 
	   		}
	   	ucErrRetVal |= CHKBIT_CRC;
	   	}


	/* CHECK FOR PROPER MSG TYPE */
	if((ucChkByteBits & CHKBIT_MSG_TYPE) &&
	   ((ucaMSG_BUFF[GMH_IDX_MSG_TYPE] & GMH_MSG_TYPE_MASK) != ucMsgType))
		{
		if(ucReportByteBits & CHKBIT_MSG_TYPE)
			{
			uchar ucGotMsgType;
			ucGotMsgType = ucaMSG_BUFF[GMH_IDX_MSG_TYPE] & GMH_MSG_TYPE_MASK;

			vSERIAL_rom_sout("MSG:MsgJammedExp ");
			//vSERIAL_UIV8out(ucMsgType);
			vSERIAL_rom_sout("(");
			if(ucMsgType < MSG_TYPE_MAX_COUNT)
				vSERIAL_rom_sout(cpaMsgName[ucMsgType]);
			vSERIAL_rom_sout(") got ");

			//vSERIAL_UIV8out(ucGotMsgType);
			vSERIAL_rom_sout("(");
			if(ucGotMsgType < MSG_TYPE_MAX_COUNT)
				vSERIAL_rom_sout(cpaMsgName[ucGotMsgType]);
			vSERIAL_rom_sout(")\r\n");
			}
		ucErrRetVal |= CHKBIT_MSG_TYPE;
		}



	/* CHECK FOR A GROUP SELECTOR MATCH */
	if((ucChkByteBits & CHKBIT_GRP_SEL) &&
	   (!ucGID_compareOnlySysGrpSelectToBytes(
	   							(uchar *)&ucaMSG_BUFF[GMH_IDX_GID_HI],
								ucReportByteBits & CHKBIT_GRP_SEL,  //report flag
								YES_CRLF
								)
	   )
	  )
		{
		ucErrRetVal |= CHKBIT_GRP_SEL;
		}



	/* CHECK FOR GROUP ID */
	if((ucChkByteBits & CHKBIT_GID) && (ucGID_getSysGrpSelectAsByte() == 0))
		{
		if(!ucGID_compareOnlySysGidToBytes(
								(uchar *)&ucaMSG_BUFF[GMH_IDX_GID_HI],
								ucReportByteBits & CHKBIT_GID,
								YES_CRLF
								)
		  )
			{
			ucErrRetVal |= CHKBIT_GID;
			}
		}



	/* CHECK DEST ID */
	if(ucChkByteBits & CHKBIT_DEST_SN)
		{
		uiMsgDestSN = uiMISC_buildUintFromBytes(
								(uchar *) &ucaMSG_BUFF[GMH_IDX_DEST_SN_HI],
								NO_NOINT
								);
		if(uiMsgDestSN != uiExpectedDestSN)
			{
			if(ucReportByteBits & CHKBIT_DEST_SN)
				{
				vSERIAL_rom_sout("MSG:BdDstSN ");  
				vMSG_showSNmismatch(uiExpectedDestSN,uiMsgDestSN, YES_CRLF);
				}
			ucErrRetVal |= CHKBIT_DEST_SN;
			}

		}/* END: if() */




	/* CHECK THE SOURCE SN */
	if(ucChkByteBits & CHKBIT_SRC_SN)
		{
		uiMsgSrcSN = uiMISC_buildUintFromBytes(
								(uchar *) &ucaMSG_BUFF[GMH_IDX_SRC_SN_HI],
								NO_NOINT
								);
		if(uiMsgSrcSN != uiExpectedSrcSN)
			{
			if(ucReportByteBits & CHKBIT_SRC_SN)
				{
				vSERIAL_rom_sout("MSG:BDSrcSN ");
				vMSG_showSNmismatch(uiExpectedSrcSN,uiMsgSrcSN,YES_CRLF);
				}
			ucErrRetVal |= CHKBIT_SRC_SN;
			}

		}/* END: if() */


	return(ucErrRetVal);

	}/* END: ucMSG_chkMsgIntegrity() */







/**********************  vMSG_showSNmismatch()  ***********************
*
* Show the values of a mismatch
*
*
*****************************************************************************/
void vMSG_showSNmismatch(
		uint uiExpectedVal,
		uint uiGotVal,
		uchar ucCRLF_flag				//YES_CRLF, NO_CRLF
		)
	{
	vSERIAL_rom_sout("EXP= #");
	vSERIAL_UIV16out(uiExpectedVal);
	vSERIAL_rom_sout(" GOT= #");			
	vSERIAL_UIV16out(uiGotVal);
	if(ucCRLF_flag)	vSERIAL_crlf();

	return;

	}/* END: vMSG_showSNmismatch() */








/*******************  vMSG_showStorageErr()  ********************************
*
*
*
*****************************************************************************/

void vMSG_showStorageErr(
		const char *cpLeadinMsg,
		unsigned long ulFailAddr,
		unsigned long ulWroteVal,
		unsigned long ulReadVal
		)
	{

	vSERIAL_rom_sout(cpLeadinMsg);
	vSERIAL_rom_sout(" at ");
	vSERIAL_HB32out(ulFailAddr);
//	vSERIAL_crlf();
	vSERIAL_rom_sout(" Wrote ");
	vSERIAL_HB32out(ulWroteVal);
	vSERIAL_rom_sout(" read ");
	vSERIAL_HB32out(ulReadVal);
	vSERIAL_rom_sout(" xor= ");
	vSERIAL_HB32out(ulWroteVal ^ ulReadVal);
	vSERIAL_crlf();

	}/* END: vMSG_showStorageErr() */






/************************  vMSG_buildMsgHdr_GENERIC() ***************************
*
* This routine builds an GENERIC msg header but has the following fixups:
*	1. ucaMSG_BUFF[OM2_IDX_EOM_IDX] only has the header size+2 in it
*	3. CRC bytes are not stuffed. (don't yet know where they are).
*
******************************************************************************/

void vMSG_buildMsgHdr_GENERIC(//MSG=Len,Type,Group,Src,Dest
		uchar ucMsgLen,
		uchar ucMsgType,
		uint uiDestSN
		)
	{
	/* BUILD THE MSG HEADER */

	/* STUFF HEADER SIZE+2 ONLY */
	ucaMSG_BUFF[GMH_IDX_EOM_IDX] = ucMsgLen;

	/* STUFF MSG TYPE */
	ucaMSG_BUFF[GMH_IDX_MSG_TYPE] = ucMsgType;

	/* STUFF THE GROUP ID NUMBER */
	ucaMSG_BUFF[GMH_IDX_GID_HI] = ucGID_getWholeSysGidHiByte();
	ucaMSG_BUFF[GMH_IDX_GID_LO] = ucGID_getWholeSysGidLoByte();

	/* STUFF THE SOURCE NUMBER */
	vL2FRAM_copySnumLo16ToBytes((uchar *)&ucaMSG_BUFF[GMH_IDX_SRC_SN_HI]);

	/* STUFF THE DESTINATION */
	vMISC_copyUintIntoBytes(
					uiDestSN,
					(uchar *)&ucaMSG_BUFF[GMH_IDX_DEST_SN_HI],
					NO_NOINT
					);

	return;

	}/* END: vMSG_buildMsgHdr_GENERIC() */






#if 0
/************************  vMSG_OM2_buildMsgHdr() ***************************
*
* This routine builds an OM2 msg header but has the following fixups:
*	1. ucaMSG_BUFF[OM2_IDX_EOM_IDX] only has the header size+2 in it
*	2. The destination bytes are 0.
*	3. Collection agent is the same as the source.
*	4. CRC bytes are not stuffed. (don't yet know where they are).
*
******************************************************************************/

void vMSG_OM2_buildMsgHdr(
		uint uiDestSN
		)
	{
	/* BUILD THE MSG */

	vMSG_buildMsgHdr_GENERIC(
							OM2_IDX_COLLECTION_TIME_LO+2,
							MSG_TYPE_OM2,
							uiDestSN
							);

	/* STUFF THE MSG SEQ NUMBER */
	ucaMSG_BUFF[OM2_IDX_MSG_SEQ_NUM] = ucMSG_incMsgSeqNum();

	/* STUFF THE LOAD FACTOR */
	ucaMSG_BUFF[OM2_IDX_LFACTOR] = 0;

	/* STUFF THE COLLECTING AGENT NUMBER */
	vL2FRAM_copySnumLo16ToBytes((uchar *)&ucaMSG_BUFF[OM2_IDX_AGENT_NUM_HI]);

	/* INSERT THE CURRENT TIME AS THE COLLECTION TIME */
	vTIME_copySysTimeToBytes(
		(uchar *)&ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_XI]
		);

	return;

	}/* END: vMSG_OM2_buildMsgHdr() */
#endif







/**********************  ucMSG_waitForMsgOrTimeout() *****************************
*
* This routine assumes a clk alarm has already been setup.
*
* RET:	1 = GOT A MSG
*		0 = Timed out
*
******************************************************************************/

uchar ucMSG_waitForMsgOrTimeout(
		void
		)
	{

	/* WAIT FOR MESSAGE COMPLETE */

	while(TRUE)					//lint !e774
		{

		if(ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT) break;

		if(ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT) break;

		if(ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT) return(1);

		}/* END: while() */

	return(0);

	}/* END: ucMSG_waitForMsgOrTimeout()*/







/***************************  vMSG_rec_obnd_msgs()  *******************************
*
* Receive the Outbound Messages.
*
*
******************************************************************************/

void vMSG_rec_obnd_msgs(
		void
		)
	{

	return;

	}/* END: vMSG_rec_obnd_msgs() */






/***********************  vMSG_showMsgBuffer() *********************************
*
* Print out the message buffer
*
******************************************************************************/

void vMSG_showMsgBuffer(
		uchar ucCRLF_termFlag,		//YES_CRLF, NO_CRLF
		uchar ucShowTypeFlag		//SHOW_MSG_RAW, SHOW_MSG_COOKED
		)
	{
	uchar ucii;
	uchar ucjj;
	uchar ucMsgSize;
	uchar ucByteCount;
	char *cpLeadStrPtr;
	char *cpTrailStrPtr;

	vSERIAL_rom_sout("MSG: "); 

//	ucMsgSize = MAX_MSG_SIZE;
	ucMsgSize = ucaMSG_BUFF[0] & MAX_MSG_SIZE_MASK;
	ucMsgSize++;
	if(ucMsgSize < 16) ucMsgSize=16;

	if(ucShowTypeFlag == SHOW_MSG_RAW) ucMsgSize = MAX_MSG_SIZE;

	for(ucii=0;  ucii<ucMsgSize;  ucii++)		//loop for all bytes
		{
		/* SETUP DEFAULTS TO SAVE CODE */
		ucByteCount = 1;
		cpLeadStrPtr = "";
		cpTrailStrPtr = ",";

		if(ucii != ucMsgSize-2)			//if CRC skip usual stuff
			{
			switch(ucii)
				{
				case 0:
					cpLeadStrPtr = "SZ=";
					//DEFAULT: ucByteCount = 1;
					//DEFAULT: cpTrailStrPtr = ",";
					break;	

				case 1:
					cpLeadStrPtr = "ID=";
					//DEFAULT: ucByteCount = 1;
					//DEFAULT: cpTrailStrPtr = ",";
					break;	

				case 2:
					cpLeadStrPtr = "GP=";
					ucByteCount = 2;
					//DEFAULT: cpTrailStrPtr = ",";
					break;	

				case 4:
					cpLeadStrPtr = "SRC=";
					ucByteCount = 2;
					//DEFAULT: cpTrailStrPtr = ",";
					break;	

				case 6:
					cpLeadStrPtr = "DST=";
					ucByteCount = 2;
					//DEFAULT: cpTrailStrPtr = ",";
					break;	

				case 8:
					cpLeadStrPtr = "SEQ=";
					//DEFAULT: ucByteCount = 1;
					//DEFAULT: cpTrailStrPtr = ",";
					break;	

				case 9:
					cpLeadStrPtr = "LD=";
					//DEFAULT: ucByteCount = 1;
					//DEFAULT: cpTrailStrPtr = ",";
					break;	

				case 10:
					cpLeadStrPtr = "AGNT=";
					ucByteCount = 2;
					//DEFAULT: cpTrailStrPtr = ",";
					break;	

				case 12:
					cpLeadStrPtr = "T=";		//time
					ucByteCount = 4;
					cpTrailStrPtr = ",\r\n";
					break;	

				case 16:
				case 19:
				case 22:
				case 25:
				case 28:
					/* ASSUME RAW SETUP */
					//DEFAULT: ucByteCount = 1;
					cpLeadStrPtr = "  ";
					cpTrailStrPtr = "";

					/* IF COOKED CHANGE THE SETUP */
					if(ucShowTypeFlag == SHOW_MSG_COOKED)
						{
						vSENSOR_showSensorName(ucaMSG_BUFF[ucii], R_JUSTIFY);
						cpLeadStrPtr = "(";
						cpTrailStrPtr = ")";
						}

					break;

				case 17:		//Sensor Data value
				case 20:
				case 23:
				case 26:
				case 29:
					cpLeadStrPtr = "=>";
					//DEFAULT: cpTrailStrPtr = ",";
					//DEFAULT: ucByteCount = 1;
					if(ucii != 29) ucByteCount = 2;
					if(ucShowTypeFlag == SHOW_MSG_COOKED) cpTrailStrPtr = ",";
					break;

				default:
					//DEFAULT: cpLeadStrPtr = "";
					//DEFAULT: ucByteCount = 1;
					//DEFAULT: cpTrailStrPtr = ",";
					break;

				}/* END: switch(ucii) */

			/* SHOW THE LEAD STRING PTR */
			vSERIAL_rom_sout(cpLeadStrPtr);

			/* SHOW THE BYTES IN HEX */
			for(ucjj=0;  ucjj<ucByteCount;  ucjj++)
				{
				vSERIAL_HB8out(ucaMSG_BUFF[ucii+ucjj]);
				}

			/* BUMP THE COUNT OVER THE SHOW BYTES */
			ucii += (ucByteCount-1);

			/* SHOW THE TRAILING STRING PTR */
			vSERIAL_rom_sout(cpTrailStrPtr);

			continue;

			}/* END: if() */

		/* SHOW THE FINAL CRC WHERE EVER IT IS */
		vSERIAL_rom_sout("  CRC=");
		vSERIAL_HB8out(ucaMSG_BUFF[ucii]);
		vSERIAL_HB8out(ucaMSG_BUFF[++ucii]);
		vSERIAL_crlf();

		}/* END: for(ucii) */

	if(ucCRLF_termFlag)
		vSERIAL_crlf();

	}/* END: vMSG_showMsgBuffer() */





/**********************  ucMSG_doSubSecXmit()  *******************************
*
* RET:	1 if msg sent on time
*		0 if msg not send (too late to send it)
*
*****************************************************************************/

uchar ucMSG_doSubSecXmit(		//RET: 1-sent, 0-too late
		uchar ucpSendTime[6],	//full start time (IN CLKS)
		uchar ucClkChoiceFlag,	//USE_CLK2, USE_CLK1
		uchar ucStartRecFlag	//YES_RECEIVER_START, NO_RECEIVER_START
		)
	{
	long lCurSec;
	long lSendTimeSec;


	/* CHECK THE START SEC AGAINST CUR TIME */
	if(ucClkChoiceFlag == USE_CLK1)
		{
		lCurSec = lTIME_getSysTimeAsLong();			//wups was SysTime clk
		}
	else
		{
		lCurSec = lTIME_getClk2AsLong();			//clk2
		}

	lSendTimeSec = (long)ulMISC_buildUlongFromBytes(&ucpSendTime[0], NO_NOINT);

	if(lCurSec > lSendTimeSec)
		{
		#if 1
		vSERIAL_rom_sout("MSG:MissdMsgTmWas ");
		vSERIAL_HBV32out((ulong)lCurSec);
		vSERIAL_rom_sout("  wanted ");
		vSERIAL_HBV32out((ulong)lSendTimeSec);
		vSERIAL_crlf();
		#endif
		return(0);
		}

	/* WAIT FOR FULL SEC TIC TO COME UP */
	if(ucClkChoiceFlag == USE_CLK2)
		{
		while(lSendTimeSec > lTIME_getClk2AsLong()); //lint !e722
		}
	else
		{
		while(lSendTimeSec > lTIME_getSysTimeAsLong()); //lint !e722
		}
	

	/* WAIT FOR SUB SECOND TIC TO COME UP */
	while(ucpSendTime[4] > SUB_SEC_TIM_H);			//SUB_SEC_HI
	while(ucpSendTime[5] > SUB_SEC_TIM_L);			//SUB_SEC_LO


	#if 0
	vSERIAL_rom_sout("MSG:BeforRadioXmt= ");
	vTIME_ShowWholeSysTimeInDuS(YES_CRLF);
	#endif

	/* SEND THE MSG */
	vRADIO_xmit_msg();

	#if 0
	vSERIAL_rom_sout("MSG:AfterRadioXmt= ");
	vTIME_ShowWholeSysTimeInDuS(YES_CRLF);
	#endif


	/* CHECK FOR IMMEDIATE START OF RECEIVER */
	if(ucStartRecFlag)	vRADIO_start_receiver();

	#if 0
	vSERIAL_rom_sout("MSG:AfterRadioRec= ");
	vTIME_ShowWholeSysTimeInDuS(YES_CRLF);
	#endif


	return(1);

	}/* vMSG_doSubSecXmit() */







/************************  vMSG_stuffFakeMsgToSRAM()  ***************************
*
* Builds a fake OM2 message and stuffs it into the SRAM
*
*
******************************************************************************/
void vMSG_stuffFakeMsgToSRAM(
		void
		)
	{

	ucaMSG_BUFF[OM2_IDX_EOM_IDX] = OM2_MSG_LAST_BYTE_NUM_UC;
	ucaMSG_BUFF[OM2_IDX_MSG_TYPE] = MSG_TYPE_OM2;

	vGID_copyWholeSysGidToBytes((uchar *)&ucaMSG_BUFF[GMH_IDX_GID_HI]);

	vL2FRAM_copySnumLo16ToBytes((uchar *)&ucaMSG_BUFF[OM2_IDX_SRC_SN_HI]);

	vL2FRAM_copySnumLo16ToBytes((uchar *)&ucaMSG_BUFF[OM2_IDX_DEST_SN_HI]);

	ucaMSG_BUFF[OM2_IDX_MSG_SEQ_NUM] = 0x50;

	/* STUFF THE LINK BYTE */
	ucaMSG_BUFF[OM2_IDX_GENERIC_LINK_BYTE] = 0;

	vL2FRAM_copySnumLo16ToBytes((uchar *)&ucaMSG_BUFF[OM2_IDX_AGENT_NUM_HI]);

	ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_XI] = 0;
	ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_HI] = 0;
	ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_MD] = 0;
	ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_LO] = 99;

	ucaMSG_BUFF[OM2_IDX_DATA_0_HI+0] = 0x1;
	ucaMSG_BUFF[OM2_IDX_DATA_0_LO+0] = 0x50;

	ucaMSG_BUFF[OM2_IDX_DATA_0_HI+2] = 0x1;
	ucaMSG_BUFF[OM2_IDX_DATA_0_LO+2] = 0x51;

	ucaMSG_BUFF[OM2_IDX_DATA_0_HI+4] = 0x1;
	ucaMSG_BUFF[OM2_IDX_DATA_0_LO+4] = 0x52;

	ucaMSG_BUFF[OM2_IDX_DATA_0_HI+4] = 0x1;
	ucaMSG_BUFF[OM2_IDX_DATA_0_LO+4] = 0x53;

	ucaMSG_BUFF[OM2_IDX_DATA_0_HI+6] = 0x1;
	ucaMSG_BUFF[OM2_IDX_DATA_0_LO+6] = 0x54;

	ucaMSG_BUFF[OM2_IDX_DATA_0_HI+8] = 0x1;
	ucaMSG_BUFF[OM2_IDX_DATA_0_LO+8] = 0x55;

	ucaMSG_BUFF[OM2_IDX_DATA_0_HI+10] = 0x1;
	ucaMSG_BUFF[OM2_IDX_DATA_0_LO+10] = 0x56;

	/* COMPUTE THE CRC */
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND); //lint !e534 //compute CRC

	/* STUFF THE MSG TO THE DISK */
	vL2SRAM_storeMsgToSram();

	#if 0
	vSERIAL_rom_sout("FakeMsgCnt="); 
	vSERIAL_HB16out(uiL2SRAM_getMsgCount());
	vSERIAL_crlf();

	vSERIAL_rom_sout("Vacancy="); 
	vSERIAL_HB16out(uiL2SRAM_getVacantMsgCount());
	vSERIAL_crlf();

	vSERIAL_rom_sout("FakeMsgBeforeStore\r\n"); 
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);

	ucL2SRAM_getCopyOfCurMsg(); //lint !e534

	vSERIAL_rom_sout("FakeMsgAfterRetrieve\r\n"); 
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);

	#endif

	return;

	}/* vMSG_stuffFakeMsgToSRAM() */






/************************  vMSG_checkSingleMsgBuffEntry()  **********************************
*
* Report an error in the msg buffer
*
*
******************************************************************************/

static void vMSG_checkSingleMsgBuffEntry(
		uint uiMsgIdx,
		uchar ucCorrectValue
		)
	{

	if(ucaMSG_BUFF[uiMsgIdx] != ucCorrectValue)
		{
		vSERIAL_rom_sout("Byt ");
		vSERIAL_HB8out((uchar) uiMsgIdx);
		vSERIAL_rom_sout(" bd,needed ");
		vSERIAL_HB8out(ucCorrectValue);
		vSERIAL_rom_sout(" got ");
		vSERIAL_HB8out(ucaMSG_BUFF[uiMsgIdx]);
		vSERIAL_crlf();
		}

	return;

	}/* END: vMSG_checkSingleMsgBuffEntry() */









/************************  vMSG_checkFakeMsg()  **********************************
*
* Check a fake OM2 message
*
*
******************************************************************************/

void vMSG_checkFakeMsg(
		void
		)
	{
	vSERIAL_rom_sout("vChkMsg \r\n");


	vMSG_checkSingleMsgBuffEntry(OM2_IDX_EOM_IDX,  OM2_MSG_LAST_BYTE_NUM_UC);
	vMSG_checkSingleMsgBuffEntry(OM2_IDX_MSG_TYPE, MSG_TYPE_OM2);

	vMSG_checkSingleMsgBuffEntry(GMH_IDX_GID_HI, ucGID_getWholeSysGidHiByte());
	vMSG_checkSingleMsgBuffEntry(GMH_IDX_GID_LO, ucGID_getWholeSysGidLoByte());

	vMSG_checkSingleMsgBuffEntry(OM2_IDX_SRC_SN_HI, ucL2FRAM_getSnumMd8());
	vMSG_checkSingleMsgBuffEntry(OM2_IDX_SRC_SN_LO, ucL2FRAM_getSnumLo8());

	vMSG_checkSingleMsgBuffEntry(OM2_IDX_DEST_SN_HI, ucL2FRAM_getSnumMd8());
	vMSG_checkSingleMsgBuffEntry(OM2_IDX_DEST_SN_LO, ucL2FRAM_getSnumLo8());

	vMSG_checkSingleMsgBuffEntry(OM2_IDX_MSG_SEQ_NUM, 0x50);

	vMSG_checkSingleMsgBuffEntry(OM2_IDX_GENERIC_LINK_BYTE, 0x00);

	vMSG_checkSingleMsgBuffEntry(OM2_IDX_AGENT_NUM_HI, ucL2FRAM_getSnumMd8());
	vMSG_checkSingleMsgBuffEntry(OM2_IDX_AGENT_NUM_LO, ucL2FRAM_getSnumLo8());

	vMSG_checkSingleMsgBuffEntry(OM2_IDX_COLLECTION_TIME_XI, 0);
	vMSG_checkSingleMsgBuffEntry(OM2_IDX_COLLECTION_TIME_HI, 0);
	vMSG_checkSingleMsgBuffEntry(OM2_IDX_COLLECTION_TIME_MD, 0);
	vMSG_checkSingleMsgBuffEntry(OM2_IDX_COLLECTION_TIME_LO, 99);

	vMSG_checkSingleMsgBuffEntry(OM2_IDX_DATA_0_HI+0, 0x1);
	vMSG_checkSingleMsgBuffEntry(OM2_IDX_DATA_0_LO+0, 0x50);

	vMSG_checkSingleMsgBuffEntry(OM2_IDX_DATA_0_HI+2, 0x1);
	vMSG_checkSingleMsgBuffEntry(OM2_IDX_DATA_0_LO+2, 0x51);

	vMSG_checkSingleMsgBuffEntry(OM2_IDX_DATA_0_HI+4, 0x1);
	vMSG_checkSingleMsgBuffEntry(OM2_IDX_DATA_0_LO+4, 0x52);

	vMSG_checkSingleMsgBuffEntry(OM2_IDX_DATA_0_HI+6, 0x1);
	vMSG_checkSingleMsgBuffEntry(OM2_IDX_DATA_0_LO+6, 0x53);

	vMSG_checkSingleMsgBuffEntry(OM2_IDX_DATA_0_HI+8, 0x1);
	vMSG_checkSingleMsgBuffEntry(OM2_IDX_DATA_0_LO+8, 0x54);

	vMSG_checkSingleMsgBuffEntry(OM2_IDX_DATA_0_HI+10, 0x1);
	vMSG_checkSingleMsgBuffEntry(OM2_IDX_DATA_0_LO+10, 0x55);

	vMSG_checkSingleMsgBuffEntry(OM2_IDX_DATA_0_HI+12, 0x1);
	vMSG_checkSingleMsgBuffEntry(OM2_IDX_DATA_0_LO+12, 0x56);

	return;

	}/* END: vMSG_checkFakeMsg() */







/************************  vMSG_incMsgSeqNum()  ******************************
*
* Increment the message seq number
*
* NOTE: The Seq number cannot be 0 or 255
*
* RET: Incremented Msg Seq Num
*
******************************************************************************/

uchar ucMSG_incMsgSeqNum( //RET: Incremented Msg Seq Num (not 0 or 255)
		void
		)
	{
	ucGLOB_curMsgSeqNum++;

	/* NOT 0 AND NOT 255 */
	if(ucGLOB_curMsgSeqNum >= 255) ucGLOB_curMsgSeqNum = 1;

	return(ucGLOB_curMsgSeqNum);

	}/* END: vMSG_incMsgSeqNum() */






/********************  ucMSG_getLastFilledEntryInOM2()  **********************
*
* RET:	 0 = illegal
*		## = index for Last entry in the msg (pts to sensor ID)
*
******************************************************************************/
uchar ucMSG_getLastFilledEntryInOM2( //RET: 0=none, ##=idx of last entry in OM2
		void
		)
	{
	uchar ucMsgLastDataStart;

	/* GET AN INDEX FOR THE LAST BEGINNING OF THE LAST ENTRY IN THE MSG */
	ucMsgLastDataStart = (ucaMSG_BUFF[0] & MAX_MSG_SIZE_MASK) - 4;


	/* NOW CHECK THAT END COUNT TO MAKE SURE ITS OK */
	switch(ucMsgLastDataStart)
		{
		case 16:						//normal msg lengths
		case 19:
		case 22:
		case 25:
			break;

		case 27:
			ucMsgLastDataStart = 25;	//had a short msg also
			break;

		default:
			ucMsgLastDataStart = 0;		//illegal length
			break;

		}/* END: switch() */

	return(ucMsgLastDataStart);

	}/* END: ucMSG_getLastFilledEntryInOM2() */







/*-------------------------------  MODULE END  ------------------------------*/

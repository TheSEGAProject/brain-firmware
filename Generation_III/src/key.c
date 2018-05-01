
/**************************  KEY.C  ******************************************
*
* Routines to accept keyboard inputs.
*
*
* V1.00 10/14/2003 wzr
*		Started
*
******************************************************************************/



/*************************  CMD TREE  ****************************************
*
* Button ÄÄÂÄ0 Quit
*  Push    ³
*          ÃÄ 1 Help
*          ³
*          ÃÄ 2 Show Tbls
*          ³
*          ÃÄ 3 Date Set
*          ³
*          ÃÄ 4 Time Set
*          ³
*          ÃÄ 5 Upload to PC
*          ³
*          ÃÄ 6 ModifyID
*          ³                 
*          ³                 
*          ÃÄ 7 ModifyOptions ÄÄÂÄ0 Quit
*          ³   (MODOPT.C)       ³
*          ³                    ÃÄ1 Help
*          ³                    ³
*          ³                    ÃÄ2 Show Option Bits
*          ³                    ³
*          ³                    ÃÄ3 Set Option Bit
*          ³                    ³
*          ³                    ÃÄ4 Clear Option Bit
*          ³                    ³
*          ³                    ÀÄ5 Set Role
*          ³    
*          ³    
*          ÃÄ 8 ModifyStorage ÄÄÂÄ0 Quit
*          ³   (MODSTORE.C)     ³
*          ³                    ÃÄ1 Help
*          ³                    ³
*          ³                    ÃÄ2 Show Flash Tables
*          ³                    ³
*          ³                    ÃÄ3 Delete Old Flash
*          ³                    ³
*          ³                    ÃÄ4 WindBack Flash Ptr
*          ³                    ³
*          ³                    ÀÄ5 Zero Flash
*          ³  
*          ³  
*          ÃÄ 9 ModifyAction  ÄÄÂÄ0 Quit
*          ³   (MODACT.C)       ³
*          ³                    ÃÄ1 Help
*          ³                    ³
*          ³                    ÃÄ2 Show Actn Tables
*          ³                    ³
*          ³                    ÃÄ3 Add Actn
*          ³                    ³
*          ³                    ÃÄ4 Delete Actn
*          ³                    ³
*          ³                    ÃÄ5 Modify Actn Interval
*          ³                    ³
*          ³                    ÃÄ6 Modify Actn Act
*          ³                    ³
*          ³                    ÀÄ7 Modify Actn YTrigger
*          ³  
*          ÃÄ10 Restart
*          ³  
*          ÃÄ11 Diagnostics
*          ³  
*          ÀÄ12 Load SD 
*              
*
**************************  CMD TREE  ****************************************/

/*lint -e526 */     /* function not defined */
/*lint -e657 */    	/* Unusual (nonportable) anonymous struct or union */
///*lint -e714 */ 	/* symbol not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
///*lint -e758 */ 	/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


#include "diag.h"			//diagnostic defines
//#include "p18f8720.h"		//processor reg description */
#include "std.h"			//standard definitions
#include "config.h"			//system configuration description file
#include "main.h"			//
#include "misc.h"			//homeless functions
//#include "ad.h" 			//AD module
#include "crc.h"			//CRC calculation module
#include "serial.h"			//serial IO port stuff
//#include "sram.h"			//Level 1 static ram routines
#include "l2sram.h"			//Level 2 static ram routines
#include "l2flash.h"		//Level 2 flash routines
//#include "fram.h"			//level 1 FRAM routines
#include "l2fram.h"			//Level 2 FRAM routines
#include "time.h"			//system time routines
#include "msg.h"			//msg routines
//#include "rts.h"			//real time scheduler
#include "stbl.h"			//Schedule table routines
#include "fulldiag.h"		//runtime diag
//#include "cmd.h"			//command table routines
#include "rad40.h"			//radix 40 routines
#include "MODACT.h"			//Keyboard action table commands
#include "daytime.h"		//Daytime routines
#include "sdctl.h"  		//SD board control module
#include "sensor.h"			//Sensor routines
#include "senseact.h"		//Sense Act name routines
#include "sdlod.h"			//SD downloader routines
#include "buz.h"			//buzzer control code
#include "MODOPT.h"		//Rom Role Flag Handler Routines
#include "MODSTORE.h"		//Modify Storage tables	
#include "numcmd.h"			//numbered command routines
#include "key.h"			//Top level Key commands

#ifdef FAKE_UPLOAD_MSGS
#if (FAKE_UPLOAD_MSGS == YES)
  #include "opmode.h"			//Operational mode routines
#endif
#endif

#ifdef FAKE_UPLOAD_MSGS
#if (FAKE_UPLOAD_MSGS == YES)
  #include "action.h"			//Action routines
#endif
#endif

#ifdef INC_ESPORT						//defined on Cmd line
#ifdef USE_ESPORT_OUTPUT
#if (USE_ESPORT_OUTPUT == YES)
  #include "esport.h"
#endif
#endif
#endif




#define UPLOAD_FROM_NOBODY	0
#define UPLOAD_FROM_SRAM	1
#define UPLOAD_FROM_FLASH	2
#define UPLOAD_FROM_FAKE	3





/*********************  EXTERNS  *********************************************/


extern const char *cpRad40Tbl;
extern volatile uchar ucaMSG_BUFF[ MAX_RESERVED_MSG_SIZE ];

extern uint uiGLOB_bad_flash_CRC_count;	//count of bad CRC's on flash msgs


/**********************  TABLES  *********************************************/

#define CMD_ARRAY_SIZE 13

/* FUNCTION DELCARATIONS (in order of numbering )*/
//     void vKEY_exitFuncPlaceHolder(void);		// 0 Quit
static void vKEY_showHelp(void);				// 1 Help 
static void vKEY_showTables(void);				// 2 Show Tables
static void vKEY_setDate(void);					// 3 Set Date
static void vKEY_setTime(void);					// 4 Set Time
static void vKEY_uploadDataToPC(void);			// 5 Upload data to PC		
static void vKEY_IDchange(void);				// 6 Modify the Wizard ID	 
//     void vMODOPT_cmdModifyOptionTbls(void);	// 7 Modify the Wizard Role 
//     void vMODSTORE_modifyStorageTbls(void);	// 8 Modify Storage Tables
//     void vMODACT_modifyActionTbls(void);		// 9 Modify Actions Entrys
static void vKEY_restartSys(void);				//10 Restart the system
static void vKEY_diagnostics(void);             //11 Run Test Routines
static void vKEY_SDlod(void);					//12 Load new SD code

const GENERIC_CMD_FUNC vCMD_func_ptrArray[CMD_ARRAY_SIZE] =
 {
 vKEY_exitFuncPlaceHolder,			// 0 Quit
 vKEY_showHelp,						// 1 Help 
 vKEY_showTables,					// 2 Show Tables
 vKEY_setDate,						// 3 Set Date
 vKEY_setTime,						// 4 Set Time
 vKEY_uploadDataToPC,				// 5 Upload data to PC			  
 vKEY_IDchange,						// 6 Modify the Wizard ID					  
 vMODOPT_cmdModifyOptionTbls,		// 7 Modify the Wizard Options
 vMODSTORE_modifyStorageTbls,		// 8 Modify Storage Tables
 vMODACT_modifyActionTbl,			// 9 Modify Actions Entrys
 vKEY_restartSys,					//10 Restart the system
 vKEY_diagnostics,					//11 Run Test Routines
 vKEY_SDlod							//12 Load new SD code
 };

const char *cpaCmdText[CMD_ARRAY_SIZE] = 
 {
 "Quit",							// 0 Quit
 "Help",							// 1 Help 
 "ShowTbls",						// 2 Show Tables
 "DateSet",							// 3 Set Date
 "TimeSet",							// 4 Set Time
 "UploadDataToPC",					// 5 Upload data to PC		
 "ModifyID",						// 6 Modify the Wizard ID	
 "ModifyOptionTbls",				// 7 Modify the Wizard Role
 "ModifyStorageTbls",				// 8 Modify Storage Tables
 "ModifyActionTbls",				// 9 Modify Actions Entrys
 "RestartSys",						//10 Restart the system
 "Diagnostics",						//11 Run Test Routines
 "LoadNewSDcode"					//12 Load new SD code
 }; /* END: cpaCmdText[] */




/********************  DECLARATIONS  *****************************************/

void vKEY_doUploadToPC(
		uchar ucUploadType	//UPLOAD_FROM_SRAM,  UPLOAD_FROM_FLASH
		);

void vKEY_showCurTime(
		void
		);




/***************************  CODE  ******************************************/



/************************  ucKEY_doKeyboardCmdInput() *************************
*
* Handle the keyboard commands
*
* RET: == 0  restart
*		> 0  continue in operational mode
*
******************************************************************************/

uchar ucKEY_doKeyboardCmdInput(
		void
		)

	{
	long lLimitTime;

	vKEY_showHelp();

	/* OUTPUT THE COMMAND PROMPT */
	vSERIAL_rom_sout("\r\n>");					

	/* WAIT FOR A KEY HIT */
	lLimitTime = lTIME_getSysTimeAsLong() + 10L;
	while(TRUE)
		{
		if(ucSERIAL_kbhit()) break;
		if(lTIME_getSysTimeAsLong() > lLimitTime)
			{
			vSERIAL_rom_sout("ToutRetToSys\r\n");
			return(1);
			}
		}


	vNUMCMD_doCmdInputAndDispatch(
				'>',						//prompt
				&cpaCmdText[0],				//command array
				&vCMD_func_ptrArray[0],		//dispatch function ptrs
				CMD_ARRAY_SIZE,				//array size
				NO_SHOW_INITIAL_PROMPT		//initial prompt flag
				);


	return(1);

	}/* END: uchar ucKEY_doKeyboardCmdInput() */





/*******************  vKEY_exitFuncPlaceHolder()  ****************************
*
*
*
******************************************************************************/
void vKEY_exitFuncPlaceHolder(
		void
		)
	{
	return;

	}/* END: vKEY_exitFuncPlaceHolder() */






/*******************  vKEY_showHelp()  ****************************************
*
*
*
******************************************************************************/
void vKEY_showHelp(
		void
		)
	{
	vKEY_showFlashMsgCounts();
	vSERIAL_crlf();
	vNUMCMD_showCmdList(cpaCmdText, CMD_ARRAY_SIZE);
	vSERIAL_crlf();

	return;

	}/* END: vKEY_showHelp() */






/*********************  vKEY_restartSys()  ***********************************
*
*
******************************************************************************/
static void vKEY_restartSys(
		void
		)
	{
	if(!ucSERIAL_confirm(YES_SHOW_ABORT_MSG)) return;


	}/* END: vKEY_restartSys() */




#if 0
/********************  vKEY_restoreFRAM()  ***********************************
*
*
*
*****************************************************************************/
void vKEY_restoreFRAM(
		void
		)
	{
	vSERIAL_rom_sout("RsetToOriginalConfig\r\n");

	if(!ucSERIAL_confirm(YES_SHOW_ABORT_MSG)) return;

	vL2FRAM_format_fram();

	return;

	}/* END: vKEY_restoreFRAM() */

#endif





/*******************  vKEY_diagnostics()  ****************************************
*
*
*
******************************************************************************/
void vKEY_diagnostics(
		void
		)
	{
	vFULLDIAG_run_module();

	return;

	}/* END: vKEY_diagnostics() */





/*******************  vKEY_showTables()  ****************************************
*
*
*
******************************************************************************/
void vKEY_showTables(
		void
		)
	{

	/* SHOW THE NAME AND THE VERSION NUMBER */
	vMAIN_printIntro();

	/* TELL HOW MANY MESSAGES THERE ARE IN SRAM AND FLASH */
	{
	ulong ulMsgCount;
	ulMsgCount = (ulong)uiL2SRAM_getMsgCount();
	vSERIAL_rom_sout("SramMsgs:New= ");
	vSERIAL_UIV32out(ulMsgCount);
	vSERIAL_bout(',');
	/* SHOW THE FLASH MSG COUNTS */
	vKEY_showFlashMsgCounts();
	vSERIAL_crlf();
	}

	/* SHOW THE FLASH MSG COUNTS */
	vKEY_showFlashMsgCounts();
	vSERIAL_crlf();

	/* SHOW THE SCHED TABLE */
	vSTBL_showAllStblEntrys();

	#if 1
	/* SHOW THE FRAM ST TBL */
	vL2FRAM_showStBlkTbl();
	vSERIAL_dash(10);
	vSERIAL_crlf();

	/* SHOW THE FRAM TRIGGER TBL */
	vL2FRAM_showYtriggerTbl();
	vSERIAL_dash(10);
	vSERIAL_crlf();

	#endif


	return;

	}/* END: vKEY_showTables() */









/*******************  vKEY_setDate()  ****************************************
*
*
*
******************************************************************************/
static void vKEY_setDate(
		void
		)
	{
	DATE_STRUCT sDate;
	TIME_STRUCT sTime;
	uchar ucaStr[12];


	/* GET THE CURRENT RAM TIME */
	vDAYTIME_convertSecsToDstructAndTstruct(
		lTIME_getSysTimeAsLong(),
		&sDate,
		&sTime
		);

	vSERIAL_rom_sout("SysDate= ");
	vDAYTIME_convertDstructToShow(&sDate, NUMERIC_FORM);
	vSERIAL_crlf();

	/* GET THE CURRENT ECLK TIME */
	if(ucTIME_getECLKsts(YES_SKP_ECLK_RAM_FLAG) >= 3)
		{
		vDAYTIME_convertSecsToDstructAndTstruct(
			lTIME_getECLKtimeAsLong(),
			&sDate,
			&sTime
			);
		vSERIAL_rom_sout("EclkDate= ");
		vDAYTIME_convertDstructToShow(&sDate, NUMERIC_FORM);
		vSERIAL_crlf();
		}
	else
		{
		vSERIAL_rom_sout("NoEclkDetectedOnThisSys\r\n");
		}


	vSERIAL_rom_sout("EntrDat(MM/DD/YY): ");
	if(ucSERIAL_getEditedInput(ucaStr,12))
		{
		vSERIAL_showXXXmsg();
		return;
		}

	if(cDAYTIME_convertDstrToDstruct(ucaStr, &sDate) < 0)
		{
		vSERIAL_showXXXmsg();
		return;
		}

	/* SET THE TIME */
	vDAYTIME_convertDstructAndTstructToSetSysTime(&sDate, &sTime);


	/* GET THE CURRENT RAM TIME */
	vDAYTIME_convertSecsToDstructAndTstruct(
		lTIME_getSysTimeAsLong(),
		&sDate,
		&sTime
		);

	vSERIAL_rom_sout("SysDate= ");
	vDAYTIME_convertDstructToShow(&sDate, NUMERIC_FORM);
	vSERIAL_crlf();

	return;

	}/* END: vKEY_setDate() */







/*******************  vKEY_setTime()  ****************************************
*
*
*
******************************************************************************/
static void vKEY_setTime(
		void
		)
	{
	DATE_STRUCT sDate;
	TIME_STRUCT sTime;
	uchar ucaStr[12];


	vSERIAL_rom_sout("Tim= ");

	vDAYTIME_convertSecsToDstructAndTstruct(
		lTIME_getSysTimeAsLong(),
		&sDate,
		&sTime
		);

	vDAYTIME_convertTstructToShow(&sTime);
	vSERIAL_crlf();

	vSERIAL_rom_sout("EnterTim(HH:MM:SS): ");
	if(ucSERIAL_getEditedInput(ucaStr,12))
		{
		vSERIAL_showXXXmsg();
		return;
		}

	if(cDAYTIME_convertTstrToTstruct(ucaStr, &sTime) < 0)
		{
		vSERIAL_showXXXmsg();
		return;
		}

	vDAYTIME_convertDstructAndTstructToSetSysTime(&sDate, &sTime);

	return;

	}/* END: vKEY_setTime() */





/*******************  vKEY_uploadDataToPC()  *********************************
*
*
*
******************************************************************************/
static void vKEY_uploadDataToPC(
		void
		)
	{
	uchar ucUploadTypeFlag;
	uchar ucaStr[4];

	vKEY_showFlashMsgCounts();

	vSERIAL_rom_sout(" UploadSram(S)orFlsh(F)? ");
	ucSERIAL_getEditedInput(&ucaStr[0], 2);		//lint !e534
	ucUploadTypeFlag = UPLOAD_FROM_NOBODY;
	if((ucaStr[0] == 's') || (ucaStr[0] == 'S')) ucUploadTypeFlag = UPLOAD_FROM_SRAM;
	if((ucaStr[0] == 'f') || (ucaStr[0] == 'F')) ucUploadTypeFlag = UPLOAD_FROM_FLASH;
	if(ucUploadTypeFlag == UPLOAD_FROM_NOBODY)
		{
		vSERIAL_showXXXmsg();
		return;
		}

	vSERIAL_rom_sout("\r\n ");
	vKEY_doUploadToPC(ucUploadTypeFlag);
	return;

	}/* END: vKEY_uploadDataToPC() */







/*******************  vKEY_clrMsgTail()  ************************************
*
* This routine zros the remainder of the msg buffer from the passed IDX to 31
*
*****************************************************************************/
void vKEY_clrMsgTail(
		uchar ucFirstFreeIdx
		)
	{
	uchar ucc;

	if(ucFirstFreeIdx >= 31) return;			//skip if full size

	for(ucc=ucFirstFreeIdx; ucc<=31;  ucc++)
		{
		ucaMSG_BUFF[ucc] = 0;

		}/* END: for(ucc) */

	return;

	}/* END: vKEY_clrMsgTail() */




/*******************  vKEY_checkAndRepairFlashMsgs()  ************************
*
* This routine is called from the PC upload routine.
* This routine is here because the FLASH memory tends to corrupt the data.
* This routine tries to repair minor obvious errors and preps the repaired
* message to be sent to the PC.
*
* NOTE: This routine assumes that there is a valid (if somewhat corrupted)
*		message in the msg buffer.
*
* NOTE: We know that there are no foreign messages in the FLASH unless this
*		unit is functioning as a hub.
*
* NOTE: If an error is found in the msg a brain message is sent in the
*		Loc 28 & 29 of the msg packet (Short data only) area.
*
*****************************************************************************/
void vKEY_checkAndRepairFlashMsgs(
		void
		)
	{
	uchar ucErrFlag;/*0=OK,1=BDCRC,2=BDSIZ1,3=BDSIZ2,4=BDTYPE,5=BDSRC,6=BDAGNT */
					/*7=BDSENSOR */
	uint uiIntVal1;
	uint uiThisSN;
	uchar ucc;


	ucErrFlag = 0;						//assume no errors in msg
	uiThisSN = uiL2FRAM_getSnumLo16AsUint(); //get a copy of current SN

	/* FIRST CHECK TO SEE IF THIS MESSAGE IS CURRENTLY CORRUPTED */
	if(!ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_REC))
		{
		/* CRC ERR -- FLAG THIS MSG */
		uiGLOB_bad_flash_CRC_count++;	//inc system err count
		/* No correction is here because stuffing the errmsg will recompute CRC */
		ucErrFlag = FLASH_MSG_ERR_CRCERR;			//set err flag

		}/* END if() */

	/* NOW CHECK FOR OTHER OBVIOUS CURRUPTIONS */ 
	ucaMSG_BUFF[0] &= MAX_MSG_SIZE_MASK;	//clr all transmission artifacts
	
	/* CHECK THE MSG FOR TOO SMALL */
	if(ucaMSG_BUFF[0] < 20)
		{
		ucaMSG_BUFF[0] = 20;				//force the minimum size
		if(ucErrFlag == 0) ucErrFlag = FLASH_MSG_ERR_SIZTOOSMALL;
		}

	/* FORCE THE MSG TO A VALID LENGTH = 20,23,26,29,31 */
	switch(ucaMSG_BUFF[0])
		{
		case 21:
		case 22:
			ucaMSG_BUFF[0] = 20;				//correct length
			if(ucErrFlag == 0) ucErrFlag = FLASH_MSG_ERR_BADSIZ2122;
			break;

		case 24:
		case 25:
			ucaMSG_BUFF[0] = 23;				//correct length
			if(ucErrFlag == 0) ucErrFlag = FLASH_MSG_ERR_BADSIZ2425;
			break;

		case 27:
		case 28:
			ucaMSG_BUFF[0] = 26;				//correct length
			if(ucErrFlag == 0) ucErrFlag = FLASH_MSG_ERR_BADSIZ2728;
			break;

		case 30:
			ucaMSG_BUFF[0] = 29;				//correct length
			if(ucErrFlag == 0) ucErrFlag = FLASH_MSG_ERR_BADSIZ30;
			break;

		default:
			break;

		}/* END: switch() */


	/* CLEAR THE REMAINDER OF THE MSG */
	vKEY_clrMsgTail(ucaMSG_BUFF[0]-1);		//clr CRC and rest of msg

	
	/* CHECK THE MSG FOR THE WRONG MSG TYPE */
	if(ucaMSG_BUFF[OM2_IDX_MSG_TYPE] != MSG_TYPE_OM2)
		{
		ucaMSG_BUFF[OM2_IDX_MSG_TYPE] = MSG_TYPE_OM2;
		if(ucErrFlag == 0) ucErrFlag = FLASH_MSG_ERR_BADTYPE;
		}

	/* CHECK THE SEQ NUMBER (0 & 255 NOT ALLOWED) */
	if((ucaMSG_BUFF[OM2_IDX_MSG_SEQ_NUM] == 0) ||
	   (ucaMSG_BUFF[OM2_IDX_MSG_SEQ_NUM] == 255))
		{
		ucaMSG_BUFF[OM2_IDX_MSG_SEQ_NUM] = 1;
		if(ucErrFlag == 0) ucErrFlag = FLASH_MSG_ERR_BADSEQNUM;
		}

	/* NOT A HUB: CHK THE SOURCE -- IT MUST BE THIS SN */
	/* NOT A HUB: CHK THE AGENT  -- IT MUST BE THIS SN */
	if(!ucL2FRAM_isHub())
		{
		uiIntVal1 = uiMISC_buildUintFromBytes(
						(uchar *)&ucaMSG_BUFF[OM2_IDX_SRC_SN_HI],
						NO_NOINT
						);
		if(uiIntVal1 != uiThisSN)
			{
			/* CORRECT THE SRC ID */
			vMISC_copyUintIntoBytes(
					uiThisSN,
					(uchar *)&ucaMSG_BUFF[OM2_IDX_SRC_SN_HI],
					NO_NOINT
					);
			if(ucErrFlag == 0) ucErrFlag = FLASH_MSG_ERR_BADSRCID;
			}



		/* NOT A HUB: CHECK THE AGENT -- IT MUST BE THIS SN */
		uiIntVal1 = uiMISC_buildUintFromBytes(
						(uchar *)&ucaMSG_BUFF[OM2_IDX_AGENT_NUM_HI],
						NO_NOINT
						);
		if(uiIntVal1 != uiThisSN)
			{
			/* CORRECT THE AGENT ID */
			vMISC_copyUintIntoBytes(
					uiThisSN,
					(uchar *)&ucaMSG_BUFF[OM2_IDX_AGENT_NUM_HI],
					NO_NOINT
					);
			if(ucErrFlag == 0) ucErrFlag = FLASH_MSG_ERR_BADAGTID;
			}

		}/* END: if(!ucL2FRAM_isHub()) */






	/* IS HUB: DEST MUST BE THIS SN */
	if(ucL2FRAM_isHub())							//only for hubs
		{
		uiIntVal1 = uiMISC_buildUintFromBytes(
						(uchar *)&ucaMSG_BUFF[OM2_IDX_DEST_SN_HI],
						NO_NOINT
						);
		if((uiIntVal1 != uiThisSN) && (uiIntVal1 != 0))
			{
			/* CORRECT THE DEST ID */
			vMISC_copyUintIntoBytes(
					uiThisSN,
					(uchar *)&ucaMSG_BUFF[OM2_IDX_DEST_SN_HI],
					NO_NOINT
					);
			if(ucErrFlag == 0) ucErrFlag = FLASH_MSG_ERR_BADDESTID;
			}

		}/* END: if(ucL2FRAM_isHub()) */




	/* CHECK FOR ILLEGAL COLLECTION TIME */
	if(ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_XI] & 0x80)	//negative time
		{
		ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_XI] = 0;
		ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_HI] = 0;
		ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_MD] = 0;
		ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_LO] = 0;

		if(ucErrFlag == 0) ucErrFlag = FLASH_MSG_ERR_BADTIME;
		}


	/* CHECK THE SENSOR NUMBER RANGE */
	for(ucc=16; ucc<=28; ucc+=3)
		{
		if(ucaMSG_BUFF[ucc] >= SENSOR_MAX_VALUE)
			{
			if(ucaMSG_BUFF[0] >= ucc+4)
				{
				if(ucErrFlag == 0) ucErrFlag = FLASH_MSG_ERR_BADSENSORNUM;

				#if 0
				#ifdef ESPORT_ENABLED			//defined in DIAG.h
				vESPORT_redirectOutput(REDIRECT_TO_ESPORT);
				vSERIAL_rom_sout("Snsr[");
				vSERIAL_UIV8out(ucc);
				vSERIAL_rom_sout("]=");
				vSERIAL_UIV8out(ucaMSG_BUFF[ucc]);
				vSERIAL_rom_sout("  Size=");
				vSERIAL_UIV8out(ucaMSG_BUFF[0]);
				vSERIAL_crlf();
				vESPORT_redirectOutput(REDIRECT_TO_SERIAL);
				#endif
				#endif
				}
			ucaMSG_BUFF[ucc] = 0;		//zro the sensor
			ucaMSG_BUFF[ucc+1] = 0;		//zro sensor data HI
			if(ucc != 28)
				ucaMSG_BUFF[ucc+2] = 0;	//zro sensor data LO
			}
		}/* END: for() */


	/* DO NOT ALLOW A DOUBLE READING IN LAST SENSOR POSITION */
	if(ucSENSEACT_getSensorDataSize(ucaMSG_BUFF[OM2_IDX_DATA_3_SENSOR_NUM]) >= 2)
		{
		ucaMSG_BUFF[OM2_IDX_DATA_3_SENSOR_NUM] = 0;
		ucaMSG_BUFF[OM2_IDX_DATA_3_HI] = 0;
		ucaMSG_BUFF[OM2_IDX_DATA_3_LO] = 0;
		if(ucErrFlag == 0) ucErrFlag = FLASH_MSG_ERR_BADSENSORINLASTENTRY;
		}


	/* IF ANYTHING WAS WRONG WITH THE MSG THEN PUT A ERR MSG IN THE MSG */
	if(ucErrFlag)
		{
		/* ADD A CRC ERR MSG TO THE END OF THIS PACKET */
		ucaMSG_BUFF[OM2_IDX_DATA_4_SENSOR_NUM] = SENSOR_BR_BOARD_MSG; //sensor #
		ucaMSG_BUFF[OM2_IDX_DATA_4_LO] = ucErrFlag;	//msg 
		ucaMSG_BUFF[0] = 31;								//new size

		#if 0
		#ifdef ESPORT_ENABLED			//defined in DIAG.h
		vESPORT_redirectOutput(REDIRECT_TO_ESPORT);
		//vSERIAL_rom_sout("Err= ");
		vSERIAL_UIV8out(ucErrFlag);
		//vSERIAL_crlf();
		vESPORT_redirectOutput(REDIRECT_TO_SERIAL);
		#endif
		#endif
		}

	/* RE-COMPUTE THE CRC */
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534 //compute the CRC

	return;

	}/* END: vKEY_checkAndRepairFlashMsgs() */






/**********************  vKEY_doUploadToPC()  *******************************
*
* upload the messages to the PC.
*
*****************************************************************************/

void vKEY_doUploadToPC(
		uchar ucUploadType	//UPLOAD_FROM_SRAM,  UPLOAD_FROM_FLASH
		)
	{
	#define UPLD_BEG_TRANS	0xE1
	#define UPLD_MSG_OK		0xE2
	#define UPLD_MSG_RESEND	0xE3
	#define UPLD_END_TRANS	0xE4

	uint uiChar;
	uchar ucTmp;
	uchar uci;
	ulong ulMsgCount;
	ulong uli;

	#ifdef FAKE_UPLOAD_MSGS
	#if (FAKE_UPLOAD_MSGS == YES)
	ulong ulRetryCounter;
	#endif
	#endif

	#ifdef FAKE_UPLOAD_MSGS
	#if (FAKE_UPLOAD_MSGS == YES)
	ucUploadType = UPLOAD_FROM_FAKE;
	#endif
	#endif

	#ifdef ESPORT_ENABLED			//defined in DIAG.h
//	vESPORT_redirectOutput(REDIRECT_TO_ESPORT);
//	vSERIAL_rom_sout("\r\nE:Uplod\r\n");
//	vESPORT_redirectOutput(REDIRECT_TO_SERIAL);
	#endif

	/* CHECK IF WE HAVE ANY MSGS TO SEND */
	ulMsgCount = 0;
	switch(ucUploadType)
		{
		case UPLOAD_FROM_SRAM:
			ulMsgCount = (ulong)uiL2SRAM_getMsgCount();
			break;

		case UPLOAD_FROM_FLASH:
			ulMsgCount = (ulong)lL2FRAM_getFlashUnreadMsgCount();
			break;

		#ifdef FAKE_UPLOAD_MSGS
		#if (FAKE_UPLOAD_MSGS == YES)
		case UPLOAD_FROM_FAKE:
			ulMsgCount = 50UL;
			break;
		#endif
		#endif

		default:
			vSERIAL_rom_sout("BdUplodTyp\r\n");
			ulMsgCount = 0;
			break;

		}/* END: switch() */

	if(ulMsgCount == 0)
		{
		vSERIAL_rom_sout("NoMsgsFnd\r\n");
		vSERIAL_rom_sout("PicExitUplod\r\n\r\n");
		return;
		}

	vSERIAL_rom_sout("\r\nMsgCnt= ");
	vSERIAL_UIV32out(ulMsgCount);
	vSERIAL_crlf();



	/*--------------  START THE UPLOAD PROCEDURE  --------------------------*/

	/* SEND A START CODE */
	vSERIAL_setBinaryMode(BINARY_MODE_ON);	//enter binary mode
	vSERIAL_bout(UPLD_BEG_TRANS);		//startup a transmission
	vSERIAL_bout(UPLD_BEG_TRANS);


	/* WAIT FOR A START SEND SIGNAL */
	uiChar = uiSERIAL_timedBin(9000);	
	if(uiChar != UPLD_BEG_TRANS)
		{
		vSERIAL_rom_sout("KEY:ToutOnSt\r\n");
		goto doUpload_xit;
		}


	/* START THE TRANSMISSION */	
	#ifdef FAKE_UPLOAD_MSGS
	#if (FAKE_UPLOAD_MSGS == YES)
	ulRetryCounter = 0;
	#endif
	#endif

	#ifdef ESPORT_ENABLED			//defined in DIAG.h
//	vESPORT_redirectOutput(REDIRECT_TO_ESPORT);
//	vSERIAL_rom_sout("ST:Cnt= ");
//	vSERIAL_UIV32out(ulMsgCount);
//	vSERIAL_crlf();
//	vESPORT_redirectOutput(REDIRECT_TO_SERIAL);
	#endif

	for(uli=0;  uli<ulMsgCount; )		//loop for all messages in buffer
		{
		/* LOAD A PACKET INTO MSG BUFFER */
		if(uiChar != UPLD_MSG_RESEND)
			{
			switch(ucUploadType)
				{
				case UPLOAD_FROM_SRAM:
					ucTmp = ucL2SRAM_getCopyOfCurMsg();
					if(ucTmp == 0) goto doUpload_xit;
					ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534
					break;

				case UPLOAD_FROM_FLASH:
					vL2FLASH_getNextMsgFromFlash();
					vKEY_checkAndRepairFlashMsgs(); //correct obvious errors
					break;

				#ifdef FAKE_UPLOAD_MSGS
				#if (FAKE_UPLOAD_MSGS == YES)
				case UPLOAD_FROM_FAKE:
					/* BUILD A FAKE OM2 */
					vOPMODE_buildMsg_OM2(
						0,						//Flags
						uiRAD40_rom_convertStrToRad40("FK1"),//Dest Serial num
						(uchar)uli,				//Msg Seq Num
						0,						//Load Factor
						uiRAD40_rom_convertStrToRad40("FK2"),//Agent SN
						0x00001500L,			//Collection Time
						2,						//Data Count
						SENSOR_LIGHT_1,			//Sensor 0 number
						0x0150,					//Sensor 0 data
						SENSOR_LIGHT_2,			//Sensor 1 number
						0x0250,					//Sensor 1 data
						0,						//Sensor 2 number
						0,						//Sensor 2 data
						0,						//Sensor 3 number
						0						//Sensor 3 data
						);

					/* NOTE: CRC has been computed for this msg by build */

					break;

				#endif
				#endif

				default:
					ulMsgCount = 0; //for loop break;
					break;

				}/* END: switch() */

			}/* END: if() */


		#ifdef FAKE_UPLOAD_MSGS
		#if (FAKE_UPLOAD_MSGS == YES)
		{
		uchar ucSavedByte;

		/* PERFORM SPECIFIC FAULTS TO TEST THE RECOVERY PROCEDURES */
		if(uli == 10)
			{
			if(ulRetryCounter == 0) ucSavedByte = ucaMSG_BUFF[0];

			#if 0	//msg fail size underrun
			ucaMSG_BUFF[0] = ((ucaMSG_BUFF[0] & 0xE0) | ((ucaMSG_BUFF[0] & 0x3F)+1));//lint !e564
			#endif

			#if 1	//msg fail size too small
			ucaMSG_BUFF[0] = (ucaMSG_BUFF[0] & 0xE0) | 13;
			#endif

			#if 0	//msg fail size too big
			ucaMSG_BUFF[0] = (ucaMSG_BUFF[0] & 0xC0) | 32;
			#endif

			if(ulRetryCounter == 3) ucaMSG_BUFF[0] = ucSavedByte; //lint !e644

			}/* END: if() */
		}
		#endif
		#endif



		/* SEND THE PACKET TO THE PC*/
		for(uci=0;
			uci<=(ucaMSG_BUFF[GMH_IDX_EOM_IDX] & MAX_MSG_SIZE_MASK);
			uci++
		   )
			{
			vSERIAL_bout(ucaMSG_BUFF[uci]);
			}

		/* WAIT FOR A PACKET OK MSG */
		uiChar = uiSERIAL_timedBin(10000);
		if((uiChar != UPLD_MSG_OK) && (uiChar != UPLD_MSG_RESEND))
			{
			#ifdef ESPORT_ENABLED			//defined in DIAG.h
//			vESPORT_redirectOutput(REDIRECT_TO_ESPORT);
//			vSERIAL_rom_sout("PktOk=Bd= ");
//			vSERIAL_HB16out(uiChar);
//			vSERIAL_crlf();
//			vESPORT_redirectOutput(REDIRECT_TO_SERIAL);
			#endif

			goto doUpload_xit;
			}

		/* IF IT WAS A RESEND THEN LOOP BACK WITHOUT A NEW MSG */
		if(uiChar == UPLD_MSG_RESEND)
			{
			#ifdef FAKE_UPLOAD_MSGS
			#if (FAKE_UPLOAD_MSGS == YES)
			ulRetryCounter++;
			#endif
			#endif
			continue;
			}

		/* MSG WAS SENT COMPLETED --  DELETE THIS MSG AND GET THE NEXT ONE */
		if(ucUploadType == UPLOAD_FROM_SRAM) vL2SRAM_delCurMsg();

		/* BUMP THE MSG COUNTER */
		uli++;

		#ifdef FAKE_UPLOAD_MSGS
		#if (FAKE_UPLOAD_MSGS == YES)
		ulRetryCounter = 0;				//reset counter
		#endif
		#endif

		}/* END: for(uli) */


	#ifdef ESPORT_ENABLED			//defined in DIAG.h
//	vESPORT_redirectOutput(REDIRECT_TO_ESPORT);
//	vSERIAL_rom_sout("LpXit\r\n");
//	vESPORT_redirectOutput(REDIRECT_TO_SERIAL);
	#endif


doUpload_xit:

	#ifdef ESPORT_ENABLED			//defined in DIAG.h
//	vESPORT_redirectOutput(REDIRECT_TO_ESPORT);
//	vSERIAL_rom_sout("EotSnt\r\n");
//	vSERIAL_rom_sout("XT:MsgCnt= ");
//	vSERIAL_UIV32out(ulMsgCount);
//	vSERIAL_rom_sout("   uli= ");
//	vSERIAL_UIV32out(uli);				//lint !e644
//	vSERIAL_crlf();
//	vESPORT_redirectOutput(REDIRECT_TO_SERIAL);
	#endif

	vSERIAL_bout(UPLD_END_TRANS);	//force an end

	uiChar = uiSERIAL_timedBin(3000);
	if(uiChar != UPLD_END_TRANS)
		{
		vSERIAL_rom_sout("NoEndReply\r\n");
		}

	vSERIAL_setBinaryMode(BINARY_MODE_OFF);
	vSERIAL_rom_sout("PicXitingUplod\r\n\r\n");
	vKEY_showFlashMsgCounts();

	vSERIAL_rom_sout("MrkedFlshMsgs= ");
	vSERIAL_UIV16out(uiGLOB_bad_flash_CRC_count);
	vSERIAL_crlf();

	return;

	}/* END: vKEY_doUploadToPC() */






/***************************  vKEY_showCurTime()  ********************************
*
* Show the current time of day
*
******************************************************************************/

void vKEY_showCurTime(
		void
		)
	{
	long lLongCurTime;
//	long lCurTimeSinceFirstHr;
	long lCurHr;
	long lCurMin;
	long lCurSec;
	long lQuotient;


	/* GRAB THE CURRENT TIME */
	lLongCurTime = lTIME_getSysTimeAsLong();

	lCurSec = lLongCurTime % 60;		//get the seconds
	lQuotient = lLongCurTime / 60;		//quotient units in minutes
	lCurMin = lQuotient % 60;			//get the minutes
	lQuotient = lQuotient / 60;			//quotient units in hours
	lCurHr = lQuotient % 24;			//get the hours

	vSERIAL_rom_sout("UpTim= ");		
	vSERIAL_UIV16out((unsigned int)lCurHr);
	vSERIAL_bout(':');
	vSERIAL_UIV8out((uchar) lCurMin);
	vSERIAL_bout(':');
	vSERIAL_UIV8out((uchar) lCurSec);

	vSERIAL_rom_sout("   Sec= ");  
	vSERIAL_I32out(lLongCurTime);
	vSERIAL_crlf();

//	lCurTimeSinceFirstHr = lLongCurTime - (long)iGLOB_Hr0_to_SysTim0_inSec;

	return;

	}/* END: vKEY_showCurTime() */




/*****************  vKEY_showFlashMsgCounts()  ********************************
*
*
*
******************************************************************************/
void vKEY_showFlashMsgCounts(
		void
		)
	{
	vSERIAL_rom_sout("FlshMsgs:Old= ");
	vSERIAL_IV32out(lL2FRAM_getFlashReadMsgCount());
	vSERIAL_rom_sout(",New= ");
	vSERIAL_UIV32out((ulong)lL2FRAM_getFlashUnreadMsgCount());
	vSERIAL_rom_sout(",Free= ");
	vSERIAL_IV32out(lL2FRAM_getFlashFreeMsgCount());
	vSERIAL_crlf();

	return;

	}/* END: vKEY_showFlashMsgcounts() */






/*****************  vKEY_IDchange()  *****************************************
*
*
*
******************************************************************************/
void vKEY_IDchange(
		void
		)
	{
	char cStr[5];
	uint uiRad40Val;

	vSERIAL_rom_sout("IdChrs=  ");
	vSERIAL_rom_sout(cpRad40Tbl+1);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_rom_sout("CurId= ");
	vL2FRAM_showSysID();
	vSERIAL_crlf();

	vSERIAL_rom_sout("NewId(3ChrMax): ");
	if(ucSERIAL_getEditedInput((uchar *)cStr,4))
		{
		vSERIAL_showXXXmsg();
		goto IDchange_exit;
		}


	uiRad40Val = uiRAD40_ram_convertStrToRad40(cStr);
	vL2FRAM_setSysID(uiRad40Val);

IDchange_exit:
	vSERIAL_rom_sout("Id= ");
	vL2FRAM_showSysID();
	vSERIAL_crlf();

	return;

	}/* END: vKEY_IDchange() */







/*******************  vKEY_versionNumHeader()  *******************************
*
*
*
******************************************************************************/
void vKEY_versionNumHeader(
		void
		)
	{

	/* SHOW THE NAME AND THE VERSION NUMBER */
	vMAIN_printIntro();

	return;

	}/* END: vKEY_versionNumHeader() */







/*****************  vKEY_SDlod()  ********************************************
*
*
*
******************************************************************************/
void vKEY_SDlod(
		void
		)
	{
	uchar ucLdrRet;

	vSERIAL_rom_sout("SDlod ");
	if(!ucSERIAL_confirm(YES_SHOW_ABORT_MSG))
		{
		return;
		}

	vSERIAL_rom_sout("SDlodUp...\r\n");
	vBUZ_blink_buzzer(4);

	ucLdrRet = ucSDLOD_run_loader();
	if(ucLdrRet != 0) return;			//don't try to start the SD if its bad


	vBUZ_blink_buzzer(2);

	ucSDCTL_start_SD_and_specify_boot(SD_BOOT_LOADER_DISABLED); //lint !e534

	return;

	}/* END: vKEY_SDlod() */



/*-------------------------------  MODULE END  ------------------------------*/

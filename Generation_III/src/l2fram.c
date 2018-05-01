
/**************************  L2FRAM.C  *****************************************
*
* Level 2 Routines to read and write the FERRO-RAM chip
*
*
* V1.00 12/05/2003 wzr
*	Started
*
******************************************************************************/

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


#include "DIAG.h"			//diagnostic defines
#include "STD.h"			//standard defines
#include "config.h" 		//configuration parameters 
#include "SERIAL.h"			//serial port routines
//#include "MAIN.H"			//
//#include "MISC.H"			//homeless functions
#include "FRAM.h"			//Ferro ram memory functions
#include "L2FRAM.h" 		//Level 2 FRAM handler routines
#include "ROM.H" 			//Rom configuration data
#include "L2FLASH.h"		//level 2 flash routines
#include "MSG.h"			//msg routines
#include "ACTION.h"			//action routines
//#include "L2SRAM.h" 		//static ram level 2 routines
#include "RAD40.h"			//radix 40 routines
#include "TIME.h"			//system type routines
#include "SENSEACT.h"		//sense act name routines
#include "SENSOR.h"			//sensor name
//#include "PICK.h"			//pick a sensor point routines
#include "MODOPT.h" 		//role flags for wiz routines


/**********************  DEFINES  *******************************************/

 #define FRAM_ID_ADDR_XI						0	//4 bytes
 #define FRAM_ID_ADDR_HI						1
 #define FRAM_ID_ADDR_MD						2
 #define FRAM_ID_ADDR_LO						3
	#define FRAM_ID_VAL_XI							'F'
	#define FRAM_ID_VAL_HI							'R'
	#define FRAM_ID_VAL_MD							'A'
	#define FRAM_ID_VAL_LO							'M'

 #define FRAM_VER_ADDR							4	//2 bytes 
 #define FRAM_VER_ADDR_HI						4
 #define FRAM_VER_ADDR_LO						5
//	 #define FRAM_VERSION_HI		0x02
//	 #define FRAM_VERSION_LO		0x08
//	 #define FRAM_VERSION (((uint)FRAM_VERSION_HI<<8) | ((uint)FRAM_VERSION_LO))

// #define FRAM_TEST_ADDR 						6	//4 bytes
//	#define FRAM_TEST_ADDR_XI		6
//	#define FRAM_TEST_ADDR_HI		7
//	#define FRAM_TEST_ADDR_MD		8
//	#define FRAM_TEST_ADDR_LO		9

	#define FRAM_TEST_VAL_POSITIVE_UL	0xCC118855
	#define FRAM_TEST_VAL_NEGATIVE_UL	(~FRAM_TEST_VAL_POSITIVE_UL)




/**********************  EXTERNS  ********************************************/


extern const uchar ucaSampRecFlags[E_ACTN_MAX_NUM_IDX];			//(in STBL.C)
extern const uint  uiaSampRecStart[E_ACTN_MAX_NUM_IDX];			//(in STBL.C)
extern const uint  uiaSampRecInterval[E_ACTN_MAX_NUM_IDX];		//(in STBL.C)
extern const ulong ulaSenseActVal[E_ACTN_MAX_NUM_IDX];			//(in STBL.C)
extern const uchar ucaDefaultStartupAction[E_ACTN_MAX_NUM_IDX];	//(in STBL.C)
extern const uchar ucaWhoCanRunBits[E_ACTN_MAX_NUM_IDX];		//(in STBL.C)
extern const uchar ucaBitMask[8];								//(in MODOPT.C)



/**********************  DECLARATIONS  ***************************************/

static long lL2FRAM_incAndWrapPtr(
		long lPtr
		);


/**********************  ucL2FRAM_chk_for_fram()  *****************************
*
* This routine:
*	1. turns on the FRAM
*	2. Tests the FRAM test locations to make sure SPI and FRAM are working
*	3. shuts off the FRAM
*
* RET:	1 = good
*		0 = bad
*
*******************************************************************************/

uchar ucL2FRAM_chk_for_fram(
		uchar ucReportMode	//FRAM_CHK_SILENT_MODE, FRAM_CHK_REPORT_MODE
		)
	{
	uchar ucRetVal;
	ulong ulTestVal;

	/* ASSUME THE RET VAL IS GOOD */
	ucRetVal = 1;

	/* WRITE THE NEG TEST VALUE IN THE TEST LOCATION */
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x11);	//clr first
	vFRAM_write_B32(FRAM_TEST_ADDR, FRAM_TEST_VAL_NEGATIVE_UL);

	/* READ IT BACK */
	ulTestVal = ulFRAM_read_B32(FRAM_TEST_ADDR);
	if(ulTestVal != FRAM_TEST_VAL_NEGATIVE_UL)
		{
		if(ucReportMode)
			{
			vSERIAL_rom_sout("FramFail ");
			vSERIAL_rom_sout(" wrote= 0x");				
			vSERIAL_HBV32out(FRAM_TEST_VAL_NEGATIVE_UL);
			vSERIAL_rom_sout(" read= 0x");					
			vSERIAL_HBV32out(ulTestVal);
			vSERIAL_rom_sout(" xor= 0x");
			vSERIAL_HBV32out(FRAM_TEST_VAL_NEGATIVE_UL ^ ulTestVal);
			vSERIAL_crlf();
			}
		ucRetVal = 0;
		}

	/* WRITE THE POS TEST VALUE IN THE TEST LOCATION */
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x22);	//clr first
	vFRAM_write_B32(FRAM_TEST_ADDR, FRAM_TEST_VAL_POSITIVE_UL);

	/* READ IT BACK */
	ulTestVal = ulFRAM_read_B32(FRAM_TEST_ADDR);
	if(ulTestVal != FRAM_TEST_VAL_POSITIVE_UL)
		{
		if(ucReportMode)
			{
			vSERIAL_rom_sout("FramFail ");
			vSERIAL_rom_sout(" wrote= 0x");				
			vSERIAL_HBV32out(FRAM_TEST_VAL_POSITIVE_UL);
			vSERIAL_rom_sout(" read= 0x");					
			vSERIAL_HBV32out(ulTestVal);
			vSERIAL_rom_sout(" xor= 0x");
			vSERIAL_HBV32out(FRAM_TEST_VAL_POSITIVE_UL ^ ulTestVal);
			vSERIAL_crlf();
			}
		ucRetVal = 0;
		}

	return(ucRetVal);

	}/* END: ucL2FRAM_chk_for_fram() */







/*******************  vL2FRAM_format_fram()  ********************************
*
* This routine formats the FRAM
*
*****************************************************************************/

void vL2FRAM_format_fram(
		void
		)
	{
	uchar ucii;


	/* WRITE THE FRAM ID AT THE FRONT OF THE MEM */
	vFRAM_write_B8(FRAM_ID_ADDR_XI, FRAM_ID_VAL_XI);
	vFRAM_write_B8(FRAM_ID_ADDR_HI, FRAM_ID_VAL_HI);
	vFRAM_write_B8(FRAM_ID_ADDR_MD, FRAM_ID_VAL_MD);
	vFRAM_write_B8(FRAM_ID_ADDR_LO, FRAM_ID_VAL_LO);

	/* WRITE THE FRAM VERSION NUMBER */
	vFRAM_write_B16(FRAM_VER_ADDR,
					FRAM_VERSION
					);

	/* WRITE THE FRAM TEST AREA */
	vFRAM_write_B32(FRAM_TEST_ADDR,
					FRAM_TEST_VAL_NEGATIVE_UL
					);


	/* WRITE THE FRAM FLASH POINTERS */
	vL2FRAM_initFramFlashPtrs();


	/* WRITE THE FRAM TIME SAVE AREA */
	vFRAM_write_B32(FRAM_TIME_SAVE_AREA_ADDR,(ulong)lTIME_getSysTimeAsLong());

	/* WRITE THE FRAM REBOOT COUNT AREA */
	vFRAM_write_B16(FRAM_REBOOT_COUNT_ADDR,0);

	/* WRITE THE FRAM WIZARD ID AREA */
	vFRAM_write_B16(FRAM_USER_ID_ADDR, uiROM_getRomConfigSnumAsUint());

	/* WRITE THE DEFAULT OPTION ARRAY INTO THE FRAM ARRAY */
	vMODOPT_copyAllRomOptionsToFramOptions(DEFAULT_ROLE_IDX); //update FRAM
	vMODOPT_copyAllFramOptionsToRamOptions();				//copy to RAM

	/* NOW WRITE THE DEFAULT EVENT START BLOCK LIST */
	vL2FRAM_writeDefaultStartBlkList();


	/*----------------  NOW INIT Y TRIGGER TABLE  --------------------------*/

	for(ucii=0; ucii<SENSOR_MAX_VALUE;  ucii++)
		{
		vL2FRAM_putYtriggerVal(ucii, 0);
		}


	return;

	}/* END: vL2FRAM_format_fram() */






/*******************  uiL2FRAM_chk_for_fram_format()  ***********************
*
* This routine checks the FRAM for formatting information
* (IE. If the Fram logo is set in bytes 0-3),
* and reports version number if formatted and 0 if not
*
*
* RET:	Version = yes FRAM is formatted
*		0 = no FRAM is not formatted
*
*****************************************************************************/

uint uiL2FRAM_chk_for_fram_format(
		void
		)
	{
	uint uiRetVal;

	uiRetVal = uiFRAM_read_B16(FRAM_VER_ADDR);
	if(uiRetVal != FRAM_VERSION)
		{
		#if 0
		vSERIAL_rom_sout("L2FRAM:VerErr,ReadVer= ");
		vSERIAL_HB16out(uiRetVal);
		vSERIAL_rom_sout(" needed= ");
		vSERIAL_HB16out(FRAM_VERSION);
		vSERIAL_crlf();
		#endif
		uiRetVal = 0;
		}

	if(ucFRAM_read_B8(FRAM_ID_ADDR_XI) != FRAM_ID_VAL_XI) uiRetVal = 0;
	if(ucFRAM_read_B8(FRAM_ID_ADDR_HI) != FRAM_ID_VAL_HI) uiRetVal = 0;
	if(ucFRAM_read_B8(FRAM_ID_ADDR_MD) != FRAM_ID_VAL_MD) uiRetVal = 0;
	if(ucFRAM_read_B8(FRAM_ID_ADDR_LO) != FRAM_ID_VAL_LO) uiRetVal = 0;

	return(uiRetVal);

	}/* END: uiL2FRAM_chk_for_fram_format() */





/*******************  vL2FRAM_force_fram_unformat()  ************************
*
*
*****************************************************************************/

void vL2FRAM_force_fram_unformat(
		void
		)
	{

	vFRAM_fillFramBlk(0, 4, 0);

	return;

	}/* END: vL2FRAM_force_fram_unformat() */







/*****************  vL2FRAM_writeDefaultStartBlkList()  ***********************
*
* This routine formats the FRAM Start Blk List
*
******************************************************************************/

void vL2FRAM_writeDefaultStartBlkList(
		void
		)
	{
	uchar ucii;
	uchar ucjj;
	uchar ucRole;
	uchar ucRoleMask;


	/*--------  WRITE THE EVENT DEFAULT START BLK LIST  ----------------------
	 *
	 * NOTE: ucii is the index into the rom action tables
	 *		 ucjj is the index into the FRAM startup table
	 *
	 *-----------------------------------------------------------------------*/

	ucRole = ucMODOPT_getCurRole();
	ucRoleMask = ucaBitMask[ucRole];

	/* LOOP FOR ALL POSSIBLE ACTIONS CHECKING IF THEY ARE TO BE STARTED */
	for(ucii=0, ucjj=0;  ucii<E_ACTN_MAX_NUM_IDX;  ucii++)
		{
		/* ucii =  rom start blk index (and action number)*/
		/* ucjj = fram start blk index */

		/* ACTION MUST BE A DEFAULT ACTION & RUNNABLE UNDER THIS ROLE */
		if((ucaDefaultStartupAction[ucii]) && 
		   (ucaWhoCanRunBits[ucii] & ucRoleMask))
			{

			/* ADD THIS ACTION TO THE LIST OF START BLKS */
			vL2FRAM_stuffStBlk(
				ucjj,							//start blk index
				ucii,							//action number
				ucaSampRecFlags[ucii],			//Start flag value
				uiaSampRecStart[ucii],			//Hr0 to Sample0
				uiaSampRecInterval[ucii],		//Sample interval time
				ulaSenseActVal[ucii]			//Sense Act Value
				);

			ucjj++;		//bump the FRAM block count

			}/* END: if() */

		}/* END: for(ucii) */

	vL2FRAM_setStBlkTblCount(ucjj);

	#if 0
	vL2FRAM_showStBlkTbl();
	vSERIAL_crlf();
	#endif

	return;

	}/* END: vL2FRAM_writeDefaultStartBlkList() */






/********************  vL2FRAM_initFramFlashPtrs()  ***************************
*
*
*
******************************************************************************/

void vL2FRAM_initFramFlashPtrs(
		void
		)
	{

	vL2FRAM_putLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR,
					FLASH_FIRST_LINEAR_ADDR_L
					);

	vL2FRAM_putLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR,
					FLASH_FIRST_LINEAR_ADDR_L
					);

	vL2FRAM_putLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR,
					FLASH_FIRST_LINEAR_ADDR_L
					);

	vFRAM_write_B16(FRAM_FLSH_REFRESH_PAGE_NFL_ADDR,0);

	return;

	}/* END: vL2FRAM_initFramFlashPtrs() */





/************************  vL2FRAM_stuffStBlk()  ****************************
*
* stuff a single start blk with the data.
*
*****************************************************************************/

void vL2FRAM_stuffStBlk(
		uchar ucStBlkNum,
		uchar ucActionNum,
		uchar ucFlagVal,
		uint  uiHrStartVal,
		uint  uiSampleIntervalVal,
		ulong ulSenseActVal
		)
	{
	vL2FRAM_putStBlkEntryVal(ucStBlkNum, FRAM_ST_BLK_ACTION_IDX, (ulong)ucActionNum);
	vL2FRAM_putStBlkEntryVal(ucStBlkNum, FRAM_ST_BLK_FLAGS_IDX,  (ulong)ucFlagVal);
	vL2FRAM_putStBlkEntryVal(ucStBlkNum, FRAM_ST_BLK_HR_ST_IDX,  (ulong)uiHrStartVal);
	vL2FRAM_putStBlkEntryVal(ucStBlkNum, FRAM_ST_BLK_INTERVAL_IDX, (ulong)uiSampleIntervalVal);
	vL2FRAM_putStBlkEntryVal(ucStBlkNum, FRAM_ST_BLK_SENSE_ACT_IDX, ulSenseActVal);

	return;

	}/* END: vL2FRAM_stuffStBlk() */





/*********************  vL2FRAM_showStBlkTbl()  ******************************
*
*
*
*
******************************************************************************/

void vL2FRAM_showStBlkTbl(
		void
		)
	{
	uchar ucii;
	uchar ucStBlkCount;
	uchar ucActionNum;
	uchar ucFlagVal;
	uint  uiHrStartVal;
	uint  uiSampleIntervalVal;
	ulong ulSenseActVal;

	ucStBlkCount = ucFRAM_read_B8(FRAM_ST_BLK_COUNT_ADDR);

	/* SHOW THE TABLE HEADER */
	vSERIAL_rom_sout("\r\n\r\n----  FRAM ST BLK TBL ----\r\n   (size=");
	vSERIAL_UIV8out(ucStBlkCount);
	vSERIAL_rom_sout(") (ID:");
	vL2FRAM_showSysID();

	vSERIAL_rom_sout(" ");
	vMODOPT_showCurRole();
	vSERIAL_rom_sout(")\r\n");

	vSERIAL_rom_sout("## ACTN FLGS  HR-ST  INTVL  SENSEACT\r\n");
	for(ucii=0; ucii<ucStBlkCount;  ucii++)
		{

		ucActionNum = (uchar) ulL2FRAM_getStBlkEntryVal(ucii, FRAM_ST_BLK_ACTION_IDX);
		ucFlagVal = (uchar) ulL2FRAM_getStBlkEntryVal(ucii, FRAM_ST_BLK_FLAGS_IDX);
		uiHrStartVal = (uint)ulL2FRAM_getStBlkEntryVal(ucii, FRAM_ST_BLK_HR_ST_IDX);
		uiSampleIntervalVal = (uint)ulL2FRAM_getStBlkEntryVal(ucii, FRAM_ST_BLK_INTERVAL_IDX);
		ulSenseActVal = ulL2FRAM_getStBlkEntryVal(ucii, FRAM_ST_BLK_SENSE_ACT_IDX);

		vSERIAL_UI8_2char_out(ucii,' ');
		vSERIAL_colTab(3);
		vACTION_showActionName(ucActionNum);
		vSERIAL_colTab(9);
		vSERIAL_HB8out(ucFlagVal);
		vSERIAL_colTab(14);
		vSERIAL_UI16out(uiHrStartVal);
		vSERIAL_colTab(21);
		vSERIAL_UI16out(uiSampleIntervalVal);
		vSERIAL_colTab(28);
		vSENSEACT_showSenseActWordInText(ulSenseActVal);
		vSERIAL_crlf();

		}/* END: for(ucii) */

	vSERIAL_crlf();

	return;

	}/* END: vL2FRAM_showStBlkTbl() */







/*********************  uiL2FRAM_get_version_num()  **************************
*
* RET:	version number
*
******************************************************************************/

uint uiL2FRAM_get_version_num(
		void
		)
	{
	return(uiFRAM_read_B16(FRAM_VER_ADDR));

	}/* END: vL2FRAM_force_fram_unformat() */







/******************  lL2FRAM_getLinearFlashPtr()  ****************************
*
*
*****************************************************************************/
long lL2FRAM_getLinearFlashPtr(
		uint uiFlashPtrFRAMaddr
		)
	{
	long lPtr;

	lPtr = (long)ulFRAM_read_B32(uiFlashPtrFRAMaddr); //read the ptr
	lPtr &= ~FLASH_MSG_SIZE_MASK_L;					  //force ptr to msg boundary

	return(lPtr);

	}/* lL2FRAM_get_linear_flash_ptr() */




/******************  vL2FRAM_putLinearFlashPtr()  ************************
*
*
*****************************************************************************/
void vL2FRAM_putLinearFlashPtr(
		uint uiFlashPtrFRAMaddr,
		long lFlashPtrToStore
		)
	{
	ulong ulPtr;

	ulPtr = (ulong)lFlashPtrToStore; //convert to ptr ulong
	ulPtr &= ~FLASH_MSG_SIZE_MASK_L; //force ptr to msg boundary

	vFRAM_write_B32(uiFlashPtrFRAMaddr, //write it out
					ulPtr
					);

	return;

	}/* vL2FRAM_putLinearFlashPtr() */





/********************  vL2FRAM_showLinearFlashPtr()  *************************
*
*
*
*****************************************************************************/

void vL2FRAM_showLinearFlashPtr(
		long lFlashPtrToShow
		)
	{

	vSERIAL_HBV32out((lFlashPtrToShow & FLASH_LINEAR_ADDR_PAGE_MASK_L) >> 9);
	vSERIAL_bout(':');
	vSERIAL_HBV32out(lFlashPtrToShow & FLASH_LINEAR_ADDR_MSG_MASK_L);

	return;

	}/* END: vL2FRAM_showLinearFlashPtr() */






/******************  vL2FRAM_incFlashOnPtr()  *******************************
*
*
*****************************************************************************/

void vL2FRAM_incFlashOnPtr(
		void
		)
	{
	long lNewFlashOnPtr;
	long lOldFlashOffPtr;
	long lOldFlashOnPtr;

	/* GET COPIES OF BOTH POINTERS */
	lOldFlashOnPtr  = lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR);
	lOldFlashOffPtr = lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR);

	/* INCREMENT THE ON POINTER */
	lNewFlashOnPtr = lL2FRAM_incAndWrapPtr(lOldFlashOnPtr);

	/* CHK IF ON PTR CAUGHT OFF PTR -- IF SO BACK UP */
	if(lNewFlashOnPtr == lOldFlashOffPtr)  lNewFlashOnPtr = lOldFlashOnPtr;

	/* WRITE OUT THE NEW POINTER */
	vL2FRAM_putLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR,
							lNewFlashOnPtr
							);

	return;

	}/* vL2FRAM_incFlashOnPtr() */





/******************  vL2FRAM_incFlashOffPtr()  *******************************
*
*
*****************************************************************************/

void vL2FRAM_incFlashOffPtr(
		void
		)
	{
	long lNewFlashOffPtr;

	long lOldFlashOffPtr;
	long lOldFlashOnPtr;
	long lOldFlashReadPtr;

	/* GET COPIES OF ON, OFF & READ PTRS */
	lOldFlashOnPtr   = lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR);
	lOldFlashOffPtr  = lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR);
	lOldFlashReadPtr = lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR);

	/* IF OFF PTR EQUALS ON PTR DON'T INC */
	if(lOldFlashOffPtr != lOldFlashOnPtr)
		{
		/* INCREMENT THE OFF PTR */
		lNewFlashOffPtr = lL2FRAM_incAndWrapPtr(lOldFlashOffPtr);

		/* WRITE OUT THE NEW POINTER */
		vL2FRAM_putLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR,
								lNewFlashOffPtr
								);

		/* IF THE OFF PTR EQUALS THE READ PTR -- INC THE READ PTR TOO */
		if(lOldFlashOffPtr == lOldFlashReadPtr)
			{
			vL2FRAM_putLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR,
									lNewFlashOffPtr
									);
			}

		}/* END: if() */

	return;

	}/* vL2FRAM_incFlashOffPtr() */






/******************  vL2FRAM_incFlashReadPtr()  *******************************
*
*
*****************************************************************************/

void vL2FRAM_incFlashReadPtr(
		void
		)
	{
	long lNewFlashReadPtr;
	long lOldFlashOnPtr;
	long lOldFlashReadPtr;

	/* GET COPIES OF ON, OFF & READ PTRS */
	lOldFlashOnPtr   = lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR);
	lOldFlashReadPtr = lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR);

	/* IF READ PTR EQUALS ON PTR DON'T INC */
	if(lOldFlashReadPtr != lOldFlashOnPtr)
		{
		/* INCREMENT THE READ PTR */
		lNewFlashReadPtr = lL2FRAM_incAndWrapPtr(lOldFlashReadPtr);

		/* WRITE OUT THE NEW POINTER */
		vL2FRAM_putLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR,
								lNewFlashReadPtr
								);

		}/* END: if() */

	return;

	}/* vL2FRAM_incFlashReadPtr() */





/*********************  iL2FRAM_getFlashRefreshPtr()  **********************
*
*
* RETS: Flash Refresh Ptr
*
*****************************************************************************/

int iL2FRAM_getFlashRefreshPtr(
		void
		)
	{
	return((int)uiFRAM_read_B16(FRAM_FLSH_REFRESH_PAGE_NFL_ADDR));

	}/* END: iL2FRAM_getFlashRefreshPtr() */





/*********************  vL2FRAM_incFlashRefreshPtr()  **********************
*
*
*****************************************************************************/

void vL2FRAM_incFlashRefreshPtr(
		void
		)
	{
	uint uiRefreshPtr;

	uiRefreshPtr = uiFRAM_read_B16(FRAM_FLSH_REFRESH_PAGE_NFL_ADDR);
	uiRefreshPtr++;
	uiRefreshPtr &= FLASH_REFRESH_PAGE_MASK_UI;
	vFRAM_write_B16(FRAM_FLSH_REFRESH_PAGE_NFL_ADDR, uiRefreshPtr);

	return;

	}/* END: vL2FRAM_incFlashRefreshPtr() */






/********************  vL2FRAM_expungeAllUploadedFlashMsgs()  ******************
*
*
******************************************************************************/

void vL2FRAM_expungeAllUploadedFlashMsgs(
		void
		)
	{
	long lFlashPtr;

	lFlashPtr = lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR);	//read READOUT ptr
	vL2FRAM_putLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR, lFlashPtr);	//write it back

	return;

	}/* END: vL2FRAM_expungeAllUploadedFlashMsgs() */







/******************  vL2FRAM_undeleteAllUploadedFlashMsgs()  ****************
*
*
*****************************************************************************/

void vL2FRAM_undeleteAllUploadedFlashMsgs(
		void
		)
	{
	long lFlashOffPtr;

	lFlashOffPtr = lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR);	//read OFF ptr
	vL2FRAM_putLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR, lFlashOffPtr);	//write it back

	return;

	}/* vL2FRAM_undeleteAllUploadedFlashMsgs() */






/*******************  lL2FRAM_getFlashUnreadMsgCount() **********************
*
* returns the number of messages in the flash from READOUT to ON.
*
******************************************************************************/

long lL2FRAM_getFlashUnreadMsgCount(
		void
		)
	{
	long lReadoutPtr;
	long lOnPtr;
	long lPtrDiff;
	long lMsgCount;

//	#define SHOW_UNREAD_MSG_COUNT 1


	#ifdef SHOW_UNREAD_MSG_COUNT
	vSERIAL_rom_sout("\r\nE:L2FRM_getFlashUnreadMsgCount:\r\n");
	#endif

	/* GET THE FRAM POINTER DATA */
	lOnPtr = lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR);

	#ifdef SHOW_UNREAD_MSG_COUNT
	vSERIAL_rom_sout("L2FRM:lOnPtr= ");
	vL2FRAM_showLinearFlashPtr(lOnPtr);
	vSERIAL_crlf();
	#endif


	lReadoutPtr = lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR);


	#ifdef SHOW_UNREAD_MSG_COUNT
	vSERIAL_rom_sout("L2FRM:lReadoutPtr= ");
	vL2FRAM_showLinearFlashPtr(lReadoutPtr);
	vSERIAL_crlf();
	#endif


	/* COMPUTE THE POINTER DIFFERENCE */
	if(lOnPtr >= lReadoutPtr)
		{
		lPtrDiff = lOnPtr - lReadoutPtr;
		}
	else
		{
		lPtrDiff = (FLASH_MAX_LINEAR_ADDR_L - lReadoutPtr) + 
				   (lOnPtr - FLASH_FIRST_LINEAR_ADDR_L);
		}

	#ifdef SHOW_UNREAD_MSG_COUNT
	vSERIAL_rom_sout("L2FRM:FLASH_MAX_LINEAR_ADDR_L= ");
	vL2FRAM_showLinearFlashPtr(FLASH_MAX_LINEAR_ADDR_L);
	vSERIAL_rom_sout(" = ");
	vSERIAL_HBV32out((ulong)FLASH_MAX_LINEAR_ADDR_L);
	vSERIAL_crlf();
	#endif


	#ifdef SHOW_UNREAD_MSG_COUNT
	vSERIAL_rom_sout("L2FRM:lPtrDiff= ");
	vSERIAL_HBV32out((ulong)lPtrDiff);
	vSERIAL_crlf();
	#endif

	lMsgCount = lPtrDiff / FLASH_MSG_SIZE_L;

	#ifdef SHOW_UNREAD_MSG_COUNT
	vSERIAL_rom_sout("L2FRM:lMsgCount= ");
	vSERIAL_HBV32out((ulong)lMsgCount);
	vSERIAL_crlf();
	#endif

	return(lMsgCount);

	}/* END: lL2FRAM_getFlashUnreadMsgCount() */






/*******************  lL2FRAM_getFlashFreeMsgCount() **********************
*
* returns the number of messages in the flash from READOUT to ON.
*
******************************************************************************/

long lL2FRAM_getFlashFreeMsgCount(
		void
		)
	{
	long lReadMsgCount;
	long lUnreadMsgCount;
	long lFreeMsgCount;


	lReadMsgCount = lL2FRAM_getFlashReadMsgCount();
	lUnreadMsgCount = lL2FRAM_getFlashUnreadMsgCount();

	lFreeMsgCount = FLASH_MAX_MSG_COUNT_L - (lReadMsgCount + lUnreadMsgCount);

	#if 0
	vSERIAL_rom_sout("\r\nReadMsgs= ");
	vSERIAL_HBV32out((ulong)lReadMsgCount);
	vSERIAL_rom_sout(" UnReadMsgs= ");
	vSERIAL_HBV32out((ulong)lUnreadMsgCount);
	vSERIAL_rom_sout(" Max= ");
	vSERIAL_HBV32out((ulong)FLASH_MAX_MSG_COUNT_L);
	vSERIAL_rom_sout(" Free= ");
	vSERIAL_HBV32out((ulong)lFreeMsgCount);
	vSERIAL_crlf();
	#endif

	return(lFreeMsgCount);

	}/* END: lL2FRAM_getFlashFreeMsgCount() */





/*********************  lL2FRAM_getFlashReadMsgCount() *********************
*
* returns the number of messages in the flash from OFF to READOUT.
*
******************************************************************************/

long lL2FRAM_getFlashReadMsgCount(
		void
		)
	{
	long lOffPtr;
	long lReadoutPtr;
	long lPtrDiff;
	long lMsgCount;

//	#define SHOW_READ_MSG_COUNT 1


	#ifdef SHOW_READ_MSG_COUNT
	vSERIAL_rom_sout("\r\nE:L2FRM_getFlashReadMsgCount\r\n");
	#endif

	/* GET THE FRAM POINTER DATA */
	lReadoutPtr = lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR);

	#ifdef SHOW_READ_MSG_COUNT
	vSERIAL_rom_sout("L2FRM:lReadoutPtr= ");
	vSERIAL_HBV32out(lReadoutPtr);
	vSERIAL_crlf();
	#endif


	lOffPtr = lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR);


	#ifdef SHOW_READ_MSG_COUNT
	vSERIAL_rom_sout("L2FRM:lOffPtr= ");
	vSERIAL_HBV32out(lOffPtr);
	vSERIAL_crlf();
	#endif


	/* COMPUTE THE POINTER DIFFERENCE */
	if(lReadoutPtr >= lOffPtr)
		{
		lPtrDiff = lReadoutPtr - lOffPtr;
		}
	else
		{
		lPtrDiff = (FLASH_MAX_LINEAR_ADDR_L - lOffPtr) + 
				   (lReadoutPtr - FLASH_FIRST_LINEAR_ADDR_L);

		}

	#ifdef SHOW_READ_MSG_COUNT
	vSERIAL_rom_sout("L2FRM:lPtrDiff= ");
	vSERIAL_HBV32out(lPtrDiff);
	vSERIAL_crlf();
	#endif

	lMsgCount = lPtrDiff / FLASH_MSG_SIZE_L;

	#ifdef SHOW_READ_MSG_COUNT
	vSERIAL_rom_sout("L2FRM:lMsgCnt= ");
	vSERIAL_HBV32out(lMsgCount);
	vSERIAL_crlf();
	#endif

	return(lMsgCount);

	}/* END: lL2FRAM_getFlashReadMsgCount() */






/**********************  lL2FRAM_incAndWrapPtr()  *************************
*
* This is a local untility routine to increment and wrap one of the 
* linear address pointers
*
****************************************************************************/

static long lL2FRAM_incAndWrapPtr(
		long lPtr
		)
	{
//	#define SHOW_INC_AND_WRAP_VALUES 1

	#ifdef SHOW_INC_AND_WRAP_VALUES
	vSERIAL_rom_sout("L2FRM:ulPtr= ");
	vSERIAL_HBV32out(lPtr);
	vSERIAL_crlf();
	#endif

	/* INCREMENT THE ON POINTER */
	lPtr += FLASH_MSG_SIZE_L;

	#ifdef SHOW_INC_AND_WRAP_VALUES
	vSERIAL_rom_sout("L2FRM:ulPtrAfterInc= ");
	vSERIAL_HBV32out(lPtr);
	vSERIAL_crlf();
	#endif


	/* WRAP THE ON POINTER IF NECESSARY */
	lPtr &= FLASH_LINEAR_ADDR_FULL_MASK_L;
	if(lPtr == 0) lPtr = FLASH_FIRST_LINEAR_ADDR_L;	//set to start addr if wrapped

	#ifdef SHOW_INC_AND_WRAP_VALUES
	vSERIAL_rom_sout("L2FRM:ulPtrAfterWrap= ");
	vSERIAL_HBV32out(lPtr);
	vSERIAL_crlf();
	#endif

	return(lPtr);

	}/* END: lL2FRAM_incAndWrapPtr() */





/******************** vL2FRAM_stuffSavedTime()  ******************************
*
*
*
******************************************************************************/

void vL2FRAM_stuffSavedTime(
		ulong ulSavedTimeVal
		)
	{

	vFRAM_write_B32(FRAM_TIME_SAVE_AREA_ADDR, ulSavedTimeVal);

	return;

	}/* END: vL2FRAM_stuffSavedTime() */





/******************** ulL2FRAM_getSavedTime()  *******************************
*
*
*
******************************************************************************/

ulong ulL2FRAM_getSavedTime(
		void
		)
	{
	return(ulFRAM_read_B32(FRAM_TIME_SAVE_AREA_ADDR));

	}/* END: ulL2FRAM_getSavedTime() */






/******************** vL2FRAM_writeRebootCount()  ******************************
*
*
*
******************************************************************************/

void vL2FRAM_writeRebootCount(
		uint uiRebootCountVal
		)
	{

	vFRAM_write_B16(FRAM_REBOOT_COUNT_ADDR, uiRebootCountVal);

	return;

	}/* END: vL2FRAM_writeRebootCount() */





/******************** uiL2FRAM_getRebootCount()  *******************************
*
*
*
******************************************************************************/

uint uiL2FRAM_getRebootCount(
		void
		)
	{
	return(uiFRAM_read_B16(FRAM_REBOOT_COUNT_ADDR));

	}/* END: uiL2FRAM_getRebootCount() */






/******************** vL2FRAM_incRebootCount()  ******************************
*
*
*
******************************************************************************/

void vL2FRAM_incRebootCount(
		void
		)
	{
	uint uiRebootCountVal;

	uiRebootCountVal = uiFRAM_read_B16(FRAM_REBOOT_COUNT_ADDR);
	uiRebootCountVal++;
	vFRAM_write_B16(FRAM_REBOOT_COUNT_ADDR, uiRebootCountVal);

	return;

	}/* END: vL2FRAM_incRebootCount() */






/************************  ucL2FRAM_isHub()  *********************************
*
* Returns == 1 if yes HUB
* Returns == 0 if not HUB
*
******************************************************************************/

uchar ucL2FRAM_isHub(
		void
		)
	{
	uchar ucRoleBits;

	ucRoleBits = ucMODOPT_getCurRole();

	/* IF YOU DO RECEIVE & YOU DO NOT SEND -- YOU ARE A HUB */ 
	if( (ucRoleBits & ROLE_RECEIVE_BIT) &&
	   ((ucRoleBits & ROLE_SEND_BIT) == 0))
		{
		return(1);
		}

	return(0);

	}/* END: ucL2FRAM_isHub() */




/************************  ucL2FRAM_isSender()  ******************************
*
* RET: 1 if yes SENDER
*      0 if not SENDER
*
******************************************************************************/

uchar ucL2FRAM_isSender(
		void
		)
	{

	return(ucMODOPT_getCurRole() & ROLE_SEND_BIT);

	}/* END: ucL2FRAM_isSender() */






/************************  ucL2FRAM_isSampler()  ******************************
*
* RET: 1 if yes SAMPLER
*      0 if not SAMPLER
*
******************************************************************************/

uchar ucL2FRAM_isSampler(
		void
		)
	{

	return(ucMODOPT_getCurRole() & ROLE_SAMPLE_BIT);

	}/* END: ucL2FRAM_isSampler() */





/************************  ucL2FRAM_isReceiver()  ******************************
*
* RET: 1 if yes Receiver
*      0 if not Receiver
*
******************************************************************************/

uchar ucL2FRAM_isReceiver(
		void
		)
	{

	return(ucMODOPT_getCurRole() & ROLE_RECEIVE_BIT);

	}/* END: ucL2FRAM_isReceiver() */








/**********************  uiL2FRAM_getSnumLo16AsUint()  ************************************
*
* Returns the two low bytes of the serial number to the caller
* 
*
******************************************************************************/

uint uiL2FRAM_getSnumLo16AsUint(
		void
		)
	{
	return(uiFRAM_read_B16(FRAM_USER_ID_ADDR));

	}/* END: uiL2FRAM_getSnumLo16AsUint() */






/***********************  vL2FRAM_copySnumLo16ToBytes()  *************************
*
******************************************************************************/

void vL2FRAM_copySnumLo16ToBytes(
		uchar *ucpToPtr
		)
	{
	uint uiSN;

	uiSN = uiL2FRAM_getSnumLo16AsUint();

	*ucpToPtr = (uchar)(uiSN >> 8);
	ucpToPtr++;
	*ucpToPtr = (uchar)uiSN;

	return;

	}/* END vL2FRAM_copySnumLo16ToBytes() */






/**********************  ucL2FRAM_getSnumMd8()  ************************************
*
* Return the Md part of the serial number.
*
*
******************************************************************************/

uchar ucL2FRAM_getSnumMd8(
		void
		)
	{
	return(ucFRAM_read_B8(FRAM_USER_ID_ADDR+0));

	}/* END: ucL2FRAM_getSnumMd8() */





/**********************  ucL2FRAM_getSnumLo8()  *******************************
*
* Return the Lo part of the serial number.
*
*
******************************************************************************/

uchar ucL2FRAM_getSnumLo8(
		void
		)
	{
	return(ucFRAM_read_B8(FRAM_USER_ID_ADDR+1));

	}/* END: ucL2FRAM_getSnumLo8() */




/**********************  vL2FRAM_setSysID()  ********************************
*
* Set the system ID
*
******************************************************************************/

void vL2FRAM_setSysID(
		uint uiSysID
		)
	{

	vFRAM_write_B16(FRAM_USER_ID_ADDR, uiSysID);

	return;

	}/* END: vL2FRAM_setSysID() */





/**********************  vL2FRAM_showSysID()  ********************************
*
* Show the system ID
*
******************************************************************************/

void vL2FRAM_showSysID(
		void
		)
	{
	uint uiSysID;

	uiSysID = uiL2FRAM_getSnumLo16AsUint();
	vRAD40_showRad40(uiSysID);

	return;

	}/* END: vL2FRAM_showSysID() */






/**********************  ucL2FRAM_getNextFreeStBlk()  *************************
*
* RET:  Next Free StBlk num (1 - 15) if 0 then no free action blks avail.
*
* NOTE: There is not an explicit way to detect a deleted St-Blk.  A deleted
*		St-Blk looks exactly like a turned off sleep blk( all zros), So the
*		method used	is to look for a zero action (SLEEP) blk that is not in
*		the block 0 position.
*
* NOTE: If the number of action blocks added >16 then no action blk will be
*		returned.
*
******************************************************************************/

uchar ucL2FRAM_getNextFreeStBlk(	//Ret: next free StBlk Num
		void
		)
	{
	uchar ucii;
	uchar ucStBlkMax;
	uchar ucActionNum;

	ucStBlkMax = ucFRAM_read_B8(FRAM_ST_BLK_COUNT_ADDR);

	/* SEARCH FOR A NO-START STBLK ENTRY */
	for(ucii=1;  ucii<ucStBlkMax;  ucii++)
		{
		ucActionNum = (uchar)ulL2FRAM_getStBlkEntryVal(ucii, FRAM_ST_BLK_ACTION_IDX);
		if(ucActionNum == 0)  return(ucii);

		}/* END: for(ucii) */

	if(ucStBlkMax >= 16) return(0);			//no action blocks left

	vFRAM_write_B8(FRAM_ST_BLK_COUNT_ADDR, ucStBlkMax+1);

	return(ucStBlkMax);

	}/* END: ucL2FRAM_getNextFreeStBlk() */





/**********************  ucL2FRAM_getStBlkTblCount()  *************************
*
* Return Start Blk Table Entry Count
*
******************************************************************************/

uchar ucL2FRAM_getStBlkTblCount(
		void
		)
	{
	return(ucFRAM_read_B8(FRAM_ST_BLK_COUNT_ADDR));

	}/* END: ucL2FRAM_getStBlkTblCount() */




/**********************  vL2FRAM_setStBlkTblCount()  **************************
*
* Return Start Blk Table Entry Count
*
******************************************************************************/

void vL2FRAM_setStBlkTblCount(
		uchar ucBlkCount
		)
	{
	vFRAM_write_B8(FRAM_ST_BLK_COUNT_ADDR, ucBlkCount);

	return;

	}/* END: ucL2FRAM_setStBlkTblCount() */






/**********************  ulL2FRAM_getStBlkEntryVal()  ************************
*
* Return StBlk entry value
*
******************************************************************************/

ulong ulL2FRAM_getStBlkEntryVal(		//RET: StBlk Entry Val
		uchar ucStBlkNum,				//blk number
		uchar ucStBlkEntryIdx			//index into the blk
		)
	{
	uint uiOffset;
	uint uiAddr;
	ulong ulRetVal;

	uiOffset = ((uint)ucStBlkNum) * FRAM_ST_BLK_SIZE;
	uiAddr = FRAM_ST_BLK_0_ADDR + uiOffset + ucStBlkEntryIdx;
	
	ulRetVal = 0UL;
	switch(ucStBlkEntryIdx)
		{
		case FRAM_ST_BLK_ACTION_IDX:				//byte
		case FRAM_ST_BLK_FLAGS_IDX:
			ulRetVal = ucFRAM_read_B8(uiAddr);
			break;

		case FRAM_ST_BLK_HR_ST_IDX:				//integer
		case FRAM_ST_BLK_INTERVAL_IDX:
			ulRetVal = uiFRAM_read_B16(uiAddr);
			break;

		case FRAM_ST_BLK_SENSE_ACT_IDX:
			ulRetVal = ulFRAM_read_B32(uiAddr);
			break;

		default:
			vSERIAL_rom_sout("L2FRM:BadStBlkIdx\r\n");
			ulRetVal = 0;
			break;

		}/* END: switch() */

	return(ulRetVal);

	}/* END: ulL2FRAM_getStBlkEntryVal() */






/**********************  vL2FRAM_putStBlkEntryVal()  *************************
*
* Return Start Block entry value
*
******************************************************************************/

void vL2FRAM_putStBlkEntryVal(			//Stuff the Value int the StBlk
		uchar ucStBlkNum,				//blk number
		uchar ucStBlkEntryIdx,			//index into the blk
		ulong ulVal						//value to put
		)
	{
	uint uiOffset;
	uint uiAddr;

	uiOffset = ((uint)ucStBlkNum) * FRAM_ST_BLK_SIZE;
	uiAddr = FRAM_ST_BLK_0_ADDR + uiOffset + ucStBlkEntryIdx;
	
	switch(ucStBlkEntryIdx)
		{
		case FRAM_ST_BLK_ACTION_IDX:				//byte
		case FRAM_ST_BLK_FLAGS_IDX:
			vFRAM_write_B8(uiAddr,(uchar)ulVal);
			break;

		case FRAM_ST_BLK_HR_ST_IDX:					//integer
		case FRAM_ST_BLK_INTERVAL_IDX:
			vFRAM_write_B16(uiAddr, (uint)ulVal);
			break;

		case FRAM_ST_BLK_SENSE_ACT_IDX:				//long
			vFRAM_write_B32(uiAddr, ulVal);

			#if 0
			vSERIAL_rom_sout("SenseActBlk=");
			vSERIAL_UIV8out(ucStBlkNum);
			vSERIAL_rom_sout(",  Idx=");
			vSERIAL_UIV8out(ucStBlkEntryIdx);
			vSERIAL_rom_sout(", Val=");
			vSERIAL_HB32out(ulVal);
			vSERIAL_crlf();
			#endif

			break;

		default:
			vSERIAL_rom_sout("L2FRM:WriteBdStBlkAddr= ");
			vSERIAL_UIV16out(uiAddr);
			vSERIAL_crlf();
			vSERIAL_rom_sout("Blk=");
			vSERIAL_UIV8out(ucStBlkNum);
			vSERIAL_rom_sout(",  Idx=");
			vSERIAL_UIV8out(ucStBlkEntryIdx);
			vSERIAL_crlf();
			break;

		}/* END: switch() */

	return;

	}/* END: vL2FRAM_putStBlkEntryVal() */






/*********************  ucL2FRAM_findStBlkAction()  ***************************
*
* Finds the first one
*
* Ret:	StBlk Num of the action found
*		255 if none found
*
******************************************************************************/

uchar ucL2FRAM_findStBlkAction(		//Ret: StBlk num,  255 if none
 		uchar ucDesiredActionNum	//action number to search for
		)
	{
	uchar ucii;
	uchar ucStBlkMax;
	uchar ucActionVal;

	ucStBlkMax = ucFRAM_read_B8(FRAM_ST_BLK_COUNT_ADDR);

	for(ucii=ucStBlkMax;  ucii>0;  ucii--)
		{
		ucActionVal = (uchar)ulL2FRAM_getStBlkEntryVal(ucii, FRAM_ST_BLK_ACTION_IDX);

		if(ucActionVal == ucDesiredActionNum) return(ucii);

		}/*END: for() */

	return(255);

	}/* END: ucL2FRAM_findStBlkAction() */





/********************  ucL2FRAm_addRuntimeStBlk()  ***************************
*
* RET:  New StBlk num  or 0 if none avail
*
* NOTE: new StBlk num of 0 means that no StBlks are available.
*
******************************************************************************/

uchar ucL2FRAM_addRuntimeStBlk(		//RET: StBlk num of new entry
 		uchar ucDesiredActionNum	//action number to search for
		)
	{
	uchar ucNewStBlkNum;

	/* GET A NEW STBLK NUMBER */
	ucNewStBlkNum = ucL2FRAM_getNextFreeStBlk();
	if(ucNewStBlkNum == 0) return(0);

	vL2FRAM_stuffStBlk(
		ucNewStBlkNum,							//start blk Num
		ucDesiredActionNum,						//action number
		ucaSampRecFlags[ucDesiredActionNum],	//flags value
		uiaSampRecStart[ucDesiredActionNum], 	//Hr0 to Sample0
		uiaSampRecInterval[ucDesiredActionNum],	//Sample interval time
		ulaSenseActVal[ucDesiredActionNum]		//Sense Act Val
		);

	return(ucNewStBlkNum);

	}/* END: ucL2FRAm_addRuntimeStBlk() */





/**********************  vL2FRAM_deleteStBlk()  ******************************
*
* This routine deletes a St-Blk from the List.
*
* NOTE: There is not an explicit way to detect a deleted St-Blk.  A deleted
*		St-Blk looks exactly like a turned off sleep blk( all zros), So the
*		method used	is to look for a zero action (SLEEP) blk that is not in
*		the block 0 position.
*
******************************************************************************/

void vL2FRAM_deleteStBlk(
 		uchar ucStBlkNum			//St Blk Num
		)
	{
	uchar ucii;
	uchar ucActionNum;
	uchar ucStBlkTop;
	uchar ucStBlkNewMax;

	/* OVERWRITE THE ACTION WITH SLEEP */
	vL2FRAM_stuffStBlk(
			ucStBlkNum,		//St blk Idx
			0,		   		//Action = E_ACTN_SLEEP
			0,				//Use Flags = USE_FLAGS_DELETED_ENTRY
			0,		   		//HrStartVal
			0,		   		//Interval
			0L		   		//SenseAct
			);

	/* IF THE DELETED STBLK WAS ON THE END REDUCE THE STBLK COUNT */
	ucStBlkTop = ucFRAM_read_B8(FRAM_ST_BLK_COUNT_ADDR);
	if(ucStBlkTop == 0) return;		//leave now if none

	/* REMOVE ALL EXTRA BLOCKS ON THE END OF THE STBLK ARRAY */
	ucStBlkNewMax = ucStBlkTop;
	for(ucii=ucStBlkTop-1;  ucii>0;  ucii--)
		{
		ucActionNum = (uchar)ulL2FRAM_getStBlkEntryVal(ucii, FRAM_ST_BLK_ACTION_IDX);
		if(ucActionNum == 0)
			{
			ucStBlkNewMax = ucii;
			continue;
			}
		break;
		}

	vL2FRAM_setStBlkTblCount(ucStBlkNewMax);

	return;

	}/* END: vL2FRAM_deleteStBlk() */







/**********************  vL2FRAM_putYtriggerVal()  ***************************
*
* stuff a trigger value
*
******************************************************************************/

void vL2FRAM_putYtriggerVal(			//Stuff Val into FRAM trigger Area
		uchar ucSensorNum,
		uint uiYtriggerVal
		)
	{
	uint uiFramAddr;

	uiFramAddr = FRAM_Y_TRIG_AREA_BEG_ADDR + (2 * ucSensorNum);

	vFRAM_write_B16(uiFramAddr, uiYtriggerVal);

	return;

	}/* END: vL2FRAM_putYtriggerVal() */







/**********************  uiL2FRAM_getYtriggerVal()  ***************************
*
* return a trigger value
*
******************************************************************************/

uint uiL2FRAM_getYtriggerVal(	//ret trigger Val from FRAM
		uchar ucSensorNum
		)
	{
	uint uiFramAddr;
	uint uiTriggerVal;

	uiFramAddr = FRAM_Y_TRIG_AREA_BEG_ADDR + (2 * ucSensorNum);

	uiTriggerVal = uiFRAM_read_B16(uiFramAddr);

	return(uiTriggerVal);

	}/* END: uiL2FRAM_getYtriggerVal() */






/**********************  vL2FRAM_showYtriggerTbl()  ***************************
*
* show the trigger table
*
******************************************************************************/

void vL2FRAM_showYtriggerTbl(
		void
		)
	{
	uchar ucii;
	uchar ucjj;

	vSERIAL_rom_sout("FramYTrigTbl:\r\n");
	vSERIAL_rom_sout("ID SNSACT  TRIG    ID SNSACT  TRIG    ID SNSACT  TRIG    ID SNSACT  TRIG\r\n");
	for(ucii=0,ucjj=0;  ucii<SENSOR_MAX_VALUE; ucii++) //ucii=sensor#  ucjj=format count
		{
		if((ucjj % 4) == 0) vSERIAL_crlf();
		vSERIAL_HB8out(ucii);
		vSERIAL_bout(' ');
		vSENSOR_showSensorName(ucii, L_JUSTIFY);
		vSERIAL_bout(' ');
		vSERIAL_UI16out(uiL2FRAM_getYtriggerVal(ucii));
		if(((ucjj+1) % 4) != 0)  vSERIAL_rom_sout("    ");
		ucjj++;

		}/* END: for(ucii) */

	vSERIAL_rom_sout("\r\n\n");

	return;

	}/* END: vL2FRAM_showYtriggerTbl() */





/*-------------------------------  MODULE END  ------------------------------*/

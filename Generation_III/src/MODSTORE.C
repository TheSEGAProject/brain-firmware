

/**************************  MODSTORE.C  ******************************************
*
* Keyboard Routines to change the Flash Storage tables.
*
*
* V1.00 01/09/2006 wzr
*		Started
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


#include "diag.h"			//diagnostic defines
//#include "p18f8720.h"		//processor reg description */
#include "std.h"			//standard definitions
#include "config.h"			//system configuration description file
//#include "main.h"			//
//#include "misc.h"			//homeless functions
//#include "ad.h" 			//AD module
//#include "crc.h"			//CRC calculation module
#include "serial.h"			//serial IO port stuff
//#include "sram.h"			//Level 1 static ram routines
//#include "l2sram.h" 		//Level 2 static ram routines
//#include "l2flash.h"		//Level 2 flash routines
#include "l2fram.h"			//Level 2 FRAM routines
//#include "time.h"			//system time routines
//#include "msg.h"			//msg routines
//#include "rts.h"			//real time scheduler
//#include "stbl.h"			//Schedule table routines
//#include "fulldiag.h"		//runtime diag
//#include "cmd.h"			//command table routines
//#include "rad40.h"  		//radix 40 routines
//#include "action.h"			//Action routines
#include "MODSTORE.h"			//key action change routines
//#include "sensor.h"			//Sensor name list
//#include "senseact.h"		//sense action names
//#include "pick.h"			//pick the point to send routines
#include "numcmd.h"			//numbered command handler module
#include "key.h"			//Top Level Key commands


/*********************  EXTERNS  *********************************************/



/*********************  DECLARATIONS  ****************************************/



/**********************  TABLES  *********************************************/



/* FUNCTION DELCARATIONS */
//     void vKEY_exitFuncPlaceHolder(void);
static void vMODSTORE_showHelp(void);
static void vMODSTORE_showTables(void);
static void vMODSTORE_deleteOldFlashMsgs(void);
static void vMODSTORE_windbackFlashPtrs(void);
static void vMODSTORE_zeroTheFlash(void);


#define CMD_ARRAY_SIZE 6

/* NOTE: This array is parallel to the Cmd array */
const char *cpaModStorCmdText[CMD_ARRAY_SIZE] = 
	{
	"Quit",										// 0 Quit
	"Help Msg",									// 1 Help
	"ShowTbls",									// 2 Show Store Tables
	"DelOldFlash Msgs",							// 3 Delete old flash
	"WindbackFlash Ptrs",						// 4 Windback Flash ptrs
	"ZeroFlash"									// 5 Zero the Flash

	}; /* END: cpaModStorCmdText[] */


const GENERIC_CMD_FUNC vMODSTOREfunctionTbl[CMD_ARRAY_SIZE] =
 {
 vKEY_exitFuncPlaceHolder,			// 0 Quit
 vMODSTORE_showHelp,				// 1 Help
 vMODSTORE_showTables,				// 2 Show Store Tables
 vMODSTORE_deleteOldFlashMsgs,		// 3 Delete Old Msgs
 vMODSTORE_windbackFlashPtrs,		// 4 Windback Flash Ptrs
 vMODSTORE_zeroTheFlash				// 5 Zero the Flash
 };




/********************  DECLARATIONS  *****************************************/





/***************************  CODE  ******************************************/


/************************  vMODSTORE_modifyStorageTbls() *************************
*
*  Modify the Flash Storage Pointers
*
******************************************************************************/

void vMODSTORE_modifyStorageTbls(
		void
		)

	{

	vKEY_showFlashMsgCounts();
	vSERIAL_crlf();


	vMODSTORE_showHelp();


	vNUMCMD_doCmdInputAndDispatch(
				'*',					//prompt
				cpaModStorCmdText,			//command array
				vMODSTOREfunctionTbl,		//dispatch function ptrs
				CMD_ARRAY_SIZE,			//array size
				YES_SHOW_INITIAL_PROMPT
				);

	return;

	}/* END: uchar ucMODSTORE_modifyStorageTbls() */






/*******************  vMODSTORE_showHelp()  ****************************************
*
*
*
******************************************************************************/
void vMODSTORE_showHelp(
		void
		)
	{
	vNUMCMD_showCmdList(cpaModStorCmdText, CMD_ARRAY_SIZE);
	vSERIAL_crlf();

	return;

	}/* END: vMODSTORE_showHelp() */






/*******************  vMODSTORE_showTables()  ****************************************
*
*
*
******************************************************************************/
void vMODSTORE_showTables(
		void
		)
	{
	vKEY_showFlashMsgCounts();
	return;

	}/* END: vMODSTORE_showTables() */






/*******************  vMODSTORE_deleteOldFlashMsgs()  ************************
*
*
*
******************************************************************************/
void vMODSTORE_deleteOldFlashMsgs(
		void
		)
	{
	vKEY_showFlashMsgCounts();

	if(!ucSERIAL_confirm(YES_SHOW_ABORT_MSG)) return;

	vL2FRAM_expungeAllUploadedFlashMsgs();
	vKEY_showFlashMsgCounts();

	return;

	}/* END: vMODSTORE_deleteOldFlashMsgs() */





/*****************  vMODSTORE_windbackFlashPtrs()  ********************************
*
*
*
******************************************************************************/
void vMODSTORE_windbackFlashPtrs(
		void
		)
	{
	vKEY_showFlashMsgCounts();

	if(ucSERIAL_confirm(YES_SHOW_ABORT_MSG))
		{
		vL2FRAM_undeleteAllUploadedFlashMsgs();
		vSERIAL_rom_sout("WindbkDone\r\n");
		vKEY_showFlashMsgCounts();
		}

	return;

	}/* END: vMODSTORE_windbackFlashPtrs() */






/*******************  vMODSTORE_zeroTheFlash()  ******************************
*
*
*
******************************************************************************/
void vMODSTORE_zeroTheFlash(
		void
		)
	{
	vKEY_showFlashMsgCounts();

	if(!ucSERIAL_confirm(YES_SHOW_ABORT_MSG)) return;

	vL2FRAM_initFramFlashPtrs();

	vKEY_showFlashMsgCounts();

	return;

	}/* END: vMODSTORE_zeroTheFlash() */


/*-------------------------------  MODULE END  ------------------------------*/



/**************************  MODACT.C  ******************************************
*
* Keyboard Routines to change the action table.
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
#include "l2sram.h" 		//Level 2 static ram routines
//#include "l2flash.h"		//Level 2 flash routines
#include "l2fram.h"			//Level 2 FRAM routines
//#include "time.h"			//system time routines
//#include "msg.h"			//msg routines
//#include "rts.h"			//real time scheduler
#include "stbl.h"			//Schedule table routines
//#include "fulldiag.h"		//runtime diag
//#include "cmd.h"			//command table routines
//#include "rad40.h"  		//radix 40 routines
#include "action.h"			//Action routines
#include "MODACT.h"			//key action change routines
#include "sensor.h"			//Sensor name list
#include "senseact.h"		//sense action names
#include "pick.h"			//pick the point to send routines
#include "numcmd.h"			//numbered command handler module
#include "key.h"			//top level keyboard routines







/*********************  EXTERNS  *********************************************/

//extern volatile uchar ucaMSG_BUFF[ MAX_RESERVED_MSG_SIZE ];

//extern int iGLOB_Hr0_to_SysTim0_inSec;	//dist (in sec) to 1st Hour in sys

extern uchar ucGLOB_StblIdx_NFL;




/*********************  DECLARATIONS  ****************************************/


void vMODACT_showCurActions(
		void
		);

void vMODACT_showAllPossibleActions(
		void
		);

void vMODACT_silentDeleteAction(	//Del Actn from SRAM and FRAM tables
		uchar ucSchedTblEntryNum	//SRAM table entry num
		);

void vMODACT_silentAddAction(	//Add Actn to SRAM and FRAM tables
		uchar ucNewActionNum	//Action num to add
		);

void vMODACT_silentChangeInterval(
		uchar ucSchedEntryNum,
		uint uiInterval
		);

void vMODACT_silentChangeSenseAct(
		uchar ucSchedEntryNum,
		ulong ulSenseActWord
		);

void vMODACT_silentChangeYtrigger(
		uchar ucSensorNum,
		uint uiTrigVal
		);

void vMODACT_showChangeableSenseActTrigs(
		void
		);


/**********************  TABLES  *********************************************/



/* FUNCTION DELCARATIONS */
//     void vKEY_exitFuncPlaceHolder(void);		// 0 Quit
static void vMODACT_showHelp(void);				// 1 Help
static void vMODACT_showActionTbls(void);		// 2 Show Action Tables
static void vMODACT_addAction(void);			// 3 Add Action
static void vMODACT_deleteAction(void);			// 4 Delete Action
static void vMODACT_modifyInterval(void);		// 5 Modify Sample Interval
static void vMODACT_modifySenseAct(void);		// 6 Modify SenseAct
static void vMODACT_modifyYtrigger(void);		// 7 Modify Y Trigger


#define CMD_ARRAY_SIZE 8

/* NOTE: This array is parallel to the Cmd array */
const char *cpaModActCmdText[CMD_ARRAY_SIZE] = 
	{
	"Quit",							// 0 Quit
	"Help",							// 1 Help
	"ShowTbls",						// 2 Show Action Tables
	"AddActnToLst",					// 3 Add Action
	"DeleteActnFrmLst",				// 4 Delete Action
	"ChgSampIntvl",					// 5 Modify Sample Interval
	"ChgSenseActLst",				// 6 Modify SenseAct
	"ChgYTrig"						// 7 Modify YTrigger
	}; /* END: cpaModActCmdText[] */


const GENERIC_CMD_FUNC vMODACTfunctionTbl[CMD_ARRAY_SIZE] =
 {
 vKEY_exitFuncPlaceHolder,			// 0 Quit
 vMODACT_showHelp,					// 1 Help
 vMODACT_showActionTbls,			// 2 Show Action Tables
 vMODACT_addAction,					// 3 Add Action
 vMODACT_deleteAction,				// 4 Delete Action
 vMODACT_modifyInterval,			// 5 Modify Sample Interval
 vMODACT_modifySenseAct,			// 6 Modify SenseAct
 vMODACT_modifyYtrigger				// 7 Modify YTrigger
 };


/*----------------------------------------------------------------*/


#define ACTION_LIST_SIZE		14

const char *cpaActionDesc[ACTION_LIST_SIZE] =
	{
	"Custom",								// 3 Roll your own
	"Batt",									// 4
	"TC 1,2",								// 5
	"TC 3,4",								// 6
	"Light 1,2",							// 7
	"SoilMoisture 1,2",						// 8
	"Light 3,4",							//16
	"Light 1,2,3,4",						//17
	"Light 1,1,1,1",						//18
	"VaisalaAveWindSpeed&Dir",				//19
	"VaisalaAveAirPress&Temp",	  			//20
	"VaisalaRelHumid&RainAccum",			//21
	"SapFlow 1,2",							//22
	"SapFlow 3,4" 							//23

	}; /* END: cpaActionDesc[] */



/* parallel array with cpaActionDescArray[] */
const unsigned char ucaChangeableActions[ACTION_LIST_SIZE] = 
	{
	 3,
	 4,
	 5,
	 6,
	 7,
	 8,
	16,
	17,
	18,
	19,
	20,
	21,
	22,
	23
	}; /* END: ucaChangeableActions[] */






/********************  DECLARATIONS  *****************************************/

uchar ucMODACT_isChangeableAction(
		uchar ucActionToCheck
		);





/***************************  CODE  ******************************************/


/************************  vMODACT_modifyActionTbl() *************************
*
*  Add,Del,Modify or Show an Action entry
*
******************************************************************************/

void vMODACT_modifyActionTbl(
		void
		)

	{

	vMODACT_showCurActions();
	vSERIAL_dash(10);
	vSERIAL_crlf();

	vMODACT_showAllPossibleActions();
	vSERIAL_dash(10);
	vSERIAL_crlf();

	vSERIAL_rom_sout("Cmds:\r\n");
	vMODACT_showHelp();


	vNUMCMD_doCmdInputAndDispatch(
				'*',					//prompt
				cpaModActCmdText,			//command array
				vMODACTfunctionTbl,		//dispatch function ptrs
				CMD_ARRAY_SIZE,			//array size
				YES_SHOW_INITIAL_PROMPT
				);

	return;

	}/* END: uchar ucMODACT_modifyActionTbl() */





/*******************  vMODACT_showHelp()  ****************************************
*
*
*
******************************************************************************/
void vMODACT_showHelp(
		void
		)
	{
	vNUMCMD_showCmdList(cpaModActCmdText, CMD_ARRAY_SIZE);
	vSERIAL_crlf();

	return;

	}/* END: vMODACT_showHelp() */




/*******************  vMODACT_addAction()  ****************************************
*
*
*
******************************************************************************/
void vMODACT_addAction(
		void
		)
	{
	char cNewActionNum;

	/* SHOW WHATS OUT THERE */
	vMODACT_showCurActions();
	vSERIAL_dash(10);
	vSERIAL_crlf();
	vMODACT_showAllPossibleActions();
	vSERIAL_dash(10);
	vSERIAL_crlf();

	vSERIAL_rom_sout("EntrActnNumToAdd: ");
	cNewActionNum = cNUMCMD_getCmdNum(E_ACTN_MAX_NUM_IDX);
	if(cNewActionNum <= 0)
		{
		goto Add_Action_exit;
		}


	/* CHECK IF IT IS A CHANGEABLE ACTION */
	if(!ucMODACT_isChangeableAction((uchar)cNewActionNum))
		{
		vSERIAL_rom_sout("NotLegalAdd\r\n\n");
		goto Add_Action_exit;
		}

	#if 1
	vSERIAL_rom_sout("ActnAddingIs: ");
	vSERIAL_IV8out(cNewActionNum);
	vSERIAL_crlf();
	#endif

	/* ITS VALID -- ADD IT TO THE SRAM TABLE */
	vMODACT_silentAddAction((uchar)cNewActionNum);

	#if 1
	vL2FRAM_showStBlkTbl();
	vSERIAL_crlf();
	#endif

	vMODACT_showCurActions();
	vSERIAL_crlf();

Add_Action_exit:
	return;

	}/* END: vMODACT_addAction() */





/****************  vMODACT_modifyInterval()  *********************************
*
*
*
******************************************************************************/

static void vMODACT_modifyInterval(
		void
		)
	{
	char cSchedEntryNum;
	uchar ucActionNum;
	uint uiInterval;

	/* SHOW WHATS OUT THERE */
	vMODACT_showCurActions();
	vSERIAL_dash(10);
	vSERIAL_crlf();
	vSERIAL_rom_sout("EntryID: ");
	cSchedEntryNum = cNUMCMD_getCmdNum(ucGLOB_StblIdx_NFL);
	if(cSchedEntryNum <= 0)
		{
		goto Interval_change_exit;
		}

	/* CHECK IF IT IS A CHANGEABLE ACTION */
	ucActionNum = (uchar)ulL2SRAM_getStblEntry(
											SCHED_ACTN_TBL_NUM,
											(uchar)cSchedEntryNum);
	if(!ucMODACT_isChangeableAction(ucActionNum))
		{
		vSERIAL_rom_sout("NotChgableIntrvl\r\n\n");
		goto Interval_change_exit;
		}

	vSERIAL_rom_sout("EnterNewIntrvl: ");
	uiInterval = (uint)ulSERIAL_UI32in();

	if(uiInterval < 120)
		{
		vSERIAL_rom_sout("IntrvlTooSmall\r\n\n");
		goto Interval_change_exit;
		}
	if(uiInterval > 14400)
		{
		vSERIAL_rom_sout("IntrvlTooBig\r\n\n");
		goto Interval_change_exit;
		}

	/* ITS VALID -- REMOVE IT FROM THE SRAM AND FRAM TABLES */
	vMODACT_silentChangeInterval((uchar)cSchedEntryNum, uiInterval);

	vMODACT_showCurActions();
	vSERIAL_crlf();

Interval_change_exit:
	return;

	}/* END: vMODACT_modifyInterval() */





/****************  vMODACT_modifySenseActl()  *********************************
*
*
*
******************************************************************************/
static void vMODACT_modifySenseAct(
		void
		)
	{
	char cSchedEntryNum;
	uchar ucActionNum;
	ulong ulSenseActWord;
	uchar ucii;
	uchar ucjj;

	/* SHOW WHATS OUT THERE */
	vMODACT_showCurActions();
	vSERIAL_dash(10);
	vSERIAL_crlf();

	vSERIAL_rom_sout("EntryID: ");
	cSchedEntryNum = cNUMCMD_getCmdNum(ucGLOB_StblIdx_NFL);
	if(cSchedEntryNum <= 0)
		{
		goto ChgSenseAct_exit;
		}

	/* CHECK IF IT IS A CHANGEABLE ACTION */
	ucActionNum = (uchar)ulL2SRAM_getStblEntry(
											SCHED_ACTN_TBL_NUM, 
											(uchar)cSchedEntryNum
											);
	if(!ucMODACT_isChangeableAction(ucActionNum))
		{
		vSERIAL_rom_sout("Can'tChangeThatSenseAct\r\n\n");
		goto ChgSenseAct_exit;
		}

	/* SHOW THE SENSE ACTS POSSIBLE */
	vSERIAL_rom_sout("SenseActsPossible:\r\n");
	vSERIAL_rom_sout("ID SNSACT SZ   ID SNSACT SZ   ID SNSACT SZ   ID SNSACT SZ\r\n");
	for(ucii=0,ucjj=0;  ucii<SENSOR_MAX_VALUE; ucii++) //ucii=sensor#  ucjj=format count
		{
		if(ucSENSEACT_getSensorUserFlag(ucii))
			{
			if((ucjj % 4) == 0) vSERIAL_crlf();
			vSERIAL_HB8out(ucii);
			vSERIAL_bout(' ');
			vSENSOR_showSensorName(ucii, L_JUSTIFY);
			vSERIAL_bout(' ');
			vSERIAL_UIV8out(ucSENSEACT_getSensorDataSize(ucii));
			if(((ucjj+1) % 4) != 0)  vSERIAL_rom_sout("    ");
			ucjj++;

			}/* END: if() */

		}/* END: for(ucii) */

	vSERIAL_rom_sout("\r\n\n");


	vSERIAL_rom_sout("CombineUpTo4ActnIDs(include 0\'s)asSingleNum,\r\n");
	vSERIAL_rom_sout("EnterNow: ");
	ulSenseActWord = ulSERIAL_H32in();

	/* LEFT JUSTIFY THE SENSE ACT WORD */
	if(ulSenseActWord != 0)
		{
		for(ucii=0;  ucii<3;  ucii++)
			{
			if(ulSenseActWord & 0xFF000000) break;
			ulSenseActWord <<= 8;
			}
		}

	#if 0
	vSERIAL_rom_sout("Thats ");
	vSERIAL_HB32out(ulSenseActWord);
	vSERIAL_rom_sout(" = ");
	vSENSEACT_showSenseActWordInText(ulSenseActWord);
	vSERIAL_crlf();
	#endif

	/* CORRECT THIS NUMBER */
	ulSenseActWord = ulSENSEACT_correctSenseActWord(ulSenseActWord);

	#if 0
	vSERIAL_rom_sout("Correctd=");
	vSERIAL_HB32out(ulSenseActWord);
	vSERIAL_rom_sout(" = ");
	vSENSEACT_showSenseActWordInText(ulSenseActWord);
	vSERIAL_crlf();
	#endif

	/* ITS VALID, STUFF THE STBL */
	vMODACT_silentChangeSenseAct((uchar)cSchedEntryNum, ulSenseActWord);

	vMODACT_showCurActions();
	vSERIAL_crlf();

ChgSenseAct_exit:
	return;

	}/* END: vMODACT_modifySenseAct() */








/****************  vMODACT_modifyYtriggerl()  *********************************
*
*
*
******************************************************************************/
static void vMODACT_modifyYtrigger(
		void
		)
	{
	char cSensorNum;
	uint uiTrigVal;

	vMODACT_showChangeableSenseActTrigs();
	vSERIAL_dash(10);
	vSERIAL_crlf();

	vSERIAL_rom_sout("EntryID: ");
	cSensorNum = cNUMCMD_getCmdNum(SENSOR_MAX_VALUE);
	if(cSensorNum <= 0)
		{
		goto ChgYtrig_exit;
		}


	if(ucSENSEACT_getSensorUserFlag((uchar)cSensorNum) == 0)
		{
		vSERIAL_rom_sout("InvldTrigID\r\n\n");
		goto ChgYtrig_exit;
		}


	vSERIAL_rom_sout("EntrTrigVal: ");
	uiTrigVal = (uint)ulSERIAL_UI32in();
	if(uiTrigVal > 32000)
		{
		vSERIAL_rom_sout("TrigTooBig\r\n\n");
		goto ChgYtrig_exit;
		}

	vMODACT_silentChangeYtrigger((uchar)cSensorNum, uiTrigVal);

	vSERIAL_rom_sout("TrigChged\r\n");
	vMODACT_showChangeableSenseActTrigs();
	vSERIAL_crlf();

ChgYtrig_exit:
	return;

	}/* END: vMODACT_modifyYtrigger() */







/*******************  vMODACT_deleteAction()  ****************************************
*
*
*
******************************************************************************/
void vMODACT_deleteAction(
		void
		)
	{
	char cSchedEntryNum;
	uchar ucActionNum;

	/* SHOW WHATS OUT THERE */
	vMODACT_showCurActions();

	vSERIAL_rom_sout("EnterActnNumToDel: ");
	cSchedEntryNum = cNUMCMD_getCmdNum(ucGLOB_StblIdx_NFL);
	if(cSchedEntryNum <= 0)
		{
		goto Remove_Action_exit;
		}

	/* CHECK IF IT IS A CHANGEABLE ACTION */
	ucActionNum = (uchar)ulL2SRAM_getStblEntry(SCHED_ACTN_TBL_NUM, (uchar)cSchedEntryNum);
	if(!ucMODACT_isChangeableAction(ucActionNum))
		{
		vSERIAL_rom_sout("NotLegalDelete\r\n\n");
		goto Remove_Action_exit;
		}

	/* ITS VALID -- REMOVE IT FROM THE SRAM AND FRAM TABLES */
	vMODACT_silentDeleteAction((uchar)cSchedEntryNum);

	#if 1
	vL2FRAM_showStBlkTbl();
	vSERIAL_crlf();
	#endif

	vMODACT_showCurActions();
	vSERIAL_crlf();


Remove_Action_exit:
	return;

	}/* END: vMODACT_deleteAction() */





/*******************  vMODACT_showAllPossibleActions()  ****************************************
*
*
*
******************************************************************************/

void vMODACT_showAllPossibleActions(
		void
		)
	{
	uchar ucii;
	uchar ucActionNum;

	vSERIAL_rom_sout("ModifiableActns:\r\n");

	for(ucii=0; ucii<ACTION_LIST_SIZE;  ucii++)
		{
		ucActionNum = ucaChangeableActions[ucii];
		vSERIAL_UI8out(ucActionNum);
		vSERIAL_rom_sout(" - ");
		vACTION_showActionName(ucActionNum);
		vSERIAL_rom_sout(" - ");
		vSERIAL_rom_sout(cpaActionDesc[ucii]);
		vSERIAL_crlf();
		}

	return;

	}/* END: vMODACT_showAllPossibleActions() */





/*******************  vMODACT_showCurActions()  ****************************************
*
*
*
******************************************************************************/

void vMODACT_showCurActions(
		void
		)
	{
	vSERIAL_rom_sout("CurActns:\r\n");
	vSTBL_showBriefStblEntrys();

	return;

	}/* END: vMODACT_showCurActions() */






/*******************  ucMODACT_isChangeableAction()  **************************
*
* Check if an action is on the changeable list
*
* RET 1 - yes
*	  0 - no
*
******************************************************************************/

uchar ucMODACT_isChangeableAction(
		uchar ucActionToCheck
		)
	{
	uchar ucii;

	for(ucii=0;  ucii<ACTION_LIST_SIZE;  ucii++)
		{
		if(ucActionToCheck == ucaChangeableActions[ucii]) return(1);
		}
	return(0);

	}/* END: ucMODACT_isChangeableAction() */





/*******************  vMODACT_silentDeleteAction()  ********************************
*
* Delete an action from the SRAM tables and the FRAM tables
*
******************************************************************************/
void vMODACT_silentDeleteAction(			//Del Actn from SRAM and FRAM tables
		uchar ucStblEntryNum	//SRAM entry num
		)
	{
	uchar ucStBlkNum;
	uchar ucActionNum;


	/* CHECK THE NUMBER FOR A CHANGEABLE ACTION */
	ucActionNum = (uchar)ulL2SRAM_getStblEntry(SCHED_ACTN_TBL_NUM, ucStblEntryNum);
	if(!ucMODACT_isChangeableAction(ucActionNum)) return;

	/* NOW FIND THE THE FRAM ST TBL ENTRY */
	ucStBlkNum = ucL2FRAM_findStBlkAction(ucActionNum);	//find actn in FRAM

	/* REMOVE THE ENTRY FROM THE SCHED TBL */
	vSTBL_deleteStblEntry(ucStblEntryNum);

	#if 1
	vSERIAL_rom_sout("DelingSramEntry ");
	vSERIAL_UIV8out(ucStblEntryNum);
	vSERIAL_crlf();
	vSERIAL_rom_sout("AlsoDelingFramStBlk ");
	vSERIAL_UIV8out(ucStBlkNum);
	vSERIAL_crlf();
	#endif

	/* REMOVE THE ENTRY FROM THE FRAM TBL */
	if(ucStBlkNum != 255) vL2FRAM_deleteStBlk(ucStBlkNum);	//remove from FRAM

	return;

	}/* END: vMODACT_silentDeleteAction() */






/*******************  vMODACT_silentAddAction()  *****************************
*
* Add an action from the SRAM tables and the FRAM tables
*
* NOTE: If the system is out of StBlks then no action will be added.
*
******************************************************************************/

void vMODACT_silentAddAction(	//Add Actn to SRAM and FRAM tables
		uchar ucNewActionNum	//Action num to add
		)
	{
	uchar ucStBlkNum;

	/* MAKE SURE THIS IS A CHANGEABLE ACTION */
	if(!ucMODACT_isChangeableAction(ucNewActionNum)) return;

	/* NOW ADD THE FRAM STARTUP ENTRY */
	ucStBlkNum = ucL2FRAM_addRuntimeStBlk(ucNewActionNum);
	if(ucStBlkNum == 0) return; //wups no StBlks are available.


	/* NOW ADD THE SRAM ENTRY TO THE SCHED TBL */
	vSTBL_addNewActionToStbl(ucNewActionNum, ucStBlkNum, 12);

	return;

	}/* END: vMODACT_silentAddAction() */




/*********************  vMODACT_silentChangeInterval()  ***********************
*
*
*
******************************************************************************/

void vMODACT_silentChangeInterval(
		uchar ucSchedEntryNum,
		uint uiInterval
		)
	{
	uchar ucActionNum;
	uchar ucStBlkNum;

	/* CHECK THE NUMBER FOR A VALID ENTRY */
	if(ucSchedEntryNum >= ucGLOB_StblIdx_NFL) return;

	/* CHECK IF IT IS A CHANGEABLE ACTION */
	ucActionNum = (uchar)ulL2SRAM_getStblEntry(SCHED_ACTN_TBL_NUM, ucSchedEntryNum);
	if(!ucMODACT_isChangeableAction(ucActionNum)) return;

	/* RANGE CHECK INTERVAL */
	if(uiInterval < 128) return;
	if(uiInterval > 14400) return;

	/* ITS GOOD -- STUFF A NEW SRAM VALUE */
	vL2SRAM_putStblEntry(SCHED_INTRVL_TBL_NUM, ucSchedEntryNum, (ulong)uiInterval);

	/* STUFF THE NEW VALUE INTO THE START TABLE */
	ucStBlkNum = ucL2FRAM_findStBlkAction(ucActionNum);
	vL2FRAM_putStBlkEntryVal(ucStBlkNum, FRAM_ST_BLK_INTERVAL_IDX, (ulong)uiInterval);

	return;

	}/* END: vMODACT_silentChangeInterval() */






/*********************  vMODACT_silentChangeSenseAct()  ***********************
*
*
*
******************************************************************************/

void vMODACT_silentChangeSenseAct(
		uchar ucSchedEntryNum,
		ulong ulSenseActWord
		)
	{
	uchar ucActionNum;
	uchar ucStBlkNum;

	/* CHECK THE SCHED IDX NUMBER FOR A VALID ENTRY */
	if(ucSchedEntryNum >= ucGLOB_StblIdx_NFL) return;

	/* CHECK IF IT IS A CHANGEABLE ACTION */
	ucActionNum = (uchar)ulL2SRAM_getStblEntry(SCHED_ACTN_TBL_NUM, ucSchedEntryNum);
	if(!ucMODACT_isChangeableAction(ucActionNum)) return;

	/* CORRECT THIS NUMBER */
	ulSenseActWord = ulSENSEACT_correctSenseActWord(ulSenseActWord);

	#if 0
	vSERIAL_rom_sout("Corrected_2=");
	vSERIAL_HB32out(ulSenseActWord);
	vSERIAL_rom_sout(" = ");
	vSENSEACT_showSenseActWordInText(ulSenseActWord);
	vSERIAL_crlf();
	#endif

	/* ITS GOOD -- STUFF A NEW SRAM VALUE */
	vL2SRAM_putStblEntry(SCHED_SENSE_ACT_TBL_NUM,	//tbl num
						ucSchedEntryNum,			//tbl idx
						ulSenseActWord				//value to stuff
						);

	/* STUFF THE NEW VALUE INTO THE START TABLE */
	ucStBlkNum = ucL2FRAM_findStBlkAction(ucActionNum);
	vL2FRAM_putStBlkEntryVal(ucStBlkNum,
							FRAM_ST_BLK_SENSE_ACT_IDX,
							ulSenseActWord
							);

	return;

	}/* END: vMODACT_silentChangeSenseAct() */






/*********************  vMODACT_silentChangeYtrigger()  ***********************
*
*
*
******************************************************************************/

void vMODACT_silentChangeYtrigger(
		uchar ucSensorNum,
		uint uiTrigVal
		)
	{

	/* CHECK FOR VALID ID NUMBER */
	if(ucSensorNum >= SENSOR_MAX_VALUE) return;
	if(ucSENSEACT_getSensorUserFlag(ucSensorNum) == 0) return;

	/* CHECK FOR VALID TRIGGER VAL */
	if(uiTrigVal > 32000) return;

	/* STUFF THE SSP TRIG TBL ENTRY */
	vPICK_putSSP_tblEntry(SSP_DELTA_Y_TRIG_TBL_NUM, ucSensorNum, (ulong)uiTrigVal);

	/* STUFF THE FRAM TBL ENTRY */
	vL2FRAM_putYtriggerVal(ucSensorNum, uiTrigVal);

	return;

	}/* END: vMODACT_silentChangeYtrigger() */





/*********************  vMODACT_showChangeableSenseActTrigs()  ***************
*
*
*
******************************************************************************/

void vMODACT_showChangeableSenseActTrigs(
		void
		)
	{
	uchar ucii;
	uchar ucjj;

	/* SHOW THE CHANGEABLE SENSEACT TRIGGERS */
	vSERIAL_rom_sout("SenseActTrigs:\r\n");
	vSERIAL_rom_sout("ID SNSACT  TRIG    ID SNSACT  TRIG    ID SNSACT  TRIG    ID SNSACT  TRIG\r\n");
	for(ucii=0,ucjj=0;  ucii<SENSOR_MAX_VALUE; ucii++) //ucii=sensor#  ucjj=format count
		{
		if(ucSENSEACT_getSensorUserFlag(ucii))
			{
			if((ucjj % 4) == 0) vSERIAL_crlf();
			vSERIAL_HB8out(ucii);
			vSERIAL_bout(' ');
			vSENSOR_showSensorName(ucii, L_JUSTIFY);
			vSERIAL_bout(' ');
			vSERIAL_UI16out((uint)ulPICK_getSSP_tblEntry(SSP_DELTA_Y_TRIG_TBL_NUM, ucii));
			if(((ucjj+1) % 4) != 0)  vSERIAL_rom_sout("    ");
			ucjj++;

			}/* END: if() */

		}/* END: for(ucii) */

	vSERIAL_crlf();

	return;

	}/* END: vMODACT_showChangeableSenseActTrigs() */






/*******************  vMODACT_showActionTbls()  ****************************
*
*
*
******************************************************************************/

void vMODACT_showActionTbls(
		void
		)
	{
	vSERIAL_rom_sout("CurActns:\r\n");
	vSTBL_showBriefStblEntrys();

	return;

	}/* END: vMODACT_showActionTbls() */





/*-------------------------------  MODULE END  ------------------------------*/

  

/**************************  RTS.C  *****************************************
*
* Routines for the REAL TIME SCHEDULER.
*
*
* V1.00 03/02/2005 wzr
*		Started
*
******************************************************************************/


/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* unusual (nonportable) anonymous struct or union */
///*lint -e714 */		/* symbol not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
///*lint -e757 */		/* global declarator not referenced */
///*lint -e752 */		/* local declarator not referenced */
///*lint -e758 */		/* global union not referenced */
///*lint -e768 */		/* global struct member not referenced */


#include <msp430x54x.h>		//processor reg description 
#include "std.h"			//standard defines
//#include "diag.h"			//Diagnostic package
#include "config.h" 		//system configuration description file
#include "main.h"			//
//#include "misc.h"			//homeless functions
//#include "reading.h"		//sensor reading module
//#include "ad.h" 			//AD module
//#include "crc.h"			//CRC calculation module
#include "l2sram.h"  		//disk storage module
#include "serial.h" 		//serial IO port stuff
//#include "sram.h"			//sram routines
//#include "radio.h"  		//event RADIO module
//#include "sdctl.h"  		//SD board control routines
//#include "sdcom.h" 		//SD communication package
//#include "l2flash.h"		//level 2 flash routines
#include "daytime.h"		//Daytime routines
#include "action.h" 		//Event action routines
#include "rts.h"			//Real Time Scheduler routines
//#include "fulldiag.h"		//full blown diagnostic defines
//#include "delay.h"  		//delay routines
//#include "buz.h"			//buzzer routines
#include "msg.h"			//radio msg helper routines
#include "rand.h"			//Random number generator
//#include "sysact.h"  		//Other events
#include "l2fram.h"			//Level 2 Ferro Ram routines 
#include "time.h"			//Time routines
#include "stbl.h"			//Sched Tbl routines
#include "MODOPT.h" 		//role flags for wiz routines
#include "RAD40.h"			//Radix 40 name converter
#include "LNKBLK.h"			//Link byte handler routines




extern uchar ucGLOB_myLevel;			//senders level +1
//extern uchar ucGLOB_myOldLevel;			//senders level +1 (old)

extern long lGLOB_initialStartupTime;	//Time used to compute uptime

extern uchar ucGLOB_StblIdx_NFL;		//Max number of sched entrys
extern int iGLOB_Hr0_to_SysTim0_inSec;	//dist from SysTim0 to Hr0
extern long lGLOB_OpMode0_inSec;		//set by discovery
extern long lGLOB_lastAwakeFrame;		//Nearest thing to cur frame
extern uchar ucGLOB_lastAwakeSlot;		//Nearest thing to cur slot
extern uchar ucGLOB_lastAwakeNSTtblNum; //Nearest thing to cur NST tbl

extern long lGLOB_lastScheduledFrame;
extern uchar ucGLOB_lastScheduledSchedSlot;

extern uchar ucGLOB_RDC4StblIdx;	//Sched tbl idx for RDC4  function

extern uchar ucGLOB_TC12StblIdx;	//DEBUG: Sched tbl idx for TC12


extern volatile union				//ucFLAG0_BYTE
  {
  uchar byte;
  struct
    {
	unsigned FLG0_BIGSUB_CARRY_BIT:1;		//bit 0 ;1=CARRY, 0=NO-CARRY
	unsigned FLG0_BIGSUB_6_BYTE_Z_BIT:1;	//bit 1 ;1=all diff 0, 0=otherwise
	unsigned FLG0_BIGSUB_TOP_4_BYTE_Z_BIT:1;//bit 2 ;1=top 4 bytes 0, 0=otherwise
	unsigned FLG0_NOTUSED_3_BIT:1;			//bit 3 ;1=SOM2 link exists, 0=none
											//SET:	when any SOM2 links exist
											//CLR: 	when the SOM2 link is lost
	unsigned FLG0_RESET_ALL_TIME_BIT:1;		//bit 4 ;1=do time  reset, 0=dont
											//SET:	when RDC4 gets finds first
											//		SOM2.
											//		or
											//		In a Hub when it is reset.
											//
											//CLR: 	when vMAIN_computeDispatchTiming()
											//		runs next.
	unsigned FLG0_SERIAL_BINARY_MODE_BIT:1;	//bit 5 1=binary mode, 0=text mode
	unsigned FLG0_HAVE_WIZ_GROUP_TIME_BIT:1;//bit 6 1=Wizard group time has
											//        been aquired from a DC4
											//      0=We are using startup time
	unsigned FLG0_NOTUSED7_BIT:1;			//bit 7
	}FLAG0_STRUCT;
  }ucFLAG0_BYTE;


extern union		//ucGLOB_debugBits1
 {
 uchar byte;
 struct
  {
  unsigned DBG_MaxIdxWriteToNST:1;	//bit 0 ;set if err, clr'd after reporting
  unsigned DBG_MaxIdxReadFromNST:1;	//bit 1 ;;set if err, clr'd after reporting
  unsigned DBG_notUsed2:1;	//bit 2 ;
  unsigned DBG_notUsed3:1;	//bit 3	;
  unsigned DBG_notUsed4:1;	//bit 4 ;
  unsigned DBG_notUsed5:1;	//bit 5 ;
  unsigned DBG_notUsed6:1;	//bit 6 ;
  unsigned DBG_notUsed7:1;	//bit 7 ;
  }debugBits1_STRUCT;
 }ucGLOB_debugBits1;




/*-----------------------  SCHEDULER TABLES HERE  ---------------------------*/

/* THIS ROM TBL IS INDEXED BY NST TBL NUM (1 or 2) & CONTAINS NST BASE ADDRS */
const USL uslaNST_baseAddr[2] = 
	{
	NST_1_TBL_BASE_ADDR,				// 0 NST TBL 0
	NST_2_TBL_BASE_ADDR					// 1 NST TBL 1
	}; /* END: uslaNST_baseAddr[] */


/* THIS ROM TBL IS INDEXED BY NST TBL NUM (1 or 2) & CONTAINS RANDOM NUM TBL BASE ADDRS */
const uchar ucaRandTblNum[2] = 
	{
	SCHED_RAND_TBL_1_NUM,				// 0 RAND TBL 0
	SCHED_RAND_TBL_2_NUM 				// 1 RAND TBL 1
	}; /* END: ucaRandTblNum[] */


/* THIS ROM TBL IS INDEXED BY NST TBL NUM (1 or 2) & CONTAINS DOWNCOUNT TBL BASE ADDRS */
const uchar ucaDcntTblNum[2] = 
	{
	SCHED_DCNT_TBL_1_NUM,				// 0 DOWNCOUNT TBL 0
	SCHED_DCNT_TBL_2_NUM				// 1 DOWNCOUNT TBL 1
	}; /* END: ucaDcntTblNum[] */


/* THIS ROM TBL IS INDEXED BY NST TBL NUM (1 or 2) & CONTAINS STS TBL BASE ADDRS */
const uchar ucaStsTblNum[2] = 
	{
	SCHED_STAT_TBL_1_NUM,				// 0 STS TBL 0
	SCHED_STAT_TBL_2_NUM				// 1 STS TBL 1
	}; /* END: ucaStsTblNum[] */



/*******************  FUNCTION DECLARATIONS HERE  ****************************/

static void vRTS_schedule_no_slot(
		uchar ucStblIdx,			//Tbl Idx of action to schedule
		long lFrameNumToSched				//Frame number to schedule
		);

static void vRTS_schedule_Scheduler_slot(
		uchar ucStblIdx,			//Tbl Idx of action to schedule
		long lFrameNumToSched				//Frame number to schedule
		);

static void vRTS_schedule_SDC4_slots(
		uchar ucStblIdx,			//Tbl Idx of action to schedule
		long lFrameNumToSched				//Frame number to schedule
		);

static void vRTS_schedule_RDC4_slots(
		uchar ucStblIdx,			//Tbl Idx of action to schedule
		long lFrameNumToSched				//Frame number to schedule
		);

static void vRTS_schedule_all_slots(
		uchar ucStblIdx,			//Tbl Idx of action to schedule
		long lFrameNumToSched				//Frame number to schedule
		);

static void vRTS_schedule_OM2_slot(
		uchar ucStblIdx,			//Tbl Idx of action to schedule
		long lFrameNumToSched				//Frame number to schedule
		);

static void vRTS_schedule_interval_slot(
		uchar ucStblIdx,			//Tbl Idx of action to schedule
		long lFrameNumToSched				//Frame number to schedule
		);

static void vRTS_schedule_loadonly_slot(
		uchar ucStblIdx,			//Tbl Idx of action to schedule
		long lFrameNumToSched				//Frame number to schedule
		);



/* DECLARE A VOID RET FUNC WITH 2 PARAMETERS FOR FUNCTIONS ABOVE */
typedef void (*SCHED_FUNC_WITH_2_PARAMS)(uchar ucStblIdx, long lFrameNumToSched);



/*-----------------  fpaSchedFuncArray[]  -----------------------------------
*
* DECLARE THE ARRAY OF POINTERS TO FUNCTIONS
* NOTE: THIS TABLE IS INDEXED BY FUNCTION NUMBER
*
*----------------------------------------------------------------------------*/
const SCHED_FUNC_WITH_2_PARAMS fpaSchedFuncArray[SCHED_FUNC_MAX_COUNT] =
	{
				/* INDEXED BY PRIORITY-USE NUMBER */
	vRTS_schedule_no_slot,			//0 = SCHED_FUNC_DORMANT_SLOT
	vRTS_schedule_Scheduler_slot,	//1 = SCHED_FUNC_SCHEDULER_SLOT
	vRTS_schedule_SDC4_slots,		//2 = SCHED_FUNC_SDC4_SLOT
	vRTS_schedule_RDC4_slots,		//3 = SCHED_FUNC_RDC4_SLOT
	vRTS_schedule_all_slots,		//4 = SCHED_FUNC_ALL_SLOT
	vRTS_schedule_OM2_slot,			//5 = SCHED_FUNC_RANDOM_SLOT
	vRTS_schedule_interval_slot,	//6 = SCHED_FUNC_INTERVAL_SLOT
	vRTS_schedule_loadonly_slot		//7 = SCHED_FUNC_LOADBASED_SLOT
	};





/*****************************  CODE STARTS HERE  ****************************/



/***********************  vRTS_putNSTentry()  ********************************
*
*
******************************************************************************/

void vRTS_putNSTentry(
		uchar ucNST_tblNum,		//NST tbl (0 or 1)
		uchar ucNST_slot,		//NST slot number
		uchar ucNST_val			//NST value
		)
	{
	usl uslNST_baseAddr;

	/* RANGE CHECK THE NST TBL NUMBER */
	if((ucNST_tblNum >= MAX_NST_TBL_COUNT) || (ucNST_slot > GENERIC_NST_LAST_IDX))
		{
		vSERIAL_rom_sout("RTS:WrtToNSToutOfRange:\r\n");
		vSERIAL_rom_sout("NSTtblNum= ");
		vSERIAL_UIV8out(ucNST_tblNum);
		vSERIAL_rom_sout(" Mx= ");
		vSERIAL_UIV8out(MAX_NST_TBL_COUNT);
		vSERIAL_rom_sout("  Slt#= ");
		vSERIAL_UIV8out(ucNST_slot);
		vSERIAL_rom_sout(" SltMx= ");
		vSERIAL_UIV8out(GENERIC_NST_LAST_IDX);
		vSERIAL_rom_sout("  DatVal= ");
		vSERIAL_UIV8out(ucNST_val);
		vSERIAL_rom_sout("\r\nAbortWriteToProtectSram\r\n");

		ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST = 1;

		return;
		}

	/* GET BASE ADDR FROM NST TBL NUMBER */
	uslNST_baseAddr = uslaNST_baseAddr[ucNST_tblNum];  


	/* RANGE CHECK THE BASE ADDR OF THE NST */
	if((uslNST_baseAddr != NST_1_TBL_BASE_ADDR) &&
	   (uslNST_baseAddr != NST_2_TBL_BASE_ADDR))
		{
		vSERIAL_rom_sout("L2SRM:NSTbaseAddrOutOfRange,NST_tblNum= ");
		vSERIAL_UIV8out(ucNST_tblNum);
		vSERIAL_rom_sout("  NST_BaseAddr= ");
		vSERIAL_UIV24out(uslNST_baseAddr);
		vSERIAL_rom_sout("\r\nAbortWritToProtectSram\r\n");
		vSERIAL_crlf();

		ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST = 1;

		return;
		}

	vL2SRAM_putGenericTblEntry(
					uslNST_baseAddr,				//Base Addr
					(usl)NST_2_TBL_BASE_ADDR,		//Base Addr max (LUL)
					ucNST_slot,						//Idx
					GENERIC_NST_LAST_IDX,			//Idx max (LUL)
					GENERIC_NST_BYTE_WIDTH,			//Width
					(ulong)ucNST_val				//value
					);
	return;

	}/* END: vRTS_putNSTentry() */





/***********************  ucRTS_getNSTentry()  ********************************
*
*
******************************************************************************/

uchar ucRTS_getNSTentry(
		uchar ucNST_tblNum,		//NST tbl (0 or 1)
		uchar ucNST_slot		//NST slot number
		)
	{
	ulong ulRetVal;
	usl uslNST_baseAddr;

	#if 0
	vSERIAL_rom_sout("E:L2SRMGetNSTentry()\r\n");
	#endif

	/* RANGE CHECK THE NST TBL NUMBER */
	if((ucNST_tblNum >= MAX_NST_TBL_COUNT) || (ucNST_slot > GENERIC_NST_LAST_IDX))
		{
		vSERIAL_rom_sout("RTS:RdOfNSToutOfRange:\r\n");
		vSERIAL_rom_sout("NSTtblNum= ");
		vSERIAL_UIV8out(ucNST_tblNum);
		vSERIAL_rom_sout(" Mx= ");
		vSERIAL_UIV8out(MAX_NST_TBL_COUNT);
		vSERIAL_rom_sout("  Slt#= ");
		vSERIAL_UIV8out(ucNST_slot);
		vSERIAL_rom_sout(" SltMx= ");
		vSERIAL_UIV8out(GENERIC_NST_LAST_IDX);
		vSERIAL_rom_sout("\r\nAbortRdToSaveSram\r\n");

		ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxReadFromNST = 1;

		return(0);
		}


	/* RANGE CHECK THE NST BASE ADDRESS */
	uslNST_baseAddr = uslaNST_baseAddr[ucNST_tblNum];  //lint !e661
	if((uslNST_baseAddr != NST_1_TBL_BASE_ADDR) &&
	   (uslNST_baseAddr != NST_2_TBL_BASE_ADDR))
		{
		vSERIAL_rom_sout("L2SRM:NSTbaseAddrOutOfRange,NST_tblNum= ");
		vSERIAL_UIV8out(ucNST_tblNum);
		vSERIAL_rom_sout("  NST_BaseAddr= ");
		vSERIAL_UIV24out(uslNST_baseAddr);
		vSERIAL_rom_sout("\r\nAbortRdofSram\r\n");
		vSERIAL_crlf();

		ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxReadFromNST = 1;

		return(0);
		}

	ulRetVal = ulL2SRAM_getGenericTblEntry(
					uslNST_baseAddr,				//Base Addr
					(usl)NST_2_TBL_BASE_ADDR,		//Base Addr max (LUL)
					ucNST_slot,						//Idx
					GENERIC_NST_LAST_IDX,			//Idx max (LUL)
					GENERIC_NST_BYTE_WIDTH			//Width
					);

	return((uchar)ulRetVal);

	}/* END: ucRTS_getNSTentry() */









/*********************  vRTS_showAllNSTentrys()  *****************************
*
*
*
******************************************************************************/

void vRTS_showAllNSTentrys(
		uchar ucNST_tblNum,		//NST tbl num (0 or 1)
		uchar ucShowStblFlag	//YES_SHOW_STBL, NO_SHOW_STBL
		)
	{
	uchar ucc;
	uchar ucStblIdx;
	uchar ucNSTidxCnt;
//	uchar ucEntryCount;
	uchar ucActnNum;
	uint uiContactSN;


	/* SHOW THE TITLE */
	vSERIAL_dash(30);
	vSERIAL_rom_sout("  NST ");
	vSERIAL_UIV8out(ucNST_tblNum);
	vSERIAL_rom_sout("  ");
	vSERIAL_dash(29);
	vSERIAL_crlf();

	/* SHOW THE NUMBER INDEX LINE */
	vSERIAL_rom_sout("     ");
	for(ucc=0;  ucc<16;  ucc++)
		{
		vSERIAL_HB8out(ucc);
		vSERIAL_rom_sout("  ");

		}/* END: for(ucc) */



//	ucEntryCount = 0;
	for(ucNSTidxCnt=0;  ucNSTidxCnt<GENERIC_NST_MAX_IDX;  ucNSTidxCnt++)
		{
		/* DO SOME COUNTER FORMATTING HERE */
		if((ucNSTidxCnt % 16) == 0)
			{
			vSERIAL_crlf();
			vSERIAL_HB8out(ucNSTidxCnt);
			vSERIAL_rom_sout(": ");
			}

		/* GET THE NST ENTRY VALUE */
		ucStblIdx = ucRTS_getNSTentry(ucNST_tblNum, ucNSTidxCnt);

		/* INDEX INTO THE STBL AND GET THE ACTION NAME */
		ucActnNum = (uchar)ulL2SRAM_getStblEntry(
									SCHED_ACTN_TBL_NUM,
									ucStblIdx
									);

		if((ucActnNum != E_ACTN_ROM2) && (ucActnNum != E_ACTN_SOM2))
			{
			vACTION_showStblActionName(ucStblIdx);
			}
		else
			{
			uiContactSN = (uint)ulL2SRAM_getStblEntry(
										SCHED_SN_TBL_NUM,	//Tbl num
										ucStblIdx			//Sched Idx
										);
			vRAD40_showRad40(uiContactSN);
			}

		if(((ucNSTidxCnt+1) % 16) != 0) vSERIAL_bout(',');

		/* BUMP THE COUNT FOR THE NEXT ONE */
//		ucEntryCount++;

		}/* END: for(ucNSTidxCnt) */

	vSERIAL_crlf();

	#if 0
	vSERIAL_rom_sout("ActivEntrysCnt= ");
	vSERIAL_UIV8out(ucEntryCount);
	#endif
	vSERIAL_crlf();

	if(ucShowStblFlag)
		{
		vSTBL_showAllStblEntrys();
//		vSERIAL_crlf();
		}

	return;

	}/* END: vRTS_showAllNSTentrys() */






/***********************  vRTS_clrNSTtbl()  ******************************
*
*
*
******************************************************************************/

void vRTS_clrNSTtbl(
		uchar ucNST_tblNum		//0 or 1
		)
	{
	uchar ucNST_slotCounter;

	#if 0
	vSERIAL_rom_sout("E:ClrNSTtbl\r\n");
	#endif

	if(ucNST_tblNum > 1)
		{
		vSERIAL_rom_sout("ClrNST:BdValForNSTidx=");
		vSERIAL_UIV8out(ucNST_tblNum);
		vSERIAL_crlf();
		ucNST_tblNum = ucNST_tblNum % 2;	//force it to legal bounds
		}

	for(ucNST_slotCounter=0;  ucNST_slotCounter<GENERIC_NST_MAX_IDX;  ucNST_slotCounter++)
		{
		vRTS_putNSTentry(ucNST_tblNum, ucNST_slotCounter, SLEEP_STBL_IDX);

		/* CHECK FOR MAX INDEX ERROR */
		if(ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST)
			{
			vSERIAL_rom_sout("RTS:MxNSTidxDetectedInClrNSTtbl\r\n");
			ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST = 0;
			}

		}/* END: for(ucNST_slotCounter) */

	#if 0
	vSERIAL_rom_sout("X:clrNSTtbl\r\n");
	#endif

	return;

	}/* END: vRTS_clrNSTtbl() */





/*********************  ucRTS_computeNSTfromFrameNum()  **********************
*
* Compute the NST table (0 or 1) from the frame number 
*
******************************************************************************/

uchar ucRTS_computeNSTfromFrameNum(
		long lFrameNumber
		)
	{
	uchar ucNST_tblNum;


	/* CHECK THE FRAME NUMBER */
	if(lFrameNumber < 0)
		{
		#if 1
		vSERIAL_rom_sout("RTS:BdFrameNum=");
		vSERIAL_IV32out(lFrameNumber);
		vSERIAL_crlf();
		#endif
		}

	/* COMPUTE WHICH NST TBALE WE ARE FILLING */
	ucNST_tblNum = (uchar)(lFrameNumber % 2L);	//compute NST tbl from frame

	return(ucNST_tblNum);

	}/* END: ucRTS_computeNSTfromFrameNum() */







/***********************  vRTS_scheduleNSTtbl()  *******************************
*
* This routine fills the next NST tbl
*
******************************************************************************/

void vRTS_scheduleNSTtbl(
		long lFrameNumber		//Frame number we are scheduling
		)
	{
	uchar ucStblIdxCnt;
	uchar ucPriorityCnt;
	uchar ucPriorityFuncVal;
	uchar ucPriorityOnlyVal;
	uchar ucFunctionOnlyVal;

	uchar ucNST_tblNum;

	/* GET THE NST THAT WE WILL BE WORKING ON */
	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNumber);

	#if 0
	vSERIAL_rom_sout("E:RTS:Sched\r\n");
	vRTS_showAllNSTentrys(ucNST_tblNum, YES_SHOW_STBL);
	#endif

	/* FIRST CLEAR THE NST WE ARE GOING TO FILL */
	vRTS_clrNSTtbl(ucNST_tblNum);

	/* MULTIPLE PASSES OVER THE SCHED TABLES BY PRIORITY */
	for(ucPriorityCnt = PRIORITY_0;  ; ucPriorityCnt += PRIORITY_INC_VAL)
		{
		#if 0
		vSERIAL_rom_sout("SchdNST:Priority=");
		vSERIAL_HB8out(ucPriorityCnt);
		vSERIAL_crlf();
		#endif

		/* WALK THE SCHED TBL BY STBL IDX */
		for(ucStblIdxCnt=0;  ucStblIdxCnt<ucGLOB_StblIdx_NFL;  ucStblIdxCnt++)
			{

			/* GET THE PRIORITY-FUNCTION VALUE FOR THIS ENTRY */
			ucPriorityFuncVal = (uchar)ulL2SRAM_getStblEntry(
										SCHED_PRIORITY_FUNC_TBL_NUM,
										ucStblIdxCnt);
			ucPriorityOnlyVal = (ucPriorityFuncVal & PRIORITY_MASK);
			ucFunctionOnlyVal = (ucPriorityFuncVal & SCHED_FUNC_MASK);

			/* VECTOR TO THE SCHED FUNCTION IF THE PRIORITY MATCHES */
			if(ucPriorityOnlyVal == ucPriorityCnt)
				{
				/* VECTOR TO THE FUNCTION */
				fpaSchedFuncArray[ucFunctionOnlyVal](ucStblIdxCnt, lFrameNumber);

				#if 0
				vSERIAL_rom_sout("PriorityCnt= ");
				vSERIAL_HB8out(ucPriorityCnt);
				vSERIAL_rom_sout(" Func= ");
				vSTBL_showPFuncName(ucFunctionOnlyVal);
				vSERIAL_rom_sout(" StblIdx= ");
				vSERIAL_HB8out(ucStblIdxCnt);
				vSERIAL_crlf();
				vRTS_showAllNSTentrys(ucNST_tblNum,YES_SHOW_STBL);
				#endif

				/* COMPUTE THE CURRENT LFACTOR FOR THIS ACTION */
				vSTBL_stuffSingleAction_LFactor(
									ucNST_tblNum,
									ucStblIdxCnt
									);

				}/* END: if() */

			}/* END: for(ucStblIdxCnt) */

		if(ucPriorityCnt== PRIORITY_MAX_VAL) break;

		}/* END: for(ucPriorityCnt) */

	/* UPDATE THE GLOBAL TO SHOW THE LAST SCHEDULER PARAMETERS */
	lGLOB_lastScheduledFrame = lFrameNumber;

	#if 0
	vSERIAL_rom_sout("X:RTS:SchedNST ");
	vSERIAL_UIV8out(ucNST_tblNum);
	vSERIAL_crlf();
	#endif

	#if 1
	vRTS_showAllNSTentrys(ucNST_tblNum,NO_SHOW_STBL);
	#endif

	return;

	}/* END: vRTS_scheduleNSTtbl() */





/********************  vRTS_schedule_no_slot()  ******************************
*
*
*
*****************************************************************************/

static void vRTS_schedule_no_slot(
		uchar ucStblIdx,			//Tbl Idx of action to schedule
		long lFrameNum				//Frame number to schedule
		)
	{
	return;
	}/*lint !e715 */





/********************  vRTS_schedule_Scheduler_slot()  ***********************
*
* This is a special routine to assign the scheduler slot because it keeps
* changing.
*
*****************************************************************************/

static void vRTS_schedule_Scheduler_slot(
		uchar ucStblIdx,			//Tbl Idx of action to schedule
		long lFrameNum				//Frame number to schedule
		)
	{
	uchar ucNST_tblNum;

	/* GET THE NST TABLE NUMBER FROM THE FRAME NUMBER */
	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNum);

	/* THE SCHEDULER IS NOW IN A FIXED SPOT (LAST SLOT OF THE NST) */
	vRTS_putNSTentry(ucNST_tblNum, 63, ucStblIdx);

	#if 0
	vSERIAL_rom_sout("SCHED_SCHED:ucStblIdx= ");
	vSERIAL_HB8out(ucStblIdx);
	vSERIAL_crlf();
	#endif

	/* UPDATE THE GLOBAL TO SHOW THE LAST SCHEDULER PARAMETERS */
	ucGLOB_lastScheduledSchedSlot = 63;

	return;

	}/* vRTS_schedule_Scheduler_slot() */	/*lint !e715 */




/******************* ucRTS_computeSDC4_slot_0()  ******************************
*
* Return the starting slot number for this level
*
*******************************************************************************/

static uchar ucRTS_computeSDC4_slot_0(
		uchar ucLevelNum
		)
	{
	uchar ucStartSlot;

	#if 0
	ucStartSlot = (uchar)((ucLevelNum +1) * 2);
	ucStartSlot %= GENERIC_NST_MAX_IDX;						//limit it
	#endif

	ucStartSlot = ucLevelNum+1;		//for now assign start slot by level

	return(ucStartSlot);

	}/* END: ucRTS_computeSDC4_slot_0() */





/*****************  uiRTS_computeSlotSpanForRDC4()  *************************
*
* RET: HI= starting slot num ,, LO= ending slot number
*
*****************************************************************************/

uint uiRTS_computeSlotSpanForRDC4(
		uchar ucBegLevel,
		uchar ucEndLevel
		)
	{
	uchar ucBegSlot;
	uchar ucEndSlot;
	uint uiBegAndSpan;

	ucBegSlot = ucRTS_computeSDC4_slot_0(ucBegLevel) -1;
	ucBegSlot %= GENERIC_NST_MAX_IDX;						//limit it

	ucEndSlot = ucRTS_computeSDC4_slot_0(ucEndLevel) +1;
	ucEndSlot %= GENERIC_NST_MAX_IDX;						//limit it

	if(ucEndSlot < ucBegSlot) ucEndSlot = ucBegSlot;

	uiBegAndSpan = ucBegSlot;
	uiBegAndSpan <<= 8;
	uiBegAndSpan |= ucEndSlot;

	return(uiBegAndSpan);

	}/* END: uiRTS_computeSlotSpanForRDC4() */





/********************  vRTS_schedule_SDC4_slots()  **************************
*
*
*****************************************************************************/

static void vRTS_schedule_SDC4_slots(
		uchar ucStblIdx,			//Tbl Idx of action to schedule
		long lFrameNumToSched		//Frame number to schedule
		)
	{
	uchar ucNST_tblNum;
	uchar ucActiveSOM2slots;
	uchar ucActiveROM2slots;
	uint uiCountTotals;
	uchar ucStartSlot;
	uchar ucReqSlot;
	uchar ucFoundSlot;
	uchar ucii;
	uchar ucBigRand;
	uchar ucSmallRand;


	/* GET THE NST TABLE NUMBER FROM THE FRAME NUMBER */
	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNumToSched);

	/* COUNT THE ACTIVE SOM2 AND ROM2 SLOTS */
	uiCountTotals = uiSTBL_countSOM2andROM2entrys();
	/* UNPACK THE COUNTS */
	ucActiveSOM2slots = (uchar)(uiCountTotals >> 8);
	ucActiveROM2slots = (uchar)uiCountTotals;

	/* COMPUTE THE BEGINNING SLOT FOR THIS WIZARD */
	ucStartSlot = ucRTS_computeSDC4_slot_0(ucGLOB_myLevel);




	/*----------  SCHEDULE SDC4 DISCOVERY FOR THE HUB  ---------------------*/

	if(ucL2FRAM_isHub())
		{
		/* HUB HAS ESTABLISHED ROM2'S -- DISCOVER IN ONLY 1 SLOT */
		if(ucActiveROM2slots != 0)
			{
			/* SELECT THE FIXED SLOT NUMBER */
			ucReqSlot = ucStartSlot+8;

			/* SEE IF THE SLOT IS FREE */
			ucFoundSlot = ucRTS_findNearestNSTslot(ucNST_tblNum,ucReqSlot);

			/* ASSIGN THE SLOT TO BE THE NEXT DISCOVERY SLOT */
			vRTS_putNSTentry(ucNST_tblNum, ucFoundSlot, ucStblIdx);

			return;
			}/* END: if() */


		/* WE ARE A HUB WITH NO ESTABLISHED ROM2'S */
		/* DISCOVER IN  8 EQUALLY SPACED SLOTS */
		for(ucii=0; ucii<8;  ucii++)
			{
			/* COMPUTE A SLOT TO CHOOSE */
			ucReqSlot = (uchar)(ucStartSlot + (ucii*8));

			/* LIMIT ANY POSSIBLE OVERRUN */
			ucReqSlot %= GENERIC_NST_MAX_IDX;

			/* SEE IF THE SLOT IS FREE */
			ucFoundSlot = ucRTS_findNearestNSTslot(ucNST_tblNum,ucReqSlot);

			/* ASSIGN THE NEXT DISCOVERY SLOT */
			vRTS_putNSTentry(ucNST_tblNum, ucFoundSlot, ucStblIdx);

			#if 0
			vSERIAL_rom_sout("RTS:MstrNoLnksWantsSlt ");
			vSERIAL_UIV8out(ucReqSlot);
			vSERIAL_rom_sout("  Lvl= ");
			vSERIAL_UIV8out(ucGLOB_myLevel);
			vSERIAL_crlf();
			#endif

			}/* END: for(ucii) */

		return;

		}/* END: if(isHub()) */



	/*----------  SCHEDULE SDC4 DISCOVERY FOR THE NON-HUB  ------------------*/

	/* WE ARE A SPOKE */
	/* NO DISCOVERY FOR A UNIT WITHOUT SOM2'S */
	if(ucActiveSOM2slots == 0) return;



	/* WE ARE A SPOKE WITH ESTABLISHED SOM2'S -- TRANSMIT ON 1 SLOT RANDOMLY */

	/* ROLL THE RANDOM NUMBER */
	ucBigRand = ucRAND_getRolledMidSysSeed();
	ucSmallRand = (ucBigRand & 0x03);

	/* CHECK IF THIS RANDOM SLOT'S NUMBER IS UP */
	if(ucSmallRand == 0x02)
		{
		#if 0
		vSERIAL_rom_sout("RTS:RndYesXQA= ");
		vSERIAL_HB8out(ucSmallRand);
		vSERIAL_rom_sout("  BigRnd= ");
		vSERIAL_HB8out(ucBigRand);
		vSERIAL_crlf();
		#endif

		/* CHOOSE THE FIXED SLOT NUMBER */
		ucReqSlot = ucStartSlot+8;

		/* SEE IF THE SLOT IS FREE */
		ucFoundSlot = ucRTS_findNearestNSTslot(ucNST_tblNum,ucReqSlot);

		/* IT ITS FREE THEN ASSIGN IT */
		if(ucReqSlot == ucFoundSlot) vRTS_putNSTentry(ucNST_tblNum, ucFoundSlot, ucStblIdx);

		}/* END: if() */

	return;

	}/* END: vRTS_schedule_SDC4_slots() */






/********************  vRTS_schedule_RDC4_slots()  ***************************
*
*
******************************************************************************/

static void vRTS_schedule_RDC4_slots(
		uchar ucStblIdx,			//Tbl Idx of action to schedule
		long lFrameNumToSched		//Frame number to schedule
		)
	{
	uchar ucNST_tblNum;
//	uchar ucStartSlot;
//	uchar ucEndSlot;
	uchar ucFoundSlot;
	uchar ucReqSlot;
	uint uiCountTotals;
	uchar ucActiveSOM2slots;
//	uchar ucActiveROM2slots;
//	uint uiSpan;

	#if 1
	vSERIAL_rom_sout("\r\nE:SCHED:RDC4:\r\n");
	#endif


	/* IF NOT SENDING OM2'S THEN LEAVE */
	if(!ucL2FRAM_isSender()) return;

	/* GET THE NST TABLE NUMBER FROM THE FRAME NUMBER */
	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNumToSched);

	/* COUNT THE ACTIVE SOM2 AND ROM2 SLOTS */
	uiCountTotals = uiSTBL_countSOM2andROM2entrys();
	ucActiveSOM2slots = (uchar)(uiCountTotals >> 8);
//	ucActiveROM2slots = (uchar)uiCountTotals;


	/*----------------  WE ARE A SPOKE  -------------------------------------*/

	/* CHECK IF WE HAVE ACQUIRED WIZARD GROUP TIME YET */
	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_HAVE_WIZ_GROUP_TIME_BIT)
		{
		/* WE HAVE GROUP TIME */
		#if 1
		vSERIAL_rom_sout("\r\nHaveGrpTim\r\n");
		#endif

		/* IF WE ALREADY HAVE AN ACTIVE SOM2 -- DONT DO RDC4 */
		if(ucActiveSOM2slots != 0) return;

		/*----------------  NO ACTIVE SOM2'S  -------------------------------*/

		/* ASSIGN RDC4'S TO A GROUP OF SLOTS 10 WIDE */

		for(ucReqSlot=7;  ucReqSlot<18;  ucReqSlot++)
			{
			/* SEE IF THE SLOT IS FREE */
			ucFoundSlot = ucRTS_findNearestNSTslot(ucNST_tblNum, ucReqSlot);

			/* ASSIGN THE SLOT */
			vRTS_putNSTentry(ucNST_tblNum, ucFoundSlot, ucStblIdx);

			}/* END: for(ucc) */

		/* ASSUME THAT WE HAVE LOST GROUP TIME */
		ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_HAVE_WIZ_GROUP_TIME_BIT = 0;

		return;

		}/* END: if() */


	/* WE ARE A SPOKE THAT HAS NEVER ACQUIRED GROUP TIME -- DO A BLANKET SEARCH */
	vRTS_schedule_all_slots(ucStblIdx, lFrameNumToSched);

	return;

	}/* END: vRTS_schedule_RDC4_slots() */






/*********************  vRTS_schedule_all_slots()  ************************
*
*
*
*****************************************************************************/

static void vRTS_schedule_all_slots(
		uchar ucStblIdx,			//Tbl Idx of action to schedule
		long lFrameNumToSched				//Frame number to schedule
		)

	{
	uchar ucNST_tblNum;
	uchar ucNSTslotCnt;
	uchar ucReadBack_NST_idx;

	#if 1
	vSERIAL_rom_sout("E:AllSlotsOn:\r\n");
	vSTBL_showSingleStblEntry(ucStblIdx, NO_HDR, NO_CRLF);
	vSERIAL_crlf();
	#endif

	/* GET THE NST TABLE NUMBER FROM THE FRAME NUMBER */
	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNumToSched);

	/* WALK THROUGH ENTIRE NST TBL FILLING ENTRYS */
	for(ucNSTslotCnt=0;  ucNSTslotCnt<GENERIC_NST_MAX_IDX;  ucNSTslotCnt++)
		{
		/* READ THE SLOT AND CHECK IF IT IS ASSIGNED ALREADY */
		ucReadBack_NST_idx = ucRTS_getNSTentry(ucNST_tblNum, ucNSTslotCnt);

		/* CHECK IF THIS SLOT IS ALREADY ASSIGNED */
		if((ucReadBack_NST_idx == GENERIC_NST_NOT_USED_VAL) ||
		   (ucReadBack_NST_idx == SLEEP_STBL_IDX))
			{
			/* ASSIGN THIS SLOT */
			vRTS_putNSTentry(ucNST_tblNum, ucNSTslotCnt, ucStblIdx);

			/* CHECK FOR MAX INDEX ERROR */
			if(ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST)
				{
				vSERIAL_rom_sout("RTS:MxNSTidxInSched_all_slots\r\n");
				ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST = 0;
				}
			}

		}/* END: for(ucNSTslotCnt) */

	#if 0
	{
	vSERIAL_rom_sout("RTS:SchedAll-  ");
	vACTION_showStblActionName(ucStblIdx);
	vSERIAL_crlf();
	//vRTS_showAllNSTentrys(ucNST_tblNum,YES_SHOW_STBL);
	}
	#endif

	#if 0
	vSERIAL_rom_sout("X:vRTS_Schedule_all_slots\r\n");
	#endif

	return;

	}/* END: vRTS_schedule_all_slots() */





/*********************  vRTS_schedule_OM2_slot()  ***************************
*
* Uses LNKBLK Table to determine how to schedule these functions
*
*****************************************************************************/

static void vRTS_schedule_OM2_slot(
		uchar ucStblIdx,			//Tbl Idx of action to schedule
		long lFrameNumToSched		//Frame number to schedule
		)
	{
	uchar ucc;
	uchar ucNST_tblNum;
	uchar ucPrevNSTtblNum;
	USL uslPreviousSeed;
	USL uslNextSeed;
	uchar ucFoundSlot;
	long lLinkTime;
	long lLinkFrame;
	uchar ucLinkSlot;
	uint uiStsVal;
	uchar ucLnkNeedsForcing;



	/* GET THE NST TABLE NUMBER FROM THE FRAME NUMBER */
	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNumToSched);

	/* GET THE PREVIOUS NST TABLE NUM */
	ucPrevNSTtblNum = (ucNST_tblNum ^ 0x01);

	#if 0
	vSERIAL_rom_sout("RTS:SRE:ThisNST= ");
	vSERIAL_UI8out(ucNST_tblNum);
	vSERIAL_rom_sout("  PrevNST= ");
	vSERIAL_UI8out(ucPrevNSTtblNum);
	vSERIAL_crlf();
	#endif


	/*---------------  ROLL THE RANDOM NUMBERS FIRST  -----------------------*/

	/* LOAD THE PREVIOUS SEED VALUE */
	uslPreviousSeed = (USL)ulL2SRAM_getStblEntry(
					ucaRandTblNum[ucPrevNSTtblNum],	//Sched tbl num
					ucStblIdx						//Sched tbl idx
					);

	/* GET THE NEXT FOREIGN SEED */
	uslNextSeed = uslRAND_getRolledFullForeignSeed(uslPreviousSeed);

	/* STASH THE NEW SEED IN THE NEXT_RAND TBL */
	vL2SRAM_putStblEntry(
			ucaRandTblNum[ucNST_tblNum],	//Sched Tbl num
			ucStblIdx,						//Sched Tbl idx
			(ulong)uslNextSeed				//Seed Value
			);

	#if 0
	vSERIAL_rom_sout("PrevRndSeed= ");
	vSERIAL_HBV32out((ulong)uslPreviousSeed);
	vSERIAL_rom_sout("  NxtSeed= ");
	vSERIAL_HBV32out((ulong)uslNextSeed);
	vSERIAL_crlf();
	#endif



	/*-------------------  COPY THE STS TBL ---------------------------------*/
	/* LOAD THE OLD STS VAL */
	uiStsVal = (uint)ulL2SRAM_getStblEntry(
							ucaStsTblNum[ucPrevNSTtblNum],
							ucStblIdx					//Sched tbl idx
							);

	uiStsVal &= 0xFF00;		//scrub off all Dcnt & LFactor values



	/* UPDATE BOTH PREV AND NEXT STS TBLS */
	vL2SRAM_putStblEntry(
						ucaStsTblNum[ucPrevNSTtblNum],
						ucStblIdx,					//Sched tbl idx
						(ulong)uiStsVal
						);
	vL2SRAM_putStblEntry(
						ucaStsTblNum[ucNST_tblNum],
						ucStblIdx,					//Sched tbl idx
						(ulong)uiStsVal
						);



	/*--------  CHECK THE FIRST REQ ENTRY FOR LNKBLK ERRS  ------------------*/

	/* ASSUME THAT NO FORCING IS REQUIRED */
	ucLnkNeedsForcing = 0;

	/* LOAD THE FIRST ENTRY OF THE LNKBLK */
	lLinkTime = lLNKBLK_readSingleLnkBlkEntry(ucStblIdx, 0); 
	lLinkFrame = lTIME_getFrameNumFromTime(lLinkTime);
	ucLinkSlot = (uchar)lTIME_getSlotNumFromTime(lLinkTime);

	/* CHECK FOR A ZRO ENTRY */
	if(lLinkTime == 0)
		{
		#if 1    /* REPORT A ZERO LINKUP ENTRY */
		vSERIAL_dash(6);
		vSERIAL_rom_sout(" Lk ");
		vSERIAL_UIV8out(ucStblIdx);
		vSERIAL_rom_sout(" isZro ");
		vSERIAL_dash(6);
		vSERIAL_crlf();
		#endif

		ucLnkNeedsForcing = 1;			//force a new linkup
		goto Force_static_correction;

		}


	/* CHECK TO SEE IF THE LINK TIME IS TOO OLD */
	if(lLinkFrame < lFrameNumToSched)
		{
		#if 1 /* REPORT A LINKUP TIME MISS */
		vSERIAL_dash(6);
		vSERIAL_rom_sout(" Lk ");
		vSERIAL_UIV8out(ucStblIdx);
		vSERIAL_rom_sout(" Late ");
		vSERIAL_dash(6);
		vSERIAL_crlf();

		vSERIAL_rom_sout("CurTim= ");
		vTIME_showTime(lTIME_getSysTimeAsLong(),FRAME_SLOT_TIME,NO_CRLF);

		vSERIAL_rom_sout("  LkTim= ");
		vTIME_showTime(lLinkTime,FRAME_SLOT_TIME,YES_CRLF);
		#endif

		ucLnkNeedsForcing = 1;			//force a new linkup
		goto Force_static_correction;
		}


	/* CHECK TO SEE IF THE LINK TIME IT IN THE FUTURE */
	if(lLinkFrame > lFrameNumToSched)
		{
		#if 1  /* REPORT A FUTURE LINK REQ */
		vSERIAL_dash(6);
		vSERIAL_rom_sout(" Lk ");
		vSERIAL_UIV8out(ucStblIdx);
		vSERIAL_rom_sout(" Future= ");
		vTIME_showTime(lLinkTime,FRAME_SLOT_TIME,NO_CRLF);
		vSERIAL_dash(6);
		vSERIAL_crlf();
		#endif

		goto sos_exit;
		}



Force_static_correction:

	/* CHECK IF WE NEED TO FORCE A LINK REQ */
	if(ucLnkNeedsForcing)
		{
		/* SETUP FOR A FORCED NEW LINKUP */
		vLNKBLK_fillLnkBlkFromMultipleLnkReq(
				ucStblIdx,
				LNKREQ_1FRAME_1LNK,
				lTIME_getSysTimeAsLong()
				);


		/* READ FORCED TIME BACK */
		lLinkTime = lLNKBLK_readSingleLnkBlkEntry(ucStblIdx, 0); 

		#if 1
		vSERIAL_dash(6);
		vSERIAL_rom_sout(" Force Static Lk ");
		vSERIAL_UIV8out(ucStblIdx);
		vSERIAL_rom_sout(" = ");
		vTIME_showTime(lLinkTime,FRAME_SLOT_TIME,NO_CRLF);
		vSERIAL_dash(6);
		vSERIAL_crlf();
		#endif

		}/* END: if() */



	/*--------  CHECK THE FIRST ENTRY FOR NST SCHEDULING ERRS  --------------*/

	ucLnkNeedsForcing = 0;				//assume everything is OK

	/* LOAD THE FIRST ENTRY OF THE LNKBLK */
	lLinkTime = lLNKBLK_readSingleLnkBlkEntry(ucStblIdx, 0); 
	lLinkFrame = lTIME_getFrameNumFromTime(lLinkTime);
	ucLinkSlot = (uchar)lTIME_getSlotNumFromTime(lLinkTime);

	/* FIND THE CLOSEST POSITION TO THIS SLOT */
	ucFoundSlot = ucRTS_findNearestNSTslot(ucNST_tblNum,ucLinkSlot);

	/* CHECK IF THERE ARE ANY SLOTS AVAILABLE */
	if(ucFoundSlot >= GENERIC_NST_MAX_IDX-1)
		{
		#if 1		/* REPORT NO SLOTS AVAILABLE */
		vSERIAL_dash(6);
		vSERIAL_rom_sout(" Lk ");
		vSERIAL_UIV8out(ucStblIdx);
		vSERIAL_rom_sout(" NoSlt ");
		vSERIAL_dash(6);
		vSERIAL_crlf();
		vRTS_showAllNSTentrys(ucNST_tblNum,YES_SHOW_STBL);
		vSERIAL_crlf();
		#endif

		ucLnkNeedsForcing = 1;			//force a new linkup
		goto Force_dynamic_correction;

		}

	/* CHECK IF WE GOT THE EXACT SLOT WE NEEDED */
	if(ucFoundSlot != ucLinkSlot)
		{
		#if 1  /* REPORT A SLOT COLLISION */
		{
		uchar ucOccupyingTblIdx;
		vSERIAL_dash(6);
		vSERIAL_rom_sout(" Lk ");
		vSERIAL_UIV8out(ucStblIdx);
		vSERIAL_rom_sout(" CollideAt ");
		vSERIAL_UIV8out(ucLinkSlot);
		vSERIAL_dash(6);
		vSERIAL_crlf();

		vSERIAL_rom_sout("NwEntry: ");
		vSTBL_showSingleStblEntry(ucStblIdx, YES_HDR, YES_CRLF);
		vSERIAL_rom_sout("CollideEntry:\r\n");
		ucOccupyingTblIdx = ucRTS_getNSTentry(ucNST_tblNum, ucLinkSlot);
		vSTBL_showSingleStblEntry(ucOccupyingTblIdx, NO_HDR, YES_CRLF);
		}
		#endif
		
		ucLnkNeedsForcing = 1;			//force a new linkup
		goto Force_dynamic_correction;

		}

Force_dynamic_correction:
	if(ucLnkNeedsForcing)
		{
		/* SETUP FOR A FORCED NEW LINKUP 2 FRAMES OUT */
		vLNKBLK_fillLnkBlkFromMultipleLnkReq(
				ucStblIdx,
				LNKREQ_2FRAME_1LNK,
				lTIME_getSysTimeAsLong()
				);

		/* READ FORCED TIME BACK */
		lLinkTime = lLNKBLK_readSingleLnkBlkEntry(ucStblIdx, 0); 

		#if 1
		vSERIAL_dash(6);
		vSERIAL_rom_sout(" Forcing Dynamic Lk ");
		vSERIAL_UIV8out(ucStblIdx);
		vSERIAL_rom_sout(" = ");
		vTIME_showTime(lLinkTime,FRAME_SLOT_TIME,NO_CRLF);
		vSERIAL_dash(6);
		vSERIAL_crlf();
		#endif

		goto sos_exit;

		}


/*-------  IF YOU ARE HERE THE 1ST LNK REQ IS GOOD IN EVERY WAY -----------*/



	/*------------  FILL THE NST FROM THE LINK BLK REQ'S --------------------*/

	/* LOOP FOR ALL ENTRYS IN THE LNK BLK */
	for(ucc=0;  ucc<ENTRYS_PER_LNKBLK_BLK_L;  ucc++)
		{
		/* LOAD A TABLE LINK TIME ENTRY */
		lLinkTime = lLNKBLK_readSingleLnkBlkEntry(ucStblIdx, ucc); 
		lLinkFrame = lTIME_getFrameNumFromTime(lLinkTime);
		ucLinkSlot = (uchar)lTIME_getSlotNumFromTime(lLinkTime);

		/* EXIT IF BLK EMPTY */
		if(lLinkTime == 0) break;

		/*------  SCHEDULING THE LNK FOR THE NEXT NST -----------------------*/

		/* FIND THE CLOSEST POSITION TO THIS SLOT */
		ucFoundSlot = ucRTS_findNearestNSTslot(ucNST_tblNum,ucLinkSlot);

		/* IF NO SLOTS AVAILABLE -- STOP NOW */
		if(ucFoundSlot >= GENERIC_NST_MAX_IDX-1)
			{
			#if 1		/* REPORT NO SLOTS AVAILABLE */
			vSERIAL_dash(6);
			vSERIAL_rom_sout(" (2)Lk ");
			vSERIAL_UIV8out(ucStblIdx);
			vSERIAL_rom_sout(" NoSlt ");
			vSERIAL_dash(6);
			vSERIAL_crlf();
			vRTS_showAllNSTentrys(ucNST_tblNum,YES_SHOW_STBL);
			vSERIAL_crlf();
			#endif

			break;
			}

		/* SKIP THIS SLOT IF NOT EXACT */
		if(ucFoundSlot != ucLinkSlot)
			{
			#if 1  /* REPORT A SLOT COLLISION */
			{
			uchar ucOccupyingTblIdx;
			vSERIAL_dash(6);
			vSERIAL_rom_sout(" (2)Lk ");
			vSERIAL_UIV8out(ucStblIdx);
			vSERIAL_rom_sout(" CollideAt ");
			vSERIAL_UIV8out(ucLinkSlot);
			vSERIAL_dash(6);
			vSERIAL_crlf();

			vSERIAL_rom_sout("NwEntry: ");
			vSTBL_showSingleStblEntry(ucStblIdx, YES_HDR, YES_CRLF);
			vSERIAL_rom_sout("CollideEntry:\r\n");
			ucOccupyingTblIdx = ucRTS_getNSTentry(ucNST_tblNum, ucLinkSlot);
			vSTBL_showSingleStblEntry(ucOccupyingTblIdx, NO_HDR, YES_CRLF);
			}
			#endif
			
			continue;
			}

		/* SLOT WAS FOUND, WAS AVAILABLE, WAS EXACT -- STUFF THE NST */
		vRTS_putNSTentry(ucNST_tblNum, ucFoundSlot, ucStblIdx);

		/* CHECK FOR MAX INDEX ERROR */
		if(ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST)			//chk bit
			{
			vSERIAL_rom_sout("MxNSTidxInSchedOM2slot\r\n"); //report
			ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST = 0;		//clr bit
			}

		}/* END: for(ucc) */


sos_exit:

	#if 1
	vSERIAL_rom_sout("EndSched-");
	vACTION_showStblActionName(ucStblIdx);
	vSERIAL_crlf();
	//vRTS_showAllNSTentrys(ucNST_tblNum,YES_SHOW_STBL);
	#endif

	return;

	}/* END: vRTS_schedule_OM2_slot() */












/***************  vRTS_schedule_interval_slot()  *****************************
*
* The Sample time terminology is as follows:
*	  
*
*
*Hr0                                               BaseTime                
*                          Sample0                             NxtSample 
* ³                          ³                         ³          ³
* ³  lHr0_to_Sample0_inSec   ³lSample0_to_BaseTime     ³          ³
* ³<------------------------>³<----------------------->³<-------->³
* ³                          ³                         ³          ³
* ³												       ³	      ³
* ³												   	   ³
* ³                          SysTim0          		   ³
* ³			            (System clk startup)    	   ³
* ³  lHr0_to_SysTim0_inSec      ³      BaseTime        ³
* ³<--------------------------->³<-------------------->³
* ³                             ³		  		       ³
*
*Hr0		                    0			       BaseTime								    
*                             Time
*
* Main Equation:
*
*lHr0_to_Sample0_inSec + lSample0_to_BaseTime = lHr0_to_SysTim0_inSec + BaseTime
*
*
*
* SysTim0:	Point on time line that the Wizard was started (Internal Time 0).
* CurTime:	Current Time in Seconds from SysTim0.
* OpMode0:	Time in seconds from SysTim0 that opmode was started.
* BaseTime: Rightmost time of CurTime or OpMode0.
*
* Hr0:		First hour mark to the left of SysTim0.
* Sample0:	First Sample time as specified by user.
* NxtSample: Time that the next sample should be taken.
*
*
*****************************************************************************/

static void vRTS_schedule_interval_slot(
		uchar ucStblIdx,			//Tbl Idx of action to schedule
		long lFrameNumToSched				//Frame number to schedule
		)
	{
	long lHr0_to_SysTim0_inSec;
	long lHr0_to_Sample0_inSec;
	long lBaseTime_inSec;
	long lEndTime_inSec;
	long lSample0_time;
	long lSample0_to_BaseTime_inSec;
	long lSampleInterval_inSec;
	long lNumberOfSamples;
	long lSampleCnt;
	long lThisSampleTime;
	long lThisSlotIdx;
	uchar ucFoundSlot;
	uchar ucFlags;
	uchar ucNST_tblNum;


	/* GET THE NST TABLE NUMBER FROM THE FRAME NUMBER */
	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNumToSched);

	/* GET SOME BASIC KNOWN DISTANCES IN LONG FORM */
	lHr0_to_SysTim0_inSec = (long)iGLOB_Hr0_to_SysTim0_inSec;	//get dist as long
	lHr0_to_Sample0_inSec = (long)ulL2SRAM_getStblEntry(SCHED_ST_TBL_NUM, ucStblIdx);

	lBaseTime_inSec = lGLOB_OpMode0_inSec + (lFrameNumToSched * SECS_PER_FRAME_L);

	lEndTime_inSec = lBaseTime_inSec + (SECS_PER_SLOT_L * SLOTS_PER_FRAME_L);
	lSampleInterval_inSec = (long)ulL2SRAM_getStblEntry(SCHED_INTRVL_TBL_NUM, ucStblIdx);


	/* GET SAMPLE-0 LOCATED IN SYSTEM TIME UNITS */
	lSample0_time = -lHr0_to_SysTim0_inSec + lHr0_to_Sample0_inSec;

	/* COMPUTE THE DIST FROM SAMPLE0 TO BASETIME */
	lSample0_to_BaseTime_inSec = lBaseTime_inSec - lSample0_time;

	/* COMPUTE THE NUM OF SAMPLES UP TO BASETIME */
	lNumberOfSamples = lSample0_to_BaseTime_inSec / lSampleInterval_inSec;

	if(ucStblIdx == ucGLOB_TC12StblIdx)
		{
		#if 1
		/* SHOW THE COMPUTED VALUES */

		vSERIAL_rom_sout("lHr0_to_SysTim0_inSec= ");
		vSERIAL_IV32out(lHr0_to_SysTim0_inSec);

		vSERIAL_rom_sout("\r\nlHr0_to_Sample0_inSec= ");
		vSERIAL_IV32out(lHr0_to_Sample0_inSec);

//		vSERIAL_rom_sout("\r\nlGLOB_OpMode0_inSec= ");
//		vSERIAL_IV32out(lGLOB_OpMode0_inSec);

		vSERIAL_rom_sout("\r\nlThis_NST_frameNum= ");
		vSERIAL_IV32out(lFrameNumToSched);



		vSERIAL_rom_sout("\r\nSLOTS_PER_FRAME_L= ");
		vSERIAL_IV32out(SLOTS_PER_FRAME_L);

		vSERIAL_rom_sout("\r\nSECS_PER_SLOT_L= ");
		vSERIAL_IV32out(SECS_PER_SLOT_L);

		vSERIAL_rom_sout("\r\nSECS_PER_FRAME_L= ");
		vSERIAL_IV32out(SECS_PER_FRAME_L);



		vSERIAL_rom_sout("\r\nlBaseTime_inSec= ");
		vSERIAL_IV32out(lBaseTime_inSec);

		vSERIAL_rom_sout("\r\nlEndTime_inSec= ");
		vSERIAL_IV32out(lEndTime_inSec);

		vSERIAL_rom_sout("\r\nlSampleInterval_inSec= ");
		vSERIAL_IV32out(lSampleInterval_inSec);

		vSERIAL_rom_sout("\r\nlSample0_time= ");
		vSERIAL_IV32out(lSample0_time);

		vSERIAL_rom_sout("\r\nlSample0_to_BaseTime_inSec= ");
		vSERIAL_IV32out(lSample0_to_BaseTime_inSec);

		vSERIAL_rom_sout("\r\nlNumberOfSamples= ");	
		vSERIAL_IV32out(lNumberOfSamples);

		vSERIAL_crlf();
		#endif
		}

	/* WE ARE NOW READY TO CALCULATE THE NEXT NST SLOTS USED BY THIS EVENT */
	/* LOOP FOR ALL SAMPLES THAT ARE WITHIN THIS FRAME */
	for(lSampleCnt=lNumberOfSamples;  ;  lSampleCnt++)
		{
		lThisSampleTime = lSample0_time + (lSampleInterval_inSec * lSampleCnt);

		if(lThisSampleTime > lEndTime_inSec) break;	//termination condition

		if(lThisSampleTime > lBaseTime_inSec)		//insist we stay in this frame
			{
			/* CONVERT THIS SAMPLE TIME TO A SLOT IDX */
			lThisSlotIdx = (lThisSampleTime - lBaseTime_inSec) / SECS_PER_SLOT_L;

			ucFoundSlot = ucRTS_findNearestNSTslot(
								ucNST_tblNum,				//NST tbl (0 or 1)
								(uchar)lThisSlotIdx	//Desired slot
								);

			if(ucStblIdx == ucGLOB_TC12StblIdx)
				{
				#if 1
				vSERIAL_rom_sout("lThisSampleTime= ");
				vSERIAL_IV32out(lThisSampleTime);
				vSERIAL_rom_sout("\r\nlThisSlotIdx= ");
				vSERIAL_IV32out(lThisSlotIdx);
				vSERIAL_rom_sout("\r\nGotSlot= ");
				vSERIAL_UI8out(ucFoundSlot);
				vSERIAL_crlf();
				#endif
				}
	
			ucFlags = (uchar)ulL2SRAM_getStblEntry(
									SCHED_FLAG_TBL_NUM,	//Sched tbl num
									ucStblIdx				//Sched tbl idx
									);

			/* IF THIS EVENT REQUIRES AND EXACT SLOT -- SKIP IT */
			if((ucFlags & F_USE_EXACT_SLOT) &&
			   (ucFoundSlot != (uchar)lThisSlotIdx))
				{
				#if 1
				vSERIAL_rom_sout("RTS:SchedIntrvl:ExactSltNotAvail\r\n");	
				#endif
				break;
				}


			/* CHK IF NO SLOTS AVAILABLE */
			if(ucFoundSlot >= GENERIC_NST_MAX_IDX)
				{
				#if 1
				vSERIAL_rom_sout("RTS:SchedIntrvl:NoNSTsltsAvail\r\n");	
				#endif
				break;
				}

			/* YES WE HAVE A SLOT -- STUFF IT */
			vRTS_putNSTentry(ucNST_tblNum, ucFoundSlot, ucStblIdx);

			/* CHECK FOR MAX INDEX ERROR */
			if(ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST)
				{
				vSERIAL_rom_sout("RTS:MxNSTidxInSchedule_interval_slot\r\n");
				ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST = 0;
				}

			}/* END: if(lThisSampleTime) */
		
		}/* END: for(lSampleCnt) */

	if(ucStblIdx == ucGLOB_TC12StblIdx)
		{
		#if 0
		vSERIAL_rom_sout("RTS:SchedIntrvl- ");
		vACTION_showStblActionName(ucStblIdx);
		//vRTS_showAllNSTentrys(ucNST_tblNum,YES_SHOW_STBL);
		#endif
		}

	return;

	}/* END: vRTS_schedule_interval_slot() */






/******************  vRTS_schedule_loadonly_slot()  ************************
*
* This schedules evenly spaced NST slots -- count is set by load
*
*****************************************************************************/

static void vRTS_schedule_loadonly_slot(
		uchar ucStblIdx,			//Tbl Idx of action to schedule
		long lFrameNumToSched		//Frame number to schedule
		)
	{
	uchar ucNST_tblNum;
	uchar ucPrevNSTtblNum;
	uchar ucc;
	uchar ucReqSlot;
	uchar ucFoundSlot;
	uchar ucLoadValOnly;
	uint uiPreviousLoad;
	uint uiNextLoad;


	/* GET THE NST TABLE NUMBER FROM THE FRAME NUMBER */
	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNumToSched);

	/* GET THE PREVIOUS NST TABLE NUM */
	ucPrevNSTtblNum = (ucNST_tblNum ^ 0x01 );


	/*------------ COPY THE LOAD NUM OVER TO NEXT LOAD  ------------------*/

	/* LOAD THE PREVIOUS LOAD VALUE */
	uiPreviousLoad = (uint)ulL2SRAM_getStblEntry(
							ucaDcntTblNum[ucPrevNSTtblNum],	//Sched tbl num
							ucStblIdx						//Sched tbl idx
							);

	/* GET THE NEW LOAD VALUE */
	uiNextLoad = uiSTBL_handleLoadDowncnt(uiPreviousLoad);


	/* STASH THE NEW LOAD IN THE NEXT TBL */
	vL2SRAM_putStblEntry(	ucaDcntTblNum[ucNST_tblNum],
						ucStblIdx,
						(ulong)uiNextLoad
						);

	#if 0
	vSERIAL_rom_sout("PrevLd= ");
	vSERIAL_UI8out(uiPreviousLoad);
	vSERIAL_rom_sout("  NxtLd = ");
	vSERIAL_UI8out(uiNextLoad);
	vSERIAL_crlf();
	#endif

	/*----------- DECIDE IF THIS ACTION IS TO RUN --------------------------*/

	/* IF NO ALARM SET  -- DON'T SCHEDULE IT */
	if(!(uiNextLoad & F_DCNT_ALARM))	goto LoadOnly_event_exit;

	/* IF DOWNCOUNT = 0  -- DON'T SCHEDULE IT */
	if((uiNextLoad & F_DCNT_COUNT_ONLY_MASK) == 0) goto LoadOnly_event_exit;	


	/*-------------  FILL THE NST ENTRIES  ----------------------------------*/

	/* TRY TO CAPTURE EQUA-DISTANT LOCATIONS */
	ucLoadValOnly = (uchar)(uiNextLoad & F_DCNT_COUNT_ONLY_MASK);
	if(ucLoadValOnly >= GENERIC_NST_MAX_IDX_MASK) ucLoadValOnly = GENERIC_NST_MAX_IDX;
	for(ucc=0;  ucc<ucLoadValOnly;  ucc++)
		{
		ucReqSlot = (uchar)(ucc * (GENERIC_NST_MAX_IDX/(ucLoadValOnly + 1)));

		/* LOOK FOR A SLOT */
		ucFoundSlot = ucRTS_findNearestNSTslot(
								ucNST_tblNum,	//NST tbl (0 or 1)
								ucReqSlot			//Desired slot
								);

		/* IF NO SLOTS AVAILABLE -- STOP NOW */
		if(ucFoundSlot >= GENERIC_NST_MAX_IDX) break;

		/* YES WE HAVE A SLOT -- STUFF IT */
		vRTS_putNSTentry(ucNST_tblNum, ucFoundSlot, ucStblIdx);

		/* CHECK FOR MAX INDEX ERROR */
		if(ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST)
			{
			vSERIAL_rom_sout("RTS:MxNSTidxInSchedule_LdOnly_slot\r\n");
			ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST = 0;
			}

		}/* END: for(ucc) */

LoadOnly_event_exit:

	#if 0
	{
	vSERIAL_rom_sout("RTS:SchedLdOnly- ");
	vACTION_showStblActionName(ucStblIdx);
	vSERIAL_crlf();
	//vRTS_showAllNSTentrys(ucNST_tblNum,YES_SHOW_STBL);
	}
	#endif

	return;

	}/* END: vRTS_schedule_loadonly_slot() */






/***********************  ucRTS_findNearestNSTslot() *************************
*
* Ret: 	0 - 63 slot number if found
*		0xCC if not found
*
*****************************************************************************/

uchar ucRTS_findNearestNSTslot(
		uchar ucNST_tblNum,		//0 or 1
		uchar ucDesiredSlot
		)
	{
	int iSlotDist;
	int iDesiredSlot;
	int iPosTestSlotNum;
	int iNegTestSlotNum;
	uchar ucFoundSlotNum;
	uchar ucThisSlotVal;



	iDesiredSlot = (int)ucDesiredSlot;			//convert desired slot to int
	ucFoundSlotNum = GENERIC_NST_NOT_USED_VAL;	//assume no free slot found

	/* HUNT STARTS AT THE DESIRED SLOT AND RADIATES OUTWARD */
	for(iSlotDist=0;  iSlotDist<GENERIC_NST_MAX_IDX;  iSlotDist++)
		{
		iPosTestSlotNum = iDesiredSlot + iSlotDist;
		iNegTestSlotNum = iDesiredSlot - iSlotDist;

		/* CHECK FOR LOOP TERMINATION CONDITION */
		if((iPosTestSlotNum >= GENERIC_NST_MAX_IDX) &&
		   (iNegTestSlotNum < 0))
			{
			break;
			}

		/* CHECK THE POSITIVE FIRST */
		if(iPosTestSlotNum < GENERIC_NST_MAX_IDX)
			{
			ucThisSlotVal = ucRTS_getNSTentry(
											ucNST_tblNum,
											(uchar)iPosTestSlotNum
											);

			if((ucThisSlotVal == GENERIC_NST_NOT_USED_VAL) ||
			   (ucThisSlotVal == SLEEP_STBL_IDX))
				{
				ucFoundSlotNum = (uchar)iPosTestSlotNum;
				break;
				}

			}/* END: if(iPosTestSlotNum) */


		/* NOW CHECK THE NEGATIVE */
		if(iNegTestSlotNum >= 0)
			{
			ucThisSlotVal = ucRTS_getNSTentry(
											ucNST_tblNum,
											(uchar)iNegTestSlotNum
											);

			if((ucThisSlotVal == GENERIC_NST_NOT_USED_VAL) ||
			   (ucThisSlotVal == SLEEP_STBL_IDX))
				{
				ucFoundSlotNum = (uchar)iNegTestSlotNum;
				break;
				}

			}/* END: if(iNegTestSlotNum) */

		}/* END: for(ucSlotDist) */

	return(ucFoundSlotNum);

	}/* END: ucRTS_findNearestNSTslot() */







/*********************  vRTS_runScheduler()  **********************************
*
*
*
******************************************************************************/

void vRTS_runScheduler(
		void
		)
	{
	long lNextFrameNum;

	/* GET THE NEXT FRAME NUMBER */
	lNextFrameNum = lGLOB_lastAwakeFrame + 1L;

	/* SAVE THE CURRENT TIME INTO FRAM IN CASE WE CRASH */
	vL2FRAM_stuffSavedTime((ulong)lTIME_getSysTimeAsLong());

	#if 0
	/* GO RUN THE EVALUATOR */
	vSYSACT_do_EvaluateSts();
	#endif

	/* GO HANDLE THE SCHEDULING */
	vRTS_scheduleNSTtbl(lNextFrameNum);

	/* COMPUTE THE SYSTEM LFACTOR */
	vSTBL_computeSysLFactor();

	/* SHOW THE NST TABLE */
	#if 0
	{
	uchar ucNST_tblNum;
	ucNST_tblNum = (uchar)(lNextFrameNum % 2L);	//compute NST tbl from frame
	vRTS_showAllNSTentrys(ucNST_tblNum,YES_SHOW_STBL);
	}
	#endif

	return;

	}/* END: vRTS_runScheduler() */






/**********************  vRTS_showActionHdrLine()  *********************************
*
*
*
******************************************************************************/
void vRTS_showActionHdrLine(
		uchar ucCRLF_termFlg	//YES_CRLF, NO_CRLF
		)
	{
	uchar ucStblIdx;
	long lUpTime;
	long lTmp;

	/* GET THE STBL NST IDX */
	ucStblIdx = ucRTS_getNSTentry(
							ucGLOB_lastAwakeNSTtblNum,
							ucGLOB_lastAwakeSlot
							);

	vDAYTIME_convertSysTimeToShowDateAndTime(NUMERIC_FORM);		//daytime
	vSERIAL_rom_sout("  ");

	vSERIAL_HBV32out((ulong)lGLOB_lastAwakeFrame);  //frame num
	vSERIAL_bout(':');
	vSERIAL_HB8out(ucGLOB_lastAwakeSlot);			//slot num

	vSERIAL_rom_sout("  ");
	vACTION_showStblActionName(ucStblIdx);

	vSERIAL_rom_sout("  ");							//Sys version num
	vMAIN_showVersionNum();
	vSERIAL_bout(':');
//	vSERIAL_rom_sout("  ");
	vMODOPT_showCurRole();
	vSERIAL_bout(':');
	vL2FRAM_showSysID();							//sys ID



	vSERIAL_rom_sout("  Up ");						//uptime
	lUpTime = (lTIME_getSysTimeAsLong() - lGLOB_initialStartupTime);
	/* COMPUTE DAYS UP */
	lTmp = lUpTime / 86400L;
	vSERIAL_IV32out(lTmp);			//Days
	vSERIAL_bout(':');
	/* SHOW REST OF UP TIME */
	lUpTime %= 86400L;
	vDAYTIME_convertSecToShow(lUpTime);

	if(ucCRLF_termFlg)
		vSERIAL_crlf();

	return;

	}/* END: vRTS_showActionHdrLine() */








/********************  vRTS_convertAllRDC4slotsToSleep()  ********************
*
* converts the current NST/Next NST RDC4's to sleep
*
******************************************************************************/
void vRTS_convertAllRDC4slotsToSleep(
		void
		)
	{
	uchar ucNST_slotCnt;
	uchar ucNST_tblNum;
	uchar ucStblIdx;

	#if 1
	vSERIAL_rom_sout("ChgRDC4toSlp\r\n");
	#endif

	/* DO BOTH NST TABLES */
	for(ucNST_tblNum = 0;  ucNST_tblNum<2;  ucNST_tblNum++)
		{
		for(ucNST_slotCnt=0;  ucNST_slotCnt<GENERIC_NST_MAX_IDX;  ucNST_slotCnt++)
			{
			/* GET THE NST ENTRIES */
			ucStblIdx = ucRTS_getNSTentry(ucNST_tblNum, ucNST_slotCnt);

			/* FIND THE SDC4 ENTRYS */
			if(ucStblIdx == ucGLOB_RDC4StblIdx)
				{
				/* FOUND ONE CONVERT IT TO SLEEP */
				vRTS_putNSTentry(ucNST_tblNum, ucNST_slotCnt, SLEEP_STBL_IDX);

				/* CHECK FOR MAX INDEX ERROR */
				if(ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST)
					{
					vSERIAL_rom_sout("RTS:MxNSTidxInConvertAllRDC4slotsToSlp\r\n");
					ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST = 0;
					}

				#if 0
				vSERIAL_rom_sout("SettingNST ");
				vSERIAL_HB8out(ucNST_tblNum);
				vSERIAL_rom_sout("  Idx ");
				vSERIAL_UIV8out(ucNST_slotCnt);
				vSERIAL_rom_sout(" toSlp (");
				vSERIAL_UIV8out(SLEEP_STBL_IDX);
				vSERIAL_rom_sout(")\r\n");
				#endif
				}

			}/* END: for(ucNST_slotCnt) */

		}/* END: for(ucNST_tblNum) */

	#if 0
	vRTS_showAllNSTentrys(0,YES_SHOW_STBL);
	vRTS_showAllNSTentrys(1,YES_SHOW_STBL);
	#endif

	return;

	}/* END: vRTS_convertAllRDC4slotsToSleep() */








/********************  ucRTS_nextNSTnum()  ***********************************
*
*
******************************************************************************/

uchar ucRTS_nextNSTnum(
		void
		)
	{
	long lNextFrameNum;
	uchar ucNextNSTnum;

	/* GET THE NEXT FRAME NUMBER */
	lNextFrameNum = lGLOB_lastAwakeFrame + 1L;

	/* GET THE NEXT NST NUMBER */
	ucNextNSTnum = (uchar)(lNextFrameNum % 2L);	//compute NST tbl from frame

	return(ucNextNSTnum);

	}/* END: ucRTS_nextNSTnum() */






/********************  ucRTS_thisNSTnum()  *****************************
*
*
******************************************************************************/

uchar ucRTS_thisNSTnum(
		void
		)
	{
	uchar ucthisNSTnum;

	/* GET THE NEXT NST NUMBER */
	ucthisNSTnum = (uchar)(lGLOB_lastAwakeFrame % 2L);	//compute NST tbl from frame

	return(ucthisNSTnum);

	}/* END: ucRTS_thisNSTnum() */






#if 0
/*********************  NOTE  ************************************
 *
 * Under the new scheduler change the scheduler runs at the
 * end of a frame so there is no need to check if it has already
 * run.
 *
 *****************************************************************/

/*********************  ucRTS_hasSchedRun()  *********************************
*
* Ret 1 if shed has run
*	  0 if sched not run
*
* if in currently in sched rets 0
*
******************************************************************************/

uchar ucRTS_hasSchedRun(
		void
		)
	{
	if(lGLOB_lastScheduledFrame > lGLOB_lastAwakeFrame) return(1);
	return(0);

	}/* END: ucRTS_hasSchedRun() */
#endif





/**********************  ucRTS_lastScheduledNSTnum()  ************************
*
* Returns the index of the last scheduled NST tbl
*
******************************************************************************/

uchar ucRTS_lastScheduledNSTnum(
		void
		)
	{

	#if 0
	/*********************  NOTE  ************************************
	 *
	 * Under the new scheduler change the scheduler runs at the
	 * end of a frame so there is no need to check if it has already
	 * run.
	 *
	 *****************************************************************/
	if(ucRTS_hasSchedRun()) return(ucRTS_nextNSTnum());
	#endif

	return(ucRTS_thisNSTnum());

	}/* END: ucRTS_lastScheduledNSTnum() */





/*-------------------------------  MODULE END  ------------------------------*/

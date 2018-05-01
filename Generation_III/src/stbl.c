
/**************************  STBL.C  *****************************************
*
* Routines to access the Stbl (sched tbl) values
*
*
* V1.00 03/02/2005 wzr
*		Started
*
******************************************************************************/


/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e716 */		/* while(1) */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e752 */		/* local declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


#include <msp430x54x.h>		//processor reg description 
#include "std.h"			//standard defines
//#include "diag.h"			//Diagnostic package
#include "config.h" 		//system configuration description file
//#include "main.h"			//
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
//#include "time.h"			//Time routines
#include "daytime.h"		//Daytime routines
#include "action.h" 		//Event action routines
#include "sensor.h"			//Sensor names
#include "stbl.h"			//Real Time Scheduler routines
//#include "fulldiag.h"		//full blown diagnostic defines
//#include "delay.h"  		//delay routines
//#include "buz.h"			//buzzer routines
//#include "msg.h"			//radio msg helper routines
#include "rand.h"			//Random number generator
//#include "event.h"		//Other events
#include "l2fram.h"			//Level 2 Ferro Ram routines 
//#include "time.h"			//Time routines
#include "rts.h"			//Real time sched routines
#include "rad40.h"			//Radix 40 name conversion
#include "senseact.h"		//Sense Act name routines
#include "MODOPT.h" 		//role flags for wiz routines







extern const uchar ucaBitMask[8];

extern uchar ucGLOB_StblIdx_NFL;		//Max number of sched entrys
extern long   lGLOB_lastAwakeFrame;		//Nearest thing to cur frame
extern uchar ucGLOB_lastAwakeSlot;		//Nearest thing to cur slot
extern uchar ucGLOB_lastAwakeNSTtblNum; //Nearest thing to cur NST tbl

extern uchar ucGLOB_SDC4StblIdx;	//Sched tbl idx for SDC4  function
extern uchar ucGLOB_RDC4StblIdx;	//Sched tbl idx for RDC4  function

extern uchar ucGLOB_TC12StblIdx;	//DEBUG: Sched tbl idx for TC12
extern uchar ucGLOB_TC34StblIdx;	//DEBUG: Sched tbl idx for TC34
extern uchar ucGLOB_LT12StblIdx;	//DEBUG: Sched tbl idx for LT12
extern uchar ucGLOB_LT34StblIdx;	//DEBUG: Sched tbl idx for LT34

extern int    iGLOB_completeSysLFactor;	//entire Signed LFactor quantity
//extern uchar ucGLOB_msgSysLFactor;		//msg (byte size) unsigned LFactor

extern uint uiGLOB_lostROM2connections;	//counts lost ROM2's
extern uint uiGLOB_lostSOM2connections;	//counts lost SOM2's

extern uint uiGLOB_ROM2attempts;		//count number of ROM2 attempts
extern uint uiGLOB_SOM2attempts;		//count number of SOM2 attempts

extern uint uiGLOB_TotalSDC4trys;		//counts number of SDC4 attempts
extern uint uiGLOB_TotalRDC4trys;		//counts number of RDC4 attempts


/*-----------------------  SCHEDULER TABLES HERE  ---------------------------*/

/* THIS ROM TBL IS INDEXED BY SCHED TBL NUM & CONTAINS THE TBL BASE ADDRS */
const USL uslStblBaseAddr[MAX_SCHED_TBL_NUM] = 	
	{
	SCHED_PRIORITY_FUNC_TBL_BASE_ADDR,	// 0 USE TBL
	SCHED_SN_TBL_BASE_ADDR,				// 1 SERIAL NUM TBL
	SCHED_RAND_TBL_1_BASE_ADDR,			// 2 RANDOM NUM TBL 0
	SCHED_RAND_TBL_2_BASE_ADDR,			// 3 RANDOM NUM TBL 1
	SCHED_DCNT_TBL_1_BASE_ADDR,			// 4 DOWNCOUNT TBL 0
	SCHED_DCNT_TBL_2_BASE_ADDR,			// 5 DOWNCOUNT TBL 1
	SCHED_ST_TBL_BASE_ADDR,				// 6 EVENT START TIME TBL
	SCHED_INTRVL_TBL_BASE_ADDR,			// 7 EVENT INTERVAL TIME TBL
	SCHED_FLAG_TBL_BASE_ADDR,			// 8 EVENT FLAG TBL
	SCHED_STAT_TBL_1_BASE_ADDR,			// 9 EVENT STS TBL 0
	SCHED_STAT_TBL_2_BASE_ADDR,			//10 EVENT STS TBL 1
	SCHED_ACTN_TBL_BASE_ADDR,			//11 EVENT ACTION NUM TBL
	SCHED_LNKBLK_COUNT_TBL_BASE_ADDR,		//12 LNKBLK IDX TBL
	SCHED_SENSE_ACT_TBL_BASE_ADDR		//13 SENSE ACT TBL
	}; /* END: uslStblBaseAddr[] */

const char *cpaStblLongName[MAX_SCHED_TBL_NUM] = 
	{
	"PFN",			// 0 PRIORITY_FUNCTION TBL
	"SN",			// 1 SERIAL NUM TBL
	"RDM0",			// 2 RANDOM NUM TBL 0
	"RDM1",			// 3 RANDOM NUM TBL 1
	"DC0",			// 4 DOWNCOUNT TBL 0
	"DC1",			// 5 DOWNCOUNT TBL 1
	"BGTIM",		// 6 EVENT START TIME TBL
	"INTVL",		// 7 EVENT INTERVAL TIME TBL
	"USE",			// 8 EVENT FLAG TBL
	"STS1",			// 9 EVENT STS 1 TBL
	"STS2",			//10 EVENT STS 2 TBL
	"ACTN",			//11 EVENT ACTION NUM TBL
	"LBLK",			//12 LNKBLK IDX TBL
	"SENS"			//13 SENSE ACT TBL
	}; /* END: cpaStblLongName[] */


#define TABLE_IDX_COLWIDTH			 2		// TABLE INDEX NUMBER 
#define	PRIORITY_FUNC_COLWIDTH		 5		// 0 PRIORITY FUNCTION TBL
#define	SN_COLWIDTH					 3		// 1 SERIAL NUM TBL
#define	RAND_NUM_0_COLWIDTH			 6		// 2 RANDOM NUM TBL 0
#define	RAND_NUM_1_COLWIDTH			 6		// 3 RANDOM NUM TBL 1

//#define	DCOUNT_0_COLWIDTH			 4		// 4 DOWNCOUNT TBL 0
#define	DCOUNT_0_COLWIDTH			 -1		// 4 DOWNCOUNT TBL 0

//#define	DCOUNT_1_COLWIDTH			 4		// 5 DOWNCOUNT TBL 1
#define	DCOUNT_1_COLWIDTH			 -1		// 5 DOWNCOUNT TBL 1

#define	START_TIME_COLWIDTH			 4		// 6 EVENT START TIME TBL
#define	INTERVAL_TIME_COLWIDTH		 8		// 7 EVENT INTERVAL TIME TBL
#define	FLAG_COLWIDTH				 2		// 8 EVENT FLAG TBL
#define	STS_0_COLWIDTH				 4		// 9 EVENT STS 0 TBL
#define	STS_1_COLWIDTH				 4		//10 EVENT STS 1 TBL
#define	ACTN_NUM_COLWIDTH			 3		//11 EVENT ACTION NUM TBL

//#define	LNKBLK_IDX_COLWIDTH			 2		//12 LNKBLK IDX TBL
#define	LNKBLK_IDX_COLWIDTH			 -1		//12 LNKBLK IDX TBL

#define	SENSE_ACT_COLWIDTH			11		//13 SENSE ACT TBL


#define PRIORITY_FUNC_COL1		(TABLE_IDX_COLWIDTH + 1)
#define	SN_COL1					(PRIORITY_FUNC_COL1 + PRIORITY_FUNC_COLWIDTH + 1)
#define	RAND_NUM_0_COL1			(SN_COL1 + SN_COLWIDTH + 1) 
#define	RAND_NUM_1_COL1			(RAND_NUM_0_COL1 + RAND_NUM_0_COLWIDTH + 1) 
#define	DCOUNT_0_COL1			(RAND_NUM_1_COL1 + RAND_NUM_1_COLWIDTH + 1)
#define	DCOUNT_1_COL1			(DCOUNT_0_COL1 + DCOUNT_0_COLWIDTH + 1)
#define	START_TIME_COL1			(DCOUNT_1_COL1 + DCOUNT_1_COLWIDTH + 1)
#define	INTERVAL_TIME_COL1		(START_TIME_COL1 + START_TIME_COLWIDTH + 1)
#define	FLAG_COL1				(INTERVAL_TIME_COL1 + INTERVAL_TIME_COLWIDTH + 1)
#define	STS_0_COL1				(FLAG_COL1 + FLAG_COLWIDTH + 1)
#define	STS_1_COL1				(STS_0_COL1 + STS_0_COLWIDTH + 1)
#define	ACTN_NUM_COL1			(STS_1_COL1 + STS_1_COLWIDTH + 1)
#define	LNKBLK_IDX_COL1			(ACTN_NUM_COL1 + ACTN_NUM_COLWIDTH + 1)
#define	SENSE_ACT_COL1			(LNKBLK_IDX_COL1 + LNKBLK_IDX_COLWIDTH + 1)


/* THIS ROM TBL IS INDEXED BY SCHED TBL NUM & CONTAINS THE COLUMN TAB # FOR EACH TBL */
const uchar ucColTabTbl[MAX_SCHED_TBL_NUM] = 
	{
	PRIORITY_FUNC_COL1	,			// 0 PRIORITY FUNCTION TBL
	SN_COL1				,			// 1 SERIAL NUM TBL
	RAND_NUM_0_COL1		,			// 2 RANDOM NUM TBL 0
	RAND_NUM_1_COL1		,			// 3 RANDOM NUM TBL 1
	DCOUNT_0_COL1		,			// 4 DOWNCOUNT TBL 0
	DCOUNT_1_COL1		,			// 5 DOWNCOUNT TBL 1
	START_TIME_COL1		,			// 6 EVENT START TIME TBL
	INTERVAL_TIME_COL1	,			// 7 EVENT INTERVAL TIME TBL
	FLAG_COL1			,			// 8 EVENT FLAG TBL
	STS_0_COL1			,			// 9 EVENT STS 1 TBL
	STS_1_COL1			,			//10 EVENT STS 2 TBL
	ACTN_NUM_COL1		,			//11 EVENT ACTION NUM TBL
	LNKBLK_IDX_COL1		,			//12 LNKBLK IDX TBL				//this entry deleted
	SENSE_ACT_COL1					//13 SENSE ACT TBL
	}; /* END: ucColTabTbl[] */



/* THIS ROM TBL IS INDEXED BY SCHED TBL NUM & CONTAINS BYTE WIDTH OF EACH TBL */
const uchar ucStblByteWidth[MAX_SCHED_TBL_NUM] = 
	{
	SCHED_PRIORITY_FUNC_TBL_BYTE_WIDTH,	// 0 PRIORITY FUNCTION TBL
	SCHED_SN_TBL_BYTE_WIDTH,			// 1 SERIAL NUM TBL
	SCHED_RAND_TBL_1_BYTE_WIDTH,		// 2 RANDOM NUM TBL 0
	SCHED_RAND_TBL_2_BYTE_WIDTH,		// 3 RANDOM NUM TBL 1
	GENERIC_DCNT_TBL_BYTE_WIDTH,		// 4 DOWNCOUNT TBL 0
	GENERIC_DCNT_TBL_BYTE_WIDTH,		// 5 DOWNCOUNT TBL 1
	SCHED_ST_TBL_BYTE_WIDTH,			// 6 EVENT START TIME TBL
	SCHED_INTRVL_TBL_BYTE_WIDTH,		// 7 EVENT INTERVAL TIME TBL
	SCHED_FLAG_TBL_BYTE_WIDTH,			// 8 EVENT FLAG TBL 
	GENERIC_STAT_TBL_BYTE_WIDTH,		// 9 EVENT STAT 1 TBL
	GENERIC_STAT_TBL_BYTE_WIDTH,		//10 EVENT STAT 2 TBL
	SCHED_ACTN_TBL_BYTE_WIDTH,			//11 EVENT ACTION NUM TBL
	SCHED_LNKBLK_COUNT_TBL_BYTE_WIDTH,	//12 LNKBLK IDX TBL
	SCHED_SENSE_ACT_TBL_BYTE_WIDTH		//13 SENSE ACT TBL
	}; /* END: ucStblByteWidth[] */



/* THIS ROM TBL IS INDEXED BY SCHED TBL NUM & CONTAINS NOT USED VALUE OF EACH TBL */
const ulong ulStblNotUsedVal[MAX_SCHED_TBL_NUM] = 
	{
	SCHED_PRIORITY_FUNC_NOT_USED_VAL,		// 0 PRIORITY FUNCTION TBL
	SCHED_SN_TBL_NOT_USED_VAL,				// 1 SERIAL NUM TBL
	SCHED_RAND_TBL_1_NOT_USED_VAL,			// 2 RANDOM NUM TBL 0
	SCHED_RAND_TBL_2_NOT_USED_VAL,			// 3 RANDOM NUM TBL 1
	GENERIC_DCNT_TBL_NOT_USED_VAL,			// 4 DOWNCOUNT TBL 0
	GENERIC_DCNT_TBL_NOT_USED_VAL,			// 5 DOWNCOUNT TBL 1
	SCHED_ST_TBL_NOT_USED_VAL,				// 6 EVENT START TIME TBL
	SCHED_INTRVL_TBL_NOT_USED_VAL,			// 7 EVENT INTERVAL TIME TBL
	SCHED_FLAG_TBL_NOT_USED_VAL,			// 8 EVENT FLAG TBL
	GENERIC_STAT_TBL_NOT_USED_VAL,			// 9 EVENT STAT 1 TBL
	GENERIC_STAT_TBL_NOT_USED_VAL,			//10 EVENT STAT 2 TBL
	SCHED_ACTN_TBL_NOT_USED_VAL,			//11 EVENT ACTION NUM TBL
	SCHED_LNKBLK_COUNT_TBL_NOT_USED_VAL,		//12 LNKBLK IDX TBL
	SCHED_SENSE_ACT_TBL_NOT_USED_VAL_UL		//13 SENSE ACT TBL
	}; /* END: ucStblNotUsedVal[] */




/*------------------  cpaPriorityFuncName[]  --------------------------------
*
* This table is indexed by Function Number and contains the Function Name
* of each Scheduler function that can be performed.
*
*----------------------------------------------------------------------------*/
const char *cpaPriorityFuncName[SCHED_FUNC_MAX_COUNT] =
	{
	"---",		//0 Dormant
	"Scd",		//1 scheduler
	"SD4",		//2 Send DC4
	"RD4",		//3 Receive DC4
	"All",		//4 All slots
	"OM2",		//5 OM2
	"Ivl",		//6 Interval
	"Lod"		//7 Load based

	}; /* END: cpaPriorityFuncName[] */


/* THIS ROM TBL IS INDEXED BY NST TBL NUM (1 or 2) & CONTAINS RANDOM NUM TBL BASE ADDRS */
extern const uchar ucaRandTblNum[2];


/* THIS ROM TBL IS INDEXED BY NST TBL NUM (1 or 2) & CONTAINS DOWNCOUNT TBL BASE ADDRS */
extern const uchar ucaDcntTblNum[2]; 


/* THIS ROM TBL IS INDEXED BY NST TBL NUM (1 or 2) & CONTAINS STS TBL BASE ADDRS */
extern const uchar ucaStsTblNum[2];



/* This table is indexed by action number and contains the PRIORITY_FUNCTION */
/* number of each action that can be performed */
const uchar ucaSampRecPriorityFunc[E_ACTN_MAX_NUM_IDX] =
	{
					/* INDEXED BY ACTION NUM */
	PRIORITY_7 | SCHED_FUNC_DORMANT_SLOT, 	//Actn  0 (SLEEP)
	PRIORITY_6 | SCHED_FUNC_INTERVAL_SLOT,	//Actn  1 (SEND TEST MSG)
	PRIORITY_6 | SCHED_FUNC_INTERVAL_SLOT,	//Actn  2 (GET FAKE DATA FROM SD)
	PRIORITY_6 | SCHED_FUNC_INTERVAL_SLOT,	//Actn  3 (BUILD YOUR OWN SENSE ROUTINE)
	PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT,	//Actn  4 (BATT SENSE)
	PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT,	//Actn  5 (TC-12)
	PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT,	//Actn  6 (TC-34)
	PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT,	//Actn  7 (LT-12)
	PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT,	//Actn  8 (SL-12)
	PRIORITY_0 | SCHED_FUNC_SCHEDULER_SLOT,	//Actn  9 (SCHEDULER)
	PRIORITY_2 | SCHED_FUNC_OM2_SLOT,		//Actn 10 (SOM2)
	PRIORITY_2 | SCHED_FUNC_OM2_SLOT, 		//Actn 11 (ROM2)
	PRIORITY_6 | SCHED_FUNC_INTERVAL_SLOT,	//Actn 12 (MOVE_SRAM_TO_FLASH)
	PRIORITY_3 | SCHED_FUNC_SDC4_SLOT,		//Actn 13 (SDC4)
	PRIORITY_3 | SCHED_FUNC_RDC4_SLOT,		//Actn 14 (RDC4)
	PRIORITY_6 | SCHED_FUNC_INTERVAL_SLOT,	//Actn 15 (REPORT)
	PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT,	//Actn 16 (LT-34)
	PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT,	//Actn 17 (LT-1234)
	PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT,	//Actn 18 (LT-1111)
	PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT,	//Actn 19 (VS_WSPEED_DIR)
	PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT,	//Actn 20 (VS_AIR_PRESS_TEMP)
	PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT,	//Actn 21 (VS_REL_HUMID_RAIN)
	PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT,	//Actn 22 (SAP-12)
	PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT	//Actn 23 (SAP-34)

	}; /* END: ucaSampRecPriority[] */


/* This rom table is parallel to the Priority table & contains */
/* the sample interval for the Action */
const uint uiaSampRecInterval[E_ACTN_MAX_NUM_IDX] =
	{
					/* INDEXED BY ACTION NUM */
	SCHED_INTRVL_TBL_NOT_USED_VAL,			//Actn  0 (SLEEP)
	SAMPLE_INTERVAL_TEST_MSG_I,				//Actn  1 (SEND TEST MSG)
	SAMPLE_INTERVAL_FAKE_SD_DATA_I,			//Actn  2 (GET FAKE DATA FROM SD)
	SAMPLE_INTERVAL_ROLL_YOUR_OWN_I,		//Actn  3 ((BUILD YOUR OWN SENSE ROUTINE)
	SAMPLE_INTERVAL_BATT_SENSE_I,			//Actn  4 (BATT SENSE)
	SAMPLE_INTERVAL_TC_12_I,				//Actn  5 (TC-12)
	SAMPLE_INTERVAL_TC_34_I,				//Actn  6 (TC-34)
	SAMPLE_INTERVAL_LT_12_I,				//Actn  7 (LT-12)
	SAMPLE_INTERVAL_SL_12_I,				//Actn  8 (SL-12)
	SCHED_INTRVL_TBL_NOT_USED_VAL,			//Actn  9 (SCHEDULER)
	SCHED_INTRVL_TBL_NOT_USED_VAL,			//Actn 10 (SOM2)
	SCHED_INTRVL_TBL_NOT_USED_VAL,			//Actn 11 (ROM2)
	SAMPLE_INTERVAL_MOVE_SRAM_TO_FLASH_I,	//Actn 12 (MOVE_SRAM_TO_FLASH)
	SCHED_INTRVL_TBL_NOT_USED_VAL,			//Actn 13 (SDC4)
	SCHED_INTRVL_TBL_NOT_USED_VAL,			//Actn 14 (RDC4)
	SAMPLE_INTERVAL_REPORT_I,				//Actn 15 (REPORT)
	SAMPLE_INTERVAL_LT_34_I,				//Actn 16 (LT-34)
	SAMPLE_INTERVAL_LT_1234_I,		 		//Actn 17 (LT-1234)
	SAMPLE_INTERVAL_LT_1111_I, 				//Actn 18 (LT-1111)
	SAMPLE_INTERVAL_VS_AVE_WSPEED_DIR_I,	//Actn 19 (VS_WSPEED_DIR)
	SAMPLE_INTERVAL_VS_AIR_PRESS_TEMP_I,	//Actn 20 (VS_AIR_PRESS_TEMP)
	SAMPLE_INTERVAL_VS_REL_HUMID_RAIN_I,	//Actn 21 (VS_REL_HUMID_RAIN)
	SAMPLE_INTERVAL_SAP_12_I,				//Actn 22 (SAP-12)
	SAMPLE_INTERVAL_SAP_34_I				//Actn 23 (SAP-34)
	}; /* END: uiaSampRecInterval[] */



/* This rom table is parallel to the Priority Table & contains */
/* the sample start times for the Action */
const uint uiaSampRecStart[E_ACTN_MAX_NUM_IDX] =
	{
						/* INDEXED BY ACTION NUM */
	0,											//Actn  0 (SLEEP)
	HR0_TO_SAMPLE0_TEST_MSG_IN_SEC,				//Actn  1 (SEND TEST MSG)
	HR0_TO_SAMPLE0_FAKE_SD_DATA_IN_SEC,			//Actn  2 (GET FAKE DATA FROM SD)
	HR0_TO_SAMPLE0_ROLL_YOUR_OWN_IN_SEC,		//Actn  3 (BUILD YOUR OWN SENSE ROUTINE)
	HR0_TO_SAMPLE0_BATT_SENSE_IN_SEC,			//Actn  4 (BATT SENSE)
	HR0_TO_SAMPLE0_TC_12_IN_SEC,				//Actn  5 (TC-12)
	HR0_TO_SAMPLE0_TC_34_IN_SEC,				//Actn  6 (TC-34)
	HR0_TO_SAMPLE0_LT_12_IN_SEC,				//Actn  7 (LT-12)
	HR0_TO_SAMPLE0_SL_12_IN_SEC,				//Actn  8 (SL-12)
	0,											//Actn  9 (SCHEDULER)
	0,											//Actn 10 (SOM2)
	0,											//Actn 11 (ROM2)
	HR0_TO_SAMPLE0_MOVE_SRAM_TO_FLASH_IN_SEC,	//Actn 12 (MOVE_SRAM_TO_FLASH)
	0,											//Actn 13 (SDC4)
	0,											//Actn 14 (RDC4)
	HR0_TO_SAMPLE0_REPORT_IN_SEC,				//Actn 15 (REPORT)
	HR0_TO_SAMPLE0_LT_34_IN_SEC,				//Actn 16 (LT-34)
	HR0_TO_SAMPLE0_LT_1234_IN_SEC,				//Actn 17 (LT-1234)
	HR0_TO_SAMPLE0_LT_1111_IN_SEC,				//Actn 18 (LT-1111)
	HR0_TO_SAMPLE0_VS_AVE_WSPEED_DIR_IN_SEC,	//Actn 19 (VS_WSPEED_DIR)
	HR0_TO_SAMPLE0_VS_AIR_PRESS_TEMP_IN_SEC,	//Actn 20 (VS_AIR_PRESS_TEMP)
	HR0_TO_SAMPLE0_VS_REL_HUMID_RAIN_IN_SEC,	//Actn 21 (VS_REL_HUMID_RAIN)
	HR0_TO_SAMPLE0_SAP_12_IN_SEC,				//Actn 22 (SAP-12)
	HR0_TO_SAMPLE0_SAP_34_IN_SEC				//Actn 23 (SAP-34)
	}; /* END: uiaSampRecStart[] */



/* This rom table is parallel to the Priority Table & contains */
/* the flags that are to be set in the sched flag tbls */
const uchar ucaSampRecFlags[E_ACTN_MAX_NUM_IDX] =
	{
				/* INDEXED BY ACTION NUM */
	USE_FLAGS_SLEEP,					//Actn  0 (SLEEP)
	USE_FLAGS_TEST_MSG,					//Actn  1 (SEND TEST MSG)
	USE_FLAGS_FAKE_SD_DATA,				//Actn  2 (GET FAKE DATA FROM SD)
	USE_FLAGS_ROLL_YOUR_OWN,			//Actn  3 (BUILD YOUR OWN SENSE ROUTINE)
	USE_FLAGS_BATT_SENSE,				//Actn  4 (BATT SENSE)
	USE_FLAGS_TC_12,					//Actn  5 (TC-12)
	USE_FLAGS_TC_34,					//Actn  6 (TC-34)
	USE_FLAGS_LT_12,					//Actn  7 (LT-12)
	USE_FLAGS_SL_12,					//Actn  8 (SL-12)
	USE_FLAGS_SCHEDULER,				//Actn  9 (SCHEDULER)
	USE_FLAGS_SOM2,						//Actn 10 (SOM2)
	USE_FLAGS_ROM2,						//Actn 11 (ROM2)
	USE_FLAGS_MOVE_SRAM_TO_FLASH,		//Actn 12 (MOVE_SRAM_TO_FLASH)
	USE_FLAGS_SDC4,						//Actn 13 (SDC4)
	USE_FLAGS_RDC4,						//Actn 14 (RDC4)
	USE_FLAGS_REPORT,					//Actn 15 (REPORT)
	USE_FLAGS_LT_34,					//Actn 16 (LT-34)
	USE_FLAGS_LT_1234,					//Actn 17 (LT-1234)
	USE_FLAGS_LT_1111,					//Actn 18 (LT-1111)
	USE_FLAGS_VS_AVE_WSPEED_DIR,		//Actn 19 (VS_WSPEED_DIR)
	USE_FLAGS_VS_AIR_PRESS_TEMP,		//Actn 20 (VS_AIR_PRESS_TEMP)
	USE_FLAGS_VS_REL_HUMID_RAIN,		//Actn 21 (VS_REL_HUMID_RAIN)
	USE_FLAGS_SAP_12,					//Actn 22 (SAP-12)
	USE_FLAGS_SAP_34					//Actn 23 (SAP-34)

	}; /* END: ucaSampRecFlags[] */




/* This rom table is parallel to the Priority Table & contains */
/* the who can run bits for each action */
const uchar ucaWhoCanRunBits[E_ACTN_MAX_NUM_IDX] =
	{
				/* INDEXED BY ACTION NUM */
	SLEEP_WHO_CAN_RUN,					//Actn  0 (SLEEP)
	TEST_MSG_WHO_CAN_RUN,				//Actn  1 (SEND TEST MSG)
	FAKE_SD_DATA_WHO_CAN_RUN,			//Actn  2 (GET FAKE DATA FROM SD)
	ROLL_YOUR_OWN_WHO_CAN_RUN,			//Actn  3 (BUILD YOUR OWN SENSE ROUTINE)
	BATT_SENSE_WHO_CAN_RUN,				//Actn  4 (BATT SENSE)
	TC12_WHO_CAN_RUN,					//Actn  5 (TC12)
	TC34_WHO_CAN_RUN,					//Actn  6 (TC34)
	LT12_WHO_CAN_RUN,					//Actn  7 (LT12)
	SL12_WHO_CAN_RUN,					//Actn  8 (SL12)
	SCHEDULER_WHO_CAN_RUN,				//Actn  9 (SCHEDULER)
	SOM2_WHO_CAN_RUN,					//Actn 10 (SOM2)
	ROM2_WHO_CAN_RUN,					//Actn 11 (ROM2)
	MOVE_SRAM_TO_FLASH_WHO_CAN_RUN,		//Actn 12 (MOVE_SRAM_TO_FLASH)
	SDC4_WHO_CAN_RUN,					//Actn 13 (SDC4)
	RDC4_WHO_CAN_RUN,					//Actn 14 (RDC4)
	REPORT_WHO_CAN_RUN,					//Actn 15 (REPORT)
	LT34_WHO_CAN_RUN,					//Actn 16 (LT34)
	LT1234_WHO_CAN_RUN,					//Actn 17 (LT1234)
	LT1111_WHO_CAN_RUN,					//Actn 18 (LT1111)
	VS_AVE_WSPEED_DIR_WHO_CAN_RUN,		//Actn 19 (VS_WSPEED_DIR)
	VS_AIR_PRESS_TEMP_WHO_CAN_RUN,		//Actn 20 (VS_AIR_PRESS_TEMP)
	VS_REL_HUMID_RAIN_WHO_CAN_RUN,		//Actn 21 (VS_REL_HUMID_RAIN)
	SAP12_WHO_CAN_RUN,					//Actn 22 (SAP12)
	SAP34_WHO_CAN_RUN					//Actn 23 (SAP34)

	}; /* END: ucaWhoCanRunBits[] */




/*----------------  ucaDefaultStartupAction ---------------------------------
*
* This rom table is parallel to the Priority Table & contains the default
* load flag for each action
*
* The way this table is used is that whenever a role is changed, the 
* startup blks in FRAM are erased and a new list of actions is installed.
* The criteria for choosing which actions are put into the new list is:
*	1. Only default actions are reloaded with a new role so this table
*	   is stepped through selecting out the default == YES values of action
*	   candidates.  Then:
*	2. Each candidate action is checked against the who can run table
*	   above and is allowed only if it passes both these tests (default
*	   and can_run).
*
*----------------------------------------------------------------------------*/


const uchar ucaDefaultStartupAction[E_ACTN_MAX_NUM_IDX] =
	{
				/* INDEXED BY ACTION NUM */
	YES,	//Actn  0 (SLEEP)
	NO,		//Actn  1 (SEND TEST MSG)
	NO,		//Actn  2 (GET FAKE DATA FROM SD)
	NO,		//Actn  3 (BUILD YOUR OWN SENSE ROUTINE)
	YES,	//Actn  4 (BATT SENSE)
	YES,	//Actn  5 (TC12)
	YES,	//Actn  6 (TC34)
	NO,		//Actn  7 (LT12)
	YES,	//Actn  8 (SL12)
	YES,	//Actn  9 (SCHEDULER)
	NO,		//Actn 10 (SOM2)
	NO,		//Actn 11 (ROM2)
	YES,	//Actn 12 (MOVE_SRAM_TO_FLASH)
	YES,	//Actn 13 (SDC4)
	YES,	//Actn 14 (RDC4)
	YES,	//Actn 15 (REPORT)
	NO,		//Actn 16 (LT34)
	YES,	//Actn 17 (LT1234)
	NO,		//Actn 18 (LT1111)
	NO,		//Actn 19 (VS_WSPEED_DIR)
	NO,		//Actn 20 (VS_AIR_PRESS_TEMP)
	NO,		//Actn 21 (VS_REL_HUMID_RAIN)
	NO,		//Actn 22 (SAP12)
	NO		//Actn 23 (SAP34)

	}; /* END: ucaDefaultStartupAction[] */






/* This rom table is parallel to the Priority Table & contains */
/* the status that is to be set in the sched sts tbls */
const uchar ucaSampRecStatus[E_ACTN_MAX_NUM_IDX] =
	{
	STS_LFACT_SLEEP,				//Actn  0 (SLEEP)
	STS_LFACT_TEST_MSG,				//Actn  1 (SEND TEST MSG)
	STS_LFACT_FAKE_SD_DATA,			//Actn  2 (GET FAKE DATA FROM SD)
	STS_LFACT_ROLL_YOUR_OWN,		//Actn  3 (BUILD YOUR OWN SENSE ROUTINE)
	STS_LFACT_BATT_SENSE,			//Actn  4 (BATT SENSE)
	STS_LFACT_TC_12,				//Actn  5 (TC-12)
	STS_LFACT_TC_34,				//Actn  6 (TC-34)
	STS_LFACT_LT_12,				//Actn  7 (LT-12)
	STS_LFACT_SL_12,				//Actn  8 (SL-12)
	STS_LFACT_SCHEDULER,			//Actn  9 (SCHEDULER)
	STS_LFACT_SOM2,					//Actn 10 (SOM2)
	STS_LFACT_ROM2,					//Actn 11 (ROM2)
	STS_LFACT_MOVE_SRAM_TO_FLASH,	//Actn 12 (MOVE_SRAM_TO_FLASH)
	STS_LFACT_SDC4,					//Actn 13 (SDC4)
	STS_LFACT_RDC4,					//Actn 14 (RDC4)
	STS_LFACT_REPORT,				//Actn 15 (REPORT)
	STS_LFACT_LT_34,				//Actn 16 (LT-34)
	STS_LFACT_LT_1234,				//Actn 17 (LT-1234)
	STS_LFACT_LT_1111,				//Actn 18 (LT-1111)
	STS_LFACT_VS_AVE_WSPEED_DIR,	//Actn 19 (VS_WSPEED_DIR)
	STS_LFACT_VS_AIR_PRESS_TEMP,	//Actn 20 (VS_AIR_PRESS_TEMP)
	STS_LFACT_VS_REL_HUMID_RAIN,	//Actn 21 (VS_REL_HUMID_RAIN)
	STS_LFACT_SAP_12,				//Actn 22 (SAP-12)
	STS_LFACT_SAP_34				//Actn 23 (SAP-34)

	}; /* END: ucaSampRecStatus[] */






/* This rom table is parallel to the Priority Table & contains */
/* the SENSE_ACT group to be performed */
const ulong ulaSenseActVal[E_ACTN_MAX_NUM_IDX] =
	{
	SENSE_ACT_SLEEP_VAL,				//Actn  0 (SLEEP)
	SENSE_ACT_TEST_MSG_VAL,				//Actn  1 (SEND TEST MSG)
	SENSE_ACT_FAKE_SD_DATA_VAL,			//Actn  2 (GET FAKE DATA FROM SD)
	SENSE_ACT_ROLL_YOUR_OWN,			//Actn  3 (BUILD YOUR OWN SENSE ROUTINE)
	SENSE_ACT_BATT_VAL,					//Actn  4 (BATT SENSE)
	SENSE_ACT_TC_12_VAL,				//Actn  5 (TC-12)
	SENSE_ACT_TC_34_VAL,				//Actn  6 (TC-34)
	SENSE_ACT_LT_12_VAL,				//Actn  7 (LT-12)
	SENSE_ACT_SL_12_VAL,				//Actn  8 (SL-12)
	SENSE_ACT_SCHEDULER_VAL,			//Actn  9 (SCHEDULER)
	SENSE_ACT_SOM2_VAL,					//Actn 10 (SOM2)
	SENSE_ACT_ROM2_VAL,					//Actn 11 (ROM2)
	SENSE_ACT_MOVE_SRAM_TO_FLASH_VAL,	//Actn 12 (MOVE_SRAM_TO_FLASH)
	SENSE_ACT_SDC4_VAL,					//Actn 13 (SDC4)
	SENSE_ACT_RDC4_VAL,					//Actn 14 (RDC4)
	SENSE_ACT_REPORT_VAL,				//Actn 15 (REPORT)
	SENSE_ACT_LT_34_VAL,				//Actn 16 (LT-34)
	SENSE_ACT_LT_1234_VAL,				//Actn 17 (LT-1234)
	SENSE_ACT_LT_1111_VAL,				//Actn 18 (LT-1111)
	SENSE_ACT_VS_AVE_WSPEED_DIR_VAL,	//Actn 19 (VS_WSPEED_DIR)
	SENSE_ACT_VS_AIR_PRESS_TEMP_VAL,	//Actn 20 (VS_AIR_PRESS_TEMP)
	SENSE_ACT_VS_REL_HUMID_RAIN_VAL,	//Actn 21 (VS_REL_HUMID_RAIN)
	SENSE_ACT_SAP_12_VAL,				//Actn 22 (SAP-12)
	SENSE_ACT_SAP_34_VAL				//Actn 23 (SAP-34)

	}; /* END: ulaSenseActVal[] */






/*************************  DECLARATIONS  *************************************/




/*****************************  CODE STARTS HERE  ****************************/






/**********************  vSTBL_clrSingleStblRow()  ***************************
*
*
*
******************************************************************************/

void vSTBL_clrSingleStblRow(
		uchar ucTblIdx		//table index
		)
	{
	uchar ucTblNum;

	/*-----------  CLR A SINGLE ROW IN THE SCHED TABLES  -------------------*/

	/* LOOP HORIZONTAL BY TBL NUM */
	for(ucTblNum = 0;  ucTblNum<MAX_SCHED_TBL_NUM;  ucTblNum++)
		{
		/* CLEAR THE TABLE ENTRY */
		vL2SRAM_putStblEntry(
						ucTblNum,
						ucTblIdx,
						ulStblNotUsedVal[ucTblNum]
						);

		}/* END: for(uiTblNum) */

	return;

	}/* END: vSTBL_clrSingleStblRow() */









/**********************  vSTBL_clrAllStblRows()  ******************************
*
*
*
******************************************************************************/

void vSTBL_clrAllStblRows(
		void
		)
	{
	uchar ucTblIdx;

	/*-----------  CLEAR ALL ENTRIES IN ALL SCHED TABLES  -------------------*/

	/* LOOP VERTICAL BY TBL INDEX */
	for(ucTblIdx=0;  ucTblIdx<GENERIC_SCHED_MAX_IDX;  ucTblIdx++)
		{

		vSTBL_clrSingleStblRow(ucTblIdx);

		}/* END: for(uiTblIdx) */

	return;

	}/* END: vSTBL_clrAllStblRows() */







/***********************  vSTBL_setupInitialStbls()  ************************************
*
* Initializes all the sched tables (NOT NST's)
*
******************************************************************************/

void vSTBL_setupInitialStbls(
		void
		)
	{
	uint uiEventSN;
	uchar uciiStBlkNum;
	uchar ucFramStBlkTblSize;
	uchar ucActionNum;
	uchar ucRole;
	uchar ucRoleMask;


	/* SET ALL TABLES TO ZERO FIRST */
	vSTBL_clrAllStblRows();		//set them all to sleep


	/*-----------  NOW SETUP THE ACTIONS  ------------------------------------*/
	ucGLOB_StblIdx_NFL = 0;		//init the ram sched tbl ptr
	uiEventSN = 0;				//setup internal serial numbers
	ucFramStBlkTblSize = ucL2FRAM_getStBlkTblCount();
	ucRole = ucMODOPT_getCurRole();
	ucRoleMask = ucaBitMask[ucRole];

	/* IF NOTHING IN START TABLE THEN JUST ADD SLEEP FUNCTION AND LEAVE */
	if(ucFramStBlkTblSize == 0)
		{
		#if 1
		vSERIAL_dash(20);
		vSERIAL_rom_sout("\r\nStBlkTblSiz=0\r\n");
		vSERIAL_dash(20);
		vSERIAL_crlf();
		#endif

		/* WRITE A SLEEP ST BLK */
		vL2FRAM_stuffStBlk(
						0,			//St Blk Idx
						0,			//Action Num = Sleep
						0,			//St Flags = all off
						0,			//Hr0 to Sample0 = 0
						0,			//interval = none
						0UL			//sense act = none
						);

		vL2FRAM_setStBlkTblCount(1); //setup the tbl count
		
		/* SETUP A DEFAULT RUN STBL ENTRY */
		vSTBL_addNewActionToStbl(
								0,			//action Num = Sleep
								0,			//St blk num
								uiEventSN	//Event num
								);
		uiEventSN++;						//inc event num

		return;
		}

	/* LOOP FOR ALL ENTRYS IN THE FRAM STARTUP TABLE */
	for(uciiStBlkNum=0;  uciiStBlkNum<ucFramStBlkTblSize;  uciiStBlkNum++)
		{
		ucActionNum = (uchar)ulL2FRAM_getStBlkEntryVal(
									uciiStBlkNum,				//Blk Num
									FRAM_ST_BLK_ACTION_IDX		//tbl idx
									);

		if(ucaWhoCanRunBits[ucActionNum] & ucRoleMask)
			{
			#if 0
			vSERIAL_rom_sout("STBL:MLdActn ");
			vACTION_showActionName(ucActionNum);
			vSERIAL_rom_sout(" intoStbl\r\n");
			#endif

			vSTBL_addNewActionToStbl(ucActionNum, uciiStBlkNum, uiEventSN);
			uiEventSN++;

			#if 0
			vSTBL_showAllStblEntrys();
			vSERIAL_crlf();
			#endif

			}/* END: if() */

		}/* END: for(uciiStBlkNum) */


	#if 1
	/* SHOW THE STBL TABLES */
	vSERIAL_rom_sout("SetupInitlStbls\r\n");
	vSTBL_showAllStblEntrys();
	vSERIAL_crlf();
	#endif

	return;

	}/* END: vSTBL_setupInitialStbls() */








/**********************  vSTBL_stuffSingleStblEntry()  *************************
*
*
******************************************************************************/
								  
void vSTBL_stuffSingleStblEntry(
		uchar ucStblEntryNum,			//Tbl Index
		uchar ucPriorityFuncVal,		//Tbl  0 USE entry
		uint uiSN,						//Tbl  1 SERIAL NUM entry
		USL uslRandNum0,				//Tbl  2 RANDOM NUM 0 entry
		USL uslRandNum1,				//Tbl  3 RANDOM NUM 1 entry
		uint uiLoadNum0,				//Tbl  4 LOAD NUM 0 entry
		uint uiLoadNum1,				//Tbl  5 LOAD NUM 1 entry
		uint uiEventStartTime,			//Tbl  6 EVENT START TIME entry
		uint uiEventIntervalTime,		//Tbl  7 EVENT INTERVAL TIME entry
		uchar ucStFlgVal,				//Tbl  8 EVENT FLAG entry
		uint uiStatVal1,				//Tbl  9 EVENT STAT entry
		uint uiStatVal2,				//Tbl 10 EVENT STAT entry
		uchar ucActionNum,				//Tbl 11 ACTION NUM entry
		uchar ucLnkBlkIdx,				//Tbl 12 LNKBLK IDX entry
		ulong ulSenseActVal				//Tbl 13 SENSE ACT entry
		)
	{
	uchar ucTblNum;
	ulong ulVal;

	/* WALK THE SCHED TABLES HORIZONTALLY */
	for(ucTblNum=0;  ucTblNum<MAX_SCHED_TBL_NUM;  ucTblNum++)
		{
		switch(ucTblNum)
			{
			case  0: ulVal = ucPriorityFuncVal;		break;
			case  1: ulVal = uiSN;					break;
			case  2: ulVal = uslRandNum0;			break;
			case  3: ulVal = uslRandNum1;			break;
			case  4: ulVal = uiLoadNum0;			break;
			case  5: ulVal = uiLoadNum1;			break;
			case  6: ulVal = uiEventStartTime;		break;
			case  7: ulVal = uiEventIntervalTime;	break;
			case  8: ulVal = ucStFlgVal;			break;
			case  9: ulVal = uiStatVal1;			break;
			case 10: ulVal = uiStatVal2;			break;
			case 11: ulVal = ucActionNum;			break;
			case 12: ulVal = ucLnkBlkIdx;			break;
			case 13: ulVal = ulSenseActVal;			break;
			default: ulVal = 0;

			}/* END: switch() */


		/* STUFF THE TABLE */
		vL2SRAM_putStblEntry(ucTblNum, ucStblEntryNum, ulVal);

		}/* END: for(ucTblNum) */

	return;

	}/* END: vSTBL_stuffSingleStblEntry() */






/*****************  vSTBL_addNewActionToStbl()  ******************************
*
* Fills an entire Sched tbl row with the specified action entrys
*
******************************************************************************/

void vSTBL_addNewActionToStbl(	//Add a new action to the Sched Tbl
		uchar ucActionNum,	  	//Action number to enter
		uchar ucStBlkNum,	  	//StBlk Num
		uint uiEventSN		  	//Serial Number to assign
		)
	{
	uchar ucThisStblIdx;
	usl uslRand1Val;
	usl uslRand2Val;
	uint uiDowncnt1Val;
	uint uiDowncnt2Val;
	uint uiStatVal1;
	uint uiStatVal2;
	uchar ucStFlags;
	uint uiStartTime;
	uint uiInterval;
	ulong ulSenseActVal;


	/* LOADUP DEFAULT VALUES FOR VARS */
	ucThisStblIdx = ucSTBL_getNewStblIdx();		//save the tbl idx

	uslRand1Val = SCHED_RAND_TBL_1_NOT_USED_VAL;	//load defaults
	uslRand2Val = SCHED_RAND_TBL_1_NOT_USED_VAL;	//load defaults
	uiDowncnt1Val = GENERIC_DCNT_TBL_NOT_USED_VAL;	//load defaults
	uiDowncnt2Val = GENERIC_DCNT_TBL_NOT_USED_VAL;	//load defaults
	uiStatVal1 = ucaSampRecStatus[ucActionNum];		//load defaults
	uiStatVal2 = ucaSampRecStatus[ucActionNum];		//load defaults

	ucStFlags = (uchar)ulL2FRAM_getStBlkEntryVal(
									ucStBlkNum,					//tbl Num
									FRAM_ST_BLK_FLAGS_IDX		//tbl idx
									);
	uiStartTime = (uint)ulL2FRAM_getStBlkEntryVal(
									ucStBlkNum,	  				//tbl Num
									FRAM_ST_BLK_HR_ST_IDX		//tbl idx
									);
	uiInterval = (uint)ulL2FRAM_getStBlkEntryVal(
									ucStBlkNum,					//tbl Num
									FRAM_ST_BLK_INTERVAL_IDX	//tbl idx
									);

	ulSenseActVal = ulL2FRAM_getStBlkEntryVal(
									ucStBlkNum,					//tbl Num
									FRAM_ST_BLK_SENSE_ACT_IDX	//tbl idx
									);

	switch(ucActionNum)
		{
		case E_ACTN_TC_12_SAMPLE:
			ucGLOB_TC12StblIdx = ucThisStblIdx;
			break;

		case E_ACTN_TC_34_SAMPLE:
			ucGLOB_TC34StblIdx = ucThisStblIdx;
			break;

		case E_ACTN_LT_12_SAMPLE:
			ucGLOB_LT12StblIdx = ucThisStblIdx;
			break;

		case E_ACTN_RUN_SCHEDULER:
			uslRand1Val = uslRAND_getRolledFullSysSeed();
			uslRand2Val = uslRAND_getRolledFullSysSeed();
			uiDowncnt1Val = GENERIC_DCNT_TBL_FIXED_DCNT_1;
			uiDowncnt2Val = GENERIC_DCNT_TBL_FIXED_DCNT_1;
			break;

		case E_ACTN_SOM2:
			uiDowncnt1Val = 20;
			uiDowncnt2Val = 20;
			break;

		case E_ACTN_ROM2:
			uiDowncnt1Val = 20;
			uiDowncnt2Val = 20;
			break;

		case E_ACTN_SDC4:
			ucGLOB_SDC4StblIdx = ucThisStblIdx;
			uslRand1Val = uslRAND_getRolledFullSysSeed();
			uslRand2Val = uslRAND_getRolledFullSysSeed();
			uiDowncnt1Val = 0;
			uiDowncnt2Val = 0;
			if(ucL2FRAM_isHub())
				{
				uiDowncnt1Val = 50;
				uiDowncnt2Val = 50;
				}
			break;

		case E_ACTN_RDC4:
			ucGLOB_RDC4StblIdx = ucThisStblIdx;
			uiDowncnt1Val = 0;
			uiDowncnt2Val = 0;
			break;

		case E_ACTN_LT_34_SAMPLE:
			ucGLOB_LT34StblIdx = ucThisStblIdx;
			break;

		default:
			break;
		}/* END: switch() */


	vSTBL_stuffSingleStblEntry(
		ucThisStblIdx,							//Tbl Index
		ucaSampRecPriorityFunc[ucActionNum],	//Tbl  0 PRIORITY_FUNCTION entry
		uiEventSN,								//Tbl  1 SERIAL NUM entry
		uslRand1Val,							//Tbl  2 RANDOM NUM 0 entry
		uslRand2Val,							//Tbl  3 RANDOM NUM 1 entry
		uiDowncnt1Val,							//Tbl  4 DOWNCOUNT 0 entry
		uiDowncnt2Val,							//Tbl  5 DOWNCOUNT 1 entry
		uiStartTime,							//Tbl  6 EVENT START TIME entry
		uiInterval,								//Tbl  7 EVENT INTERVAL TIME entry
		ucStFlags,								//Tbl  8 EVENT FLAG entry
		uiStatVal1,								//Tbl  9 EVENT STAT 1 entry
		uiStatVal2,								//Tbl 10 EVENT STAT 2 entry
		ucActionNum,							//Tbl 11 ACTION NUM entry
		SCHED_LNKBLK_COUNT_TBL_NOT_USED_VAL,		//Tbl 12 LNKBLK IDX entry (uchar)
		ulSenseActVal							//Tbl 13 SENSE ACT entry
		);

	return;

	}/* END: vSTBL_addNewActionToStbl() */







/********************  vSTBL_deleteStblEntry()  ******************************
*
*
*
******************************************************************************/

void vSTBL_deleteStblEntry(
		uchar ucStblEntryNum				//Sched Idx
		)
	{

	vSTBL_stuffSingleStblEntry(
			ucStblEntryNum,							// Entry idx
			PRIORITY_7 | SCHED_FUNC_DORMANT_SLOT,	//  0 Use Flags
			(uint)0,								//  1 Serial number
			(usl)0,									//  2 Random Num 0
			(usl)0,									//  3 Random Num 1
			(uint)0,								//  4 Down Count 0
			(uint)0,								//  5 Down Count 1
			(uint)0,								//  6 Start Time
			SCHED_INTRVL_TBL_NOT_USED_VAL,			//  7 Interval
			0,										//  8 St Flags (USE_FLAGS_DELETED_ENTRY)
			STS_LFACT_SLEEP,						//  9 Event status 0
			STS_LFACT_SLEEP,						// 10 Event status 1
			(uchar)E_ACTN_SLEEP_UL,					// 11 Action Num
			SCHED_LNKBLK_COUNT_TBL_NOT_USED_VAL,		// 12 LNKBLK IDX entry (uchar)
			SENSE_ACT_SLEEP_VAL						// 13 Sense Act
			);

	return;

	} /* END: vSTBL_deleteStblEntry() */







/*****************  vSTBL_showSingleStblEntry()  ******************************
*
* NOTE: This routine is not CRLF terminated so you must put one on if desired
*
* NOTE: This routine now puts out a columnar output.
*
******************************************************************************/

void vSTBL_showSingleStblEntry(
		uchar ucStblEntryNum,
		uchar ucHDRflag,			//YES_HDR, NO_HDR
		uchar ucCRLF_termFlag		//YES_CRLF, NO_CRLF
		)
	{
	uchar ucTblNum;
	ulong ulVal;

	/* DO NOT SHOW THIS ENTRY IF IT IS DORMANT UNLESS ITS ENTRY 0 */
	if(ucStblEntryNum != 0)
		{
		ulVal = ulL2SRAM_getStblEntry(SCHED_PRIORITY_FUNC_TBL_NUM, ucStblEntryNum);
		if((ulVal & SCHED_FUNC_MASK) == SCHED_FUNC_DORMANT_SLOT) return;
		}

	if(ucHDRflag)
		{
		vSERIAL_rom_sout("## P FTN Name Rand1  Rand2  St     Ivl    F Sts1 Sts2 Act SAct\r\n");
//		vSERIAL_rom_sout("## P FTN Name Rand1  Rand2 Dct1 Dct2   St    Ivl    F Sts1 Sts2 Act SAct\r\n");
//		vSERIAL_rom_sout("## P FTN Name Rand1  Rand2 Dct1 Dct2   St    Ivl    F Sts1 Sts2 Act Idx SAct\r\n");
		}


	/* SHOW THE TABLE HEADER */
//	vSERIAL_rom_sout("T");
	vSERIAL_HB8out(ucStblEntryNum);
//	vSERIAL_rom_sout("-");

	/* WALK SCHED TABLE HORIZONTALLY */
	for(ucTblNum=0;  ucTblNum<MAX_SCHED_TBL_NUM;  ucTblNum++)
		{
		/* GET THE CONTENTS OF THE SCHED ENTRY */
		ulVal = ulL2SRAM_getStblEntry(ucTblNum, ucStblEntryNum);

		if(ucSERIAL_readColTab() > ucColTabTbl[ucTblNum]) vSERIAL_bout(' ');
		vSERIAL_colTab( ucColTabTbl[ucTblNum] );

		switch(ucTblNum)
			{
			case SCHED_PRIORITY_FUNC_TBL_NUM:					/* use = schedule priority */
				vSTBL_showPFuncPriority((uchar)ulVal);
				vSERIAL_bout(' ');
				vSTBL_showPFuncName((uchar)ulVal);
				break;

			case SCHED_SN_TBL_NUM:
				//vSERIAL_I16outFormat((int)ulVal,4);
				vRAD40_showRad40((uint)ulVal);
				break;

			case SCHED_ACTN_TBL_NUM:
				if(ulVal >= E_ACTN_MAX_NUM_IDX)	ulVal = 0; //no tbl overrun
				vACTION_showActionName((uchar)ulVal);
				#if 0
				vSERIAL_bout('(');
				vSERIAL_HB8out((uchar)ulVal);	//show the action number
				vSERIAL_bout(')');
				#endif
				break;

			case SCHED_DCNT_TBL_1_NUM:
			case SCHED_DCNT_TBL_2_NUM:
				//vSERIAL_HB16out((uint)ulVal);
				break;

			case SCHED_INTRVL_TBL_NUM:
				vDAYTIME_convertSecToShow((long)ulVal);
				break;

			case SCHED_LNKBLK_COUNT_TBL_NUM:
				//vSERIAL_HB8out((uchar)ulVal);
				break;

			case SCHED_SENSE_ACT_TBL_NUM:
				vSENSEACT_showSenseActWordInText(ulVal);
				break;

			default:
				switch(ucStblByteWidth[ucTblNum])
					{
					case 1:
						vSERIAL_HB8out((uchar)ulVal);
						break;

					case 2:
						vSERIAL_HB16out((uint)ulVal);
						break;

					case 3:
						vSERIAL_HB24out((usl)ulVal);
						break;

					default:
						vSERIAL_HB32out(ulVal);			//show val
						break;
					}

			}/* END: switch() */

		}/* END: for(uiTblNum) */

	if(ucCRLF_termFlag)
		vSERIAL_crlf();

	return;

	}/* END: vSTBL_showSingleStblEntry() */






/*****************  vSTBL_showAllStblEntrys()  *******************************
*
*
*
*******************************************************************************/

void vSTBL_showAllStblEntrys(
		void
		)
	{
	uchar ucStblEntryNum;

	#if 0
	vSERIAL_rom_sout("STBL:ucGLOB_StblIdx_NFL=");
	vSERIAL_HB8out(ucGLOB_StblIdx_NFL);
	vSERIAL_crlf();
	#endif

	/* SHOW THE 0 TBL ENTRY WITH A HEADER */
	vSTBL_showSingleStblEntry(0, YES_HDR, YES_CRLF);

	/* LOOP VERTICAL BY TBL INDEX */
	for(ucStblEntryNum=1;  ucStblEntryNum<ucGLOB_StblIdx_NFL;  ucStblEntryNum++)
		{
		vSTBL_showSingleStblEntry(ucStblEntryNum, NO_HDR, YES_CRLF);

		}/* END: for(uiTblIdx) */

	vSERIAL_crlf();

	return;

	}/* END: vSTBL_showAllStblEntrys() */




/****************  vSTBL_showBriefStblEntrys()  ******************************
*
*
*
******************************************************************************/

void vSTBL_showBriefStblEntrys(
		void
		)
	{
	uchar ucii;
	ulong ulVal;

	vSERIAL_rom_sout("        SramStblEntrys\r\n");
	vSERIAL_rom_sout("### USE     St   Ivl   F  ACTN SENSEACT\r\n");

	for(ucii=0; ucii<ucGLOB_StblIdx_NFL;  ucii++)
		{

		vSERIAL_UI8out(ucii);

		vSERIAL_colTab(4);
		ulVal = ulL2SRAM_getStblEntry(SCHED_PRIORITY_FUNC_TBL_NUM, ucii);
		vSTBL_showPFuncName((uchar)ulVal);

		vSERIAL_colTab(9);
		ulVal = ulL2SRAM_getStblEntry(SCHED_ST_TBL_NUM, ucii);
		vSERIAL_UI16out((uint)ulVal);

		vSERIAL_colTab(14);
		ulVal = ulL2SRAM_getStblEntry(SCHED_INTRVL_TBL_NUM, ucii);
		vSERIAL_UI16out((uint)ulVal);

		vSERIAL_colTab(22);
		ulVal = ulL2SRAM_getStblEntry(SCHED_FLAG_TBL_NUM, ucii);
		vSERIAL_HB8out((uchar)ulVal);

		vSERIAL_colTab(26);
		ulVal = ulL2SRAM_getStblEntry(SCHED_ACTN_TBL_NUM, ucii);
		vACTION_showActionName((uchar)ulVal);

		vSERIAL_colTab(31);
		ulVal = ulL2SRAM_getStblEntry(SCHED_SENSE_ACT_TBL_NUM, ucii);
		vSENSEACT_showSenseActWordInText(ulVal);

		vSERIAL_crlf();		

		}/* END: for() */


	}/* END: vSTBL_showBriefStblEntrys() */






/**********************  ucSTBL_searchStblsForMatch()  *********************************
*
* RET:	Sched Idx of the Match
*		Out of range Sched Idx if no Match
*
******************************************************************************/

uchar ucSTBL_searchStblsForMatch( //Ret: Match=StblIdx, No Match=Out of range Sched Idx
		uint uiSearch_SN,				//serial number
		uchar ucSearch_actionNum		//search first by action number
		)
	{
	uchar ucStblEntryNum;
	uint uiSN;
	uchar ucAction;

	for(ucStblEntryNum=0;  ucStblEntryNum<ucGLOB_StblIdx_NFL;  ucStblEntryNum++)
		{
		ucAction = (uchar)ulL2SRAM_getStblEntry(
								SCHED_ACTN_TBL_NUM,		//tbl num
								ucStblEntryNum						//sched idx
								);
		if(ucAction == ucSearch_actionNum)
			{
			uiSN = (uint)ulL2SRAM_getStblEntry(
								SCHED_SN_TBL_NUM,			//tbl num
								ucStblEntryNum 					//sched idx
								);

			if(uiSN == uiSearch_SN) break;	//we found it
			}

		}/* END: for() */

	return(ucStblEntryNum);

	}/* END: ucSTBL_searchStblsForMatch() */







/**********************  vSTBL_showAllROM2Entrys()  *********************************
*
*
*
*
******************************************************************************/

void vSTBL_showAllROM2Entrys(
		void
		)
	{

	/* GIVE HEADER MESSAGE */
//	vSERIAL_rom_sout("ROM2SchdTblEntrys:\r\n");

	vSTBL_showAllActionEntrys(E_ACTN_ROM2);

	return;

	}/* END: vSTBL_showAllROM2Entrys() */







/**********************  vSTBL_showAllSOM2Entrys()  *********************************
*
*
******************************************************************************/

void vSTBL_showAllSOM2Entrys(
		void
		)
	{

	/* GIVE HEADER MESSAGE */
//	vSERIAL_rom_sout("SOM2SchdTblEntrys:\r\n");

	vSTBL_showAllActionEntrys(E_ACTN_SOM2);

	return;

	}/* END: vSTBL_showAllSOM2Entrys() */







/**********************  vSTBL_showTblBaseName()  *****************************
*
* This routine searches the table for a table base addr index
* and then prints out the table name.
*
*****************************************************************************/
void vSTBL_showTblBaseName(
		usl uslBaseAddr
		)
	{
	uchar ucTblNum;

	/* WALK THE STBL BASE ADDR LIST AND RETURN AN INDEX OF THE FOUND VAL */

	for(ucTblNum=0; ucTblNum<MAX_SCHED_TBL_NUM;  ucTblNum++)
		{
		if(uslBaseAddr == uslStblBaseAddr[ucTblNum]) break;

		}/* END for() */

	if(ucTblNum >= MAX_SCHED_TBL_NUM)
		{
		vSERIAL_rom_sout("TblNotFnd");
		return;
		}

	vSERIAL_rom_sout(cpaStblLongName[ucTblNum]);
	return;

	}/* END: vSTBL_showTblBaseName() */






/**********************  vSTBL_showSOM2andROM2counts()  *****************************
*
* This routine searches the table for all ROM2 and SOM2 entrys
* and then prints out the list
*
* NOTE: This routine was re-written to eliminate the array storage and
*		that is why it is in 3 passes.
*
*****************************************************************************/
void vSTBL_showSOM2andROM2counts(
		uchar ucCRLF_termFlag		//YES_CRLF, NO_CRLF
		)
	{
	uchar ucc;
	uchar ucActiveSOM2LocalIdx;
	uchar ucActiveROM2LocalIdx;
	uchar ucActionNum;
	uint uiStsVal;
	uchar ucMissCnt;
	uint uiSN;
	uint uiCountTotals;


	/* FIRST PASS COUNTS HOW MANY ROMS AND SOM2 THERE ARE */
	uiCountTotals = uiSTBL_countSOM2andROM2entrys();
	ucActiveSOM2LocalIdx = (uchar)(uiCountTotals >> 8);
	ucActiveROM2LocalIdx = (uchar) uiCountTotals;


	/* PASS 2: SHOW THE SOM2 STUFF */
	vSERIAL_rom_sout("SOM2#");
	vSERIAL_UIV8out(ucActiveSOM2LocalIdx);

	if(ucActiveSOM2LocalIdx != 0)
		{
		vSERIAL_bout('>');

		for(ucc=0;  ucc<ucGLOB_StblIdx_NFL;  ucc++)
			{
			ucActionNum = (uchar)ulL2SRAM_getStblEntry(SCHED_ACTN_TBL_NUM, ucc);

			if(ucActionNum == E_ACTN_SOM2)
				{
				/* SHOW THE NAME */
				uiSN = (uint)ulL2SRAM_getStblEntry(SCHED_SN_TBL_NUM, ucc);
				vRAD40_showRad40(uiSN);


				/* SHOW THE MISS COUNT IN PARENTHESIS */
				uiStsVal = (uint)ulL2SRAM_getStblEntry(
									ucaStsTblNum[ucRTS_lastScheduledNSTnum()],
									ucc
									);
				ucMissCnt = (uchar)((uiStsVal >> 8) & 0x7);
				if(ucMissCnt != 0)
					{
					vSERIAL_bout('(');
					vSERIAL_UIV8out(ucMissCnt);
					vSERIAL_bout(')');
					}

				vSERIAL_bout(',');

				}

			}/* END: for(ucc) */

		}/* END: if() */

	vSERIAL_bout(' ');



	/* PASS 3: SHOW THE ROM2 STUFF */
	vSERIAL_rom_sout("ROM2#");
	vSERIAL_UIV8out(ucActiveROM2LocalIdx);

	if(ucActiveROM2LocalIdx != 0)
		{
		vSERIAL_bout('<');

		for(ucc=0;  ucc<ucGLOB_StblIdx_NFL;  ucc++)
			{
			ucActionNum = (uchar)ulL2SRAM_getStblEntry(SCHED_ACTN_TBL_NUM, ucc);

			#if 0
			vSERIAL_rom_sout("\r\nActnNum= ");
			vSERIAL_UIV8out(ucActionNum);
			vSERIAL_crlf();
			#endif

			if(ucActionNum == E_ACTN_ROM2)
				{
				/* SHOW THE NAME */
				uiSN = (uint)ulL2SRAM_getStblEntry(SCHED_SN_TBL_NUM, ucc);
				vRAD40_showRad40(uiSN);


				/* SHOW THE MISS COUNT IN PARENTHESIS */
				uiStsVal = (uint)ulL2SRAM_getStblEntry(
									ucaStsTblNum[ucRTS_lastScheduledNSTnum()],
									ucc
									);
				ucMissCnt = (uchar)((uiStsVal >> 8) & 0x7);
				if(ucMissCnt != 0)
					{
					vSERIAL_bout('(');
					vSERIAL_UIV8out(ucMissCnt);
					vSERIAL_bout(')');
					}

				vSERIAL_bout(',');

				}

			}/* END: for(ucc) */

		vSERIAL_bout(' ');

		}/* END: if() */

	if(ucCRLF_termFlag)	vSERIAL_crlf();

	return;

	}/* END: vSTBL_showSOM2andROM2counts() */





/**********************  vSTBL_showRDC4andSDC4counts()  *****************************
*
* This routine searches the table for all SDC4 and RDC4 entrys
* and then prints out the list
*
*****************************************************************************/

void vSTBL_showRDC4andSDC4counts(
		uchar ucCRLF_termFlag		//YES_CRLF, NO_CRLF
		)
	{
	uchar ucc;
	uchar ucNSTentryVal;
	uchar ucSDC4count;
	uchar ucRDC4count;

	#if 0
	vSERIAL_rom_sout("\r\nSTBL:showRDC4 LastAwakeSlot= ");
	vSERIAL_UIV8out(ucGLOB_lastAwakeSlot);
	vSERIAL_rom_sout("  LastAwakeNSTtblNum= ");
	vSERIAL_UIV8out(ucGLOB_lastAwakeNSTtblNum);
	vSERIAL_crlf();
	#endif


	/* NOW COUNT HOW MANY ENTRYS OF EACH ARE IN THE CURRENT NST */
	ucSDC4count = 0;
	ucRDC4count = 0;
	for(ucc=ucGLOB_lastAwakeSlot;  ucc<GENERIC_NST_MAX_IDX;  ucc++)
		{
		ucNSTentryVal = ucRTS_getNSTentry(ucGLOB_lastAwakeNSTtblNum,ucc);
		if(ucGLOB_SDC4StblIdx == ucNSTentryVal) ucSDC4count++; //count SDC4's
		if(ucGLOB_RDC4StblIdx == ucNSTentryVal)	ucRDC4count++; //count RDC4's
		}

	vSERIAL_rom_sout("RDC4#");
	vSERIAL_UIV8out(ucRDC4count);
	vSERIAL_rom_sout(" SDC4#");
	vSERIAL_UIV8out(ucSDC4count);
	if(ucCRLF_termFlag)	vSERIAL_crlf();

	return;

	}/* END: vSTBL_showRDC4andSDC4counts() */






/**********************  vSTBL_deleteAllROM2slots()  **************************
*
*
*
*****************************************************************************/

void vSTBL_deleteAllROM2slots(
		void
		)
	{
	uchar ucc;
	uchar ucActionNum;

	/* WALK THE ENTIRE SCHED TBL */
	for(ucc=0;  ucc<ucGLOB_StblIdx_NFL;  ucc++)
		{
		ucActionNum = (uchar)ulL2SRAM_getStblEntry(SCHED_ACTN_TBL_NUM, ucc);

		if(ucActionNum == E_ACTN_ROM2) vSTBL_deleteStblEntry(ucc); //delete

		}/* END: for() */

	return;

	}/* END: vSTBL_deleteAllROM2slots() */







/****************  uiSTBL_countSOM2andROM2entrys()  *************************
*
* Return a packed uint with SOM2 count in HI half and ROM2 count in LO half
*
******************************************************************************/

uint uiSTBL_countSOM2andROM2entrys(
		void
		)
	{
	uchar ucc;
	uchar ucActiveSOM2slots;
	uchar ucActiveROM2slots;
	uchar ucActionNum;
	uint uiCountTotals;


	/* COUNT ALL THE ACTIVE ROM2 ENTRIES WE HAVE */
	ucActiveSOM2slots = 0;		//count
	ucActiveROM2slots = 0;		//count
	for(ucc=0;  ucc<ucGLOB_StblIdx_NFL;  ucc++)
		{
		ucActionNum = (uchar)ulL2SRAM_getStblEntry(SCHED_ACTN_TBL_NUM, ucc);
		if(ucActionNum == E_ACTN_SOM2) ucActiveSOM2slots++;
		if(ucActionNum == E_ACTN_ROM2) ucActiveROM2slots++;
		}/* END: for() */

	uiCountTotals = ucActiveSOM2slots;
	uiCountTotals <<= 8;
	uiCountTotals |= ucActiveROM2slots;

	return(uiCountTotals);

	}/* END: uiSTBL_countSOM2andROM2entrys() */









/*******************  ucSTBL_getNewStblIdx()  ********************************
*
*
******************************************************************************/

uchar ucSTBL_getNewStblIdx(
		void
		)
	{
	uchar ucStblEntryNum;
	uchar ucPFunc;

	/* SEARCH FOR AN EMPTY IDX -- IF NONE FOUND -- USE A NEW ONE */
	for(ucStblEntryNum=1;  ucStblEntryNum<ucGLOB_StblIdx_NFL;  ucStblEntryNum++)
		{
		/* LOAD THE USE */
		ucPFunc = (uchar)ulL2SRAM_getStblEntry(
							SCHED_PRIORITY_FUNC_TBL_NUM,
							ucStblEntryNum
							);

		/* IF NOT DELETED THEN CONTINUE */
		if((ucPFunc & SCHED_FUNC_MASK) == SCHED_FUNC_DORMANT_SLOT) break;

		}/* END: for() */

	/* IF NONE FOUND -- USE A NEW ENTRY */
	if(ucStblEntryNum >= ucGLOB_StblIdx_NFL) ucGLOB_StblIdx_NFL++;

	return(ucStblEntryNum);

	}/* END: ucSTBL_getNewStblIdx() */






/******************* vSTBL_stuffSingleAction_LFactor()  ************************
*
* NOTE: This routine only computes and stuffs the action LFactors for actions
*		that generate messages internally (ie. not for outside).
* 
*
******************************************************************************/
void vSTBL_stuffSingleAction_LFactor(
		uchar ucNSTtblNum,			//NST tbl num (0 or 1)
		uchar ucStblIdx			//index of event being evaluated
		)
	{
	uchar ucStFlgVal;
	uchar ucActionVal;
	uint uiDownCountFlags;
	uchar ucLFactor;
	uint uiLFactor;
	uchar ucPFuncVal;
	uint uiIntervalVal;
	uint uiStsVal;
	uint uiLoadTblVal;

	/* FIND OUT IF THIS ACTION GENERATES MSGS */
	ucStFlgVal = (uchar)ulL2SRAM_getStblEntry(
							SCHED_FLAG_TBL_NUM,	//Sched tbl num
							ucStblIdx	  	//Sched tbl idx
							);

	/* IF THIS ACTION DOES NOT MAKE OM2'S -- LEAVE */
	if((ucStFlgVal & F_USE_MAKE_OM2) == 0) return;


	/* GET ACTION NUM */
	ucActionVal = (uchar)ulL2SRAM_getStblEntry(
							SCHED_ACTN_TBL_NUM,	//Sched tbl num
							ucStblIdx	  	//Sched tbl idx
							);

	/*---------------  HANDLE INTERVAL  EVENTS ------------------------------*/

	/* FIND OUT IF THIS ACTION IS AN INTERVVAL EVENT */
	ucPFuncVal = (uchar)ulL2SRAM_getStblEntry(
							SCHED_PRIORITY_FUNC_TBL_NUM,	//Sched tbl num
							ucStblIdx						//Sched tbl idx
							);
	
	if((ucPFuncVal & SCHED_FUNC_MASK) == SCHED_FUNC_INTERVAL_SLOT)
		{
		uiIntervalVal = (uint)ulL2SRAM_getStblEntry(
							SCHED_INTRVL_TBL_NUM,	//Sched tbl num
							ucStblIdx				//Sched tbl idx
							);

		uiLFactor = 3600 / uiIntervalVal;
		goto Stuff_LFactor_and_exit;
		}

	/*-----------------------------------------------------------------------*/


	/* GET THE LOAD TBL ENTRY */
	uiLoadTblVal = (uint)ulL2SRAM_getStblEntry(
							SCHED_DCNT_TBL_1_NUM,	//Sched tbl num
							ucStblIdx		  	//Sched tbl idx
							);


	/* NOTE: THE NUMBER 14 IS THE NUMBER OF FRAMES PER HOUR */
	uiDownCountFlags = uiLoadTblVal & (F_DCNT_COUNTING | F_DCNT_STOP_AT_ONE);
	switch(uiDownCountFlags)
		{
		case 0:							//fixed count, stop at 0
		case F_DCNT_STOP_AT_ONE:	//fixed count, stop at 1
			uiLFactor = (14 * (uiLoadTblVal & F_DCNT_COUNT_ONLY_MASK));
			if((ucActionVal == E_ACTN_SOM2) || (ucActionVal == E_ACTN_ROM2))
				{
				uiLFactor *=5;	//assume 5 msgs per communication;
				}
			break;
										//counting, stop at 1
		case F_DCNT_COUNTING | F_DCNT_STOP_AT_ONE: 
			uiLFactor = 14 - ((uiLoadTblVal & F_DCNT_COUNT_ONLY_MASK) -1);
			break;

		case F_DCNT_COUNTING:	//counting, stop at 0
			uiLFactor = 0;
			break;

		default:
			uiLFactor = 1;
			break;

		}/*END: switch() */

Stuff_LFactor_and_exit:

	if(uiLFactor > 63) uiLFactor = 63;
	ucLFactor = (uchar)uiLFactor;

	/* LOAD THE CURRENT STS ENTRY */
	uiStsVal = (uint)ulL2SRAM_getStblEntry(
							ucaStsTblNum[ucNSTtblNum],
							ucStblIdx
							);
	uiStsVal &= 0xFF00;		//clr the old LFactor
	uiStsVal |= ucLFactor;	//or on the new LFactor

	/* STUFF A NEW LFACTOR VALUE */
	vL2SRAM_putStblEntry(
						ucaStsTblNum[ucNSTtblNum],
						ucStblIdx,
						(ulong)uiStsVal
						);

	#if 0
	vSERIAL_rom_sout("LFact=");
	vSERIAL_UIV16out(uiLFactor);
	vSERIAL_rom_sout("  LFact=");
	vSERIAL_UIV8out(ucLFactor);
	vSERIAL_rom_sout("  entry=");
	vSERIAL_UIV8out(ucStblIdx);
	vSERIAL_rom_sout(" Actn=");
	vACTION_showActionName(ucActionVal);
	vSERIAL_crlf();
	#endif

	return;

	}/* END: vSTBL_stuffSingleAction_LFactor() */








/******************* vSTBL_computeSysLFactor()  *******************************
*
* Computed once per frame in runScheduler(). 
*
******************************************************************************/
void vSTBL_computeSysLFactor(
		void
		)
	{
	uchar ucTblIdxCnt;
	int iNewSysLFactor;
	uchar ucStFlgVal;
	uchar ucActionVal;
	uchar ucLFactVal;
	int iLFactVal;
	int iMsgsPerHour;

	/* INIT THE NEW VALUE */
	iNewSysLFactor = 0;


	/* LOOP VERTICAL BY TBL INDEX -- COMPUTING NEW LFACTOR VALUE */
	for(ucTblIdxCnt=0;  ucTblIdxCnt<ucGLOB_StblIdx_NFL;  ucTblIdxCnt++)
		{
		/* GET FLAGS TO MAKE SURE THIS IS AN OM2 MAKING ENTITY */
		ucStFlgVal = (uchar)ulL2SRAM_getStblEntry(
							SCHED_FLAG_TBL_NUM,	//Sched tbl num
							ucTblIdxCnt		  	//Sched tbl idx
							);

		/* GET ACTION NUM TO FIND SOM2 FUNCS SO COMPUTATION HAS PROPER SIGN */
		ucActionVal = (uchar)ulL2SRAM_getStblEntry(
							SCHED_ACTN_TBL_NUM,	//Sched tbl num
							ucTblIdxCnt		  	//Sched tbl idx
							);

		/* GET THE STATUS TBL ENTRY TO GET THE LFACTOR */ 
		ucLFactVal = (uchar)ulL2SRAM_getStblEntry(
							ucaStsTblNum[ucRTS_thisNSTnum()],
							ucTblIdxCnt
							);

		/* GET AN INTEGER COPY OF THIS LFACTOR WITHOUT FLAGS */
		iLFactVal = (int)(ucLFactVal & F_STS_COUNT_ONLY_MASK);

		/*---------- COMPUTE THE SYSTEM LFACTOR -----------------------------*/

		switch(ucActionVal)
			{
			case E_ACTN_SOM2:
			case E_ACTN_ROM2:

				/* ASSUME MSGS PER HOUR IS MULTIPLE NUMBER */
				iMsgsPerHour = (iLFactVal * 14 * 7);

				/* CHECK THE ASSUMPTION AND RECOMPUTE IF NECESSARY */
				if(ucLFactVal & F_STS_COUNTING)
					{
					/* LINKUP_DCNT IS A GAP TIME SO COMPUTE GAP */
					/* Msgs/Hour = ((3600 sec/hr)*(7msgs/link))/((x Frames/link) * 256 sec/frame)) */
					/* iMsgsPerHour = (3600 * 7)/(iLFactVal * 256); */
					iMsgsPerHour = (98 / iLFactVal);
					}

				/* USE NEGATIVE IF ITS GOING OUT */
				if(ucActionVal == E_ACTN_SOM2) iMsgsPerHour = -iMsgsPerHour;

				break;

			default:
				iMsgsPerHour = iLFactVal;
				if((ucStFlgVal & F_USE_MAKE_OM2) == 0) break;
				break;

			}/* END: switch(ucActionVal) */

		iNewSysLFactor += iMsgsPerHour;

		}/* END: for(ucTblIdxCnt) */


	/* ADD IN THE NUMBER OF MSGS ALREADY AQUIRED AS AN INSTANT RATE */
	iMsgsPerHour = (int)uiL2SRAM_getMsgCount();;
	iNewSysLFactor += iMsgsPerHour;

	/* STUFF THE COMPLETE SYS LFACTOR */
	iGLOB_completeSysLFactor = iNewSysLFactor;

	/* CONVERT COMPLETE LFACTOR INTO A MSG LFACTOR */
	if(iNewSysLFactor < 0 ) iNewSysLFactor = 0;		//no negative rates
	if(iNewSysLFactor >255) iNewSysLFactor = 255;	//limit it to 1 byte val
//	ucGLOB_msgSysLFactor = (uchar)iNewSysLFactor;	//update to new val

	#if 1
	vSERIAL_rom_sout("NewLd=");
	vSERIAL_IV16out(iNewSysLFactor);
	vSERIAL_crlf();
	#endif

	return;

	}/* END: vSTBL_computeSysLFactor() */








/******************* vSTBL_showStatsOnLFactor()  *******************************
*
* Computed once per frame in runScheduler(). 
*
******************************************************************************/
void vSTBL_showStatsOnLFactor(
		void
		)
	{
	uchar ucTblIdxCnt;
	int iNewSysLFactor;
	uchar ucStFlgVal;
	uchar ucActionVal;
	uchar ucLFactVal;
	uint uiMsgCount;
	int iLFactVal;
	int iMsgsPerHour;


	/* SHOW TABLE HEADER */
	vSERIAL_crlf();
	vSERIAL_dash(13);
	vSERIAL_rom_sout("  LOAD FACTOR STATS  ");
	vSERIAL_dash(11);
	vSERIAL_crlf();
	vSERIAL_rom_sout("Tbl Act Act Flg  LFact Formula     Msg   Tot\r\n");
	vSERIAL_rom_sout("Ent Num Nam Val    Val             Cnt   Val\r\n");

	/* LOOP VERTICAL BY TBL INDEX -- COMPUTING NEW LFACTOR VALUE */
	iNewSysLFactor = 0;
	for(ucTblIdxCnt=0;  ucTblIdxCnt<ucGLOB_StblIdx_NFL;  ucTblIdxCnt++)
		{
		/* GET FLAGS TO MAKE SURE THIS IS AN OM2 MAKING ENTITY */
		ucStFlgVal = (uchar)ulL2SRAM_getStblEntry(
							SCHED_FLAG_TBL_NUM,	//Sched tbl num
							ucTblIdxCnt		  	//Sched tbl idx
							);

		/* GET ACTION NUM TO FIND SOM2 FUNCS SO COMPUTATION HAS PROPER SIGN */
		ucActionVal = (uchar)ulL2SRAM_getStblEntry(
							SCHED_ACTN_TBL_NUM,	//Sched tbl num
							ucTblIdxCnt		  	//Sched tbl idx
							);

		/* GET THE STATUS TBL ENTRY TO GET THE LFACTOR */ 
		ucLFactVal = (uchar)ulL2SRAM_getStblEntry(
							ucaStsTblNum[ucRTS_thisNSTnum()],
							ucTblIdxCnt
							);

		/* GET AN INTEGER COPY OF THIS LFACTOR WITHOUT FLAGS */
		iLFactVal = (int)(ucLFactVal & F_STS_COUNT_ONLY_MASK);


		vSERIAL_UI8_2char_out(ucTblIdxCnt,' ');
		vSERIAL_rom_sout("  ");
		vSERIAL_UI8out(ucActionVal);
		vSERIAL_rom_sout(" ");
		vACTION_showActionName(ucActionVal);
		vSERIAL_rom_sout("  ");
		vSERIAL_HB8out(ucStFlgVal);
		vSERIAL_rom_sout(" ");
		vSERIAL_I16out(iLFactVal);

		/*---------- COMPUTE THE SYSTEM LFACTOR -----------------------------*/

		switch(ucActionVal)
			{
			case E_ACTN_SOM2:
			case E_ACTN_ROM2:
				if(ucLFactVal & F_STS_COUNTING)
					{
					/* LINKUP_DCNT IS A GAP TIME SO COMPUTE GAP */
					/* Msgs/Hour = ((3600 sec/hr)*(7msgs/link))/((x Frames/link) * 256 sec/frame)) */
					/* iMsgsPerHour = (3600 * 7)/(iLFactVal * 256); */
					iMsgsPerHour = (98 / iLFactVal);

					vSERIAL_bout(' ');
					if(ucActionVal == E_ACTN_SOM2)
						{
						iMsgsPerHour = -iMsgsPerHour;
						vSERIAL_bout('-');
						}
					vSERIAL_rom_sout("(98/");
					vSERIAL_IV16out(iLFactVal);
					vSERIAL_rom_sout(")=");
					break;
					}

				/* LINKUP_DCNT IF A MULTIPLE LINKUP SO COMPUTE MULTIPLES */
				iMsgsPerHour = (iLFactVal * 14 * 7);
				vSERIAL_bout('*');
				if(ucActionVal == E_ACTN_SOM2)
					{
					iMsgsPerHour = -iMsgsPerHour;
					vSERIAL_rom_sout(" -");
					}
				vSERIAL_rom_sout("(14*7)=");

				break;

			default:
				iMsgsPerHour = iLFactVal;
				if((ucStFlgVal & F_USE_MAKE_OM2) == 0) break;
				break;

			}/* END: switch(ucActionVal) */

		iNewSysLFactor += iMsgsPerHour;

		/*----------  FINISH REST OF TABLE --------------*/

		vSERIAL_colTab(32);
		vSERIAL_I16out(iMsgsPerHour);
		vSERIAL_I16out(iNewSysLFactor);
		vSERIAL_crlf();

		}/* END: for(ucTblIdxCnt) */

	uiMsgCount = uiL2SRAM_getMsgCount();
	iMsgsPerHour = (int)uiMsgCount;
	iNewSysLFactor += iMsgsPerHour;

	vSERIAL_rom_sout("Msgcnt=");
	vSERIAL_colTab(17);
	vSERIAL_UI16out(uiMsgCount);
	vSERIAL_colTab(32);
	vSERIAL_I16out(iMsgsPerHour);
	vSERIAL_colTab(38);
	vSERIAL_I16out(iNewSysLFactor);
	vSERIAL_crlf();


	vSERIAL_colTab(40);
	vSERIAL_rom_sout("ÄÄÄÄ\r\n");


	vSERIAL_rom_sout("Load=");
	vSERIAL_colTab(16);
	vSERIAL_I16out(iGLOB_completeSysLFactor);
	vSERIAL_colTab(38);
	vSERIAL_I16out(iNewSysLFactor);
	vSERIAL_crlf();


//	vSERIAL_rom_sout("MsgLFactor=");
//	vSERIAL_colTab(19);
//	vSERIAL_UI8out(ucGLOB_msgSysLFactor);
//	vSERIAL_crlf();


//	vSERIAL_crlf();

	return;

	}/* END: vSTBL_showStatsOnLFactor() */







/*******************  vSTBL_showLinkupSchedule()  ****************************
*
*
******************************************************************************/

void vSTBL_showLinkupSchedule(
		void
		)
	{
	uchar ucStblIdx;
	uchar ucAction;
	const char *cStrPtr;
	uint uiContactSN;
	uchar ucFoundStblIdx;
//	uchar ucFoundCount;
	uchar ucNSTslot;
	uint uiDcntVal;
	uchar ucc;
	usl uslRand[2];
	long lFrameCounter;
	uchar ucSlotNum;


	/* WALK THE SCHED TABLE LOOKING FOR ROM2 AND SOM2 ENTRYS */
	for(ucStblIdx=0;  ucStblIdx<ucGLOB_StblIdx_NFL;  ucStblIdx++)
		{
		/* GET THE ACTION VAL */
		ucAction = (uchar)ulL2SRAM_getStblEntry(
								SCHED_ACTN_TBL_NUM,		//tbl num
								ucStblIdx				//sched idx
								);

		/* IF NOT A MATCH THEN CONTINUE */
		if((ucAction != E_ACTN_ROM2) && (ucAction != E_ACTN_SOM2)) continue;

		/* GOT ONE -- SHOW THE NEXT SCHEDULE ENTRYS */

		cStrPtr = "ROM2<";
		if(ucAction != E_ACTN_ROM2)	cStrPtr = "SOM2>";
		vSERIAL_rom_sout(cStrPtr);

		/* SHOW THE CONTACT LINK ID  */
		uiContactSN = (uint)ulL2SRAM_getStblEntry(
								SCHED_SN_TBL_NUM,		//tbl num
								ucStblIdx				//sched idx
								);

		//vSERIAL_UIV16out(uiContactSN);
		vRAD40_showRad40(uiContactSN);

		vSERIAL_colTab(12);
		

		/* SEARCH FOR ANY LINKUP IN THIS NST */
//		ucFoundCount = 0;
		for(ucNSTslot=0;  ucNSTslot<GENERIC_NST_MAX_IDX;  ucNSTslot++)
			{
			ucFoundStblIdx = ucRTS_getNSTentry(ucGLOB_lastAwakeNSTtblNum,
												ucNSTslot
												);
			if(ucFoundStblIdx == ucStblIdx)
				{
				vSERIAL_HBV32out((ulong)lGLOB_lastAwakeFrame);
				vSERIAL_bout(':');
				vSERIAL_HB8out(ucNSTslot);
				vSERIAL_bout(',');
				//ucFoundCount++;
				}
			}

		vSERIAL_colTab(22);
		vSERIAL_rom_sout("³");

		/* COMPUTE NEXT CONTACT TIME (APPROX) */
		uiDcntVal = (uint)ulL2SRAM_getStblEntry(
									ucaDcntTblNum[ucRTS_thisNSTnum()],
									ucStblIdx
									);

		cStrPtr = " Mult ";
		if(uiDcntVal & F_DCNT_COUNTING) cStrPtr = " Skip ";
		vSERIAL_rom_sout(cStrPtr);
		vSERIAL_UIV8out(uiDcntVal & F_DCNT_COUNT_ONLY_MASK);
		vSERIAL_rom_sout(" ³ ");

		/* COMPUTE INTO THE FUTURE IF SKIPPING */
		if((uiDcntVal & F_DCNT_COUNTING) ||
		  ((uiDcntVal & F_DCNT_COUNT_ONLY_MASK) == 1))
			{
			/* GET THE STARTING RANDOM NUMBER */
			uslRand[1] = (usl)ulL2SRAM_getStblEntry(
									ucaRandTblNum[ucRTS_thisNSTnum()],
									ucStblIdx
									);

			lFrameCounter = lGLOB_lastAwakeFrame;
			

			/* COUNT OUT INTO THE FUTURE */
			for(ucc=0;  ucc<(uchar)(uiDcntVal & F_DCNT_COUNT_ONLY_MASK);  ucc++)
				{
				uslRand[ucc%2] = uslRAND_getRolledFullForeignSeed(uslRand[(ucc+1)%2]);
				lFrameCounter++;
				}

			/* SAVE THE FINAL SLOT NUMBER */
			ucSlotNum = (uchar)((uslRand[(ucc+1)%2] >> 8) & GENERIC_NST_MAX_IDX_MASK);

			/* NOW GIVE THE NEXT SCHEDULED LINKUP */
			vSERIAL_rom_sout(" ");
			vSERIAL_HBV32out((ulong)lFrameCounter);
			vSERIAL_bout(':');
			vSERIAL_HB8out(ucSlotNum);
			}

		vSERIAL_crlf();


		vSTBL_showSingleStblEntry(ucStblIdx, YES_HDR, YES_CRLF);
		vSERIAL_crlf();


		}/* END: for(ucStblIdx) */

	vSERIAL_crlf();

	return;

	}/* END: vSTBL_showLinkupSchedule() */





/******************** uiSTBL_handleLoadDowncnt() ********************************
*
* RET: new load downcount val
*
*    15       14       13       12       11       10       9        8
*ÚÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄ¿
*³Counting³ Alarm  ³Stop @ 1³        ³        ³        ³        ³        ³
*³ ÄÄÄÄÄÄ ³ ÄÄÄÄÄÄ ³ ÄÄÄÄÄÄ ³        ³        ³		   ³        ³        ³
*³ Fixed  ³   0    ³Stop @ 0³        ³        ³        ³        ³        ³
*ÀÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÙ
*    7        6        5        4        3         2       1        0
*ÚÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄ¿
*³        ³        ³        ³  Down Count     ³        ³        ³        ³
*³<--------------------------------------------------------------------->³
*³        ³        ³        ³        ³        ³        ³        ³        ³
*ÀÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÙ
*
* 1.If (Counting bit = fixed number) (signified by bit 15 = 0) then the
*	downcount is assumed to mean that this entry is to be loaded into the
*	NST the number of times indicated in the downcount, and the alarm
*	bit will be returned set indicating an action should be taken.
*
* 2.If (Counting bit = Counting) (signified by bit 15 = 1) then the
*	value in downcount is decremented by 1 each frame and when the
*	value of either 1 or 0 (as indicated by bit 13 = 1 or 0) is reached
*	the alarm is set as follows:
*	  a. For stop at 1 (Alarm is set every time value is 1).
*	  b. For stop at 0 (Alarm is set on first encounter with 0 and not again).
*
*
* SUMMARY:
*	1.if fixed => No Dec, Set alarm every time even if downcount=0.
*	2.if counting & stop at 1 => Dec if not 1, if(Count=1) set alarm.
*	3.if counting & stop at 0 => Dec if not 0, if(First time Count=0) set alarm.
*
*****************************************************************************/
uint uiSTBL_handleLoadDowncnt(
		uint uiOldLoadVal
		)
	{
	uint uiNewLoadVal;
	uint uiLoadValOnly;


	/* COPY NEW LOAD VALUE FROM OLD LOAD VAL */
	uiNewLoadVal = uiOldLoadVal;

	/* SEPARATE OUT THE COUNT ONLY */
	uiLoadValOnly = uiNewLoadVal & F_DCNT_COUNT_ONLY_MASK;

	/* ASSUME A CLEARED ALARM */
	uiNewLoadVal &= ~F_DCNT_ALARM; //clr the alarm



	/* HANDLE NON-COUNTING */
	if(!(uiOldLoadVal & F_DCNT_COUNTING))
		{
		uiNewLoadVal |= F_DCNT_ALARM; //set the alarm
		goto HandleLoad_exit;
		}

	/* HANDLE THE DOWNCOUNT TO ONE */
	if(uiOldLoadVal & F_DCNT_STOP_AT_ONE)
		{
		/* COUNT DOWN THE VALUE */
		if(uiLoadValOnly > 1) uiLoadValOnly--;	//do downcount

		/* IF WE JUST HIT ONE NOW -- SET THE ALARM */
		if(uiLoadValOnly == 1)
			{
			uiNewLoadVal |= F_DCNT_ALARM; //set the alarm
			goto Re_mergeLoad_and_exit;			//leave
			}
		}


	/* HANDLE THE DOWNCOUNT TO ZRO */
	if(!(uiOldLoadVal & F_DCNT_STOP_AT_ONE))
		{

		/* IF WE ALREADY HIT 0 JUST LEAVE */
		if(uiLoadValOnly == 0) goto Re_mergeLoad_and_exit;

		/* OTHERWISE COUNT DOWN */
		if(uiLoadValOnly > 0) uiLoadValOnly--;	//do downcount

		/* IF WE JUST HIT ZRO NOW -- SET THE ALARM */
		if(uiLoadValOnly == 0)
			{
			uiNewLoadVal |= F_DCNT_ALARM;	//set alarm
			goto Re_mergeLoad_and_exit;				//leave
			}
		}

Re_mergeLoad_and_exit:

	/* RE-MERGE THE FLAGS WITH THE COUNTER */
	uiNewLoadVal &= F_DCNT_FLAGS_ONLY_MASK;
	uiNewLoadVal |= uiLoadValOnly;

HandleLoad_exit:

	return(uiNewLoadVal);

	}/* END: uiSTBL_handleLoadDowncnt() */






/*******************  vSTBL_showAllActionEntrys()  ****************************
*
*
******************************************************************************/

void vSTBL_showAllActionEntrys(
		uchar ucActionToShow
		)
	{
	uchar ucTblIdx;
	uchar ucAction;

	for(ucTblIdx=0;  ucTblIdx<ucGLOB_StblIdx_NFL;  ucTblIdx++)
		{
		/* LOOK FOR THE MATCHING ACTION */
		ucAction = (uchar)ulL2SRAM_getStblEntry(
								SCHED_ACTN_TBL_NUM,		//tbl num
								ucTblIdx				//tbl idx
								);

		/* IF NOT A MATCH THEN CONTINUE */
		if(ucAction != ucActionToShow) continue;

		/* SHOW THIS ENTRY */
		vSTBL_showSingleStblEntry(ucTblIdx, NO_HDR, YES_CRLF);


		}/* END: for() */

	return;

	}/* END: vSTBL_showAllActionEntrys() */






/**********************  vSTBL_showLnkStats()  ******************************
*
*
*
******************************************************************************/
void vSTBL_showLnkStats(
		void
		)
	{
	/* SHOW THE REBOOT COUNT */
	vSERIAL_rom_sout(  "Reboots =");
	vSERIAL_UI16out(uiL2FRAM_getRebootCount());
	vSERIAL_crlf();

	/* SHOW LOST ROM2'S CONNECTIONS */
	vSERIAL_rom_sout(  "ROM2Lost=");
	vSERIAL_UI16out(uiGLOB_lostROM2connections);
	/* SHOW TOTAL ROM2'S ATTEMPTED LINKUPS */
	vSERIAL_rom_sout(", ROM2Trys=");
	vSERIAL_UI16out(uiGLOB_ROM2attempts);
	/* SHOW SDC4 DISCOVERY ATTEMPTS */
	vSERIAL_rom_sout(", SDC4Trys=");
	vSERIAL_UI16out(uiGLOB_TotalSDC4trys);
	vSERIAL_crlf();

	/* SHOW LOST SOM2'S CONNECTIONS */
	vSERIAL_rom_sout(  "SOM2Lost=");
	vSERIAL_UI16out(uiGLOB_lostSOM2connections);
	/* SHOW TOTAL SOM2'S ATTEMPTED LINKUPS */
	vSERIAL_rom_sout(", SOM2Trys=");
	vSERIAL_UI16out(uiGLOB_SOM2attempts);
	/* SHOW RDC4 DISCOVERY ATTEMPTS */
	vSERIAL_rom_sout(", RDC4Trys=");
	vSERIAL_UI16out(uiGLOB_TotalRDC4trys);
	vSERIAL_crlf();


	vSERIAL_crlf();

	return;

	}/* END: vSTBL_showLnkStats() */






/**********************  vSTBL_showPFuncName()  *******************************
*
*
*
******************************************************************************/
void vSTBL_showPFuncName(
		uchar ucPFuncVal	//Priority function number
		)
	{
	ucPFuncVal &= SCHED_FUNC_MASK;

	if(ucPFuncVal >= SCHED_FUNC_MAX_COUNT)
		{
		vSERIAL_rom_sout("---");
		return;
		}
	vSERIAL_rom_sout(cpaPriorityFuncName[ucPFuncVal]);	//show Function name

	return;

	}/*END: vSTBL_showPFuncName() */





/*********************  vSTBL_showPFuncPriority()  ***************************
*
*
*
*
******************************************************************************/
void vSTBL_showPFuncPriority(
		uchar ucPFuncVal	//Priority function number
		)
	{
	ucPFuncVal >>= 5;
	vSERIAL_HB4out(ucPFuncVal);
	return;

	}/*END: vSTBL_showPFuncPriority() */



/*-------------------------------  MODULE END  ------------------------------*/

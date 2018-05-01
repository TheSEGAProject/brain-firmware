

/**************************  DISCOVER.C  **************************************
*
* Discovery routines packaged here
*
*
* V1.00 12/22/2003 wzr
*		Started
*
******************************************************************************/


/*lint -e526 */		/* function not defined */
/*lint -e563 */		/* label not referencecd */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
///*lint -e714 */		/* symbol not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
///*lint -e757 */		/* global declarator not referenced */
///*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


#include "std.h"			//std include
//#include "diag.h"			//diagnostic defines
#include "config.h"			//system configuration definitions
//#include "main.h"			//main defines
#include "radio.h"			//radio C portions
//#include "ad.h" 			//internal A/D
//#include "delay.h"  		//approx delay routine
#include "crc.h"			//CRC calculator routine
#include "misc.h"			//homeless routines
//#include "sram.h"			//static ram routines
#include "rand.h"			//random number generator
//#include "mstor.h"	 	//message storage handler
//#include "temp.h"			//onewire bus
//#include "action.h" 		//event action module
//#include "key.h"			//keyboard handler
#include "l2fram.h"			//level 2 fram routines
#include "serial.h"			//serial port
#include "time.h"			//world time handling package
#include "buz.h"			//buzzer handler routines
#include "msg.h"			//msg handler routines
#include "rts.h"			//Real time scheduler
#include "stbl.h"			//Schedule table routines
#include "l2sram.h" 		//Level 2 SRAM
#include "action.h" 		//actions to be performed
#include "gid.h"			//group ID routines
//#include "opmode.h" 		//operational mode functions
#include "report.h"			//reporting functions
//#include "t0.h" 			//Timer T0 routines
#include "rad40.h"			//Radix 40 routines
#include "sensor.h"			//sensor name list
#include "MODOPT.h"			//Modify Options routines
#include "LNKBLK.h"			//Radio Link routines 


/****************************  DEFINES  **************************************/

//#define TESTING_MSG_TIME_TRANSFER    YES
#ifndef TESTING_MSG_TIME_TRANSFER
  #define TESTING_MSG_TIME_TRANSFER  NOPE
#endif


#define MAX_LINKS_PER_HALFSLOT		6



/********************************  GLOBALS  **********************************/


extern volatile uchar ucaMSG_BUFF[ MAX_RESERVED_MSG_SIZE ];
	
extern uchar ucGLOB_myLevel;		//senders level +1

//extern uchar ucGLOB_CurMsgSeqNum;
extern uchar ucGLOB_StblIdx_NFL;

//extern long lGLOB_lastAwakeFrame;	 	//Nearest thing to cur frame
//extern long lGLOB_lastScheduledFrame;	//last scheduled frame number
extern long lGLOB_lastAwakeLinearSlot;	//Nearest thing to cur linear slot
 
extern long lGLOB_OpMode0_inSec;		//Time when OP mode started

extern int iGLOB_Hr0_to_SysTim0_inSec;	//dist from Hr0 to SysTim0

extern uchar ucGLOB_radioChannel;		//Current radio channel number (0 - 127)



extern volatile union					//ucFLAG0_BYTE
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
											//		In a hub when it is reset.
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

#ifdef RDC4_DIAG_ENABLED
extern volatile union					//ucFLAG1_BYTE
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
#endif

//extern uchar ucGLOB_CurMsgSeqNum;

extern uint uiGLOB_TotalSDC4trys;	//counts number of SDC4 attempts
extern uint uiGLOB_TotalRDC4trys;	//counts number of RDC4 attempts



/******************************  DECLARATIONS  *******************************/

void vDISCOVER_HalfSlot_SDC4(
		long lCurSec
		);




/*******************************  CODE  **************************************/






/************************  vDISCOVER_buildMsgHdr_DC4() ***************************
*
* This routine builds a DC4 msg header
*
******************************************************************************/

static void vDISCOVER_buildMsgHdr_DC4(
		long lSyncTimeSec			//the Sync time Sec value
		)
	{
	/* BUILD THE MSG */

	vMSG_buildMsgHdr_GENERIC(
				DC4_MSG_LAST_BYTE_NUM_UC,	//last byte num
				MSG_TYPE_DC4,				//msg num
				0							//dest SN
				);

	/* STUFF THE SYNC TIME */
	vMISC_copyUlongIntoBytes(
				(ulong)lSyncTimeSec,
				(uchar *) &ucaMSG_BUFF[DC4_IDX_SYNC_TIME_XI],
				NO_NOINT
				);
	vMISC_copyUintIntoBytes(
				DC4_SYNC_IN_CLKS_UI,
				(uchar *) &ucaMSG_BUFF[OM1_IDX_SYNC_TIME_SUB_HI],
				NO_NOINT
				);

	/* STUFF THE MSG SRC ID LEVEL */
	ucaMSG_BUFF[DC4_IDX_SRC_LEVEL] = ucGLOB_myLevel;

	/* STUFF THE HR0_TO_SYSTIM0 VALUE */
	vTIME_copyHr0_to_sysTim0_toBytes(
					(uchar *)&ucaMSG_BUFF[DC4_IDX_HR0_TO_SYSTIM0_IN_SEC_B16]
					);

	/* COMPUTE THE CRC */
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534 //compute the CRC

	return;

	}/* END: vDISCOVER_buildMsgHdr_DC4() */





#ifdef RDC4_DIAG_ENABLED
/***********************  vDISCOVER_RecFakeDC4()  ***************************
*
*
*
******************************************************************************/

static void vDISCOVER_RecFakeDC4(
		void
		)
	{
	long lDC4ReplyTime;

	/* SET THE FAKE MSG REPLY TIME */
	lDC4ReplyTime = 41;

	/* BUILD THE MSG */
	vDISCOVER_buildMsgHdr_DC4(lDC4ReplyTime);

	/* FAKE THE LEVEL TO BE FROM THE HUB */
	ucaMSG_BUFF[DC4_IDX_SRC_LEVEL] = 0;

	/* RECOMPUTE THE CRC */
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534

	/* FAKE THE RECEIVED FLAG */
	ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT = 1;

	return;

	}/* END: vDISCOVER_RecFakeDC4() */
#endif







/************************  vDISCOVER_buildMsgHdr_DC5() ***************************
*
* This routine builds a DC5 msg header
*
******************************************************************************/

static void vDISCOVER_buildMsgHdr_DC5(
		uint uiDestSN,				//dest SN
		usl uslRandSeed				//Rand seed to use
		)
	{
	/* BUILD THE MSG */

	vMSG_buildMsgHdr_GENERIC(
							DC5_MSG_LAST_BYTE_NUM_UC,	//last byte num
							MSG_TYPE_DC5,				//msg num
							uiDestSN					//Dest SN
							);

	/* STUFF THE RAND SEED */
	vMISC_copyUslIntoBytes(uslRandSeed, (uchar *)&ucaMSG_BUFF[DC5_IDX_SEED_HI], NO_NOINT);

	/* COMPUTE THE CRC */
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534 //compute the CRC

	return;

	}/* END: vDISCOVER_buildMsgHdr_DC5() */








#ifdef SDC4_DIAG_ENABLED
/***********************  vDISCOVER_RecFakeDC5()  ****************************
*
*
*
******************************************************************************/

static void vDISCOVER_RecFakeDC5(
		void
		)
	{

	/* BUILD THE DC5 MSG */
	vDISCOVER_buildMsgHdr_DC5(
		uiL2FRAM_getSnumLo16AsUint(), //dest SN
		uslRAND_getRolledFullSysSeed()	//rand num
		);

	/* FAKE THE RECEIVED FLAG */
	ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT = 1;

	return;

	}/* END: vDISCOVER_RecFakeDC5() */
#endif










/********************* vDISCOVER_putROM2inStbl()  ****************************
*
* This routine adds a new STBL entry for an ROM2 link
*
******************************************************************************/

void vDISCOVER_putROM2inStbl(
		uint uiSrcSN,					//src Serial Num
		usl uslRandNum					//Random Seed for tbl entry
		)
	{
	uchar ucStblIdx;
	uchar ucFlagVal;

	#if 0
	vSERIAL_rom_sout("ROM2<");
	vRAD40_showRad40(uiSrcSN);
	vSERIAL_crlf();
	#endif


#if 0    /* CHANGE TO ALGORITHM ALLOWS MULTIPLE LINKS */

	/* LOOK FOR A PREVIOUS/NEW ENTRY IDX FOR ENTRY */
	ucStblIdx = ucSTBL_searchStblsForMatch(
						uiSrcSN,   				//src SN
						E_ACTN_ROM2				//action
						);

	if(ucStblIdx < ucGLOB_StblIdx_NFL)
		{
		#if 1
		vSERIAL_rom_sout("ROM2LkDup\r\n");
		vSTBL_showSingleStblEntry(ucStblIdx, NO_HDR, YES_CRLF);
		#endif

		return;	//leave
		}
#endif



	#if 0
	/*********************  NOTE  ************************************
	 *
	 * Under the new scheduler change the scheduler runs at the
	 * end of a frame so there is no need to check if it has already
	 * run.
	 *
	 *****************************************************************/
	/* CHECK IF THE SCHEDULER HAS RUN ALREADY */
	if(ucRTS_hasSchedRun())
		{
		/* SCHED HAS ALREADY RUN -- ROLL THE RAND NUM TO CATCH UP*/
		#if 1
		vSERIAL_rom_sout("RollRnd\r\n");
		#endif
		uslRandNum = uslRAND_getRolledFullForeignSeed(uslRandNum);
		}
	#endif

	/* SETUP THE ROM2 FLAG VALUE */
	ucFlagVal = F_USE_EXACT_SLOT+F_USE_THIS_RAND;
	ucStblIdx = ucSTBL_getNewStblIdx();


	/* STUFF THE SCHED TBL WITH A ROM2 */
	vSTBL_stuffSingleStblEntry(
		ucStblIdx,						  		//Tbl Index (uchar)
		(PRIORITY_2 | SCHED_FUNC_OM2_SLOT),		//Tbl  0 PFunc entry (uchar)
		uiSrcSN,						   		//Tbl  1 SERIAL NUM entry (uint)
		uslRandNum,						   		//Tbl  2 RANDOM NUM 0 entry (usl)
		uslRandNum,						   		//Tbl  3 RANDOM NUM 1 entry (usl)
		GENERIC_DCNT_TBL_FIXED_DCNT_1,			//Tbl  4 DOWNCOUNT 0 entry (uint)
		GENERIC_DCNT_TBL_FIXED_DCNT_1,			//Tbl  5 DOWNCOUNT 1 entry (uint)
		0,								   		//Tbl  6 EVNT START TIME entry (uint)
		0,								   		//Tbl  7 EVNT INTERVAL entry (uint)
		ucFlagVal,								//Tbl  8 EVNT FLAG entry (uchar)
		GENERIC_STAT_TBL_LNKUP_CNT_1,			//Tbl  9 EVNT STAT 1 entry (uint)
		GENERIC_STAT_TBL_LNKUP_CNT_1,			//Tbl 10 EVNT STAT 2 entry (uint)
		E_ACTN_ROM2,					   		//Tbl 11 ACTN NUM entry (uchar)
		SCHED_LNKBLK_COUNT_TBL_NOT_USED_VAL,	//Tbl 12 LNKBLK IDX entry (uchar)
		SCHED_SENSE_ACT_TBL_NOT_USED_VAL_UL		//Tbl 13 SENSE ACT entry (ulong)
		);


	/* SETUP THE LNKBLK FOR THE NEXT OM2 COMMUNICATION */
	vLNKBLK_fillLnkBlkFromMultipleLnkReq(
			ucStblIdx,
			LNKREQ_1FRAME_1LNK,
			lTIME_getSysTimeAsLong()
			);


	#if 1
	vSTBL_showSingleStblEntry(ucStblIdx, NO_HDR, YES_CRLF);
//	vSERIAL_rom_sout("ROM2<");
//	vRAD40_showRad40(uiSrcSN);
//	vSERIAL_crlf();
	#endif

	return;

	}/* END: vDISCOVER_putROM2inStbl() */

	





/********************* vDISCOVER_putSOM2inStbl()  ****************************
*
* This routine adds an SOM2 entry into STBL when a link has been established
*
******************************************************************************/

void vDISCOVER_putSOM2inStbl(
		uint uiDestSN,					//Dest Serial Num
		usl uslRandNum					//Random Seed for tbl entry
		)
	{
	uchar ucStblIdx;
	uchar ucFlagVal;


	/* LOOK FOR A PREVIOUS/NEW ENTRY IDX FOR ENTRY */
	ucStblIdx = ucSTBL_searchStblsForMatch(
						uiDestSN,  				//dest SN
						E_ACTN_SOM2				//action
						);
	if(ucStblIdx < ucGLOB_StblIdx_NFL)
		{
		#if 1
		vSERIAL_rom_sout("SameROM2lk\r\n");
		vSTBL_showSingleStblEntry(ucStblIdx, NO_HDR, YES_CRLF);
		#endif
		return;
		}


	#if 0
	/*********************  NOTE  ************************************
	 *
	 * Under the new scheduler change the scheduler runs at the
	 * end of a frame so there is no need to check if it has already
	 * run.
	 *
	 *****************************************************************/
	/* CHECK IF THE SCHEDULER HAS RUN ALREADY */
	if(ucRTS_hasSchedRun())
		{
		/* SCHED HAS ALREADY RUN -- ROLL THE RAND NUM TO CATCH UP*/
		uslRandNum = uslRAND_getRolledFullForeignSeed(uslRandNum);
		}
	#endif


	/* SETUP THE SOM2 FLAG VALUE */
	ucFlagVal = F_USE_EXACT_SLOT+F_USE_THIS_RAND;
	ucStblIdx = ucSTBL_getNewStblIdx();


	/* STUFF THE SCHED TBL WITH A SOM2 */
	vSTBL_stuffSingleStblEntry(
		ucStblIdx,							  	//Tbl Index
		(PRIORITY_1 | SCHED_FUNC_OM2_SLOT),		//Tbl  0 USE entry
		uiDestSN,							   	//Tbl  1 SERIAL NUMBER entry
		uslRandNum,							   	//Tbl  2 RANDOM NUMBER 0 entry
		uslRandNum,							   	//Tbl  3 RANDOM NUMBER 1 entry
		GENERIC_DCNT_TBL_FIXED_DCNT_1,			//Tbl  4 DOWNCOUNT 0 entry
		GENERIC_DCNT_TBL_FIXED_DCNT_1,			//Tbl  5 DOWNCOUNT 1 entry
		0,								   		//Tbl  6 EVENT START TIME entry
		0,								   		//Tbl  7 EVENT INTERVAL TIME entry
		ucFlagVal,								//Tbl  8 EVENT FLAG entry
		GENERIC_STAT_TBL_LNKUP_CNT_1,			//Tbl  9	EVENT STAT 1 entry
		GENERIC_STAT_TBL_LNKUP_CNT_1,			//Tbl 10	EVENT STAT 2 entry
		E_ACTN_SOM2,					   		//Tbl 11 ACTION NUMBER entry
		SCHED_LNKBLK_COUNT_TBL_NOT_USED_VAL,	//Tbl 12 LNKBLK IDX entry (uchar)
		SCHED_SENSE_ACT_TBL_NOT_USED_VAL_UL		//Tbl 13 SENSE ACT entry (ulong)
		);


	/* SETUP THE LNKBLK FOR THE NEXT OM2 COMMUNICATION (USING CLK2 TIME) */
	vLNKBLK_fillLnkBlkFromMultipleLnkReq(
				ucStblIdx,
				LNKREQ_1FRAME_1LNK,
				lTIME_getClk2AsLong()
				);


	#if 1
	vSERIAL_rom_sout("NwSOM2lk>");
	//vSERIAL_UIV16out(uiDestSN);
	vRAD40_showRad40(uiDestSN);
	vSERIAL_crlf();
	vSTBL_showSingleStblEntry(ucStblIdx, NO_HDR, YES_CRLF);
	#endif

	return;

	}/* END: vDISCOVER_putSOM2inStbl() */









/************************  vDISCOVER_RDC4()  ********************************
*
*
*
*****************************************************************************/
void vDISCOVER_RDC4(
		void
		)
	{
	ulong ulRandSubSlot;
	ulong ulSlotStartTime_sec;
	ulong ulMsgXmitOffset_sec;
	ulong ulMsgXmitOffset_ns;
	ulong ulMsgXmitOffset_subsec_ns;
	ulong ulMsgXmitTime_sec;
	uint uiMsgXmitTime_tics;
	uint uiMsgXmitTime_clks;
	uint uiDC4SrcSN;
	uchar ucDC4SrcLevel;
	usl uslDC5RandSeed;
	uchar ucaDC5sndTime[MAX_LINKS_PER_HALFSLOT];
	uchar ucXmitRetVal;
	uchar ucFoundStblIdx;
	long lSysTimeInSec;
	long lClk2TimeInSec;
	long lTimeDiffInSec;



	/* CODE STARTS HERE */
//	vT0_start_T0_timer();		//debug TIMER -- no code in front of this

	/* SET THE FREQUENCY */
	vRADIO_setBothFrequencys(
				ucRADIO_getChanAssignment(DISCOVERY_CHANNEL_INDEX),
				NO_FORCE_FREQ
				);

	/* START RECEIVER AS IMMEDIATELY AS POSSIBLE*/
	#ifndef RDC4_DIAG_ENABLED
	/* START RECEIVER FOR DC4 */
	vRADIO_start_receiver();
	#endif

	/* INC THE RDC4 COUNTER */
	uiGLOB_TotalRDC4trys++;

	#ifdef RDC4_DIAG_ENABLED
	vDISCOVER_RecFakeDC4();
	#endif


	#if 1
	/* REPORT TO CONSOLE */
	vSTBL_showSOM2andROM2counts(NO_CRLF);
//	vSERIAL_rom_sout(" ");
//	vSTBL_showRDC4andSDC4counts(NO_CRLF);
	vSERIAL_rom_sout(" Ch");
	vSERIAL_UIV8out(ucGLOB_radioChannel);
	vSERIAL_crlf();
	#endif


	/*------------------  LOOK FOR A DC4 MSG  -------------------------------*/

	/* WAIT FOR DC4 MSG */
	while(TRUE) //lint !e716 !e774
		{
		/* WAIT FOR RECEIVED MSG OR TIMEOUT */
		if(!ucMSG_waitForMsgOrTimeout())
			{
			/* TIMED OUT */
			#if 0
			vSERIAL_rom_sout("RDC4:Tout\r\n"); 
			vTIME_ShowWholeSysTimeInDuS(YES_CRLF);
			#endif

			goto Exit_RDC4;
			}

		#if 1
		/* REPORT THAT WE HAVE A MSG (DC4) FROM SOMEBODY */
		vSERIAL_bout('M');
		#endif


		/* GOT A MSG -- CHK FOR: CRC, MSGTYPE, GROUPID, DEST_SN */
		if(ucMSG_chkMsgIntegrity(	//RET: Bit Err Mask, 0 if OK
			CHKBIT_CRC+CHKBIT_MSG_TYPE,		//chk flags
			CHKBIT_CRC+CHKBIT_MSG_TYPE,		//report flags
			MSG_TYPE_DC4,					//msg type
			0,								//src SN
			0								//Dst SN
		   ))
			{/* MSG IS BAD -- GO RESTART */
			goto Restart_receiver_for_RDC4;
			}

#if 0	//this should be here but is waiting for clarity on time issue.
		/* MSG IS GOOD */
		/* SAVE NEW TIME IN CLK2 -- SO ALARM TIME (CLK1) IS STILL GOOD */
		vTIME_setWholeClk2FromBytes((uchar *)&ucaMSG_BUFF[DC4_IDX_SYNC_TIME_XI]);
		ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_HAVE_WIZ_GROUP_TIME_BIT = 1;	//we have group time
#endif


		/* NOW DECIDE IF WE ARE GOING TO ACCEPT THE INVITATION */
		/* CHECK THE LEVEL */
		ucDC4SrcLevel = ucaMSG_BUFF[DC4_IDX_SRC_LEVEL];
		if(ucDC4SrcLevel > ucGLOB_myLevel)
			{
			#if 1
			/* POST A REJECT MSG */
			vSERIAL_rom_sout("RDC4:RjtLvl, My=");
			vSERIAL_HB8out(ucGLOB_myLevel);
			vSERIAL_rom_sout(" DC4=");
			vSERIAL_HB8out(ucDC4SrcLevel);
			vSERIAL_crlf();
			#endif

			goto Restart_receiver_for_RDC4;
			}


		/* CHECK FOR A PRE-EXISTING LINK */
		uiDC4SrcSN = uiMISC_buildUintFromBytes(
								(uchar *)&ucaMSG_BUFF[DC4_IDX_SRC_SN_HI],
								NO_NOINT
								);
		ucFoundStblIdx = ucSTBL_searchStblsForMatch(
								uiDC4SrcSN,
								E_ACTN_SOM2
								);

		/* IF PRE-EXISTING -- DON'T RECONNECT */
		if(ucFoundStblIdx < ucGLOB_StblIdx_NFL)
			{
			#if 1
			/* REPORT TO CONSOLE THAT WE ALREADY HAVE THIS LINK */
			vSERIAL_rom_sout("RDC4:PrevLk=");
			//vSERIAL_UIV16out(uiDC4SrcSN);
			vRAD40_showRad40(uiDC4SrcSN);
			vSERIAL_crlf();
			//vSTBL_showSingleStblEntry(ucFoundStblIdx, YES_CRLF);
			#endif

			goto Restart_receiver_for_RDC4;
			}
 
		/* MSG IS GOOD, LEVEL IS GOOD, NOT-PRE-EXISTING -- GO CONNECT */
 		break;		//RDC4 msg is good

Restart_receiver_for_RDC4:

		#ifndef RDC4_DIAG_ENABLED
		/* START RECEIVER FOR DC4 */
		vRADIO_start_receiver();
		#endif

		#ifdef RDC4_DIAG_ENABLED
		vDISCOVER_RecFakeDC4();
		#endif

		}/* END: while() */




	/* DISCOVER MSG IS GOOD */
	/*-----------------  NOTE:  ----------------------------------------------
	* This time save should be placed higher up
	* The current higher up code is if'd out for now
	* Code is placed here until there is more clarity on time issue 
	*------------------------------------------------------------------------*/

	/* SAVE NEW TIME IN CLK2 -- SO ALARM TIME (CLK1) IS STILL GOOD */
	vTIME_setWholeClk2FromBytes((uchar *)&ucaMSG_BUFF[DC4_IDX_SYNC_TIME_XI]);

	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_HAVE_WIZ_GROUP_TIME_BIT == 0)
		{
		ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_HAVE_WIZ_GROUP_TIME_BIT = 1;	//we have group time
		ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_RESET_ALL_TIME_BIT = 1;
		}

	/* MSG IS GOOD -- WE ARE ACCEPTING THE INVITATION */
	#if 1
	/* REPORT THAT THE DISCOVERY MSG IS GOOD */
	vSERIAL_bout('G');
	#endif


	/*---------  STASH THE DATA FROM THE DC4  -----------------------------*/

	/* SAVE THE LEVEL */
	ucGLOB_myLevel = ucDC4SrcLevel+1;

	iGLOB_Hr0_to_SysTim0_inSec = (int)uiMISC_buildUintFromBytes(
			(uchar *)&ucaMSG_BUFF[DC4_IDX_HR0_TO_SYSTIM0_IN_SEC_HI],
			NO_NOINT
			);

	/* SAVE THE GROUP ID */
	vGID_setWholeSysGidFromBytes(
			(uchar *)&ucaMSG_BUFF[DC4_IDX_GID_HI]
			);


	/* DETERMINE IF THE TIME HAS SHIFTED 2 SECONDS OR MORE */
	lSysTimeInSec = lTIME_getSysTimeAsLong();
	lClk2TimeInSec = lTIME_getClk2AsLong();
	lTimeDiffInSec = lSysTimeInSec - lClk2TimeInSec;
	if(lTimeDiffInSec < 0) lTimeDiffInSec = -lTimeDiffInSec; //abs value
	if(lTimeDiffInSec >= 2)
		{
		/* REPORT THAT THE TIME HAS CHANGED */
		vREPORT_buildReportAndLogIt(
			SENSOR_TIMECHANGE_HI,				//Sensor 0
			(uint)(((ulong)lClk2TimeInSec)>>16),//Sensor 0 data
			SENSOR_TIMECHANGE_LO,				//Sensor 1
			(uint)(lClk2TimeInSec&0xFFFF),		//Sensor 1 data
			OPTPAIR_RPT_TIM_CHGS_TO_RDIO,		//Radio OptionPairIdx
			OPTPAIR_RPT_TIM_CHGS_TO_FLSH		//Flash OptionPairIdx
			);
		}


	#if 0
	/* REPORT THAT WE HAVE A NEW LINK */
	vSERIAL_rom_sout("BegSOM2>");
	//vSERIAL_UIV16out(uiDC4SrcSN);
	vRAD40_showRad40(uiDC4SrcSN);
	vSERIAL_rom_sout(" @ ");  
	vTIME_showWholeSysTimeInDuS(YES_CRLF);
	#endif

	/*-----------------  COMPUTE THE DC5 REPLY TIME  ------------------------*/

	/* NOW PICK A RANDOM ANSWER SLOT */
	ulRandSubSlot = (ulong)(ucRAND_getRolledMidSysSeed() % MAX_LINKS_PER_HALFSLOT);

	#if 0
	vSERIAL_rom_sout("ulRandSubSlot= ");
	vSERIAL_HBV32out( ulRandSubSlot);
	vSERIAL_crlf();
	#endif


	/* COMPUTE THE SLOT START TIME */
	/* NOTE: MUST USE CLK2 TIME BECAUSE INNER LEVEL MAY BE WAY DIFFERENT */
	ulSlotStartTime_sec = (ulong)lTIME_getClk2AsLong();

	#if 0
	vSERIAL_rom_sout("SltStTm=");
	vSERIAL_HBV32out(ulSlotStartTime_sec);
	vSERIAL_crlf();
	#endif


	/* COMPUTE THE XMIT OFFSET FOR REPLY */
	ulMsgXmitOffset_ns = DC5_SUBSLOT_ST_OFFSET_IN_ns_UL +
						(ulRandSubSlot * DC5_SUBSLOT_WIDTH_IN_ns_UL);
	
	#if 1
	/* REPORT THE OFFSET TIME AND SLOT NUMBER */
	vSERIAL_rom_sout("DC5Offst=");
//	vSERIAL_HBV32out( ulMsgXmitOffset_ns);
//	vSERIAL_bout('(');
	vSERIAL_HB8out((uchar)ulRandSubSlot);
//	vSERIAL_bout(')');
	vSERIAL_crlf();
	#endif

	/* CONVERT TRANSMIT TIME TO SEC AND OFFSET */
	ulMsgXmitOffset_sec = ulMsgXmitOffset_ns / 1000000000UL;
	ulMsgXmitOffset_subsec_ns = ulMsgXmitOffset_ns % 1000000000UL;

	#if 0
	vSERIAL_rom_sout("ulMsgXmitOffset_sec= ");
	vSERIAL_HBV32out( ulMsgXmitOffset_sec);
	vSERIAL_crlf();
	vSERIAL_rom_sout("ulMsgXmitOffset_subsec_ns= ");
	vSERIAL_HBV32out( ulMsgXmitOffset_subsec_ns);
	vSERIAL_crlf();
	#endif

	/* COMPUTE THE TRANSMIT TIME */
	ulMsgXmitTime_sec = ulSlotStartTime_sec + ulMsgXmitOffset_sec;

	/* CONVERT THE SUBSEC-TIME-IN-SEC TO SUBSEC-TIME-IN-TICS */
	uiMsgXmitTime_tics = (uint)(ulMsgXmitOffset_subsec_ns / CLK_nS_PER_LTIC_L);
	uiMsgXmitTime_clks = uiMsgXmitTime_tics | 0x8000;

	/* PACK THE WHOLE TIME  */
	vMISC_copyUlongIntoBytes(					//pack FULL sec part
				ulMsgXmitTime_sec,
				&ucaDC5sndTime[0],
				NO_NOINT
				);
	vMISC_copyUintIntoBytes(
				uiMsgXmitTime_clks,				//pack SUBSEC part
				&ucaDC5sndTime[4],
				NO_NOINT
				);

	#if 0
	{
	/* REPORT TO CONSOLE TRANSMIT TIMES */
	uchar ucjj;
	vSERIAL_rom_sout("XmtTm=");
	for(ucjj=0; ucjj<MAX_LINKS_PER_HALFSLOT; ucjj++)
		{
		vSERIAL_HB8out(ucaDC5sndTime[ucjj]);
		}/* END: for() */

	vSERIAL_rom_sout(" =");
	vTIME_showWholeTimeInDuS(&ucaDC5sndTime[0], YES_CRLF);
	}
	#endif

	/*-----------------------  SEND THE DC5  --------------------------------*/

	uslDC5RandSeed = uslRAND_getRolledFullSysSeed();		//get a new rand seed

	/* BUILD THE DC5 MSG */
	vDISCOVER_buildMsgHdr_DC5(
						uiDC4SrcSN,		//dest SN
						uslDC5RandSeed	//Rand seed
						);


	/* SEND THE DISCOVERY-REPLY */
	ucXmitRetVal = ucMSG_doSubSecXmit(	//RET: 1=sent, 0=too late
							&ucaDC5sndTime[0],
							USE_CLK2,
							NO_RECEIVER_START
							);

	/* PUT RADIO ASLEEP */
	vRADIO_quit();

	/* CHECK THE SYSTIME IN CASE REPLY WAS TOO LATE */
	vTIME_setSysTimeFromClk2();

	/* NOW CHECK IF REPLY WAS TOO LATE */
	if(ucXmitRetVal == 0)
		{
		/* YES REPLY WAS LATE */
		#if 1
		/* REPORT TO CONSOLE */
		vSERIAL_rom_sout("RDC5:MsgTmLate");
		vSERIAL_rom_sout("  StTm=");
		vSERIAL_UIV32out((ulong)ulSlotStartTime_sec);
		vSERIAL_rom_sout("	SndTm=");
		vTIME_showWholeTimeInDuS(&ucaDC5sndTime[0], NO_CRLF);
		vSERIAL_rom_sout("  RndSlt=");
		vSERIAL_UIV32out((ulong)ulRandSubSlot);
		vSERIAL_crlf();
		#endif

		vREPORT_buildReportAndLogIt(
			SENSOR_SOM2_LINK_INFO,				//Sensor 0
			uiDC4SrcSN,							//Sensor 0 data
			SENSOR_SOM2_INFO_REASON,			//Sensor 1
			SOM2_LINK_ESTABLISHED,				//Sensor 1 data
			OPTPAIR_RPT_SOM2_LNKS_TO_RDIO,		//Radio OptionPairIdx
			OPTPAIR_RPT_SOM2_LNKS_TO_FLSH		//Flash OptionPairIdx
			);

		goto Exit_RDC4;

		}/* END: if(ucXmitRetVal == 0) */


	/* REPLY WAS NOT LATE */
	#if 0
	vSERIAL_rom_sout("RDC4:StTm=");
	vSERIAL_HBV32out(ulSlotStartTime_sec);
	vSERIAL_rom_sout(" SentDC5@");
	vTIME_showWholeTimeInDuS(&ucaDC5sndTime[0], NO_CRLF);
	vSERIAL_rom_sout(" RndSlt=");
	vSERIAL_HBV32out(ulRandSubSlot);
	vSERIAL_crlf();
	#endif



	/*--------------  LOG ENTRY IN THE SCHED TABLES  ------------------------*/

	vDISCOVER_putSOM2inStbl(
					uiDC4SrcSN,					//dest serial num
					uslDC5RandSeed				//Random seed
					);

	#if 0
	/* REPORT THAT THE SOM2 ENTRY HAS BEEN CHANGED */
	vSERIAL_rom_sout("SOM2EntChg\r\n");
	#endif


	/* CLEAR ALL THE RDC4 SO WE DON'T WASTE ENERGY */
	vRTS_convertAllRDC4slotsToSleep();

	
	#if 1
	/* SHOW THE RESULTS SO FAR */
	vSTBL_showSOM2andROM2counts(NO_CRLF);
	vSERIAL_rom_sout(" ");
	vSTBL_showRDC4andSDC4counts(YES_CRLF);
	#endif

	vREPORT_buildReportAndLogIt(
			SENSOR_SOM2_LINK_INFO,				//Sensor 0
			uiDC4SrcSN,							//Sensor 0 data
			SENSOR_SOM2_INFO_REASON,			//Sensor 1
			SOM2_LINK_ESTABLISHED,				//Sensor 1 data
			OPTPAIR_RPT_SOM2_LNKS_TO_RDIO,		//Radio OptionPairIdx
			OPTPAIR_RPT_SOM2_LNKS_TO_FLSH		//Flash OptionPairIdx
			);

	/* NOTIFY ANY AUDIO LISTENERS THAT WE HAVE A NET LINK */
	vBUZ_tune_TaDah_TaDah();


Exit_RDC4:

	/* PUT RADIO ASLEEP */
	vRADIO_quit();

	#if 0
	vT0_stop_T0_timer();
	vSERIAL_rom_sout("T0= ");
	vT0_show_T0_in_uS();
	vSERIAL_crlf();
	#endif

	return;

	}/* END: vDISCOVER_RDC4() */







/************************  vDISCOVER_SDC4()  ********************************
*
* This routine calls the SDC4 code twice to avoid a beat against another units
* clock that causes misses.
*
*****************************************************************************/

void vDISCOVER_SDC4(
		void
		)
	{
	uchar ucT3Val;
	long lCurSec;
	long lSendMsgSec;
	long lHalfSlotMark;
	long lEndSlotMark;


//	vT0_Start_T0_timer();				//debug Timer

	/* SET THE FREQUENCY */
	vRADIO_setBothFrequencys(
				ucRADIO_getChanAssignment(DISCOVERY_CHANNEL_INDEX),
				NO_FORCE_FREQ
				);

	/* INC THE SDC4 COUNTER */
	uiGLOB_TotalSDC4trys++;

	#if 1
	vSTBL_showSOM2andROM2counts(NO_CRLF);
	vSERIAL_rom_sout(" ");
//	vSTBL_showRDC4andSDC4counts(NO_CRLF);

	vSERIAL_rom_sout("Ch");
	vSERIAL_UIV8out(ucGLOB_radioChannel);

	vSERIAL_crlf();
	#endif

	/* GET THE CURRENT SECOND */
	lCurSec = lTIME_getSysTimeAsLong();

	/* COMPUTE THE START TIME SECOND */
	lSendMsgSec = (lGLOB_lastAwakeLinearSlot * SECS_PER_SLOT_L) + 
				lGLOB_OpMode0_inSec;

	/* MAKE SURE THAT THIS SEC AND START SEC ARE THE SAME */
	if(lCurSec != lSendMsgSec)
		{
		#if 1
		vSERIAL_rom_sout("SDC4:SecTmMiss,CurTm=");
		vSERIAL_HBV32out((ulong)lCurSec);
		vSERIAL_rom_sout(" SndTm=");
		vSERIAL_HBV32out((ulong)lSendMsgSec);
		vSERIAL_crlf();
		#endif
		goto Exit_SDC4;
		}

	/* COMPUTE THE FIRST HALF SLOT MARK */
	lHalfSlotMark = lCurSec+2L;

	/* SETUP THE T3 TIMER TO STOP AT THE FIRST HALF SLOT POINT */
	ucT3Val = ucTIME_setT3AlarmToSecMinus200ms(lHalfSlotMark);
	if(ucT3Val != 0)
		{
		vSERIAL_rom_sout("SDC4:1stXmt:T3fail=");
		vSERIAL_HB8out(ucT3Val);
		vSERIAL_crlf();
		goto Exit_SDC4;
		}

	#if 0
	/* SHOW THE DEBUG HEADER */
	vSERIAL_rom_sout("SDC4:AfterT3-1setup ");
	vTIME_ShowWholeSysTimeInDuS(YES_CRLF);
	#endif

	/* GO DO THIS FIRST HALF OF THE SLOT */
	vDISCOVER_HalfSlot_SDC4(lCurSec);

	#if 0
	/* SHOW THE DEBUG HEADER */
	vSERIAL_rom_sout("SDC4:AfterHlfSltCall ");
	vTIME_ShowWholeSysTimeInDuS(YES_CRLF);
	#endif

	/* COMPUTE THE SECOND HALF SLOT MARK */
	lEndSlotMark = lCurSec+4L;

	/* SETUP THE T3 TIMER TO STOP AT THE HALF SLOT POINT */
	ucT3Val = ucTIME_setT3AlarmToSecMinus200ms(lEndSlotMark);
	if(ucT3Val != 0)
		{
		vSERIAL_crlf();
		vSERIAL_rom_sout("SDC4:2ndXmt:T3fail=");
		vSERIAL_HB8out(ucT3Val);
		vSERIAL_crlf();
		goto Exit_SDC4;
		}

	#if 0
	/* SHOW THE DEBUG HEADER */
	vSERIAL_rom_sout("SDC4:AfterT3-2setup");
	vTIME_ShowWholeSysTimeInDuS(YES_CRLF);
	#endif

	/* GO DO 2ND HALF OF THE SLOT */
	vDISCOVER_HalfSlot_SDC4(lHalfSlotMark);


Exit_SDC4:

	vRADIO_quit();			//put radio to bed

	#if 0
	STOP_T0_TIMER;
	vSERIAL_rom_sout("T0=");
	vT0_show_T0_in_uS();
	vSERIAL_crlf();
	#endif

	return;

	}/* END: vDISCOVER_SDC4() */








/**********************  vDISCOVER_HalfSlot_SDC4()  *************************
*
* NOTE: Radio receiver is off when this routine exits.
*
*****************************************************************************/

void vDISCOVER_HalfSlot_SDC4(
		long lCurSec
		)
	{
	uchar ucIntegrityRetVal;
	uint uiOtherGuysSN;
	usl uslRandNum;
	uchar ucaSendDC4Time[MAX_LINKS_PER_HALFSLOT];
	uchar ucXmitRetVal;
	uint uiaLinkSN[MAX_LINKS_PER_HALFSLOT];
	uchar ucLinkSNidx;

	/* CLR THE LINK SERIAL NUMS ARRAY */
	for(ucLinkSNidx = 0;  ucLinkSNidx < MAX_LINKS_PER_HALFSLOT;  ucLinkSNidx++)
		{
		uiaLinkSN[ucLinkSNidx] = 0;
		}

	/* INIT SN ARRAY IDX */
	ucLinkSNidx = 0;

	/* BUILD FIRST PART OF DC4 */
	vDISCOVER_buildMsgHdr_DC4(lCurSec);

	/* BUILD WHOLE TIME ARRAY */
	vMISC_copyUlongIntoBytes(
				(ulong)lCurSec,
				&ucaSendDC4Time[0],
				NO_NOINT
				);
	vMISC_copyUintIntoBytes(
				MSG_ST_DELAY_IN_CLKS_UI,
				&ucaSendDC4Time[4],
				NO_NOINT
				);

	#if 0
	/* SHOW THE ACTUAL REQUIRED START TIME */
	vSERIAL_rom_sout("SDC4snd=");
	vTIME_showWholeTimeInDuS(&ucaSendDC4Time[0], YES_CRLF);
	vSERIAL_rom_sout("SDC4sync=");
	vTIME_showWholeTimeInDuS((uchar *)&ucaMSG_BUFF[DC4_IDX_SYNC_TIME_XI], YES_CRLF);
	#endif


	/* SHIP THE MSG */
	#ifdef SDC4_DIAG_ENABLED
	ucXmitRetVal = ucMSG_doSubSecXmit(&ucaSendDC4Time[0], USE_CLK1, NO_RECEIVER_START);
	#else
	ucXmitRetVal = ucMSG_doSubSecXmit(&ucaSendDC4Time[0], USE_CLK1, YES_RECEIVER_START);
	#endif

	#if 0
	/* SHOW THE DEBUG HEADER */
	vSERIAL_rom_sout("HlfSDC4:AfterSnd ");
	vTIME_ShowWholeSysTimeInDuS(YES_CRLF);
	#endif


	if(ucXmitRetVal == 0)
		{
		#if 1
		vSERIAL_rom_sout("SDC4:Late\r\n");
		#endif
		goto Exit_HalfSlot_SDC4;		//too late -- exit the slot
		}

	#if 0
	vSERIAL_rom_sout("SntDC4, ");
	#endif



	/*--------  NOW WAIT FOR SD5 MSGS BACK FROM ORPHANS --------------------*/
	while(TRUE)	//lint !e716 !e774	//LOOP FOR ALL SUB-SLOTS
		{
		while(TRUE)	//lint !e716 !e774 //WAIT FOR A GOOD DC5
			{
			#ifdef SDC4_DIAG_ENABLED
			vDISCOVER_RecFakeDC5();
			#endif

			/* CHECK FOR A TIMEOUT -- IF SO -- LEAVE */
			if(!ucMSG_waitForMsgOrTimeout())
				{
				#if 0
				vSERIAL_rom_sout("HlfSDC4:Tout=");
				vTIME_ShowWholeSysTimeInDuS(YES_CRLF);
				#endif

				goto Exit_HalfSlot_SDC4;		//no replys -- leave
				}

			#if 1
			vSERIAL_rom_sout("M-");
			#endif

			/* GOT A MSG -- CHK FOR: CRC, MSGTYPE, GROUPID, DEST_SN */
			ucIntegrityRetVal = ucMSG_chkMsgIntegrity( //RET: Bit Err Mask, 0 if OK
				CHKBIT_CRC+CHKBIT_MSG_TYPE+CHKBIT_GRP_SEL+CHKBIT_GID+CHKBIT_DEST_SN,
				//CHKBIT_CRC+CHKBIT_MSG_TYPE,
				CHKBIT_CRC+CHKBIT_MSG_TYPE+CHKBIT_GRP_SEL+CHKBIT_GID+CHKBIT_DEST_SN,
				MSG_TYPE_DC5,					//msg type
				0,								//src SN
				uiL2FRAM_getSnumLo16AsUint()	//Dst SN
				);

			if(!ucIntegrityRetVal) break;	//found one break out now

			#ifndef SDC4_DIAG_ENABLED
			/* RESTART THE RADIO */
			vRADIO_start_receiver();
			#endif

			#if 1
			vSERIAL_rom_sout("Rjt\r\n");
			#endif

			}/* END: while(TRUE) */


		/* RECEIVED A GOOD DC5 */
		#if 1
		vSERIAL_rom_sout("G\r\n");
		#endif

		/* SAVE THE MSG DATA */
		uiOtherGuysSN = uiMISC_buildUintFromBytes(
				(uchar *)&ucaMSG_BUFF[DC5_IDX_SRC_SN_HI],
				NO_NOINT
				);
		uslRandNum = ulMISC_buildUlongFromBytes(
				(uchar *)&ucaMSG_BUFF[DC5_IDX_SEED_HI],
				NO_NOINT
				);

		/* START THE RECEIVER BACK UP TO GET THE NEXT ONE */
		vRADIO_start_receiver();

		/* STASH THE LINKUP SN */
		uiaLinkSN[ucLinkSNidx++] = uiOtherGuysSN;

		/* ENTER THIS ONE IN THE SCHED TABLE */ 
		vDISCOVER_putROM2inStbl(
				uiOtherGuysSN,				//SN
				uslRandNum					//Random seed
				);

		#if 1
		/* REPORT TO CONSOLE */
		vSERIAL_rom_sout("ROM2<");
		vRAD40_showRad40(uiOtherGuysSN);
		//vSERIAL_rom_sout(" @ ");
		//vTIME_showWholeSysTimeInDuS(NO_CRLF);
		vSERIAL_rom_sout(" Rnd=");
		vSERIAL_HB24out(uslRandNum);
		vSERIAL_crlf();
		#endif

		#if 1
		vSTBL_showSOM2andROM2counts(NO_CRLF);
		vSERIAL_rom_sout(" ");
		vSTBL_showRDC4andSDC4counts(YES_CRLF);
		#endif

		}/* END: while(TRUE) */

Exit_HalfSlot_SDC4:

	vRADIO_quit();			//go into sleep


	/* REPORT THE LINKUPS */
	{
	uchar ucc;

	if(ucLinkSNidx == 0) return;		//leave if nothing to report


	/* BUILD UP TO 3 OM2 REPORTS */
	for(ucc=0;  ucc<ucLinkSNidx;  ucc++)
		{
		/* START A NEW OM2 */
		if((ucc % 2) == 0)
			{
			/* START A NEW REPORT */
			vREPORT_buildEmptyReportHull();
			}

		/* ADD AN ENTRY TO THE REPORT */
		ucREPORT_addSensorDataToExistingReport(
					SENSOR_ROM2_LINK_INFO,		//Sensor num 0
					uiaLinkSN[ucc],				//Sensor Data 0
					SENSOR_ROM2_INFO_REASON,	//Sensor num 1
					ROM2_LINK_ESTABLISHED		//Sensor Data 2
					); //lint !e534 !e676

		/* FINISH AN OLD OM2 */
		if(((ucc % 2) == 1) || (ucc+1 == ucLinkSNidx)) //if full or at lp end
			{
			/* WRITE OUT THE REPORT */
			vREPORT_logReport(OPTPAIR_RPT_ROM2_LNKS_TO_RDIO,
									  OPTPAIR_RPT_ROM2_LNKS_TO_FLSH
									  );
			}


		}/*END: for(ucc) */

	}


	return;

	}/* END: vDISCOVER_HalfSlot_SDC4() */





/* -----------------------  END OF MODULE  --------------------------------- */

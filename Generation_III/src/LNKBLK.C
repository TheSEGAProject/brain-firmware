

/**************************  LNKBLK.C  *****************************************
*
* Routines to handle the LNKBLK radio link values
*
*
* V1.00 06/29/2008 wzr
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
///*lint -e752 */		/* local declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


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
#include "sram.h"			//sram routines
//#include "radio.h"  		//event RADIO module
//#include "sdctl.h"  		//SD board control routines
//#include "sdcom.h" 		//SD communication package
//#include "l2flash.h"		//level 2 flash routines
#include "time.h"			//Time routines
//#include "daytime.h"		//Daytime routines
//#include "action.h" 		//Event action routines
//#include "sensor.h"			//Sensor names
#include "LNKBLK.h"			//Real Time Scheduler routines
//#include "fulldiag.h"		//full blown diagnostic defines
//#include "delay.h"  		//delay routines
//#include "buz.h"			//buzzer routines
#include "msg.h"			//radio msg helper routines
#include "rand.h"			//Random number generator
//#include "event.h"		//Other events
//#include "l2fram.h"			//Level 2 Ferro Ram routines 
//#include "time.h"			//Time routines
//#include "rts.h"			//Real time sched routines
#include "rad40.h"  		//Radix 40 name conversion
//#include "senseact.h"		//Sense Act name routines
//#include "MODOPT.h" 		//role flags for wiz routines







//extern const uchar ucaBitMask[8];

//extern uchar ucGLOB_StblIdx_NFL;		//next free loc in the sched tables

//extern long   lGLOB_lastAwakeLinearSlot;//Nearest thing to cur linear slot a
//extern long   lGLOB_lastAwakeFrame;		//Nearest thing to cur frame
//extern uchar ucGLOB_lastAwakeSlot;		//Nearest thing to cur slot
extern uchar ucGLOB_lastAwakeNSTtblNum; //Nearest thing to cur NST tbl

//extern int    iGLOB_completeSysLFactor;	//entire Signed LFactor quantity


extern const uchar ucaRandTblNum[2];



/*************************  DECLARATIONS  *************************************/




/*****************************  CODE STARTS HERE  ****************************/


/*******************  lLNKBLK_readSingleLnkBlkEntry()  ***********************
*
* Reads a single Link Blk entry
*
******************************************************************************/

long lLNKBLK_readSingleLnkBlkEntry(
		uchar ucStblIdx,			//Sys Tbl Idx
		uchar ucLnkBlkEntryNum		//entry number in the LnkBlk
		)
	{
	long lLnkBlkIdx;
	long lLnkBlkEntryNum;
	long lLnkBlkAddr;
	long lReadVal;


	lLnkBlkIdx      = (long)ucStblIdx;
	lLnkBlkEntryNum = (long)ucLnkBlkEntryNum;

	lLnkBlkAddr = (long)LNKBLK_TBL_BASE_ADDR +
			 		(lLnkBlkIdx * BYTES_PER_LNKBLK_BLK_L) + 
			 		 lLnkBlkEntryNum * BYTES_PER_LNKBLK_ENTRY_L;

	lReadVal = (long)ulSRAM_read_B32((ulong)lLnkBlkAddr);

	return(lReadVal);

	}/* END: lLNKBLK_readSingleLnkBlkEntry() */








/*******************  vLNKBLK_writeSingleLnkBlkEntry()  **********************
*
* Writes a single Link Blk entry
*
******************************************************************************/

void vLNKBLK_writeSingleLnkBlkEntry(
		uchar ucStblIdx,			//Sys Tbl Idx
		uchar ucLnkBlkEntryNum,		//entry number in the LnkBlk
		long lLnkBlkVal 			//value to write into the LnkBlk
		)
	{
	long lLnkBlkIdx;
	long lLnkBlkEntryNum;
	long lLnkBlkAddr;


	lLnkBlkIdx      = (long)ucStblIdx;
	lLnkBlkEntryNum = (long)ucLnkBlkEntryNum;

	lLnkBlkAddr = (long)LNKBLK_TBL_BASE_ADDR +
			 		(lLnkBlkIdx * BYTES_PER_LNKBLK_BLK_L) + 
			 		 lLnkBlkEntryNum * BYTES_PER_LNKBLK_ENTRY_L;

	vSRAM_write_B32((usl)lLnkBlkAddr, (ulong)lLnkBlkVal);

	return;

	}/* END: vLNKBLK_writeSingleLnkBlkEntry() */








/**********************  vLNKBLK_clrSingleLnkBlk()  ***************************
*
* Zeros a single link Blk of 8 bytes.
*
******************************************************************************/

void vLNKBLK_clrSingleLnkBlk(
		uchar ucStblIdx			//Sys Tbl Idx
		)
	{
	uchar ucjj;

	/* LOOP HORIZONTAL ACROSS ALL ENTRYS IN A BLK */
	for(ucjj=0; ucjj<ENTRYS_PER_LNKBLK_BLK_L;  ucjj++)
		{

		vLNKBLK_writeSingleLnkBlkEntry(ucStblIdx, ucjj, 0L);

		}/* END: for(ucjj) */

	return;

	}/* END: vLNKBLK_clrSingleLnkBlk() */







/*********************  vLNKBLK_zeroEntireLnkBlkTbl(()  **********************
*
* Clear the entire LnkBlk table
*
******************************************************************************/

void vLNKBLK_zeroEntireLnkBlkTbl(
		void
		)
	{
	uchar ucii;

	/* LOOP VERTICAL FOR ALL THE ENTRYS IN THE LNKBLK TBL */
	for(ucii=0;  ucii < GENERIC_SCHED_MAX_IDX;  ucii++)
		{

		vLNKBLK_clrSingleLnkBlk(ucii);

		}/* END: for(uslii) */


	}/* END: vLNKBLK_zeroEntireLnkBlkTbl() */







/**********************  vLNKBLK_showSingleLnkBlk()  *************************
*
* Show all Entrys in a single LnkBlk
*
******************************************************************************/

void vLNKBLK_showSingleLnkBlk(
		uchar ucStblIdx,			//Sys Tbl Idx
		uchar ucTimeFormFlag,		//FRAME_SLOT_TIME,HEX_SEC_TIME,INT_SEC_TIME
		uchar ucShowZroEntryFlag,	//YES_SHOW_ZRO_ENTRYS, NO_SHOW_ZRO_ENTRYS
		uchar ucCRLF_flag			//YES_CRLF, NO_CRLF
		)
	{
	uchar ucjj;
	long lTime;
	uint uiSN;


	/* CHECK IF WE ARE DUMPING ZERO ENTRYS */
	lTime = lLNKBLK_readSingleLnkBlkEntry(ucStblIdx, 0);
	if((!ucShowZroEntryFlag) && (lTime == 0L)) return;

	/* SHOW THE TABLE INDEX NUMBER */
	vSERIAL_HB8out(ucStblIdx);
	vSERIAL_rom_sout(": ");

	/* SHOW THE SERIAL NUMBER */
	uiSN = (uint)ulL2SRAM_getStblEntry(SCHED_SN_TBL_NUM, ucStblIdx);
	vRAD40_showRad40(uiSN);
	vSERIAL_rom_sout("= ");


	/* LOOP HORIZONTAL FOR ALL TABLE ENTRYS */
	for(ucjj=0; ucjj<ENTRYS_PER_LNKBLK_BLK_L;  ucjj++)
		{
		lTime = lLNKBLK_readSingleLnkBlkEntry(ucStblIdx, ucjj);
		if((!ucShowZroEntryFlag) && (lTime == 0L)) break;

		vTIME_showTime(lTime, ucTimeFormFlag, NO_CRLF);

		if(ucjj != ENTRYS_PER_LNKBLK_BLK_L -1)
			{
			vSERIAL_rom_sout(", ");
			}

		}/* END: for(ucjj) */

	if(ucCRLF_flag) vSERIAL_crlf();

	return;

	}/* END: vLNKBLK_showSingleLnkBlk() */








/**********************  vLNKBLK_showAllLnkBlkTblEntrys()  *******************
*
* Dumps all LnkBlk entrys
*
******************************************************************************/

void vLNKBLK_showAllLnkBlkTblEntrys(
		void
		)
	{
	uchar ucii;

	/* LOOP VERTICAL FOR ALL LINK BLOCKS */
	for(ucii=0;  ucii < 16;  ucii++)
//	for(ucii=0;  ucii < ucGLOB_StblIdx_NFL;  ucii++)
		{

		vLNKBLK_showSingleLnkBlk(ucii,					//System Tbl Num
								FRAME_SLOT_TIME,		//Time Form
								NO_SHOW_ZRO_ENTRYS, 	//Show Zero Entrys Flag
								YES_CRLF				//end with CR-LF
								);

		}/* END: for(uslii) */

	return;

	}/* END: vLNKBLK_showAllLnkBlkTblEntrys() */










/**********************  lLNKBLK_computeTimeForSingleLnk()  *************************
*
* Ret: Time For the next link = (BaseTime + DeltaFrame + RandSlot)
*
******************************************************************************/

long lLNKBLK_computeTimeForSingleLnk(
		uchar ucStblIdx,		//Sys Tbl index of the lnk
		uchar ucSpecificLnkReq,	//Link Req = 5bits,,3bits => deltaFrame,,delta Link
		long lBaseTime			//Base time to compute the next lnk from
		)
	{
	uchar ucc;
	long lFrameCount;
	uchar ucEndLnkNum;
	usl uslSavedSeed;
	long lReqSlot;
	long lLnkStartTime;

	/* UNPACK THE LINK REQUEST */
	lFrameCount = (long)(ucSpecificLnkReq >> 3);
	ucEndLnkNum =       (ucSpecificLnkReq & 0x07);		//0000 0111

   #if 0
	vSERIAL_rom_sout("lFrameCount= ");
	vSERIAL_IV32out(lFrameCount);
	vSERIAL_crlf();
	vSERIAL_rom_sout("ucEndLnkNum= ");
	vSERIAL_UIV8out(ucEndLnkNum);
	vSERIAL_crlf();
   #endif

	/* LOAD THE CURRENT SEED NUMBER */
	uslSavedSeed = ulL2SRAM_getStblEntry(
						ucaRandTblNum[ucGLOB_lastAwakeNSTtblNum],	//Sched tbl num
						ucStblIdx						//Sched tbl idx
						);
   #if 0
	vSERIAL_rom_sout("First Seed val= ");
	vSERIAL_HB24out(uslSavedSeed);
	vSERIAL_crlf();
   #endif


	/* ROLL THE SEED FOR THE NUMBER OF FRAMES INTO THE FUTURE */
	for(ucc=0;  ucc<(uchar)lFrameCount;  ucc++)
		{

		/* ROLL THE SEED FOR THE NUMBER OF FRAMES IN THE FUTURE */
		uslSavedSeed = uslRAND_getRolledFullForeignSeed(uslSavedSeed);

	   #if 0
		vSERIAL_rom_sout("Frame loop Seed[");
		vSERIAL_UIV8out(ucc);
		vSERIAL_rom_sout("] val= ");
		vSERIAL_HB24out(uslSavedSeed);
		vSERIAL_crlf();
	   #endif


		}/* END: for() */


	/* ROLL THE SEED FOR THE LINK THAT IS DESIRED  */
	for(ucc=0;  ucc<ucEndLnkNum;  ucc++)
		{

		/* ROLL THE SEED FOR THE NUMBER OF FRAMES IN THE FUTURE */
		uslSavedSeed = uslRAND_getRolledFullForeignSeed(uslSavedSeed);

	   #if 0
		vSERIAL_rom_sout("LnkLoopSeed[");
		vSERIAL_UIV8out(ucc);
		vSERIAL_rom_sout("] val= ");
		vSERIAL_HB24out(uslSavedSeed);
		vSERIAL_crlf();
	   #endif

		}/* END: for() */


   #if 0
	vSERIAL_rom_sout("EndSeedVal= ");
	vSERIAL_HB24out(uslSavedSeed);
	vSERIAL_crlf();
   #endif


	/* COMPUTE SLOT NUMBER FROM THE SEED */
	lReqSlot = (long)((uslSavedSeed >> 8) & GENERIC_NST_MAX_IDX_MASK);



   #if 0
	vSERIAL_rom_sout("ReqSlt= ");
	vSERIAL_HB32out((ulong)lReqSlot);
	vSERIAL_rom_sout("  * 4= ");
	vSERIAL_HB32out((ulong)(lReqSlot * 4));
	vSERIAL_crlf();
   #endif


	/* COMPUTE THE FRAME START TIME (IN SLOTS) */

	/*----------------------  NOTE:  -----------------------------------------
	*
	* The following computation puts the intermediate products into the 
	* variable lLnkStartTime to save stack space.  So the lLnkStartTime is
	* a multiple use variable.
	*
	*------------------------------------------------------------------------*/

	/* COMPUTE THE BASETIME-FRAME-0-NUM */
	lLnkStartTime = lTIME_getFrameNumFromTime(lBaseTime);

	/* CONVERT THE BASETIME-FRAME-0-NUM TO A LINEAR SLOT NUMBER */
	lLnkStartTime *= SLOTS_PER_FRAME_L;


   #if 0
	vSERIAL_rom_sout("LnkBegTim(part1)= ");
	vSERIAL_HB32out((ulong)(lLnkStartTime * 4L));
	vSERIAL_rom_sout("  =  ");
	vTIME_ShowTime((lLnkStartTime * 4L),HEX_SEC_TIME,NO_CRLF);
	vSERIAL_rom_sout("   (Frame base time)");
	vSERIAL_crlf();
   #endif

    /* ADD LNKREQ DELTA FRAME COUNT TO BASETIME TO GET LNKUP-FRAME-0-NUM */
	lLnkStartTime += (lFrameCount * SLOTS_PER_FRAME_L); //+delta frames (in slots)

   #if 0
	vSERIAL_rom_sout("LnkBegTim(part2)= ");
	vSERIAL_HB32out((ulong)(lLnkStartTime * 4L));
	vSERIAL_rom_sout("  =  ");
	vTIME_ShowTime((lLnkStartTime * 4L),HEX_SEC_TIME,NO_CRLF);
	vSERIAL_rom_sout("   (Added frame count)");
	vSERIAL_crlf();
   #endif

    /* ADD RANDOM DELTA SLOT CNT TO LNKUP-FRAME-0-NUM TO GET LNKUP-SLOT-NUM */
	lLnkStartTime += lReqSlot;							//+delta slots (in slots)

   #if 0
	vSERIAL_rom_sout("LnkBegTim(part3)= ");
	vSERIAL_HB32out((ulong)(lLnkStartTime * 4L));
	vSERIAL_rom_sout("  =  ");
	vTIME_ShowTime((lLnkStartTime * 4L),HEX_SEC_TIME,NO_CRLF);
	vSERIAL_rom_sout("   (Added Slot count)");
	vSERIAL_crlf();
   #endif

    /* MULTIPLY LNKUP-SLOT-NUM BY SECS_PER_SLOT TO GET LNKUP-TIME (IN SECS) */
	lLnkStartTime *= SECS_PER_SLOT_L;  				//convert to seconds

   #if 1
	vSERIAL_rom_sout("NxtLnk= ");
	vTIME_showTime(lLnkStartTime,FRAME_SLOT_TIME,YES_CRLF);
   #endif

	return(lLnkStartTime);

	}/* END: lLNKBLK_computeTimeForSingleLnk() */








/*****************  vLNKBLK_fillLnkBlkFromMultipleLnkReq()  ******************
*
* Fills the Link Blk with Link Times determined from the Link Req
*
******************************************************************************/

void vLNKBLK_fillLnkBlkFromMultipleLnkReq(
		uchar ucStblIdx,	   	//Sys Tbl index
		uchar ucMultipleLnkReq,	//Lnk Req = 5bits,,3bits => deltaFrame,,delta Link
		long lBaseTime		    //Base Time to compute the lnks from
		)
	{
	uchar ucc;
	uchar ucUnshiftedFrameCount;
	uchar ucEndLnkNum;
	uchar ucLnkSpecificReq;
	long lLnkTime;

	/* UNPACK THE LINK REQUEST */
	ucUnshiftedFrameCount = ucMultipleLnkReq & 0xF8;	// 1111 1000
	ucEndLnkNum     = ucMultipleLnkReq & 0x07;			// 0000 0111

   #if 0
	vSERIAL_rom_sout("lUnshiftedFrameCount= ");
	vSERIAL_UI8out(ucUnshiftedFrameCount);
	vSERIAL_crlf();
	vSERIAL_rom_sout("ucLnkCount= ");
	vSERIAL_UIV8out(ucEndLnkNum);
	vSERIAL_crlf();
   #endif


	/* CLEAR THE BLOCK BEFORE WE BEGIN */
	vLNKBLK_clrSingleLnkBlk(ucStblIdx);


	/* LOOP FOR ALL REQUESTS */
	for(ucc=0;  ucc<ucEndLnkNum;  ucc++)
		{

		/* BUILD THE LINK-SPECIFIC-REQ */
		ucLnkSpecificReq = ucUnshiftedFrameCount | (ucc+1);

		/* GET THE SPECIFIC LINK REQ TURNED INTO TIME */
		lLnkTime = lLNKBLK_computeTimeForSingleLnk(
						ucStblIdx,
						ucLnkSpecificReq,
						lBaseTime
						);

		/* STASH THE LINK TIME IN THE LNKBLK TBL */
		vLNKBLK_writeSingleLnkBlkEntry(ucStblIdx, ucc, lLnkTime);

		}/* END: for(ucc) */


	return;

	}/* END: vLNKBLK_fillLnkBlkFromMultipleLnkReq() */







/****************  vLNKBLK_showLnkReq()  *************************************
*
*
*
*****************************************************************************/
void vLNKBLK_showLnkReq(
		uchar ucLnkReq
		)
	{

	vSERIAL_UIV8out(ucLnkReq >> 3);
	vSERIAL_bout(':');
	vSERIAL_UIV8out(ucLnkReq & 0x7);

	return;

	}/* END: vLNKBLK_showLnkReq() */









/***************  ucLNKBLK_computeMultipleLnkReqFromSysLoad()  ***************
*
* Compute a Multiple link Request from the system load factor 
*
*
******************************************************************************/

uchar ucLNKBLK_computeMultipleLnkReqFromSysLoad(
		int iSysLoad	//system load in msgs/hr
		)
	{
	uchar ucLnkReq;
	long lSysLoad;
	long lTimeToReachLnkThreshold_inSec;


	#if 0
	vSERIAL_rom_sout("MAX_LNK_DIST_IN_SEC_L=");
	vSERIAL_IV32out(MAX_LNK_DIST_IN_SEC_L);
	vSERIAL_crlf();

	vSERIAL_rom_sout("SECS_PER_FRAME_L=");
	vSERIAL_IV32out(SECS_PER_FRAME_L);
	vSERIAL_crlf();

	vSERIAL_rom_sout("MIN_LNK_DIST_IN_SEC_L=");
	vSERIAL_IV32out(MIN_LNK_DIST_IN_SEC_L);
	vSERIAL_crlf();

	#endif

	/* CONVERT THE SYSTEM LOAD TO A LONG */
	lSysLoad = ((long)iSysLoad);

	/* IF LOAD IS GREATER THAN 0 THEN WE WILL HAVE A BUILD UP OF MESSAGES */
	/* WE ARE INTERESTED IN KNOWING WHEN THIS BUILD UP WILL REACH THE */
	/* LINK TRANSFER THRESHOLD */
	lTimeToReachLnkThreshold_inSec =
		(LNK_MSG_TRANSFER_THRESHOLD_L * 3600L) / lSysLoad;


	/* CHECK TO SEE IF THE LOAD IS NEGATIVE, IF SO GIVE THE MAX LNKREQ */
	if(iSysLoad <= 0)
		{
		ucLnkReq = MAX_LNKREQ;
		goto CMLRFSL_exit;
		}

	#if 0
	vSERIAL_rom_sout("DeltaTime= ");
	vSERIAL_IV32out(lTimeToReachLnkThreshold_inSec);
	vSERIAL_crlf();
	#endif
	
	/* IF TIME TO REACH THRESHOLD IS BEYOND MAX LINK DISTANCE THEN ASSIGN MAX */
	if(lTimeToReachLnkThreshold_inSec > MAX_LNK_DIST_IN_SEC_L)
		{
//		vSERIAL_rom_sout("A\r\n");
		ucLnkReq = MAX_LNKREQ;
		goto CMLRFSL_exit;
		}

	/* IF TIME TO REACH THRESHOLD IS GREATER THAN DIST OF 1 FRAME ASSIGN FRAME */
	if(lTimeToReachLnkThreshold_inSec >= SECS_PER_FRAME_L)
		{
//		vSERIAL_rom_sout("B\r\n");
		ucLnkReq = (uchar)(lTimeToReachLnkThreshold_inSec / SECS_PER_FRAME_L);
		ucLnkReq = (uchar)((ucLnkReq << 3) | 1); 	//build link req
		goto CMLRFSL_exit;
		}
	
	/* IF WE ARE HERE THEN REACH DIST IS LESS THAN 1 FRAME */



	/* IF TIME TO REACH THRESHOLD IS LESS THAN THE MINIMUM THEN USE MIN */
	if(lTimeToReachLnkThreshold_inSec <= MIN_LNK_DIST_IN_SEC_L)
		{
//		vSERIAL_rom_sout("C\r\n");
		ucLnkReq = MIN_LNKREQ;
		goto CMLRFSL_exit;
		}

	/* IF LESS THAN 1 FRAME AND MORE THAN MINIMUM -- ASSIGN LINK COUNT */
//	vSERIAL_rom_sout("D\r\n");
	ucLnkReq  = (uchar)(lTimeToReachLnkThreshold_inSec / MIN_LNK_DIST_IN_SEC_L);
	ucLnkReq  = 8 - ucLnkReq;
	ucLnkReq |= (1<<3);
	//goto CMLRFSL_exit;

CMLRFSL_exit:

	#if 0
	vSERIAL_rom_sout("Ld= ");
	vSERIAL_IV16out(iSysLoad);
	vSERIAL_rom_sout("=> Tim= ");
	vSERIAL_IV32out(lTimeToReachLnkThreshold_inSec);
	vSERIAL_rom_sout("=> LkRq= ");
	vSERIAL_rom_rout("LkRq=");
	vLNKBLK_showLnkReq(ucLnkReq);
	vSERIAL_crlf();
	#endif

	#if 1
	vSERIAL_rom_sout("LkRq=");
	vLNKBLK_showLnkReq(ucLnkReq);
	vSERIAL_crlf();
	#endif



	return(ucLnkReq);

	}/* END: ucLNKBLK_computeMultipleLnkReqFromSysLoad() */




/*-------------------------------  MODULE END  ------------------------------*/

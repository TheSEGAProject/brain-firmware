

/**************************  SYSACT.C  *****************************************
*
* Routines to perform SYSACT functions during
*
*
* V1.00 10/04/2003 wzr
*		Started
*
******************************************************************************/

#include "diag.h"			//Diagnostic package

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
//*lint -e714 */  	/* symbol not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
//*lint -e757 */  	/* global declarator not referenced */
//*lint -e752 */  	/* local declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


//#include <msp430x54x.h>	//processor reg description 
#include "std.h"			//standard defines
#include "config.h" 		//system configuration description file
//#include "main.h"			//
#include "misc.h"			//homeless functions
//#include "reading.h"		//sensor reading module
//#include "ad.h" 			//AD module
//#include "crc.h"			//CRC calculation module
#include "l2sram.h"  		//disk storage module
#include "serial.h" 		//serial IO port stuff
#include "l2fram.h" 		//level 2 fram routines
//#include "sram.h"			//static ram routines
//#include "sysact.h" 		//event SYSACT module
//#include "sdctl.h"		//SD board control routines
//#include "sdcom.h" 		//SD communication package
#include "l2flash.h"		//level 2 flash routines
//#include "time.h"			//Time routines
//#include "action.h" 		//SYSACT action routines
#include "sysact.h"			//SYSACT generator routines
#include "msg.h"			//msg handling routines
//#include "key.h"			//keyboard handler
#include "rts.h"			//Real Time Scheduler
#include "stbl.h"			//Schedule Table routines
#include "pick.h"			//SSP table handler routines
#include "MODOPT.h"			//Modify Options routines
#include "LNKBLK.h"			//LnkBlk table handler routines





//extern const rom uchar ucaDcntTblNum[2];
//extern uchar ucGLOB_myLevel;		//Level indicating direction toward hub
//extern long lGLOB_lastAwakeFrame; 	//Nearest thing to cur frame
extern uchar ucGLOB_lastAwakeSlot;	//Nearest thing to cur slot
extern uchar ucGLOB_testByte;		//counts thermocouple onewire dropouts
extern uchar ucGLOB_testByte2;		//counts button return type 2 errors
extern uchar ucGLOB_lastAwakeNSTtblNum; //nearest thing to cur NST tbl num

//extern uchar ucGLOB_StblIdx_NFL;

#if 0
extern volatile union						//ucFLAG0_BYTE
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
#endif

//extern uchar ucGLOB_SDC4StblIdx;
//extern uchar ucGLOB_RDC4StblIdx;
extern int iGLOB_completeSysLFactor;



void vSYSACT_showSystat(
		void
		);









/*****************************  CODE STARTS HERE  ****************************/




/***************************  vSYSACT_do_sleep()  ***************************
*
* SLEEP UNTIL SLOT START OR BUTTON PUSH 
* This event already has the wakeup alarm set for it when entered.
*
*****************************************************************************/
void vSYSACT_do_sleep(
		void
		)
	{
	unsigned char ucSleepRetVal;


	/*-----------------  ANNOUNCE THE STATUS  -------------------------------*/
	#if 1
	/* SHOW MESSAGE COUNT */
	vSERIAL_rom_sout("M");					//SRAM msg count
	vSERIAL_UIV16out(uiL2SRAM_getMsgCount());

	/* SHOW FLASH MESSAGE COUNT */
	vSERIAL_rom_sout(" F");
	vSERIAL_UIV24out((usl)lL2FRAM_getFlashUnreadMsgCount()); 

	/* SHOW LOAD FACTOR */
	vSERIAL_rom_sout(" Ld");
	vSERIAL_IV16out(iGLOB_completeSysLFactor);
	#endif

	#if 1
	vSERIAL_rom_sout(" ");
	vSTBL_showSOM2andROM2counts(NO_CRLF);
	//vSERIAL_rom_sout(" ");
	//vSTBL_showRDC4andSDC4counts(NO_CRLF);
	#endif

	/* SHOW THE TEST BYTE DATA  */
	#if 1
	if(ucGLOB_testByte)				//if test is running
		{
		vSERIAL_bout('*');
		vSERIAL_UIV8out(ucGLOB_testByte);
		vSERIAL_bout('*');
		}
	#endif

	#if 1
	if(ucGLOB_testByte2)			//if test is running
		{
		vSERIAL_bout('-');
		vSERIAL_UIV8out(ucGLOB_testByte2);
		vSERIAL_bout('-');
		}
	#endif

	vSERIAL_crlf();



	/*---------------  DO SYSTAT STATUS REPORT HERE  -------------------------*/

	if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_SHOW_SYSTAT))
		{
		if((ucGLOB_lastAwakeSlot % 11) == 3)	//status time?
			{
			vSYSACT_showSystat();

			}/* END: if() */

		}/* END: if() */


	/*-----------------  DO THE ACTUAL SLEEP FUNCTION  ----------------------*/

	ucSleepRetVal = ucMISC_sleep_until_button_or_clk(SLEEP_MODE);

	switch(ucSleepRetVal)
		{
		case 0:								/* NORMAL WAKEUP */
			break;

		case 1:								/* HANDLE A BUTTON PUSH */
			#if  0
			vSERIAL_rom_sout("Bttn"); 
			vSERIAL_HB8out(ucGLOB_lastAwakeSlot);
			vSERIAL_crlf();
			#endif

//			ucKEY_doKeyboardCmdInput();		//lint !e534
			break;

		case 2:							/* CHECK FOR PAST DUE ALARM RET */
			vSERIAL_rom_sout("AlrmOverrun\r\n");

			if(ucGLOB_testByte2 != 255) ucGLOB_testByte2++;	//inc test count

			#if 0
			vSERIAL_rom_sout("CurTime= ");
			vSERIAL_IV32out(lThisAwakeTime);

			vSERIAL_rom_sout("\r\nAlarm= ");
			vSERIAL_IV32out(lNextAwakeTime);

			vSERIAL_rom_sout("\r\nlOpUpTimeInSec= ");
			vSERIAL_IV32out(lOpUpTimeInSec);

			vSERIAL_rom_sout("\r\nlThisLinearSlot= ");
			vSERIAL_IV32out(lThisLinearSlot);

			vSERIAL_rom_sout("\r\nlSecsUsedInThisSlot= ");
			vSERIAL_IV32out(lSecsUsedInThisSlot);
			#endif

			vSERIAL_crlf();
			break;

		default:
			break;

		}/* END: switch() */

	return;

	} /* END: vSYSACT_do_sleep() */






/************************  vSYSACT_do_move_SRAM_to_FLASH()  ***********************
*
* This action already has the wakeup alarm set for it when entered.
*
* This action copies the SRAM store and forward to FLASH. Check in ACTION.h
* to for actual frequency but current frequency is 1 time per frame.
*
* Usually this routines is only run on hubs, but if the startup bit is
* set it will run for spokes also.
*
****************************************************************************/
void vSYSACT_do_move_SRAM_to_FLASH(
		void
		)
	{
	ulong uli;
	ulong ulMsgCount;

	/* CHECK IF THERE ARE ANY MESSAGES */
	ulMsgCount = uiL2SRAM_getMsgCount();

	#if 0
	vSERIAL_rom_sout("MN:Msgcnt= ");
	vSERIAL_UIV32out(ulMsgCount);
	vSERIAL_crlf();
	#endif

	if(ulMsgCount == 0) goto Move_SRAM_to_FLASH_exit;

	/* LOOP FOR ALL THE MESSAGES */
   	for(uli=0;  uli<ulMsgCount; uli++)
		{
		/* COPY THE SRAM MSG TO THE MSG BUFFER */
		if(ucL2SRAM_getCopyOfCurMsg() == 0)
			{
			vSERIAL_rom_sout("MN:SramCopyErr\r\n");
			}
	
		/* STUFF THE MSG TO THE FLASH */
		vL2FLASH_storeMsgToFlash();

		/* DELETE THE SRAM MSG */
		vL2SRAM_delCurMsg();

		}/* END: for(uli) */
	
	#if 1
	vSERIAL_rom_sout("Fsh>ptr=");
	vSERIAL_UIV32out((ulong)iL2FRAM_getFlashRefreshPtr()); //lint !e571
	vSERIAL_rom_sout(", cop\'d=");
	vSERIAL_UIV32out(ulMsgCount);
	vSERIAL_rom_sout(", old=");
	vSERIAL_IV32out(lL2FRAM_getFlashReadMsgCount());
	vSERIAL_rom_sout(", nw=");
	vSERIAL_IV32out(lL2FRAM_getFlashUnreadMsgCount());
	vSERIAL_rom_sout(", free=");
	vSERIAL_IV32out(lL2FRAM_getFlashFreeMsgCount());
	vSERIAL_rom_sout(", mx=");
	vSERIAL_IV32out((ulong)FLASH_MAX_MSG_COUNT_L);
	vSERIAL_crlf();
	#endif

Move_SRAM_to_FLASH_exit:

	return;

	}/* END: vSYSACT_do_move_SRAM_to_FLASH() */






#if 0

/***************************  vSYSACT_do_EvaluateSts()  **********************
*
* Evaluate status and adjust load
*
* This routine calls a potpourri of evaluation functions
*
*****************************************************************************/
void vSYSACT_do_EvaluateSts(
		void
		)
	{

	return;

	} /* END: vSYSACT_do_EvaluateSts() */

#endif










/*******************  vSYSACT_showSystat()  *********************************
*
* Show the system status information
*
*****************************************************************************/
void vSYSACT_showSystat(
		void
		)
	{

	/*---------------  DO SYSTAT STATUS REPORT HERE  -------------------------*/



	/*-------------------- SHOW THE HEADER LINE -----------------------------*/
	vSERIAL_crlf();
	vSERIAL_dash(15);
	vSERIAL_rom_sout("  SYSTAT  ");
	vSERIAL_dash(25);
	vSERIAL_crlf();




	/*-----------------  SHOW STATS ON LFACTORS  ----------------------------*/
	if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_SHOW_STS_ON_LFACTRS))
		{
		vSTBL_showStatsOnLFactor();
//		vSERIAL_dash(5);
		vSERIAL_crlf();

		}/* END: if(OPTPAIR_SHOW_STS_ON_LFACTRS) */


	/*-----------  SHOW THE CURRENT NST & STBLS  ----------------------------*/
	#if 1
	vRTS_showAllNSTentrys(ucGLOB_lastAwakeNSTtblNum,YES_SHOW_STBL);
	vSERIAL_dash(5);
	vSERIAL_crlf();
	#endif


	/*------------  SHOW THE LNKBLK REQ SCHEDULE  ---------------------------*/
	vLNKBLK_showAllLnkBlkTblEntrys();
	vSERIAL_dash(5);
	vSERIAL_crlf();


	/*--------------------  SHOW THE SSP  -----------------------------------*/
	//vPICK_showEntireSSPtblFromSRAM(YES_SHOW_ALL_ENTRYS);
	vPICK_showEntireSSPtblFromSRAM( NO_SHOW_ALL_ENTRYS);

	vSERIAL_dash(5);
	vSERIAL_crlf();




	/*-----------  SHOW THE STATISTICS FOR THE RADIO LINKS  -----------------*/

	#if 1
	vSTBL_showLnkStats();
	#endif

	vSERIAL_dash(50);	//give a finishing line 
	vSERIAL_crlf();


	return;

	} /* END: vSYSACT_showSystat() */





/*-------------------------------  MODULE END  ------------------------------*/

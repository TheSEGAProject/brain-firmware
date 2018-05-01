
/**************************  TIME.C  ******************************************
*
* Time routines here
*
* V1.00 10/15/2002 wzr
*		Started
*
******************************************************************************/

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* function not referenced */
/*lint -e752 */		/* local declarator not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */
/*lint -e768 */		/* global struct member not referenced */



#include "diag.h"			//diagnostic defines
#include <msp430x54x.h>		//processor register description
#include "std.h"			//common definitions
#include "config.h" 		//system configuration definitions
//#include "main.h"			//mains defines
//#include "misc.h"			//misc functions
#include "delay.h"  		//delay routines
#include "serial.h" 		//serial port io pkg
#include "time.h"			//time routines
#include "misc.h"			//misc utility routines
#include "bigsub.h"			//big subtract routines
#include "l2fram.h"			//level 2 ferro ram
//#include "opmode.h" 		//opmode msg routines
//#include "action.h" 		//action routines
//#include "report.h" 		//msg reporting routines
#include "button.h"			//button routines
#include "MSG.h"			//Msg routines




extern volatile uchar ucALARM_TIME[4];
extern volatile uchar ucCLK_TIME[4];
extern volatile uchar ucCLK2_TIME[4];


extern long lGLOB_initialStartupTime;		//Time used to compute uptime

extern int iGLOB_Hr0_to_SysTim0_inSec;		//dist from SysTim0 to Hr0

extern volatile uchar ucaBigMinuend[6];
extern volatile uchar ucaBigSubtrahend[6];
extern volatile uchar ucaBigDiff[6];

extern volatile union							//ucFLAG0_BYTE
  {
  uchar byte;
  struct
    {
	unsigned FLG0_BIGSUB_CARRY_BIT:1;	 		//bit 0 ;1=CARRY, 0=NO-CARRY
	unsigned FLG0_BIGSUB_6_BYTE_Z_BIT:1;		//bit 1 ;1=all diff 0, 0=otherwise
	unsigned FLG0_BIGSUB_TOP_4_BYTE_Z_BIT:1;	//bit 2 ;1=top 4 bytes 0, 0=otherwise
	unsigned FLG0_REDIRECT_COMM_TO_ESPORT_BIT:1;//bit 3 ;1=REDIRECT, 0=COMM1
	unsigned FLG0_RESET_ALL_TIME_BIT:1;			//bit 4 ;1=do time  reset, 0=dont
												//SET:	when RDC4 gets finds first
												//		SOM2.
												//		or
												//		In a hub when it is reset.
												//
												//CLR: 	when vMAIN_computeDispatchTiming()
												//		runs next.
	unsigned FLG0_SERIAL_BINARY_MODE_BIT:1;		//bit 5 1=binary mode, 0=text mode
	unsigned FLG0_HAVE_WIZ_GROUP_TIME_BIT:1;	//bit 6 1=Wizard group time has
												//        been aquired from a DC4
												//      0=We are using startup time
	unsigned FLG0_ECLK_OFFLINE_BIT:1;			//bit 7 1=ECLK is not being used
												//      0=ECLK is being used
	}FLAG0_STRUCT;
  }ucFLAG0_BYTE;



extern volatile union							//ucFLAG2_BYTE
		{
		uchar byte;

		struct
		 {
	  	 unsigned FLG2_T3_ALARM_MCH_BIT:1;		//bit 0 ;1=Alarm, 0=no alarm
		 unsigned FLG2_T1_ALARM_MCH_BIT:1;		//bit 1 ;1=match, 0=no match
		 unsigned FLG2_BUTTON_INT_BIT:1;		//bit 2 ;1=XMIT, 0=RECEIVE
		 unsigned FLG2_CLK_INT_BIT:1;			//bit 3	;1=clk ticked, 0=no tic
		 unsigned FLG2_X_FROM_MSG_BUFF_BIT:1;	//bit 4
		 unsigned FLG2_R_BUSY_BIT:1;			//bit 5 ;int: 1=REC BUSY, 0=IDLE
		 unsigned FLG2_R_BARKER_ODD_EVEN_BIT:1;	//bit 6 ;int: 1=odd, 0=even
		 unsigned FLG2_R_BITVAL_BIT:1;			//bit 7 ;int: 
		 }FLAG2_STRUCT;

		}ucFLAG2_BYTE;




/*************************  OVERVIEW  ****************************************
*
* Time is kept by the Hub in seconds (4 bytes) since the Hub was started.
* Sub-Second time is Kept in the Timer A1 Registers TA1R
* 
* The name of the zro time point is called Systime0.
*
* The first Hr mark that occurs before SysTime0 is called Hr0.
*
*
* Whole time (seconds and SubSeconds) is kept in three different formats
* depending on how it is used.
* 
* 1. Dtics: Disjoint tic form . This is the native data form.  it consists of
*	 6 bytes of data. The first 4 bytes are seconds.  The last 2 bytes are
*	 Timer 1 format that is to say 15 bits of sub-second data which is
*	 incremented 32768 times per second.  The high order bit is set so that
*	 each second the timer will roll over.
*
* 2. Ltics Linear Tic form.  This is 6 bytes of continuous data where the high
*	 order bit of the timer has been removed so that the data is in pure tics.
*	 Each tic being one 32768th of a sec.
*
* 3. There is a printed form called Decimal uS form that is simply seconds
*	 above the decimal and micro seconds to the right of the decimal.
* 
******************************************************************************/


/*****************************  CODE  ***************************************/



/**********************  vTIME_init() ****************************************
*
*
*
*****************************************************************************/

void vTIME_init(
		void
		)
	{
//	long lLastKnownTime;
	long lECLKtime;

	/* INIT THE TIME */
	lGLOB_initialStartupTime = (long)ulL2FRAM_getSavedTime();	//default = FRAM saved time
	if(ucTIME_getECLKsts(YES_SKP_ECLK_RAM_FLAG) >= 3)
		{
		lECLKtime = lTIME_getECLKtimeAsLong();	//use ECLK time
		if(lECLKtime >= 0L) lGLOB_initialStartupTime = lECLKtime;
		#if 1
		/* REPORT EXTERNAL CLK TIME */
		vSERIAL_rom_sout("Eclk=");
		vSERIAL_HB32out((ulong) lGLOB_initialStartupTime);
		vSERIAL_crlf();
		#endif
		}
	if(lGLOB_initialStartupTime < 0L)
		{
		vSERIAL_rom_sout("\r\nNegTm,RSet\r\n");
		lGLOB_initialStartupTime = 269943802L;
		}
	vTIME_setSysTimeFromLong(lGLOB_initialStartupTime);		//set the time


	/* SET THE RESET-TIME BIT */
	ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_RESET_ALL_TIME_BIT = 1;

	/* SET CLK2 TIME TO INIT VALUE */
	vTIME_setClk2FromLong(lGLOB_initialStartupTime);

	/* SET THE ALARM TO A VERY HI VALUE */
	vTIME_setAlarmFromLong((long)0x70000000);
	
	/* CLEAR ALL ALARMS */
	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT = 0;
	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT = 0;
	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_CLK_INT_BIT = 0;

	iGLOB_Hr0_to_SysTim0_inSec = 0;


	/* START THE TIME CLK */
	SUB_SEC_TIM = 0;						// init clk to 0

	SUB_SEC_TIM_CTL = SUB_SEC_CTL_INIT;		//TURN TA1 ON, INTERUPT ENABLED, CLEARED PREV INT, ACLK INPUT

	ENDSLOT_TIM_CTL &= ~ENDSLOT_INTFLG_BIT;	//clr prev int
	ENDSLOT_TIM_CTL |= ENDSLOT_INTEN_BIT;	//enable the int

	__bis_SR_register(GIE);					// Yes interrupts

	return;

	}/* END: vTIME_init() */






/***********************  lTIME_getSysTimeAsLong()  **********************************
*
* Return UpTime as a 31bit long value. 
*
* NOTE: This value is returned as a long rather than an ulong
*		because frequently the end result is used in arithmetic calculations.
*		
******************************************************************************/

long lTIME_getSysTimeAsLong(
		void
		)
	{
	return((long)ulMISC_buildUlongFromBytes((uchar *)&ucCLK_TIME[0], YES_NOINT));

	}/* END lTIME_getSysTimeAsLong() */





/***********************  vTIME_copySysTimeToBytes()  *************************
*
******************************************************************************/

void vTIME_copySysTimeToBytes(
		uchar *ucpToPtr				//4 bytes
		)
	{
	vMISC_copyBytesToBytes((uchar *)&ucCLK_TIME[0], ucpToPtr, 4, YES_NOINT);
	return;

	}/* END vTIME_copySysTimeToBytes() */





/***********************  vTIME_setSysTimeFromLong()  *************************
*
* This routine is the final set system time routine to be called.
*		
******************************************************************************/

void vTIME_setSysTimeFromLong(
		long lNewSysTime
		)
	{
	vMISC_copyUlongIntoBytes((ulong)lNewSysTime, (uchar *)&ucCLK_TIME[0], YES_NOINT);

	#if 1
	vSERIAL_rom_sout("SetTim=");
	vSERIAL_HBV32out((ulong)lNewSysTime);
	vSERIAL_crlf();
	#endif

	/* SAVE TIME IN IN FRAM */
	vL2FRAM_stuffSavedTime((ulong)lNewSysTime);

	/* SAVE THE TIME IN ECLK IF WE HAVE ONE */
	if(ucTIME_getECLKsts(NO_SKP_ECLK_RAM_FLAG) > 1)
		{
		vTIME_setECLKtimeFromLong(lNewSysTime);
		}

	return;

	}/* END vTIME_setSysTimeFromLong() */






/********************  vTIME_setSysTimeFromClk2()  ***************************
*
* This routine calles the base set sys time routine.
*		
******************************************************************************/

void vTIME_setSysTimeFromClk2(
		void
		)
	{
	long lNewSysTime;

	lNewSysTime = (long)ulMISC_buildUlongFromBytes(
								(uchar *)&ucCLK2_TIME[0],
								YES_NOINT
								);

	vTIME_setSysTimeFromLong(lNewSysTime);

	return;

	}/* END vTIME_setSysTimeFromClk2() */





/***********************  ucTIME_getECLKsts()  ****************************
*
* Detects if we have an ECLK  
* 
* may need change! because there is no ECLK
*
* RET:	0 -  NO ECLK There is no response to onewire ping.
*		1 -  NO ECLK The ECLK is offline determined by ram flag.
*		2 - YES ECLK But the ECLK is not ticking.
*		3 - YES ECLK and the ECLK is ticking.
*		
******************************************************************************/

uchar ucTIME_getECLKsts(//0=noPing, 1=OfflineFlg, 2=notTicking, 3=OK
		uchar ucSkpRamFlag		//YES_SKP_ECLK_RAM_FLAG, NO_SKP_ECLK_RAM_FLAG
		)
	{
/*
	uchar ucTmp;
	long lTime;
	long lTime2;
	uchar ucRetVal;
	uchar ucButtonEnabledFlag;

	// FIRST CHECK IF THE ECLK IS OFFLINE //
	if(!ucSkpRamFlag)
		{
		if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_ECLK_OFFLINE_BIT) return(1);	//Eclk is offline
		}


	// SAVE THE ROUTINE ENTRY STATE OF THE BUTTON //
	ucButtonEnabledFlag = FALSE;		//assume button state = disabled
	if(BUTTON_IE_PORT & BUTTON_BIT)
		ucButtonEnabledFlag = TRUE;		//wups button state = enabled

	// NOW FLIP THE BUTTON INTO AN OUTPUT PULLUP LINE //
	vBUTTON_quit();						//shut off the button
	BUTTON_DIR_PORT |= BUTTON_BIT;		//reverse button (into an output)
	BUTTON_OUT_PORT |= BUTTON_BIT;		//pullup ECLK on reversed button
	vDELAY_wait100usTic(10);			//wait 1us


	// NEXT -- PING THE CLK TO SEE IF ITS THERE //
	ucTmp = ucONEWIRE_reset_pulse();
	if(!ucTmp)
		{
		ucRetVal = 0;
		ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_ECLK_OFFLINE_BIT = 1;	//no ECLK -- set flg
		goto GES_exit;						//no ping returned
		}

	// CHECK IF THE ECLK IS TICKING //
	lTime = lTIME_getECLKtimeAsLong();
	vDELAY_wait100usTic(12000);					//wait about 1.2 sec
	lTime2 = lTIME_getECLKtimeAsLong();
	lTime++;

	if((lTime == lTime2) || (lTime+1 == lTime2)) //allow Sec+1 & Sec+2
		{
		ucRetVal = 3;
		ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_ECLK_OFFLINE_BIT = 0;	//YES ECLK -- set flg
		goto GES_exit;							//Tic is good
		}

	ucRetVal = 2;								//Tic is Bad


GES_exit:

	// RESTORE THE BUTTON TO ITS STATE ON ROUTINE ENTRY //
	if(ucButtonEnabledFlag)
		{
		BUTTON_OUT_PORT &= ~BUTTON_BIT; //force outgoing latch to 0
		BUTTON_DIR_PORT &= ~BUTTON_BIT;	//turn button around into an input
		vBUTTON_init();					//turn button back on
		}

	return(ucRetVal);
*/
return(0);
	}/* END ucTIME_getECLKsts() */







/***********************  lTIME_getECLKtimeAsLong()  **************************
*
* Return ECLK time as a 31bit long value. 
* may need change! because there is no ECLK
*
* NOTE: If the ECLK is not responding this routine returns 0.
*
* NOTE: This value is returned as a long rather than an ulong
*		because frequently the end result is used in arithmetic calculations.
*		
******************************************************************************/

long lTIME_getECLKtimeAsLong(
		void
		)
	{
/*
	uchar ucECLKval[5];
	long lECLKtime;
	uchar ucButtonEnabledFlag;

	// SAVE THE ROUTINE ENTRY STATE OF THE BUTTON //
	ucButtonEnabledFlag = FALSE;		//assume button state = disabled
	if(BUTTON_IE_PORT & BUTTON_BIT)
		ucButtonEnabledFlag = TRUE;		//wups button state = enabled

	// NOW FLIP THE BUTTON INTO AN OUTPUT PULLUP LINE //
	vBUTTON_quit();						//shut off the button
	BUTTON_DIR_PORT |= BUTTON_BIT;		//reverse button (into an output)
	BUTTON_OUT_PORT |= BUTTON_BIT;		//pullup ECLK on reversed button
	vDELAY_wait100usTic(10);			//wait 1us

	vONEWIRE_init();

	if(!ucONEWIRE_reset_pulse())
		{
//		vSERIAL_rom_sout("NoEclk\r\n");
		lECLKtime = 0L;
		goto GETAL_exit;
		}

	vONEWIRE_bout(ECLK_SKIP_ROM_CMD);
	vONEWIRE_bout(ECLK_READ_CLK_CMD);
	ucECLKval[0] = ucONEWIRE_bin();	//STS
	ucECLKval[1] = ucONEWIRE_bin();	//LO
	ucECLKval[2] = ucONEWIRE_bin();	//MD
	ucECLKval[3] = ucONEWIRE_bin();	//HI
	ucECLKval[4] = ucONEWIRE_bin();	//XI

	ucONEWIRE_reset_pulse();		//lint !e534

	lECLKtime = ucECLKval[4];		//XI
	lECLKtime <<= 8;				//lint !e703
	lECLKtime |= ucECLKval[3];		//HI
	lECLKtime <<= 8;				//lint !e703
	lECLKtime |= ucECLKval[2];		//MD
	lECLKtime <<= 8;				//lint !e703
	lECLKtime |= ucECLKval[1];		//LO

GETAL_exit:

	vONEWIRE_quit();

	// RESTORE THE BUTTON TO ITS STATE ON ROUTINE ENTRY //
	if(ucButtonEnabledFlag)
		{
		BUTTON_OUT_PORT &= ~BUTTON_BIT; //force outgoing latch to 0
		BUTTON_DIR_PORT &= ~BUTTON_BIT;	//turn button around into an input
		vBUTTON_init();					//turn button back on
		}

	#if 0
	vSERIAL_rom_sout("E-STS=");
	vSERIAL_HB8out(ucECLKval[0]);
	vSERIAL_rom_sout(", Eclk=");
	vSERIAL_HB32out((ulong)lECLKtime);
	vSERIAL_crlf();
	#endif

	return(lECLKtime);
*/
return(0);
	}/* END lTIME_getECLKtimeAsLong() */







/***********************  vTIME_setECLKtimeFromLong()  ***********************
*
* NOTE: if not ECLK is present or does not respond the time is not set.
*		
* may need change! because there is no ECLK
******************************************************************************/

void vTIME_setECLKtimeFromLong(
		long lNewECLKtime
		)
	{
/*
	uchar ucECLKval[5];
	uchar ucButtonEnabledFlag;

	// FILL THE ECLK VAL ARRAY //
	ucECLKval[0] = 0x0C;
	ucECLKval[1] = (uchar)lNewECLKtime;
	ucECLKval[2] = (uchar)(lNewECLKtime >>  8);	//lint !e704
	ucECLKval[3] = (uchar)(lNewECLKtime >> 16);	//lint !e704
	ucECLKval[4] = (uchar)(lNewECLKtime >> 24); //lint !e704

	// SAVE THE ROUTINE ENTRY STATE OF THE BUTTON //
	ucButtonEnabledFlag = FALSE;		//assume button state = disabled
	if(BUTTON_IE_PORT & BUTTON_BIT)
		ucButtonEnabledFlag = TRUE;		//wups button state = enabled

	// NOW FLIP THE BUTTON INTO AN OUTPUT PULLUP LINE //
	vBUTTON_quit();						//shut off the button
	BUTTON_DIR_PORT |= BUTTON_BIT;		//reverse button (into an output)
	BUTTON_OUT_PORT |= BUTTON_BIT;		//pullup ECLK on reversed button
	vDELAY_wait100usTic(10);			//wait 1us

	vONEWIRE_init();

	if(!ucONEWIRE_reset_pulse())
		{
		vSERIAL_rom_sout("NoWriteEclk\r\n");
		goto SETFL_exit;
		}

	vONEWIRE_bout(ECLK_SKIP_ROM_CMD);

	vONEWIRE_bout(ECLK_WRITE_CLK_CMD);

	vONEWIRE_bout(ucECLKval[0]);
	vONEWIRE_bout(ucECLKval[1]);
	vONEWIRE_bout(ucECLKval[2]);
	vONEWIRE_bout(ucECLKval[3]);
	vONEWIRE_bout(ucECLKval[4]);

	ucONEWIRE_reset_pulse();		//lint !e534


SETFL_exit:

	vONEWIRE_quit();

	// RESTORE THE BUTTON TO ITS STATE ON ROUTINE ENTRY //
	if(ucButtonEnabledFlag)
		{
		BUTTON_OUT_PORT &= ~BUTTON_BIT; //force outgoing latch to 0
		BUTTON_DIR_PORT &= ~BUTTON_BIT;	//turn button around into an input
		vBUTTON_init();					//turn button back on
		}

	#if 0
	vSERIAL_rom_sout("setEclk:Read=");
	vSERIAL_HB32out((ulong)lNewECLKtime);
	vSERIAL_crlf();
	vSERIAL_rom_sout("setEclk:Set= ");
	vSERIAL_HB32out((ulong)lTIME_getECLKtimeAsLong());
	vSERIAL_crlf();
	#endif
	
	vDELAY_wait100usTic(50);	//ECLK startup delay
*/
	return;

	}/* END vTIME_setECLKtimeFromLong() */








/***********************  uiTIME_getSubSecAsUint()  **********************************
*
* Return Sub Sec Time as Uint. 
*
******************************************************************************/

uint uiTIME_getSubSecAsUint(
		void
		)
	{
	uint uiSubSec;

	uiSubSec = SUB_SEC_TIM;

	return(uiSubSec);

	}/* END uiTIME_getSubSecAsUint() */





/***********************  vTIME_copySubSecToBytes()  *************************
*
******************************************************************************/

void vTIME_copySubSecToBytes(
		uchar *ucpToPtr
		)
	{
	__bic_SR_register(GIE);				// No interrupts

	*(ucpToPtr+0) = SUB_SEC_TIM;		

	__bis_SR_register(GIE);				// Yes interrupts

	return;

	}/* END vTIME_copySubSecToBytes() */





/***********************  vTIME_setSubSecFromUint()  **********************************
*
*		
******************************************************************************/

void vTIME_setSubSecFromUint(
		uint uiNewSubSec
		)
	{
	__bic_SR_register(GIE);				// No interrupts

	SUB_SEC_TIM = uiNewSubSec;
	
	__bis_SR_register(GIE);				// Yes interrupts

	return;

	}/* END vTIME_setSubSecFromUint() */





/***********************  vTIME_setSubSecFromBytes()  **********************************
*
*		
******************************************************************************/

void vTIME_setSubSecFromBytes(
		uchar *ucpFromPtr
		)
	{
	__bic_SR_register(GIE);				// No interrupts

	SUB_SEC_TIM = *ucpFromPtr;
	
	__bis_SR_register(GIE);				// Yes interrupts

	return;

	}/* END vTIME_setSubSecFromBytes() */













/***********************  vTIME_copyWholeSysTimeToBytesInDtics()  *************************
*
*
*
******************************************************************************/

void vTIME_copyWholeSysTimeToBytesInDtics(
		uchar *ucpToPtr				//6 bytes (Ret in Disjoint time format)
		)
	{
	__bic_SR_register(GIE);				// No interrupts

	vMISC_copyBytesToBytes((uchar *)&ucCLK_TIME[0], ucpToPtr, 4, NO_NOINT);

	*(ucpToPtr+4) = SUB_SEC_TIM;			

	__bis_SR_register(GIE);				// Yes interrupts

	return;

	}/* END vTIME_copyWholeSysTimeToBytesInDtics() */







/**************  vTIME_copyWholeSysTimeToBytesLtics()  ************************
*
*
*
******************************************************************************/

void vTIME_copyWholeSysTimeToBytesInLtics(
		uchar *ucpToPtr			//6 bytes (Ret = Linear Tics)
		)
	{
	uchar ucTime_1[6];

	vTIME_copyWholeSysTimeToBytesInDtics(&ucTime_1[0]);	//get time in Dtics

	vTIME_convertDticsToLtics(&ucTime_1[0], ucpToPtr); //convert to Ltics

	return;

	}/* END vTIME_copyWholeSysTimeToBytesInuS() */





/******************  vTIME_setWholeSysTimeFromBytesInDtics()  **********************
*
*		
******************************************************************************/

void vTIME_setWholeSysTimeFromBytesInDtics(
		uchar *ucpFromPtr			//6 bytes ptr (Src in Disjoint time format)
		)
	{
	__bic_SR_register(GIE);				// No interrupts

	vMISC_copyBytesToBytes(ucpFromPtr, (uchar *)&ucCLK_TIME[0], 4, NO_NOINT);

	SUB_SEC_TIM = *(ucpFromPtr+4);		//HI first

	__bis_SR_register(GIE);				// Yes interrupts

	return;

	}/* END vTIME_setWholeSysTimeFromBytesInDtics() */




/******************  vTIME_showWholeSysTimeInDuS()  *******************************
*
*
*
******************************************************************************/
void vTIME_showWholeSysTimeInDuS(		//shown in uS decimal form
		uchar ucCRLF_termFlag		//YES_CRLF, NO_CRLF
		)
	{
	uchar ucTime[6];

	vTIME_copyWholeSysTimeToBytesInDtics((uchar *)&ucTime[0]);
	vTIME_showWholeTimeInDuS((uchar *)&ucTime[0], ucCRLF_termFlag);

	return;

	}/* END: vTIME_showWholeSysTimeInDuS() */







/****************  vTIME_showWholeTimeInDuS()  *******************************
*
*
*
******************************************************************************/

void vTIME_showWholeTimeInDuS(		//show in Decimal uS form
		uchar *ucpTimeArray,	//6 bytes (Disjoint time format)
		uchar ucCRLF_termFlag	//YES_CRLF, NO_CRLF
		)
	{
	long lSec;
	long lSubSec;
	long lSubSecTime_ns;
	long lSubSecTime_us;

	/* PACK THE SUB-SEC PART OF TIME INTO A LONG */
	lSubSec = (long)uiMISC_buildUintFromBytes(ucpTimeArray+4, YES_NOINT);
	lSubSec &= 0x7FFFL;
	/* PACK THE WHOLE PART OF TIME INTO A LONG */
	lSec = (long)ulMISC_buildUlongFromBytes(ucpTimeArray, YES_NOINT);

	#if 0
	vSERIAL_rom_sout("WholeTm= ");
	vSERIAL_HB32out((ulong)lSec);
	vSERIAL_rom_sout("  lSubSec = ");
	vSERIAL_HBV32out((ulong)lSubSec);
	vSERIAL_crlf();
	#endif

	/* COMPUTE THE SUBSEC TICS INTO SUBSEC uS */
	lSubSecTime_ns = lSubSec * CLK_nS_PER_LTIC_L;
	lSubSecTime_us = lSubSecTime_ns / 1000L;

	/* SHOW THE VALUE */
	vSERIAL_IV32out(lSec);
	vSERIAL_bout('.');
	vSERIAL_UI32MicroDecOut(lSubSecTime_us); //in uS.

	if(ucCRLF_termFlag)
		vSERIAL_crlf();

	return;

	}/* END: vTIME_showWholeTimeInDuS() */









/******************  vTIME_showWholeTimeDiffInDuS()  *****************************
*
*
*
******************************************************************************/

void vTIME_showWholeTimeDiffInDuS(	//shown in uS decimal form
		uchar *ucpBegTimeArray,	//6 bytes ptr (Disjoint time format)
		uchar *ucpEndTimeArray,	//6 bytes ptr (Disjoint time format)
		uchar ucCRLF_termFlag  	//YES_CRLF, NO_CRLF
		)
	{
	uchar ucaDiffTime_inLtics[6];
	ulong ulFullSecTics;
	uint uiSubSecTics;
	ulong ulSubSecTics;
	ulong ulSubSec_nS;
	ulong ulSubSec_uS;


	/* COMPUTE THE DIFF */
	ulTIME_computeTimeDiffInLtics(
								ucpBegTimeArray,
								ucpEndTimeArray,
								ucaDiffTime_inLtics
								);	//lint !e534



	/* SPLIT THE TICS INTO SUB SEC AND FULL SEC */
	ulFullSecTics = ulMISC_buildUlongFromBytes(&ucaDiffTime_inLtics[0], NO_NOINT);
	ulFullSecTics <<= 1;						//mult by 2
	if(ucaBigDiff[4] & 0x80) ulFullSecTics |= 1;


	uiSubSecTics = uiMISC_buildUintFromBytes(&ucaDiffTime_inLtics[4], NO_NOINT);
	uiSubSecTics &= 0x7FFF;

	#if 0
	/* SHOW IT IN TICS */
	vSERIAL_HBV32out(ulFullSecTics);
	vSERIAL_bout(':');
	vSERIAL_HB16out(uiSubSecTics);		//in tics
	vSERIAL_rom_sout(" = ");
	#endif

	#if 1
	/* SHOW IT IN uS decimal form */
	ulSubSecTics = uiSubSecTics;
	ulSubSec_nS = ulSubSecTics * CLK_nS_PER_LTIC_UL;
	ulSubSec_uS = ulSubSec_nS / 1000;
	vSERIAL_UIV32out(ulFullSecTics);
	vSERIAL_bout('.');
	vSERIAL_UI32MicroDecOut((long)ulSubSec_uS); //in uS.
	#endif

	if(ucCRLF_termFlag)
		vSERIAL_crlf();

	return;

	}/* END: vTIME_showWholeTimeDiffInDuS() */






/****************  ulTIME_computeTimeDiffInLtics()  **************************
*
*
*
******************************************************************************/

ulong ulTIME_computeTimeDiffInLtics(
		uchar *ucpBegTime_inDtics,	//6 bytes (Disjoint time format)
		uchar *ucpEndTime_inDtics,	//6 bytes (Disjoint time format)
		uchar *ucpDiffTime_inLtics  //6 bytes (Linear Time format)
		)
	{
	ulong ulTics;

	/* COPY THE END TIME INTO THE MINUEND */
	vMISC_copy6ByteTo6Byte(
						ucpEndTime_inDtics,
						(uchar *)ucaBigMinuend,
						NO_NOINT
						);

	/* CONVERT MINUEND TO LINEAR TICS */
	vTIME_convertDticsToLtics(
							(uchar *)ucaBigMinuend,
							(uchar *)ucaBigMinuend
							);

	/* COPY THE START TIME INTO THE SUBTRAHEND */
	vMISC_copy6ByteTo6Byte(
						ucpBegTime_inDtics,
						(uchar *)&ucaBigSubtrahend[0],
						NO_NOINT
						);

	/* CONVERT SUBTRAHEND TO LINEAR TICS */
	vTIME_convertDticsToLtics(
							(uchar *)&ucaBigSubtrahend[0],
							(uchar *)&ucaBigSubtrahend[0]
							);

	/* COMPUTE (ENDTIME - CURTIME) */
	vBIGSUB_doBigDiff();


	/* COPY OUT THE DIFFERENCE */
	ulTics = ulMISC_buildUlongFromBytes(
								(uchar *)&ucaBigDiff[2],
								NO_NOINT
								);

	/* COPY THE DIFFERENCE ARRAY OUT ALSO */
	vMISC_copy6ByteTo6Byte(
						(uchar *)&ucaBigDiff[0],
						ucpDiffTime_inLtics,
						NO_NOINT
						);

	return(ulTics);

	}/* END: ulTIME_computeTimeDiffInLtics() */






/****************  vTIME_convertDticsToLtics()  *********************
*
*
*
******************************************************************************/

void vTIME_convertDticsToLtics(
		uchar *ucpSrcTimeArray,			//6bytes (in Disjoint time format)
		uchar *ucpLinearTimeArray		//6bytes (in Linear time format)
		)
	{
	ulong ulSrcTimeFullSec;
	uint uiSrcTimeSubSec;
	ulong ulConvertedFullSec;
	uint uiConvertedSubSec;


	/* LOAD THE UNCONVERTED TIME */
	ulSrcTimeFullSec = ulMISC_buildUlongFromBytes(
									ucpSrcTimeArray,
									NO_NOINT
									);
	uiSrcTimeSubSec = uiMISC_buildUintFromBytes(
									ucpSrcTimeArray+4,
									NO_NOINT
									);

	/* CONVERT THE SUB SEC TIME FIRST */
	uiConvertedSubSec = uiSrcTimeSubSec & 0x7FFF;

	/* NOW CONVERT THE FULL SEC TIME */
	ulConvertedFullSec = ulSrcTimeFullSec >> 1;
	if(ulSrcTimeFullSec & 1UL) uiConvertedSubSec |= 0x8000;

	/* STUFF THE RESULTS */
	vMISC_copyUlongIntoBytes(
					ulConvertedFullSec,
					ucpLinearTimeArray,
					NO_NOINT
					);
	vMISC_copyUintIntoBytes(
					uiConvertedSubSec,
					ucpLinearTimeArray+4,
					NO_NOINT
					);

	return;

	}/* END: vTIME_convertDticsToLtics() */








/***********************  lTIME_getAlarmAsLong()  **********************************
*
* Return UpTime as a 32bit long value. 
*
* NOTE: This value is returned as a long rather than an ulong
*		because frequently the end result is used in arithmetic calculations.
*		
******************************************************************************/

long lTIME_getAlarmAsLong(
		void
		)
	{
	return((long)ulMISC_buildUlongFromBytes((uchar *)&ucALARM_TIME[0], NO_NOINT));

	}/* END lTIME_getAlarmAsLong() */





/***********************  vTIME_copyAlarmToBytes()  *************************
*
******************************************************************************/

void vTIME_copyAlarmToBytes(
		uchar *ucpToPtr
		)
	{
	vMISC_copyBytesToBytes((uchar *)&ucALARM_TIME[0], ucpToPtr, 4, NO_NOINT);
	return;

	}/* END vTIME_copyAlarmToBytes() */





/***********************  vTIME_setAlarmFromLong()  **************************
*
*		
******************************************************************************/

void vTIME_setAlarmFromLong(
		long lNewAlarm
		)
	{
	long lCurTime;

	vMISC_copyUlongIntoBytes((ulong)lNewAlarm, (uchar *)&ucALARM_TIME[0], NO_NOINT);
	
	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT = 0;	//clr the alarm flag

	/* CHECK IF THE ALARM IS BEING SET BEHIND THE TIME */
	#if 1
	lCurTime = lTIME_getSysTimeAsLong();
	if(lCurTime > lNewAlarm)
		{
		#if 1
		vSERIAL_rom_sout("Err:(Alrm<Tm),Tm=");
		vSERIAL_HBV32out((ulong)lCurTime);
		vSERIAL_rom_sout("h  Alrm=");
		vSERIAL_HBV32out((ulong)lNewAlarm);
		vSERIAL_rom_sout("h\r\n");
		#endif

		/* FORCE THE ALARM FLAG */
		ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT = 1;
		}
	#endif

	#if 0
	vSERIAL_rom_sout("AlrmSet=");
	vSERIAL_IV32out(lNewAlarm);
	vSERIAL_crlf();
	#endif

	return;

	}/* END vTIME_setAlarmFromLong() */








/***********************  vTIME_setAlarmFromBytes()  **********************************
*
*		
******************************************************************************/

void vTIME_setAlarmFromBytes(
		uchar *ucpFromPtr
		)
	{
	long lCurTime;
	long lCurAlarm;

	vMISC_copyBytesToBytes(ucpFromPtr, (uchar *)&ucALARM_TIME[0], 4, NO_NOINT);
	lCurAlarm = lTIME_getAlarmAsLong();

	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT = 0;	//clr the alarm

	/* CHECK IF THE ALARM IS BEING SET BEHIND THE TIME */
	lCurTime = lTIME_getSysTimeAsLong();
	if(lCurTime > lCurAlarm)
		{
		#if 1
		vSERIAL_rom_sout("Err2:(Alrm<Tm)=TM=");
		vSERIAL_HB32Fout((ulong)lCurTime);
		vSERIAL_rom_sout(" Alrm=");
		vSERIAL_HB32Fout((ulong)lCurAlarm);
		vSERIAL_crlf();
		#endif

		/* FORCE THE ALARM FLAG */
		ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT = 1;
		}

	#if 0
	vSERIAL_rom_sout("AlrmSet=");
	vSERIAL_UIV32out((ulong)lTIME_getAlarmAsLong());
	vSERIAL_crlf();
	#endif

	return;

	}/* END vTIME_setAlarmFromBytes() */






/********************  iTIME_getHr0_to_sysTim0_asInt()  **********************
*
*
*
******************************************************************************/

int iTIME_getHr0_to_sysTim0_asInt(
		void
		)
	{

	return(iGLOB_Hr0_to_SysTim0_inSec);

	}/* END: iTIME_getHr0_to_sysTim0_asInt() */






/***********************  vTIME_copyHr0_to_sysTim0_toBytes()  ****************
*
*
******************************************************************************/

void vTIME_copyHr0_to_sysTim0_toBytes(
		uchar *ucpToPtr
		)
	{
	vMISC_copyUintIntoBytes((uint)iGLOB_Hr0_to_SysTim0_inSec, ucpToPtr, NO_NOINT);
	return;

	}/* END vTIME_copyHr0_to_sysTim0_toBytes() */







/**********************  lTIME_get_Hr_From_SysTim_InSecs()  *********************
*
* This routine accepts a Long value in Alarm units (seconds) and
* returns the Wrld Time hour number.
*
*
******************************************************************************/

long lTIME_get_Hr_From_SysTim_InSecs(
		long lUpTimeSecs
		)
	{
	long lTmp;
	long lWrldHr;

	if(lUpTimeSecs > (long)iGLOB_Hr0_to_SysTim0_inSec)
		{
		lTmp = (lUpTimeSecs - (long)iGLOB_Hr0_to_SysTim0_inSec);
		lWrldHr = (lTmp / 3600L) + 1L;
		}
	else
		{
		lWrldHr = 0;
		}

	return(lWrldHr);

	}/* END: lTIME_get_Hr_From_SysTim_InSecs() */





/**********************  iTIME_get_HrOffset_From_SysTim_InSecs()  ******************
*
* This routine accepts a long value in SysTim units (seconds) and
* returns the Wrld hour offset number in secs.
*
*
******************************************************************************/

int iTIME_get_HrOffset_From_SysTim_InSecs(
		long lUpTimeSecs
		)
	{
	long lTmp;
	long lWrldOffsetSecs;

	if(lUpTimeSecs > (long)iGLOB_Hr0_to_SysTim0_inSec)
		{
		lTmp = (lUpTimeSecs - (long)iGLOB_Hr0_to_SysTim0_inSec);
		lWrldOffsetSecs = (lTmp % 3600L);
		}
	else
		{
		lWrldOffsetSecs = (3600L - ((long)iGLOB_Hr0_to_SysTim0_inSec - lUpTimeSecs));
		}

	return((int)lWrldOffsetSecs);

	}/* END: iTIME_get_HrOffset_From_SysTim_InSecs() */






/***********************  lTIME_getClk2AsLong()  **********************************
*
* Return Clk2 as a 31bit long value. 
*
* NOTE: This value is returned as a long rather than an ulong
*		because frequently the end result is used in arithmetic calculations.
*		
******************************************************************************/

long lTIME_getClk2AsLong(
		void
		)
	{
	return((long)ulMISC_buildUlongFromBytes((uchar *)&ucCLK2_TIME[0], YES_NOINT));

	}/* END lTIME_getClk2AsLong() */






/***********************  vTIME_copyClk2ToBytes()  *************************
*
******************************************************************************/

void vTIME_copyClk2ToBytes(
		uchar *ucpToPtr
		)
	{
	vMISC_copyBytesToBytes((uchar *)&ucCLK2_TIME[0], ucpToPtr, 4, YES_NOINT);
	return;

	}/* END vTIME_copyClk2ToBytes() */





/***********************  vTIME_setClk2FromLong()  **********************************
*
*		
******************************************************************************/

void vTIME_setClk2FromLong(
		long lNewClk2
		)
	{
	vMISC_copyUlongIntoBytes((ulong)lNewClk2, (uchar *)&ucCLK2_TIME[0], YES_NOINT);

	return;

	}/* END vTIME_setClk2FromLong() */





/***********************  vTIME_setClk2FromBytes()  **********************************
*
*		
******************************************************************************/

void vTIME_setClk2FromBytes(
		uchar *ucpFromPtr
		)
	{
	vMISC_copyBytesToBytes(ucpFromPtr, (uchar *)&ucCLK2_TIME[0], 4, YES_NOINT);
	return;

	}/* END vTIME_setClk2FromBytes() */









/***********************  vTIME_copyWholeClk2ToBytes()  *************************
*
*
*
******************************************************************************/

void vTIME_copyWholeClk2ToBytes(
		uchar *ucpToPtr				//6 bytes (Ret = Disjoint time format)
		)
	{
	__bic_SR_register(GIE);				// No interrupts

	vMISC_copyBytesToBytes((uchar *)&ucCLK2_TIME[0], ucpToPtr, 4, NO_NOINT);

	*(ucpToPtr+4) = SUB_SEC_TIM;

	__bic_SR_register(GIE);				// YES interrupts

	return;

	}/* END vTIME_copyWholeClk2ToBytes() */






/******************  vTIME_setWholeClk2FromBytes()  **********************
*
*		
******************************************************************************/

void vTIME_setWholeClk2FromBytes(
		uchar *ucpFromPtr			//6 bytes ptr (Src = Disjoint time format)
		)
	{
	__bic_SR_register(GIE);				// No interrupts

	vMISC_copyBytesToBytes(ucpFromPtr, (uchar *)&ucCLK2_TIME[0], 4, NO_NOINT);

	SUB_SEC_TIM = *(ucpFromPtr+4);

	__bis_SR_register(GIE);				// YES interrupts

	return;

	}/* END vTIME_setWholeClk2FromBytes() */




/******************  vTIME_showWholeClk2()  *******************************
*
*
*
******************************************************************************/

void vTIME_showWholeClk2(			//shown in uS decimal form
		uchar ucCRLF_termFlag		//YES_CRLF, NO_CRLF
		)
	{
	uchar ucTime[6];

	vTIME_copyWholeClk2ToBytes((uchar *)&ucTime[0]);
	vTIME_showWholeTimeInDuS((uchar *)&ucTime[0], ucCRLF_termFlag);

	return;

	}/* END: vTIME_showWholeClk2() */






/***********************  uiTIME_getT3AsUint()  **********************************
*
* Return Sub Sec Time as Uint. 
*
******************************************************************************/

uint uiTIME_getT3AsUint(
		void
		)
	{
	uint uiSubSec;

	uiSubSec = ENDSLOT_TIM;

	return(uiSubSec);

	}/* END uiTIME_getT3AsUint() */






/****************  vTIME_setT3AlarmToSecMinus200ms()  *******************************
*
* The T3 timer is an uptimer and sets the alarm when it rolls over.
*
* RET:	0 = OK (T3 has been set)
*		1 = Too Late  (time has passed)
*		2 = Too Close (less than 1/2 sec)
*		3 = Too Early (more than 8 sec)
*
******************************************************************************/

uchar ucTIME_setT3AlarmToSecMinus200ms(
		long lSlotEndSec
		)
	{
	uchar ucRetVal;
	usl uslWholeHiHalf;
	usl uslWholeLoHalf;
	usl uslT3ticTime;
	uint uiT3StartTic;


	ucRetVal = 0;				//assume T3 alarm is good

	/*-----------  SETUP T3 ----------------------------------*/
	ENDSLOT_TIM_CTL = 0x0182;	// 16-bit, Prescale=1:4, ACLK input, Timer off, int cleared and enabled
							
	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT = 0;		//clr T3 alarm


	/*--- GET READY TO COMPUTE THE TIME DIFF BETWEEN SLOTENDSEC AND CURTIME --*/
	
	/* FILL THE T1 END TIME ARRAY FIRST */
	vMISC_copyUlongIntoBytes(
					(ulong)(lSlotEndSec-1),			//Src
					(uchar *)&ucaBigMinuend[0],		//Dest
					NO_NOINT
					);
	vMISC_copyUintIntoBytes(
					0xE666,							//Src (200 ms from end of slot)
					(uchar *)&ucaBigMinuend[4],		//Dest
					NO_NOINT
					);
	#if 0
	vSERIAL_rom_sout("BigMinuend=");
	vSERIAL_HB6ByteOut((uchar *)&ucaBigMinuend[0]);
	vSERIAL_crlf();
	#endif




	/* CONVERT THE NUMBER TO LINEAR TICS */
	vTIME_convertDticsToLtics(
							(uchar *)&ucaBigMinuend[0],
							(uchar *)&ucaBigMinuend[0]
							);
	#if 0
	vSERIAL_rom_sout("CnvrtdBgMinuend=");
	vSERIAL_HB6ByteOut((uchar *)&ucaBigMinuend[0]);
	vSERIAL_crlf();
	#endif


	/* NOW FILL THE CURRENT TIME ARRAY */
	vTIME_copyWholeSysTimeToBytesInDtics((uchar *)ucaBigSubtrahend);
	#if 0
	/* build a test time to check this routine */
	ucaBigSubtrahend[0] = 0;			//debug
	ucaBigSubtrahend[1] = 0;			//debug
	ucaBigSubtrahend[2] = 0;			//debug
	ucaBigSubtrahend[3] = 12;			//debug
	ucaBigSubtrahend[4] = 0xE6;			//debug
	ucaBigSubtrahend[5] = 0x65;			//debug
	#endif

	#if 0
	vSERIAL_rom_sout("BgSubtrahend=");
	vSERIAL_HB6ByteOut((uchar *)&ucaBigSubtrahend[0]);
	vSERIAL_crlf();
	#endif

	/* CONVERT TO LINEAR TICS */
	vTIME_convertDticsToLtics(
							(uchar *)&ucaBigSubtrahend[0],
							(uchar *)&ucaBigSubtrahend[0]
							);
	#if 0
	vSERIAL_rom_sout("CnvrtdBgSubtrahend=");
	vSERIAL_HB6ByteOut((uchar *)&ucaBigSubtrahend[0]);
	vSERIAL_crlf();
	#endif

	/* COMPUTE (ENDTIME - CURTIME) */
	vBIGSUB_doBigDiff();

	#if 0
	vSERIAL_rom_sout("BgDiff=");
	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_6_BYTE_Z_BIT)
		vSERIAL_bout('Z');
	else
		vSERIAL_bout(' ');

	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_TOP_4_BYTE_Z_BIT)
		vSERIAL_bout('4');
	else
		vSERIAL_bout(' ');

	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_CARRY_BIT)
		vSERIAL_bout('C');
	else
		vSERIAL_bout(' ');
	vSERIAL_HB6ByteOut((uchar *)&ucaBigDiff[0]);
	vSERIAL_crlf();
	#endif


	/* GET THE TWO HALVES OF THE DATA */
	uslWholeHiHalf = ulMISC_buildUlongFromBytes(
								(uchar *)&ucaBigDiff[0],
								NO_NOINT
								);
	#if 0
	vSERIAL_rom_sout("uslWholeHiHalf=");
	vSERIAL_HB24out(  uslWholeHiHalf);
	vSERIAL_crlf();
	#endif


	uslWholeLoHalf = ulMISC_buildUlongFromBytes(
								(uchar *)&ucaBigDiff[3],
								NO_NOINT
								);
	#if 0
	vSERIAL_rom_sout("uslWholeLoHalf=");
	vSERIAL_HB24out(  uslWholeLoHalf);
	vSERIAL_crlf();
	#endif


	/*------------------  RANGE CHECK THE DATA  ----------------------------*/

	/* CHECK IF WE ARE TOO EARLY  (MORE THAN 8 SECS AWAY FROM END OF SLOT) */
	if((uslWholeHiHalf != 0) || (uslWholeLoHalf > 0x03FFFF))
		{
		#if 0
		vSERIAL_rom_sout("T3:Early\r\n");
		#endif
		ucRetVal = 3;
		goto Exit_SetT3Alarm;
		}

	/* CHECK IF WE ARE TOO LATE */
	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_CARRY_BIT)
		{
		#if 0
		vSERIAL_rom_sout("T3:Late\r\n");
		#endif
		ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT = 1;
		ucRetVal = 1;
		goto Exit_SetT3Alarm;
		}

	/* CHECK IF WE ARE TOO CLOSE  (1/2 sec)*/
	if(uslWholeLoHalf < 0x004000)
		{
		#if 0
		vSERIAL_rom_sout("T3:TooClose\r\n");
		#endif
		ucRetVal = 2;
		goto Exit_SetT3Alarm;
		}



	/*-----  IF WE ARE HERE THEN THE T3 TIMER IS WITHIN RANGE --------------*/


	/* SCALE T1 TICS TO T3 TICS */
	uslT3ticTime = (uslWholeLoHalf >> 2);

	#if 0
	vSERIAL_rom_sout("uslT3ticTim=");
	vSERIAL_HB24out(  uslT3ticTime);
	vSERIAL_crlf();
	#endif

	/* CONVERT T3 TIME TO A START TIC */
	uiT3StartTic = (uint)(0x010000 - uslT3ticTime);

	#if 0
	vSERIAL_rom_sout("uiT3StTic=");
	vSERIAL_HB16out(  uiT3StartTic);
	vSERIAL_crlf();
	#endif


	/*------- IF WE ARE HERE WE ARE READY TO SET T3 TIMER  ------------------*/
	
	/* STUFF THE T3 TIMER */

	TB0R = uiT3StartTic;

	/* START THE T3 TIMER */
	ENDSLOT_TIM_CTL = 0x0192;			//turn it on with interrupts
	__bis_SR_register(GIE);				// YES interrupts

Exit_SetT3Alarm:

	return(ucRetVal);

	}/* END: vTIME_setT3AlarmToSecMinus200ms() */






/******************  lTIME_convertTimeToLinearSlotNum()  **********************
*
*
*
******************************************************************************/

long lTIME_convertTimeToLinearSlotNum(
		long lTime
		)
	{
	long lLinearSlotNum;

	lLinearSlotNum = lTime / SECS_PER_SLOT_L;

	return(lLinearSlotNum);

	}/* END: lTIME_convertTimeToLinearSlotNum() */







/******************  lTIME_getFrameNumFromTime()  ****************************
*
* Return the frame number of a given time stamp
*
******************************************************************************/

long lTIME_getFrameNumFromTime(
		long lTime
		)
	{
	long lLinearSlotNum;
	long lFrame;

	lLinearSlotNum = lTIME_convertTimeToLinearSlotNum(lTime);
	lFrame = lLinearSlotNum / SLOTS_PER_FRAME_I;

	return(lFrame);

	}/* END: lTIME_getFrameNumFromTime() */








/******************  lTIME_getSlotNumFromTime()  ****************************
*
* Return the slot number of a given time stamp
*
******************************************************************************/

long lTIME_getSlotNumFromTime(
		long lTime
		)
	{
	long lLinearSlotNum;
	long lSlot;

	lLinearSlotNum = lTIME_convertTimeToLinearSlotNum(lTime);
	lSlot = lLinearSlotNum % SLOTS_PER_FRAME_I;

	return(lSlot);

	}/* END: lTIME_getSlotNumFromTime() */







/******************  vTIME_showTime()  ***************************************
*
*
*
******************************************************************************/

void vTIME_showTime(
		long lTime,
		uchar ucTimeFormFlag,	//FRAME_SLOT_TIME,HEX_SEC_TIME,INT_SEC_TIME
		uchar ucCRLF_flag		//NO_CRLF,YES_CRLF
		)
	{
	long lLinearSlotNum;
	long lFrame;
	long lSlot;

	switch(ucTimeFormFlag)
		{
		case FRAME_SLOT_TIME:
			lLinearSlotNum = lTIME_convertTimeToLinearSlotNum(lTime);

			lFrame = lLinearSlotNum / SLOTS_PER_FRAME_I;
			lSlot  = lLinearSlotNum % SLOTS_PER_FRAME_I;

			vSERIAL_HBV32out((ulong)lFrame);
			vSERIAL_bout(':');
			vSERIAL_HB8out((uchar)lSlot);
			break;

		case INT_SEC_TIME:
			vSERIAL_IV32out(lTime);
			break;

		case HEX_SEC_TIME:
		default:
			vSERIAL_HB32out((ulong)lTime);
			break;

		}/* END: switch() */

	if(ucCRLF_flag) vSERIAL_crlf();

	return;

	}/* END: vTIME_showTime() */








/* --------------------------  END of MODULE  ------------------------------- */

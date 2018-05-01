
/**************************  MISC.C  ******************************************
*
* Miscelaneous routines here
*
*
* V1.00 10/15/2002 wzr
*		Started
*
******************************************************************************/

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* function not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */
/*lint -e768 */		/* global struct member not referenced */


#include <msp430x54x.h>		//register and ram definition file 
#include "std.h"			//common definitions
#include "config.h"			//system configuration definitions
//#include "main.h"			//mains defines
#include "misc.H"			//misc functions
#include "DELAY.h"			//delay routines
#include "serial.H"			//serial port IO pkg
//#include "radio.h"  		//radio C portions
//#include "rand.h"			//random number generator
#include "AD.h" 			//10 bit onboard A/D converter
//#include "BUZ.h"			//buzzer control routines
#include "TIME.h"			//system time routines
#include "BUTTON.h"			//button handler


/*----------------------  DEFINES  ------------------------------------------*/

/* LOW VOLTAGE DETECT DEFINITIONS */
#define BATT_LOW_V300	SVSMHRRL_6
#define BATT_LOW_V270	SVSMHRRL_5
#define BATT_LOW_V240	SVSMHRRL_4
#define BATT_LOW_V226	SVSMHRRL_3
#define BATT_LOW_V214	SVSMHRRL_2
#define BATT_LOW_V194	SVSMHRRL_1
#define BATT_LOW_V174	SVSMHRRL_0

/* LEVELs from PIC
#define BATT_LOW_V464	//00001111
#define BATT_LOW_V433	//00001101
#define BATT_LOW_V413	//00001100
#define BATT_LOW_V392	//00001011
#define BATT_LOW_V372	//00001010
#define BATT_LOW_V361	//00001001
#define BATT_LOW_V341	//00001000
#define BATT_LOW_V310	//00000111
#define BATT_LOW_V289	//00000110
#define BATT_LOW_V278	//00000101
#define BATT_LOW_V258	//00000100
*/


/*----------------------  EXTERNS  ------------------------------------------*/

extern volatile union							//ucFLAG2_BYTE
		{
		uchar byte;

		struct
		 {
	  	 unsigned FLG2_T3_ALARM_MCH_BIT:1;		//bit 0 ;1=Alarm, 0=no alarm
		 unsigned FLG2_T1_ALARM_MCH_BIT:1;		//bit 1
		 unsigned FLG2_BUTTON_INT_BIT:1;		//bit 2 ;1=XMIT, 0=RECEIVE
		 unsigned FLG2_CLK_INT_BIT:1;			//bit 3	;1=REC DONE, 0=NOT DONE
		 unsigned FLG2_X_FROM_MSG_BUFF_BIT:1;	//bit 4
		 unsigned FLG2_R_BUSY_BIT:1;			//bit 5 ;int: 1=REC BUSY, 0=IDLE
		 unsigned FLG2_R_BARKER_ODD_EVEN_BIT:1;	//bit 6 ;int: 1=odd, 0=even
		 unsigned FLG2_R_BITVAL_BIT:1;			//bit 7 ;int: 
		 }FLAG2_STRUCT;

		}ucFLAG2_BYTE;


/*--------------------  DECLARATIONS  ---------------------------------------*/

uchar ucMISC_chkLowVoltage(//2=Ok Voltage, 1=Bad Detector, 0=Low Voltage
		uchar ucLowVoltLevel /* LOW_V289, LOW_V278 ... (see defines) */
		);




/**************************  vMISC_setLED()  **************************************
*
*	May be deleted if LED goes
*
******************************************************************************/

void vMISC_setLED(
		uchar ucLEDstate		//LED_ON or LED_OFF
		)
	{

	if(ucLEDstate)
		LED_PORT |= LED_BIT;
	else
		LED_PORT &= ~LED_BIT;

	return;

	}/* END vMISC_setLED() */







/************************  vMISC_blinkLED()  **************************************
*
*
* 
*****************************************************************************/
void vMISC_blinkLED(
		uchar ucBlinkCount
		)
	{
	uchar uc;

	for(uc=0;  uc<ucBlinkCount;  uc++)
		{
		LED_PORT |= LED_BIT;

		vDELAY_wait100usTic(800);

		LED_PORT &= ~LED_BIT;

		vDELAY_wait100usTic(800);

		}

	return;

	}/* END: vMISC_blinkLED() */










/**************** ulMISC_buildGenericFromBytes()  ***********************
*
* Routine to collect consecutive bytes into an ulong. 
*
******************************************************************************/

static ulong ulMISC_buildGenericFromBytes(
		uchar *ucpBytes,					//src bytes
		uchar ucByteCount,					//number of bytes to build
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{
	ulong ulTmp;

	if(ucIntFlag)							//disables interrupts
		{
		  __bic_SR_register(GIE);
		}

	ulTmp = 0;
	switch(ucByteCount)
		{
		case 4:
			ulTmp |= (ulong) *ucpBytes++;
			ulTmp <<= 8;
			/* fall through */
		case 3:								
			ulTmp |= (ulong) *ucpBytes++;
			ulTmp <<= 8;
			/* fall through */
		case 2:								
			ulTmp |= (ulong) *ucpBytes++;
			ulTmp <<= 8;
			/* fall through */
		case 1:								
			ulTmp |= (ulong) *ucpBytes;
			/* fall through */
		default:							
			break;

		}/* END: switch() */

	if(ucIntFlag)
		{
		  __bis_SR_register(GIE);
		}

	return(ulTmp);

	}/* END: ulMISC_buildGenericFromBytes() */







/**************** ulMISC_buildUlongFromBytes()  *******************************
*
* Routine to collect 4 consecutive bytes into an ulong. 
*
******************************************************************************/

ulong ulMISC_buildUlongFromBytes(
		uchar *ucpBytes,
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{
	return(ulMISC_buildGenericFromBytes((uchar *)ucpBytes, 4, ucIntFlag));

	}/* END: ulMISC_buildUlongFromBytes() */






/**************** uiMISC_buildUintFromBytes()  *******************************
*
* Routine to collect 2 consecutive bytes into an ulong. 
*
******************************************************************************/

uint uiMISC_buildUintFromBytes(
		uchar *ucpBytes,
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{
	uint uiVal;

	uiVal = (uint)ulMISC_buildGenericFromBytes(ucpBytes, 2, ucIntFlag);

	#if 0
	vSERIAL_rom_sout("MISC:uiVal=");
	vSERIAL_HBV16out(uiVal);
	vSERIAL_crlf();
	#endif

	return(uiVal);

	}/* END: uiMISC_buildUintFromBytes() */







/**************** vMISC_copyGenericIntoBytes()  *********************************
*
* Routine to copy up to 4 bytes into bytes
*
******************************************************************************/

static void vMISC_copyGenericIntoBytes(
		ulong ulLongVal,					//src value
		uchar *ucpToPtr,					//dest byte ptr
		uchar ucByteCount,					//number of bytes to copy
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{

	if(ucIntFlag)
		{
		  __bic_SR_register(GIE);
		}

	switch(ucByteCount)
		{
		case 4:					//lint !e616 //long
			*ucpToPtr = (uchar)((ulLongVal >> 24) & 0xFF);
			ucpToPtr++;
			/* fall through */
		case 3:					//lint !e616 //usl
			*ucpToPtr = (uchar)((ulLongVal >> 16) & 0xFF);
			ucpToPtr++;
			/* fall through */
		case 2:					//lint !e616 //int
			*ucpToPtr = (uchar)((ulLongVal >>  8) & 0xFF);
			ucpToPtr++;
			/* fall through */
		case 1:					//lint !e616 //char
			*ucpToPtr = (uchar)(ulLongVal & 0xFF);
			/* fall through */
		default:				//lint !e616
			break;

		}/* END: /switch() */

	if(ucIntFlag)
		{
		  __bis_SR_register(GIE);
		}

	return;

	}/* END: vMISC_copyGenericIntoBytes() */







/**************** vMISC_copyUlongIntoBytes()  *********************************
*
* Routine to copy an ulong into 4 ram bytes
*
******************************************************************************/

void vMISC_copyUlongIntoBytes(
		ulong ulVal,
		uchar *ucpToPtr,
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{
	vMISC_copyGenericIntoBytes(ulVal, (uchar *)ucpToPtr, 4, ucIntFlag);

	return;

	}/* END: vMISC_copyUlongIntoBytes() */




/**************** vMISC_copyUslIntoBytes()  *********************************
*
* Routine to copy an ulong into 4 ram bytes
*
******************************************************************************/

void vMISC_copyUslIntoBytes(
		usl uslVal,
		uchar *ucpToPtr,
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{
	vMISC_copyGenericIntoBytes((ulong)uslVal, (uchar *)ucpToPtr, 3, ucIntFlag);

	return;

	}/* END: vMISC_copyUslIntoBytes() */






/**************** vMISC_copyUintIntoBytes()  *********************************
*
* Routine to copy an ulong into 4 ram bytes
*
******************************************************************************/

void vMISC_copyUintIntoBytes(
		uint uiVal,
		uchar *ucpToPtr,
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{
	vMISC_copyGenericIntoBytes((ulong)uiVal, (uchar *)ucpToPtr, 2, ucIntFlag);

	return;

	}/* END: vMISC_copyUintIntoBytes() */






/**************** vMISC_copy6ByteTo6Byte()  *********************************
*
* Routine to copy a 6byte to a 6byte array
*
******************************************************************************/

void vMISC_copy6ByteTo6Byte(
		uchar *ucpFromPtr,			//Src array ptr
		uchar *ucpToPtr,			//Dest array ptr
		uchar ucIntFlag				//YES_NOINT,  NO_NOINT
		)
	{
	vMISC_copyBytesToBytes(ucpFromPtr, ucpToPtr, 6, ucIntFlag);
	return;

	}/* END: vMISC_copy6ByteTo6Byte() */








/**************** ulMISC_copyBytesToBytes() **********************************
*
* Routine to copy n bytes to n bytes in ram
*
******************************************************************************/

void vMISC_copyBytesToBytes(
		uchar *ucpFromPtr,
		uchar *ucpToPtr,
		uchar ucByteCnt,
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{
	uchar ucc;

	if(ucIntFlag)
		{
		  __bic_SR_register(GIE);
		}

	for(ucc=0; ucc<ucByteCnt;  ucc++)
		{
		*ucpToPtr = *ucpFromPtr;
		ucpToPtr++;
		ucpFromPtr++;
		}/* END: for() */

	if(ucIntFlag)
		{
		  __bis_SR_register(GIE);
		}

	return;

	}/* END: vMISC_copyBytesToBytes() */





/********************  ucMISC_chkLowVoltage()  **********************************
*
*
* RET:	2 = OK Voltage
*		1 = Bad Detector (Timed out)
*		0 = Low Voltage
*
******************************************************************************/

uchar ucMISC_chkLowVoltage(//2=Ok Voltage, 1=Bad Detector, 0=Low Voltage
		uchar ucLowVoltLevel /* LOW_V300, LOW_V194 ... (see defines) */
		)
	{
	uchar ucii;

	PMMCTL0_H = PMMPW_H;			//Password to allow PMM access
	SVSMHCTL |= ucLowVoltLevel;		/* Low Volt Detection Limit */
	PMMRIE &= ~(SVMHIE+SVSMLDLYIE);	/* make sure Interrupts are disabled */
	SVSMHCTL_H |= SVMHE;			/* turn on the Detector */

	for(ucii=0;  ucii<200; ucii++)	/* no more than 50us to stabalize - needs change! for timeing */
		{
		if(PMMIFG_L & SVSMLDLYIFG_L) break;
		}
	if(ucii >= 200) return(1);		/* timeout Detector is bad or too slow */

	PMMIFG_L &= ~SVMHIFG_L;				/* clr possible spurious int */

	for(ucii=0;  ucii<200;  ucii++)
		{
		if(PMMIFG_L |= SVMHIFG_L) return(0);	/* found low voltage */
		}/* END: for() */

	SVSMHCTL_H |= SVMHE;			/* turn off Detector to save power */
	PMMCTL0_H = 0xA6;				//wrong Password to block PMM access
	return(2);						/* voltage OK */

	}/* END: ucMISC_chkLowVoltage() */



/*******************  uiMISC_doCompensatedBattRead() *************************
*
* 	This section is being left until it is determined that it will be needed
* 	Set to return a fake value of 4 volts for now.
* 	This will definately need to change!
* 
* Special routine to compute the battery voltage even after we have
* dropped below the 3.3V level from the regulator.
*
* NOTE: During some routine battery margin tests we discovered that the 
*		battery voltage computation did not correspond to the actual 
*		external battery voltage.  After some more specific investigative
*		testing we found that when the battery voltage drops the A/D stops
*		reading linearly at about 3.7V.  We also discovered was that under
*		the 3.7V thershold the A/D reports a rising reading but to our good
*		luck this rising reading is nearly linear, so the A/D reading graph
*		has two parts that can be approximated by two linear functions.  The
*		left linear function is approximated by the function:
*						X = .0064935Y and the
*		right linear function is approximated by the function:
*						X = -.046154Y + 29.90769
*		where X is in volts and Y is the A/D reading value.
*
*		The problem to solve was to determine from a single battery reading
*		which portion of the A/D curve (LEFT or RIGHT) we were on.  To 
*		solve this question we used the LVD (LOW VOLTAGE DETECT) functions
*		that are built into the PIC processor.  By interrogating the LVD
*		and determining which side of the A/D curve we are on we can 
*		get an approximate reading of the external voltage in milliVolts
*		that in only off about (+/-) 6 millivolts.  This is close enough
*		for us to estimate the battery strength and protect the processor
*		and other chips from malfunctions due to low voltage.
*
*		The following voltages are the death voltages for components on the
*		brain board: (decending order):
*
*						Fram chip	2.7V
*						Flash chip	2.5V
*						SRAM chip	2.5V
*						Processor	2.2V
*						Radio		2.2V
*						Buzzer		2.0V
*
*		Because the FRAM is essential for long term operation we do not
*		allow the processor to continue running when the external battery
*		voltage falls below about 2.8V.
*
*		In addition we will not allow a restart of the system if the 
*		external battery voltage is below 3.8V.  The assumption here is
*		that there is a good chance that if the system is restarting then
*		a human operator is near, and the batterys can be changed before
*		a unit is deployed in the field with a low battery pack.
*
*
******************************************************************************/

uint uiMISC_doCompensatedBattRead( 			//RET: Batt Voltage in mV
		void
		)
	{
/*
	uchar ucVal;
	long lVal;
	long lBattReading_in_mV;
	long lM;						//slope
	long lB;						//intercept
	
	lM = 649;						//slope * 100000;
	lB = 0;							//intercept * 100000;

	ucVal = ucMISC_chkLowVoltage(BATT_LOW_V310);
	if((ucVal == 1) || (ucVal == 0))
		{
		// LOW OR BAD BATTERY MEANS WE'RE USING THE RIGHT PART OF A/D CURVE 
//		vSERIAL_rom_sout("LO: ");	//debug
		lM = -4615;					//slope * 100000;
		lB = 2970769;				//intercept * 100000;
		}

	lVal = (long)uiAD_full_init_setup_read_and_shutdown(BATT_AD_CHAN);
//	vSERIAL_IV32out(lVal);							//debug

	lBattReading_in_mV = ((lVal * lM) + lB)/100;
	if((lVal % 100) >= 50) lBattReading_in_mV++;

//	vSERIAL_IV32out(lBattReading_in_mV);			//debug

	return((uint)lBattReading_in_mV);

	}// END: uiMISC_doCompensatedBattRead() 
*/

return (4000); // added in to fake the reading until it can be tested on the hardware, will change!
}



/****************  ucMISC_sleep_until_button_or_clk() ***********************
* 
*  code here likely to change! needs a going over based on sleep mode selections
* 
* NOTE: This routine returns immediately if alarm time is past current time.
*
* RET:	0  if awakened from sleep by timer alarm
*		1  if awakened from sleep by button push
*		2  if by prince (not really) (alarm has already gone off)
*
******************************************************************************/

uchar ucMISC_sleep_until_button_or_clk( //RET 0=Timer, 1=button, 2=past alarm
		uchar ucHibernateFlag	//SLEEP_MODE or HIBERNATE_MODE(no clk)
		)
	{
	long lCurTime;
	long lAlarmTime;
	uchar ucRetVal;


	/* CONVERT THE ALARM TIME TO A LONG */
	lAlarmTime = lTIME_getAlarmAsLong();
	lCurTime   = lTIME_getSysTimeAsLong();

	if(ucHibernateFlag != HIBERNATE_MODE)
		{
		/* IF CURRENT TIME IS PAST ALARM TIME -- THEN RETURN IMMEDIATELY */
		if(lCurTime >= lAlarmTime)
			{
			ucRetVal = 2;
			goto Sleep_immediate_exit;
			}
		}

	/* SHUT OFF THE SERIAL PORT */
	vSERIAL_quit();

	vRADIO_ForceRXMode();					//antenna to receive, might change!

	ADC12CTL0_L &= ~ADC12ENC_L;				//Disable A/D Conversion allowing changes
	ADC12CTL0_L &= ~ADC12ON_L;				//Turn Off A/D
	vDELAY_wait100usTic(20);				//2ms settle time

	vBUTTON_init();

	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT = 0;	//clr alarm flag
	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT = 0;	//clr alarm flag

	if(ucHibernateFlag == HIBERNATE_MODE)
		{
			// Decide about hibernation and make change! to all that follows.
//		T1CONbits.TMR1ON = 0;				//shut off T1 clk
//		T1CONbits.T1OSCEN = 0;				//shut off T1 osc
		}
/*
DoSleep:
	_asm
		SLEEP
		NOP									//lint !e40 !e522 !e10
	_endasm;								//lint !e40 !e522 !e10
*/

	ucRetVal = 1;
	if(ucBUTTON_isButtonFlgSet()) goto Wakeup;	//check if its a button

	ucRetVal = 0;
	if(ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT) goto Wakeup;
	if(ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT) goto Wakeup;

//	goto DoSleep;


Wakeup:

	if(ucHibernateFlag == HIBERNATE_MODE)
		{
//		T1CONbits.T1OSCEN = 1;				//Turn T1 osc
//		T1CONbits.TMR1ON = 1;				//Turn ON T1 clk
		}

	vSERIAL_init();		//500us wait

Sleep_immediate_exit:

	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT = 0;	//clr alarm flag
	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT = 0;	//clr alarm flag

	return(ucRetVal);

	}/* END: ucMISC_sleep_until_button_or_clk() */


/* --------------------------  END of MODULE  ------------------------------- */

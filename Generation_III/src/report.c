

/**************************  REPORT.C  *****************************************
*
* Routines to perform REPORTs during events
*
*
* V1.00 10/04/2003 wzr
*		Started
*
******************************************************************************/

/*************************  NOTE  ********************************************
*
*   INTERNAL REPORTING NOTE
*
* Reports are a wizards internal reporting system that are logged to FLASH and
* can also be sent to the radio (stored in SRAM).  These routines are here
* to help make the reporting task easier.
*
*
* When a Hub does internal reporting it only stores to flash and does
* not store to SRAM because in the Hub the SRAM data is simply copied 
* to the FLASH anyway. If it copied to SRAM and FLASH it would simply put
* a double entry in FLASH eventually.  Because of this the store routine
* does not allow a Hub to copy a report to SRAM even if the
* COPY_TO_SRAM flag is set. The master has two situations for internal 
* reporting:
*		1. OFF
*		2. COPY TO FLASH
*
* Spokes have a different situation.  They do not copy the SRAM to FLASH
* so they can have four different situations for internal reporting:
*		1. OFF
*		2. COPY TO FLASH ONLY
*		3. COPY TO SRAM ONLY
*		4. COPY TO FLASH AND SRAM
*
*****************************************************************************/







#include "DIAG.h"			//Diagnostic package

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e752 */		/* local declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


//#include <msp430x54x.h>		//processor reg description */
#include "STD.h"			//standard defines
//#include "CONFIG.h" 		//system configuration description file
//#include "MAIN.H"			//
//#include "MISC.H"			//homeless functions
//#include "READING.h"		//sensor reading module
//#include "AD.h" 			//AD module
#include "CRC.h"			//CRC calculation module
#include "L2SRAM.h"  		//disk storage module
//#include "SERIAL.h" 		//serial IO port stuff
#include "L2FRAM.h" 		//level 2 fram routines
//#include "SRAM.H"			//static ram routines
//#include "REPORT.h" 		//event REPORT module
//#include "SDCTL.h"  		//SD board control routines
//#include "SDCOM.h" 		//SD communication package
#include "L2FLASH.h"		//level 2 flash routines
#include "TIME.h"			//Time routines
//#include "ACTION.h" 		//Event action routines
#include "REPORT.h"			//report generator routines
#include "MSG.h"			//msg handling routines
#include "OPMODE.h"			//operational mode msg routines
//#include "RTS.h"			//scheduler code
#include "MODOPT.h"			//Modify Options routines



extern volatile uchar ucaMSG_BUFF[ MAX_RESERVED_MSG_SIZE ];
//extern uchar ucGLOB_CurMsgSeqNum;



/*****************************  CODE STARTS HERE  ****************************/



/************************  vREPORT_logReport() *******************************
*
* Stuffs a report msg to FLASH & SRAM
*
******************************************************************************/

void vREPORT_logReport(
		uchar ucReportRadioIdxPair,		//Radio OptionPairIdx
		uchar ucReportFlashIdxPair		//Flash OptionPairIdx
		)
	{

	if(ucMODOPT_readSingleRamOptionBit(ucReportRadioIdxPair))
		vL2SRAM_storeMsgToSramIfAllowed();	//store report to SRAM

	if(ucMODOPT_readSingleRamOptionBit(ucReportFlashIdxPair))
		vL2FLASH_storeMsgToFlash();			//Store report to Flash

	return;

	}/* END: vREPORT_logReport() */




/******************  vREPORT_buildReportAndLogIt() ***************************
*
* Build the report message and log it to flash
*
******************************************************************************/

void vREPORT_buildReportAndLogIt(
		uchar ucSensorNum0,				//Sensor num  0
		uint uiReportNum0,				//Sensor Data 0
		uchar ucSensorNum1,				//Sensor num  1
		uint uiReportNum1,				//Sensor Data 1
		uchar ucReportRadioIdxPair,		//Radio OptionPairIdx
		uchar ucReportFlashIdxPair		//Flash OptionPairIdx
		)
	{
	uchar ucCount;

	/* COMPUTE THE ENTRY COUNT FOR THE MSG*/
	ucCount = 1;						//assume only 1 msg entry
	if(ucSensorNum1 != 0) ucCount++;	//wups it was a two msg entry

	/* BUILD THE MESSAGE */
 	vOPMODE_buildMsg_OM2(
			0,							//LAST_PKT_BIT & NO_DATA_PKT_BIT
			0,							//dest SN
			ucMSG_incMsgSeqNum(),		//msg seq num
			0,							//Load Factor
			uiL2FRAM_getSnumLo16AsUint(),//Agent SN
			lTIME_getSysTimeAsLong(),	//Collection Time
			ucCount,					//Data count
			ucSensorNum0,				//Sensor 0
			uiReportNum0,				//Sensor 0 data
			ucSensorNum1,				//Sensor 1
			uiReportNum1,				//Sensor 1 data
			0,							//Sensor 2
			0,							//Sensor 2 data
			0,							//Sensor 3
			0							//Sensor 3 data
			);

	/* GO DO THE REPORT */
	vREPORT_logReport(	ucReportRadioIdxPair,
						ucReportFlashIdxPair
						 );

	return;

	}/* END: vREPORT_buildReportAndLogIt() */





/******************  vREPORT_buildEmptyReportHull() **************************
*
* Build only the envelope portion of the OM2 (IE. no data)
*
******************************************************************************/

void vREPORT_buildEmptyReportHull(
		void
		)
	{
	/* BUILD THE MESSAGE */
 	vOPMODE_buildMsg_OM2(
				0,							//LAST_PKT_BIT & NO_DATA_PKT_BIT
				0,							//dest SN
				ucMSG_incMsgSeqNum(),		//msg seq num
				0,							//Load Factor
				uiL2FRAM_getSnumLo16AsUint(),//Agent SN (Thats us)
				lTIME_getSysTimeAsLong(),	//Collection Time
				0,							//Data count
				0,							//Sensor 0
				0,							//Sensor 0 data
				0,							//Sensor 1
				0,							//Sensor 1 data
				0,							//Sensor 2
				0,							//Sensor 2 data
				0,							//Sensor 3
				0							//Sensor 3 data
				);

	return;

	}/* END: vREPORT_buildEmptyReportHull() */








/***************  ucREPORT_addSensorDataToExistingReport() *******************
*
* Routine to add additional sensor data to a report that is already in the 
* msg buffer.
*
* NOTE: this routine does not send the report it mearly add an entry
*
*
* RET:	1 - New Sensor Data was added to report
*		0 - Not enough room to add sensor data to report
*
******************************************************************************/

uchar ucREPORT_addSensorDataToExistingReport(
		uchar ucSensor0,			//Sensor num  0
		uint uiData0,				//Sensor Data 0
		uchar ucSensor1,			//Sensor num  1 (0 if not to be added)
		uint uiData1				//Sensor Data 1
		)
	{
	uchar ucPairDesired;
	uchar ucEndOfMsgByte;
	uchar ucPairSt;
	uchar ucPairAvail;


	/* FIND OUT HOW MANY DATUMS WE WANT TO ADD */
	ucPairDesired = 1;						//assume we're inserting 1 new pair
	if(ucSensor0 == 0) return(1);			//if none we return 'done'
	if(ucSensor1 != 0) ucPairDesired++;		//Looks like 2 pair

	/* GET THE CURRENT SIZE OF THE MSG */
	ucEndOfMsgByte = ucaMSG_BUFF[OM2_IDX_EOM_IDX] & MAX_MSG_SIZE_MASK;

	/* THE FOLLOWING SWITCH IS REALLY JUST A SIZE FILTER FOR ILLEGAL SIZES */
	switch(ucEndOfMsgByte)
		{
		case 17:				//4 Pair left, No Pair in use
		case 20:				//3 Pair left, 1 Pair in use
		case 23:	   			//2 Pair left, 2 Pair in use
		case 26:				//1 Pair left, 3 Pair in use
			break;

		case 29:
		case 31:
		default:
			return(0);			//not enough room or bad size

		}/* END: switch(ucEndOfMsgByte) */

	/* CHECK IF WE HAVE ENOUGH ROOM TO DO THE INSERT */
	ucPairAvail = (29 - ucEndOfMsgByte)/3;
	if(ucPairDesired > ucPairAvail) return(0);	//not enough room

	/* GET THE START FOR THE DATA -- AND STUFF THE DATA */
	ucPairSt = ucEndOfMsgByte -1;
	vOPMODE_stuffDataPairToOM2(ucSensor0, uiData0, ucPairSt);

	/* STUFF A 2ND DATA */
	if(ucPairDesired == 2)
		{
		ucPairSt += 3;
		vOPMODE_stuffDataPairToOM2(ucSensor1, uiData1, ucPairSt);
		}

	/* STUFF THE NEW SIZE */
	ucaMSG_BUFF[OM2_IDX_EOM_IDX] &= 0xC0;		//	1100 0000
	ucaMSG_BUFF[OM2_IDX_EOM_IDX] |= (ucPairSt + 4);

	/* COMPUTE THE CRC */
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534

	return(1);

	}/* END: ucREPORT_addSensorDataToExistingReport() */






/*********************  vREPORT_doRegularReport()  ***************************
*
* This was supposed to be an action item routine that reported on a regular
* basis.  The overhead of this routine is currently considered to be too high
* a cost in radio use and flash storage to be feasible.
*
******************************************************************************/

void vREPORT_doRegularReport(
		void
		)
	{
	return;
	}




/*-------------------------------  MODULE END  ------------------------------*/

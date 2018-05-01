

/**************************  ACTION.C  *****************************************
*
* Routines to perform actions during events
*
*
* V1.00 10/04/2003 wzr
*		Started
*
******************************************************************************/


#include "std.h"			//standard defines
//#include "diag.h"			//diagnostic defines
#include "config.h"			//system configuration description file
//#include "main.h"			//
#include "misc.h"			//homeless functions
//#include "reading.h"		//sensor reading module
//#include "ad.h"  			//AD module
//#include "crc.h"			//CRC calculation module
#include "serial.h"			//serial IO port stuff
#include "L2fram.h"			//level 2 fram routines
//#include "sram.h"			//sram routines
#include "l2sram.h"			//level 2 sram
#include "action.h" 		//event action module
//#include "sdctl.h"		//SD board control routines
#include "sdcom.h"			//SD communication package
#include "l2flash.h"		//level 2 flash routines
#include "time.h"			//Time routines
#include "msg.h"			//msg handling routines
#include "rts.h"			//Real Time Scheduler
#include "sysact.h"			//Event handler routines
#include "discover.h"		//discovery routines
#include "opmode.h"			//operational mode routines
#include "sdspi.h"			//SD board SPI routines
#include "report.h"			//Periodic reporting functions
//#include "delay.h"  		//wait routines
#include "sensor.h"			//sensor list names
#include "senseact.h"		//sense act
#include "pick.h"			//Dynamic reporting functions
#include "MODOPT.h" 		//Modify Options routines



/*******************  EXTERNS  ***********************************************/


extern volatile uchar ucaMSG_BUFF[ MAX_RESERVED_MSG_SIZE ];

extern volatile uchar ucaSDSPI_BUFF[ MAX_SDSPI_SIZE ];

//extern uchar ucGLOB_curMsgSeqNum;

extern uchar ucGLOB_lastAwakeStblIdx;	//Nearest thing to cur sched idx 

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



/*******************  FUNCTION DECLARATIONS  *********************************/

void vACTION_do_test_msg(void);
void vACTION_get_fake_SD_data(void);
void vACTION_do_Batt_Sense(void);

static void vACTION_read_sensors_and_store_OM2(void);


/* DECLARE A FUNCTION TYPE FIRST */
typedef void (*GENERIC_EVENT_FUNC)(void);



const GENERIC_EVENT_FUNC vEventPtrArray[E_ACTN_MAX_NUM_IDX] =
 {
 					/* INDEXED BY ACTION NUM */
 vSYSACT_do_sleep,							// 0 - none
 vACTION_do_test_msg,						// 1 - do test msg
 vACTION_get_fake_SD_data,					// 2 - send fake SD data
 vACTION_read_sensors_and_store_OM2,		// 3 - do roll your own sensor routine
 vACTION_do_Batt_Sense,						// 4 - check the batt voltage
 vACTION_read_sensors_and_store_OM2,		// 5 - read TC1 & TC2
 vACTION_read_sensors_and_store_OM2,		// 6 - read TC3 & TC4
 vACTION_read_sensors_and_store_OM2,		// 7 - read LIGHT 1 & 2
 vACTION_read_sensors_and_store_OM2,		// 8 - read SOIL moisture 1 & 2
 vRTS_runScheduler,							// 9 - Evalulor and scheduler for next NST
 vOPMODE_SOM2,								//10 - inbound of OM2 msgs
 vOPMODE_ROM2,								//11 - Receive OM2 msgs
 vSYSACT_do_move_SRAM_to_FLASH,				//12 - copy SRAM msgs to FLASH in Hub
 vDISCOVER_SDC4,							//13 - send discovery DC4 packets
 vDISCOVER_RDC4,							//14 - rec discovery DC4 packets
 vREPORT_doRegularReport,					//15 - periodic reporting function
 vACTION_read_sensors_and_store_OM2,		//16 - read LIGHT 3 & 4
 vACTION_read_sensors_and_store_OM2,		//17 - read LIGHT 1,2,3,4
 vACTION_read_sensors_and_store_OM2,		//18 - read LIGHT 1,1,1,1
 vACTION_read_sensors_and_store_OM2,		//19 - Vaisala ave wind sp & dir
 vACTION_read_sensors_and_store_OM2,		//20 - Vaisala air press & temp
 vACTION_read_sensors_and_store_OM2,		//21 - Vaisala rel humid
 vACTION_read_sensors_and_store_OM2,		//22 - read SAP1 & SAP2
 vACTION_read_sensors_and_store_OM2			//23 - read SAP3 & SAP4
 };








/* THIS ROM TBL IS INDEXED BY ACTION NUM & CONTAINS THE NAME OF EACH ACTION */
const char *cpaActionName[E_ACTN_MAX_NUM_IDX] = 
	{
	/* INDEXED BY ACTION NUM */
	"---",			// actn  0 sleep
	"TMg",			// actn  1 Test msg
	"FkM",			// actn  2 Fake SD msg
	"Cus",			// actn  3 Roll your own sensor routine
	"Bat",			// actn  4 Sample the batt voltage
	"T12",			// actn  5 TC_12
	"T34",			// actn  6 TC_34
	"L12",			// actn  7 LT_12 take light 1 & 2 samples
	"S12",			// actn  8 SL_12 take soil 1 & 2 samples
	"Sch",			// actn  9 run the scheduler
	"SM2",			// actn 10 send om2 packets
	"RM2",			// actn 11 Receive om2 packets
	"Fsh",			// actn 12 Move SRAM to FLASH
	"SD4",			// actn 13 send    DC4 msg
	"RD4",			// actn 14 receive DC4 msg
	"Rpt",			// actn 15 periodic reporting
	"L34",			// actn 16 (Duke) Take light 3 & 4 samples
	"L14",			// actn 17 Light 1,2,3,4 samples
	"L11",			// actn 18 Light 1,1,1,1 samples
	"VWD",			// actn 19 Vaisala wind speed & dir
	"VPT",			// actn 20 Vaisala air press & temp
	"VHR",			// actn 21 Vaisala rel humid
	"SF1",			// actn 22 Sap Flow 1 & 2
	"SF3"			// actn 23 Sap Flow 3 & 4
	}; /* END: cpaActionName[] */



/*****************************  CODE STARTS HERE  ****************************/



/************************  vACTION_dispatch_to_event() ********************************
*
* Dispatch to the event specified.
*
*
******************************************************************************/

void vACTION_dispatch_to_event(
		uchar ucNSTtblNum,		//table num (0 or 1)
		uchar ucNSTslotNum		//slot num in table
		)
	{
	uchar ucActionNum;


	/* GET THE SCHED TBL IDX OF THIS DISPATCH */
	ucGLOB_lastAwakeStblIdx = ucRTS_getNSTentry(ucNSTtblNum, ucNSTslotNum);

	/* GET THE ACTION NUMBER FOR THIS DISPATCH */
	ucActionNum = (uchar)ulL2SRAM_getStblEntry(
				   		SCHED_ACTN_TBL_NUM,			//Tbl num
				   		ucGLOB_lastAwakeStblIdx	//Tbl Idx
				   		);

	/* SHOW THE ACTION HEADER LINE */
	vRTS_showActionHdrLine(YES_CRLF);

	/* DISPATCH TO THE ROUTINE */
	vEventPtrArray[ucActionNum]();

	return;

	}/* END: vACTION_dispatch_to_event() */




/***************** vACTION_showActionName()  *********************************
*
*
*
******************************************************************************/
void vACTION_showActionName(
		uchar ucActionNum
		)
	{
	vSERIAL_rom_sout(cpaActionName[ucActionNum]);
	return;

	}/* END: vACTION_showActionName() */





/***************** vACTION_showStblActionName()  ****************************
*
*
*
******************************************************************************/

void vACTION_showStblActionName(
		uchar ucStblIdx				//takes an Stbl index
		)
	{
	uchar ucActionNum;

	if(ucStblIdx >= GENERIC_SCHED_MAX_IDX)
		{
		vSERIAL_showXXXmsg();
		return;
		}

	ucActionNum = (uchar)ulL2SRAM_getStblEntry(SCHED_ACTN_TBL_NUM, ucStblIdx);
	vACTION_showActionName(ucActionNum);

	return;

	}/* END: vACTION_showStblActionName() */






/******************  vACTION_read_sensors_into_buffer() **********************
*
* Reads sensors from 1 up to 4 and builds a message containing the results
* then stashes it in the the SRAM for radio transmission and FLASH for backup
*
******************************************************************************/

void vACTION_read_sensors_into_buffer(//first 0 sensor num ends read
		uchar ucSensorID[4]	//Sensors [0]=first, [3]=last
		)
	{
	uchar ucc;
	uint uiSensorReading[OM2_MAX_DATA_ENTRY_COUNT];
	uchar ucSampleCount;
	uchar ucCurSensorID;

	/* ZRO SENSOR READINGS */
	for(ucc=0;  ucc<4;  ucc++) { uiSensorReading[ucc] = 0; }

	/* READ ALL THE REQUIRED SENSORS */
	ucSampleCount = 1;
	for(ucc=0;  ucc<4;  ucc++)
		{
		ucCurSensorID = ucSensorID[ucc];

		/* IF ZRO SENSOR_NUM END THE READ */
		if(ucCurSensorID == 0) break;

		/* REQUEST THE SENSOR DATA */
		if(ucSDCOM_awaken_SD_and_do_full_msg_transfer(ucCurSensorID) == 0)
 			{
			vSERIAL_rom_sout("ACTN:1:SDmsgErrOn ");
			vSENSOR_showSensorName(ucCurSensorID, L_JUSTIFY);
			vSERIAL_crlf();
			}

		/* SAVE RESULTS IN LOCAL ARRAY */
		uiSensorReading[ucc] = uiMISC_buildUintFromBytes(
								(uchar *)&ucaSDSPI_BUFF[3],
								NO_NOINT
								);

		/* SAVE A SECOND WORD IF THIS IS A DUAL DATA SENSOR */
		if((ucc != 3) && (ucSENSEACT_getSensorDataSize(ucCurSensorID) == 2))
			{
			ucc++;
			uiSensorReading[ucc] = uiMISC_buildUintFromBytes(
								(uchar *)&ucaSDSPI_BUFF[5],
								NO_NOINT
								);
			}

		ucSampleCount = ucc+1;

		}/* END: for(ucc) */

	#if 0
	vSERIAL_rom_sout("SnsrCnt=");
	vSERIAL_UIV8out(ucSampleCount);
	vSERIAL_crlf();
	#endif

	/* BUILD THE OM2 MESSAGE */
  	vOPMODE_buildMsg_OM2(
				0,									//LAST_PKT_BIT & NO_DATA_PKT_BIT
				0,									//dest SN
				ucMSG_incMsgSeqNum(),				//msg seq num
				0,									//Load Factor
				uiL2FRAM_getSnumLo16AsUint(),		//Agent SN
				lTIME_getSysTimeAsLong(),			//Collection Time
				ucSampleCount,						//Filled sensor count
				ucSensorID[0],uiSensorReading[0],	//Sensor data 1
				ucSensorID[1],uiSensorReading[1],	//Sensor data 2
				ucSensorID[2],uiSensorReading[2],	//Sensor data 3
				ucSensorID[3],uiSensorReading[3]	//Sensor data 4
				);

	#if 0
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);
	#endif

	return;

	}/* END: vACTION_read_sensors_into_buffer() */







/****************  vACTION_read_sensors_and_store_OM2() **********************
*
* Reads sensors and stores the resulting OM2 in memory
*
******************************************************************************/

static void vACTION_read_sensors_and_store_OM2(
		void
		)
	{
	uchar ucSensorID[4];
	ulong ulVal;
	uchar ucii;


	/* FILL THE SENSOR ID array */
	ulVal = ulL2SRAM_getStblEntry(SCHED_SENSE_ACT_TBL_NUM,ucGLOB_lastAwakeStblIdx);
	if(ulVal == 0UL) return;			//if no sensor numbers are listed leave

	/* MOVE THE SENSOR ID TO INDIVIDUAL ARRAY LOCATIONS */
	for(ucii=3;  ucii<4;  ucii--)
		{
		ucSensorID[ucii] = (uchar)ulVal;
		ulVal >>= 8;
		}

	#if 0
	vSERIAL_rom_sout("SnsrLneup = ");
	for(ucii=0; ucii<4;  ucii++)
		{
		vSENSEACT_showSensorShortName(ucSensorID[ucii]);
		if(ucii != 3) vSERIAL_bout(',');
		}
	vSERIAL_crlf();
	#endif

	/* GET THE SENSOR OM2 INTO THE MSG BUFFER */
	vACTION_read_sensors_into_buffer(ucSensorID);

	/*--------------- BEG: 31 July 2007 edit -------------------------------*/
	/* Dynamic Reporting of Sensor Data
	 * Activated 31 July 2007 by Kenji Yamamoto */

	/* CHK IF THE DATA IS DIFFERENT THAN PREV MEASURMENT */
	if(ucPICK_putTriggeredDataIntoSSP())
		{

		/* STORE MSG TO SRAM STORE & FORWARD FOR RADIO */
		vL2SRAM_storeMsgToSramIfAllowed();

		#if 1
		/* REPORT TO CONSOLE NEW DATA HAS BEEN FOUND & SENT TO SRAM */
		vSERIAL_rom_sout("ACT:NwDatPt\r\n");
		#endif

		}/* END: if() */


	/* STUFF A COPY OF MSG TO FLASH, TO ALWAYS SAVE DATA */
	vL2FLASH_storeMsgToFlash();

	/*------------------ END: 31 July 2007 edit -----------------------------*/

	#if 1
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);
	#endif

	return;

	}/* END: vACTION_read_sensors_and_store_OM2() */







/************************  vACTION_do_test_msg() *****************************
*
* Build a message containing containing fake data.
*
*
******************************************************************************/

void vACTION_do_test_msg(
		void
		)

	{
	if(!ucL2FRAM_isSender()) return;	//no msg if not sending

	/* BUILD THE MESSGE */
	vMSG_stuffFakeMsgToSRAM();

	/* CHECK THE MESSAGE */
	vMSG_checkFakeMsg();

	#if 1
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);
	#endif

	#if 1
	vSERIAL_rom_sout("EndEvnt\r\n");
	#endif

	return;

	}/* END: vACTION_do_test_msg() */








/************************  vACTION_get_fake_SD_data() *************************
*
* Record fake data and stuff the data as a msg
*
******************************************************************************/

void vACTION_get_fake_SD_data(
		void
		)

	{

	if(!ucL2FRAM_isSampler()) return;		//non-samplers do not do this

	#if 1
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);
	#endif

	#if 1
	vSERIAL_rom_sout("EndEvnt\r\n");
	#endif

	return;

	}/* END: vACTION_get_fake_SD_data() */







/*******************  vACTION_showBattReading()  *****************************
*
*
*
*****************************************************************************/

void vACTION_showBattReading(
		void
		)
	{
	uint uiBattReading_in_mV;
	uint uiVolts_whole;
	uint uiVolts_fraction;

	uiBattReading_in_mV = uiMISC_doCompensatedBattRead();

	uiVolts_whole = uiBattReading_in_mV / 1000;
	vSERIAL_UIV16out(uiVolts_whole);
	vSERIAL_bout('.');

	uiVolts_fraction = uiBattReading_in_mV - (uiVolts_whole * 1000);
	vSERIAL_UIV16out(uiVolts_fraction);
	vSERIAL_rom_sout("V");

	return;

	}/* END: vACTION_showBattReading() */








/************************  vACTION_do_Batt_Sense() ***************************
*
* Perform a battery sensing operation.
*
******************************************************************************/

void vACTION_do_Batt_Sense(
		void
		)

	{
	uint uiBattReading_in_mV;

	/* GET THE BATTERY READING */
	uiBattReading_in_mV = uiMISC_doCompensatedBattRead(); // below 2.8V is reset

	#if 1
	vSERIAL_rom_sout("  Batt= ");
	vACTION_showBattReading();
	vSERIAL_crlf();
	#endif

	/* RESTART SYSTEM IF BATTERY IS BAD */
	if(uiBattReading_in_mV < 2800)
		{
		vSERIAL_rom_sout("BattBdRstrt\r\n");

		/* REPORT BATTERY DEATH */
		vREPORT_buildReportAndLogIt(
			SENSOR_BR_BOARD_MSG,				//Sensor 0 num
			BATTERY_LOW_RESTART,				//Sensor 0 data
			0,									//Sensor 1 num
			0,									//Sensor 1 data
			OPTPAIR_RPT_BATT_DEAD_RSTRT_TO_RDIO,//Radio OptionPairIdx
			OPTPAIR_RPT_BATT_DEAD_RSTRT_TO_FLSH	//Flash OptionPairIdx
			);

		vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);

		}

	/* REPORT BATTERY VOLTAGE */
	vREPORT_buildReportAndLogIt(
			SENSOR_BATT_VOLTAGE,				//Sensor 0 num
			uiBattReading_in_mV,				//Sensor 0 data
			0,									//Sensor 1 num
			0,									//Sensor 1 data
			OPTPAIR_RPT_BATT_VOLTS_TO_RDIO,		//Radio OptionPairIdx
			OPTPAIR_RPT_BATT_VOLTS_TO_FLSH		//Flash OptionPairIdx
			);

	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);

	return;

	}/* END: vACTION_do_Batt_Sense() */






/*-------------------------------  MODULE END  ------------------------------*/

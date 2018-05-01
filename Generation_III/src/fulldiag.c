

/**************************  FULLDIAG.C  *************************************
*
* Routines to fully diagnose the system
*
*
* V1.00	01/19.2005 wzr
*		started
*
******************************************************************************/

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e716 */		/* while(1) ... */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */



#include "std.h"			//standard defines
//#include "diag.h"			//diagnostic header
#include "config.h" 		//configuration parameters 
#include "serial.h"			//serial port routines
#include <msp430x54x.h>		//processor reg description
//#include "main.h"			//
#include "misc.h"			//homeless functions
#include "fram.h"			//Ferro ram memory functions
#include "l2fram.h"			//Level 2 FRAM routines
#include "fulldiag.h"		//full system diagnostic functions
//#include "l2flash.h"		//level 2 flash routines
#include "delay.h"			//delay timer routines
#include "buz.h"			//Buzzer routines
#include "thermo.h"			//Thermocouple computation routines
#include "sdcom.h"			//Level 2 SD board control
//#include "sdctl.h"			//SD board control routines
#include "radio.h"			//radio control routines
#include "dradio.h"			//Diag Radio routines
#include "msg.h"			//msg helper routines
#include "sdspi.h"			//SD board SPI handler
#include "action.h"			//Action sampler routines
#include "sensor.h"			//Sensor name routine
//#include "MODOPT.h" 		//Modify Options routines


extern volatile uchar ucaMSG_BUFF[ MAX_RESERVED_MSG_SIZE ];

extern volatile uchar ucaSDSPI_BUFF[ MAX_SDSPI_SIZE ];

extern volatile union						//ucGLOB_diagFlagByte1
  {
	uchar byte;
	struct
	 {
	 unsigned DIAG_mute_bit:1;					//bit 0 ;1=MUTE, 0=SOUND
	 unsigned DIAG_test_repeat_looping_bit:1;	//bit 1 ;1=LOOPING, 0=NOT LOOPING
	 unsigned DIAG_only_a_single_test_bit:1;	//bit 2	;1=ONLY A SINGLE, 0=ALL
	 unsigned DIAG_only_a_single_group_bit:1;	//bit 3 ;1=ONLY A GROUP, 0=ALL
	 unsigned DIAG_exit_out_to_top_bit:1;		//bit 4 ;1=EXIT OUT, 0=NO EXIT OUT
	 unsigned DIAG_exit_out_to_prev_test_bit:1;	//bit 5 ;1=GOTO PREV, 0=NO GOTO PREV
	 unsigned DIAG_exit_out_to_next_test_bit:1;	//bit 6 ;1=GOTO NEXT, 0=NO GOTO NEXT
	 unsigned DIAG_quit_out_to_return_bit:1;	//bit 7 ;1=TOTO RETURN, 0=NO QUIT
	 }diagFlagByte1_STRUCT;

  }ucGLOB_diagFlagByte1;

#define DIAG_FLAG_BYTE_1_INIT_VAL 0x00			// 0b00000000



extern volatile union						//ucGLOB_diagFlagByte2
  {
	uchar byte;
	struct
	 {
	 unsigned DIAG_tell_whats_running_bit:1;		//bit 0 ;1=TELL, 0=NO TELL
	 unsigned DIAG_change_looping_sts_bit:1;		//bit 1 ;1=CHANGE LOOPING, 0=NO CHANGE
	 unsigned DIAG_halt_on_error_bit:1;				//bit 2 ;1=halt, 0=no halt
	 unsigned DIAG_partially_automated_run_bit:1;	//bit 3	;1=an 'A' was hit, 0=no 'A' hit
	 unsigned DIAG_not_used_4_bit:1;				//bit 4 ;
	 unsigned DIAG_not_used_5_bit:1;				//bit 5 ;
	 unsigned DIAG_not_used_6_bit:1;				//bit 6 ;
	 unsigned DIAG_not_used_7_bit:1;				//bit 7 ;
	 }diagFlagByte2_STRUCT;

  }ucGLOB_diagFlagByte2;

#define DIAG_FLAG_BYTE_2_INIT_VAL 0x00				// 0b00000000


const char *AutoTestExitStr = "AutoTstXit\r\n";

/* DECLARATIONS */
uchar ucFULLDIAG_exit_next_repeat(
		void
		);






/* DECLARE A FUNCTION TYPE FIRST */
typedef uchar (*GENERIC_DIAG_FUNC)(void);


/************************  FRAM tables here  *********************************/

#define FRAM_DIAG_ARRAY_SIZE 3

#define FRAM_TEST_0_REPEAT_COUNT 10
#define FRAM_TEST_1_REPEAT_COUNT 10
#define FRAM_TEST_2_REPEAT_COUNT 10


/* THIS IS A PARALLEL ARRAY TO THE FRAM ARRAY */
const char *cpFRAM_FuncDesc[FRAM_DIAG_ARRAY_SIZE] =
 {
 "TstLoLevelFramCalls",
 "TstLevel2FramCalls",
 "TstToCatch1stByteBadProblm"
 };



/* FUNCTION DECLARATIONS HERE */
static uchar ucFULLDIAG_FRAM_0(void);
static uchar ucFULLDIAG_FRAM_1(void);
static uchar ucFULLDIAG_FRAM_2(void);


const GENERIC_DIAG_FUNC ucFRAM_DIAG_PtrArray[FRAM_DIAG_ARRAY_SIZE] =
 {
 ucFULLDIAG_FRAM_0,
 ucFULLDIAG_FRAM_1,
 ucFULLDIAG_FRAM_2
 };


/* PARALLEL ARRAY CONTAINING THE REPEAT LOOP COUNTS FOR EACH TEST */
const uint uiaFRAM_testRepeatCountArray[FRAM_DIAG_ARRAY_SIZE] =
 {
 FRAM_TEST_0_REPEAT_COUNT,
 FRAM_TEST_1_REPEAT_COUNT,
 FRAM_TEST_2_REPEAT_COUNT
 };



/**********************  FLASH tables here  **********************************/

#define FLASH_DIAG_ARRAY_SIZE 1

#define FLASH_TEST_0_REPEAT_COUNT 10
#define FLASH_TEST_1_REPEAT_COUNT 10
#define FLASH_TEST_2_REPEAT_COUNT 10


/* THIS IS A PARALLEL ARRAY TO THE FLASH ARRAY */
const char *cpFLASH_FuncDesc[FLASH_DIAG_ARRAY_SIZE] =
 {
 "NIY"
 };


/* FUNCTION DECLARATIONS HERE */
static uchar ucFULLDIAG_FLASH_0(void);


const GENERIC_DIAG_FUNC ucFLASH_DIAG_PtrArray[FLASH_DIAG_ARRAY_SIZE] =
 {
 ucFULLDIAG_FLASH_0
 };


/* PARALLEL ARRAY CONTAINING THE REPEAT LOOP COUNTS FOR EACH TEST */
const uint uiaFLASH_testRepeatCountArray[FLASH_DIAG_ARRAY_SIZE] =
 {
 FLASH_TEST_0_REPEAT_COUNT
 };



/*********************  SDBOARD tables here  *********************************/

#define SDBOARD_DIAG_ARRAY_SIZE 2

#define SDBOARD_TEST_0_REPEAT_COUNT 10
#define SDBOARD_TEST_1_REPEAT_COUNT 10
#define SDBOARD_TEST_2_REPEAT_COUNT 10


/* THIS IS A PARALLEL ARRAY TO THE SDBOARD ARRAY */
const char *cpSDBOARD_FuncDesc[SDBOARD_DIAG_ARRAY_SIZE] =
 {
 "BrkOutOfSleepBitFollowProg",
 "ChkIfSdBbdVernumXmitsOK",
 };


/* FUNCTION DECLARATIONS HERE */
static uchar ucFULLDIAG_SDBOARD_0(void);
static uchar ucFULLDIAG_SDBOARD_1(void);


const GENERIC_DIAG_FUNC ucSDBOARD_DIAG_PtrArray[SDBOARD_DIAG_ARRAY_SIZE] =
 {
 ucFULLDIAG_SDBOARD_0,
 ucFULLDIAG_SDBOARD_1
 };


/* PARALLEL ARRAY CONTAINING THE REPEAT LOOP COUNTS FOR EACH TEST */
const uint uiaSDBOARD_testRepeatCountArray[SDBOARD_DIAG_ARRAY_SIZE] =
 {
 SDBOARD_TEST_0_REPEAT_COUNT,
 SDBOARD_TEST_1_REPEAT_COUNT
 };





/************************  RADIO tables here  *********************************/

#define RADIO_DIAG_ARRAY_SIZE 4

#define RADIO_TEST_0_REPEAT_COUNT 10
#define RADIO_TEST_1_REPEAT_COUNT 10
#define RADIO_TEST_2_REPEAT_COUNT 10
#define RADIO_TEST_3_REPEAT_COUNT 10


/* THIS IS A PARALLEL ARRAY TO THE RADIO ARRAY */
const char *cpRADIO_FuncDesc[RADIO_DIAG_ARRAY_SIZE] =
 {
 "TstRdioXmitPwr",
 "TstRdioPath",
 "ChkSndRecSwtchDIODE",
 "Sniffer"
 };


/* FUNCTION DECLARATIONS HERE */
static uchar ucFULLDIAG_RADIO_0(void);
static uchar ucFULLDIAG_RADIO_1(void);
static uchar ucFULLDIAG_RADIO_2(void);
static uchar ucFULLDIAG_RADIO_3(void);


const GENERIC_DIAG_FUNC ucRADIO_DIAG_PtrArray[RADIO_DIAG_ARRAY_SIZE] =
 {
 ucFULLDIAG_RADIO_0,
 ucFULLDIAG_RADIO_1,
 ucFULLDIAG_RADIO_2,
 ucFULLDIAG_RADIO_3
 };


/* PARALLEL ARRAY CONTAINING THE REPEAT LOOP COUNTS FOR EACH TEST */
const uint uiaRADIO_testRepeatCountArray[RADIO_DIAG_ARRAY_SIZE] =
 {
 RADIO_TEST_0_REPEAT_COUNT,
 RADIO_TEST_1_REPEAT_COUNT,
 RADIO_TEST_2_REPEAT_COUNT,
 RADIO_TEST_3_REPEAT_COUNT
 };








/********************* SENSOR DIAGNOSTIC TABLES HERE  ***********************/

#define SENSOR_DIAG_ARRAY_SIZE 6

#define SENSOR_TEST_0_REPEAT_COUNT 10
#define SENSOR_TEST_1_REPEAT_COUNT 10
#define SENSOR_TEST_2_REPEAT_COUNT 10
#define SENSOR_TEST_3_REPEAT_COUNT 10
#define SENSOR_TEST_4_REPEAT_COUNT 10
#define SENSOR_TEST_5_REPEAT_COUNT 10


/* THIS IS A PARALLEL ARRAY TO THE SENSOR ARRAY */
const char *cpSENSOR_FuncDesc[SENSOR_DIAG_ARRAY_SIZE] =
 {
 "ReadLight",
 "ReadSoilMoists",
 "ReadThermos",
 "VaisalaAveWindSp&Dir",
 "VaisalaAirPress&AirTemp",
 "VaisalaRelHum&RainAccum"
 };


/* FUNCTION DECLARATIONS HERE */
static uchar ucFULLDIAG_SENSOR_0(void);
static uchar ucFULLDIAG_SENSOR_1(void);
static uchar ucFULLDIAG_SENSOR_2(void);
static uchar ucFULLDIAG_SENSOR_3(void);
static uchar ucFULLDIAG_SENSOR_4(void);
static uchar ucFULLDIAG_SENSOR_5(void);


/* ARRAY OF POINTERS TO THE DIAGNOSTIC FUNCTIONS */
const GENERIC_DIAG_FUNC ucSENSOR_DIAG_PtrArray[SENSOR_DIAG_ARRAY_SIZE] =
 {
 ucFULLDIAG_SENSOR_0,			//array of functions
 ucFULLDIAG_SENSOR_1,
 ucFULLDIAG_SENSOR_2,
 ucFULLDIAG_SENSOR_3,
 ucFULLDIAG_SENSOR_4,
 ucFULLDIAG_SENSOR_5
 };


/* PARALLEL ARRAY CONTAINING THE REPEAT LOOP COUNTS FOR EACH TEST */
const uint uiaSENSOR_testRepeatCountArray[SENSOR_DIAG_ARRAY_SIZE] =
 {
 SENSOR_TEST_0_REPEAT_COUNT,
 SENSOR_TEST_1_REPEAT_COUNT,
 SENSOR_TEST_2_REPEAT_COUNT,
 SENSOR_TEST_3_REPEAT_COUNT,
 SENSOR_TEST_4_REPEAT_COUNT,
 SENSOR_TEST_5_REPEAT_COUNT
 };







/************************  BUZ tables here  *********************************/

#define BUZ_DIAG_ARRAY_SIZE 1

#define BUZ_TEST_0_REPEAT_COUNT 1


/* THIS IS A PARALLEL ARRAY TO THE BUZ ARRAY */
const char *cpBUZ_FuncDesc[BUZ_DIAG_ARRAY_SIZE] =
 {
 "AllBuzSounds",
 };


/* FUNCTION DECLARATIONS HERE */
static uchar ucFULLDIAG_BUZ_0(void);


/* ARRAY OF POINTERS TO THE DIAGNOSTIC FUNCTIONS */
const GENERIC_DIAG_FUNC ucBUZ_DIAG_PtrArray[BUZ_DIAG_ARRAY_SIZE] =
 {
 ucFULLDIAG_BUZ_0,
 };


/* PARALLEL ARRAY CONTAINING THE REPEAT LOOP COUNTS FOR EACH TEST */
const uint uiaBUZ_testRepeatCountArray[BUZ_DIAG_ARRAY_SIZE] =
 {
 BUZ_TEST_0_REPEAT_COUNT
 };









/***************  TOP (GROUP) ARRAY STUFF HERE  ******************************/

#define NUMBER_OF_DIAG_GROUPS 6


const GENERIC_DIAG_FUNC *pfpGroupPtrArray[NUMBER_OF_DIAG_GROUPS] =
 {
 ucFRAM_DIAG_PtrArray,
 ucFLASH_DIAG_PtrArray,
 ucSDBOARD_DIAG_PtrArray,
 ucRADIO_DIAG_PtrArray,
 ucSENSOR_DIAG_PtrArray,
 ucBUZ_DIAG_PtrArray
 };


/* THIS IS A PARALLEL ARRAY to GroupPtrArray OF ARRAY SIZES */
const uchar ucaDiagCountPerGroup[NUMBER_OF_DIAG_GROUPS] =
 {
 FRAM_DIAG_ARRAY_SIZE,
 FLASH_DIAG_ARRAY_SIZE,
 SDBOARD_DIAG_ARRAY_SIZE,
 RADIO_DIAG_ARRAY_SIZE,
 SENSOR_DIAG_ARRAY_SIZE,
 BUZ_DIAG_ARRAY_SIZE
 };


/* THIS IS PARALLEL ARRY TO GroupPtrArray OF GROUP NAMES */
const char *cpaGroupName[NUMBER_OF_DIAG_GROUPS] =
 {
 "FramTest",
 "FlashTest",
 "SD-Test",
 "RadioTest",
 "SensorTest",
 "BuzTest"
 };


/* THIS IS A PARALLEL ARRAY TO GroupPtrArray of FUNCTION DESCRIPTORS */
const char **cppGroupsFuncDesc[NUMBER_OF_DIAG_GROUPS] =
 {
 cpFRAM_FuncDesc,
 cpFLASH_FuncDesc,
 cpSDBOARD_FuncDesc,
 cpRADIO_FuncDesc,
 cpSENSOR_FuncDesc,
 cpBUZ_FuncDesc
 };


/* PARALLEL ARRAY that tells what the repeat loop count is for each test */
const uint *uipaGROUPS_testRepCntPtr[NUMBER_OF_DIAG_GROUPS] =
  {
  uiaFRAM_testRepeatCountArray,
  uiaFLASH_testRepeatCountArray,
  uiaSDBOARD_testRepeatCountArray,
  uiaRADIO_testRepeatCountArray,
  uiaSENSOR_testRepeatCountArray,
  uiaBUZ_testRepeatCountArray
  };








#if 0

/********************  ucFULLDIAG_getSingleCharCmd()  ************************
*
* This routine accepts a sting of acceptable characters and
* returns the position in the string that matches.
*
* RET:	char position of matched char
*		if no match is found -- it does not exit
*
*****************************************************************************/

uchar ucFULLDIAG_getSingleCharCmd(
		const char *cpMsgOutStr,			//lint !e125
		const char *cpAcceptableCharSet	//lint !e125
		)
	{
	uchar ucChar;
	uchar ucMatchChar;
	uchar uci;
	uchar ucForBreakFlg;

	while(TRUE)		//lint !e774
		{
		/* SETUP, GIVE THE LEADING MSG, AND GET AN ANSWER */
		vSERIAL_rom_sout(cpMsgOutStr);
		ucChar = ucSERIAL_bin();	
		vSERIAL_bout(ucChar);					//echo the char

		/* SEARCH THE ACCEPTABLE STRING FOR A MATCH */
		ucForBreakFlg = 0;
		for(uci=0; ;  uci++)
			{
			ucMatchChar = (uchar)cpAcceptableCharSet[uci];

			if(ucMatchChar == 0) break;

			if(ucMatchChar == ucChar)
				{
				ucForBreakFlg = 1;
				break;
				}

			if((ucMatchChar >= 'a') && (ucMatchChar <= 'z'))
				{
				uchar ucRaisedMatchChar = ucMatchChar & ~0b00100000;
				if(ucRaisedMatchChar == ucChar)
					{
					ucForBreakFlg = 1;
					break;
					}
				}

			}/* END: for() */

		/* IF A MATCH WAS FOUND LEAVE */
		if(ucForBreakFlg == 1)
			{
			vSERIAL_crlf();
			break;
			}

		vSERIAL_showXXXmsg();

		}/* END: while(TRUE) */

	return(uci);

	}/* END: ucFULLDIAG_getSingleCharCmd() */

#endif






/********************  ucFULLDIAG_getChoiceOrAll()  ************************
*
* This routine accepts a number or the letter 'A' or 'X'.
*   returns the number if it was a number
*   returns the letter 'A' if it was an 'A'
*   returns the letter 'X' if it was an 'X' or 'Q'
*
* RET:	char position of matched char
*		if no match is found -- it does not exit
*
*****************************************************************************/

uchar ucFULLDIAG_getChoiceOrAll(
		const char *cpMsgOutStr,			//lint !e125
		uchar ucMaxNumPlusOne
		)
	{
	unsigned long ulValue;
	uchar ucStr[13];

	while(TRUE)		//lint !e774
		{
		/* SETUP, GIVE THE LEADING MSG, AND GET AN ANSWER */
		vSERIAL_rom_sout(cpMsgOutStr);

		/* GET THE STRING REPLY */
		if(ucSERIAL_getEditedInput(ucStr, 13))
			{
			vSERIAL_showXXXmsg();
			return('X');
			}

		/* NOW CHECK IF ITS A LEGAL NUMBER */
		if((ucStr[0] >= '0') && (ucStr[0] <= '9'))	//was it numeric?
			{
			/* IT WAS NUMERIC TEXT -- CONVERT IT TO A NUMBER */
			ulValue = (unsigned long)lSERIAL_AsciiToNum(ucStr, UNSIGNED, DECIMAL);
			/* RANGE CHECK THE NUMBER */
			if(ulValue < ucMaxNumPlusOne)
				{
				/* IT WAS A GOOD NUMBER  -- RETURN IT */
				return((uchar) ulValue);
				}
			}

		/* NOW CHECK IF THE STRING ENTERED WAS AN 'A' */
		if((ucStr[0] == 'a') || (ucStr[0] == 'A'))
			{
			return('A');
			}

		/* NOW CHECK IF THE STRING ENTERED WAS AN 'A' */
		if((ucStr[0] == 'x') || (ucStr[0] == 'X') ||
		   (ucStr[0] == 'q') || (ucStr[0] == 'Q'))
			{
			return('X');
			}


		/* IF WE GOT HERE THE ENTRY WAS INVALID */
		vSERIAL_showXXXmsg();

		}/* END: while(TRUE) */

	}/* END: ucFULLDIAG_getChoiceOrAll() */









/****************  ucFULLDIAG_doRunTimeKeyHitChk()  **************************
*
* RunTime Keys are:
*	 none - RET= 0  none 
*		  - RET= 1  no action key
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*
* RET:	0 if not key
*		n if an action key was hit
*			(flag byte was also set)
*
*****************************************************************************/

uchar ucFULLDIAG_doRunTimeKeyHitChk(
		void
		)
	{
	uchar ucChar;
	uchar ucRetVal;

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;	//assume none

	if(ucSERIAL_kbhit())
		{
		ucChar = ucSERIAL_bin();
		switch(ucChar)
			{
			case 'h':		//Toggle Halt on Error flag
			case 'H':
				if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_halt_on_error_bit)
					{
					ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_halt_on_error_bit = 0;
					vSERIAL_rom_sout("\r\nNoHaltOnErrs\r\n");
					}
				else
					{
					ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_halt_on_error_bit = 1;
					vSERIAL_rom_sout("\r\nHaltOnErrs\r\n");
					}

				ucRetVal = DIAG_ACTION_KEY_TOGGLE_HALT_ON_ERR;
				break;

			case 'l':		//toggle looping
			case 'L':
				ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_change_looping_sts_bit = 1;
				if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_test_repeat_looping_bit)
					vSERIAL_rom_sout("\r\nDiagIsUnlooped\r\n");
				else
					vSERIAL_rom_sout("\r\nDiagIsLooped\r\n");

				ucRetVal = DIAG_ACTION_KEY_TOGGLE_LOOP_FLAG;
				break;

			case 'm':		//toggle the mute bit
			case 'M':
				if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_mute_bit)
					{
					ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_mute_bit = 0;
					vSERIAL_rom_sout("\r\nBuzIsUnmute\r\n");
					}
				else
					{
					ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_mute_bit = 1;
					vSERIAL_rom_sout("\r\nBuzIsMute\r\n");
					}

				ucRetVal = DIAG_ACTION_KEY_TOGGLE_MUTE_FLAG;
				break;


			case 'n':		//go to next test
			case 'N':
				ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_next_test_bit = 1;
				vSERIAL_rom_sout("\r\nGoNxtTst\r\n");

				ucRetVal = DIAG_ACTION_KEY_NEXT_TEST_FLAG;
				break;

			case 'p':		//go to previous test
			case 'P':
				ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_prev_test_bit = 1;
				vSERIAL_rom_sout("\r\nGoPrevTst\r\n");

				ucRetVal = DIAG_ACTION_KEY_PREV_TEST_FLAG;
				break;

			case 'q':		//quit out of all tests and exit routine
			case 'Q':
				ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_quit_out_to_return_bit = 1;
				vSERIAL_rom_sout("\r\nQuitTsts\r\n");

				ucRetVal = DIAG_ACTION_KEY_QUIT;
				break;

			case 's':		//Stop running
			case 'S':
				vSERIAL_rom_sout("\r\nPrgmStop(S)toGo...");
				while(TRUE)	//lint !e774
					{
					ucChar = ucSERIAL_bin();
					if((ucChar == 's') || (ucChar == 'S')) break;
					}
				ucRetVal = DIAG_ACTION_KEY_PLAIN_KEY;
				break;

			case 't':		//Tell what this diag is
			case 'T':
				ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_tell_whats_running_bit = 1;
				ucRetVal = DIAG_ACTION_KEY_TELL;
				break;

			case 'x':		//exit out of test and start over 
			case 'X':
				ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_top_bit = 1;
				vSERIAL_rom_sout("\r\nXitToTopDiag\r\n");
				ucRetVal = DIAG_ACTION_KEY_EXIT;
				break;

			default:
				ucRetVal = DIAG_ACTION_KEY_PLAIN_KEY;
				break;

			}/* END: switch(ucChar) */

		}/* END: if() */

	return(ucRetVal);

	}/* END: ucFULLDIAG_doRunTimeKeyHitChk() */









/**********************  vFULLDIAG_run_module()  *****************************
*
* This is the full diagnostic main control routine.
*
*****************************************************************************/

void vFULLDIAG_run_module(
		void
		)
	{
	uchar ucChar;

	uchar ucGroupAnsIdx;
	uchar ucGroupIdx;

	uchar ucTestAnsIdx;
	uchar ucTestNumIdx;

	uchar ucc;

	uchar ucGroupLpStart;
	uchar ucGroupLpEnd;
	uchar ucGroupLpInc;
	uchar ucGidx;

	uchar ucTestLpStart;
	uchar ucTestLpEnd;
	uchar ucTestLpInc;
	uchar ucTidx;
	uchar ucTestRet;

	uint uiRepLpStart;
	uint uiRepLpEnd;
	uint uiRepLpInc;
	uint uiRidx;


	while(TRUE)	//lint !e716 !e774
		{
		/* INITIALIZE */
		ucGLOB_diagFlagByte1.byte = DIAG_FLAG_BYTE_1_INIT_VAL;
		ucGLOB_diagFlagByte2.byte = DIAG_FLAG_BYTE_2_INIT_VAL;


		/***********  PICK OUT A GROUP FIRST  ****************/

		vSERIAL_rom_sout("DiagGrpsAre:\r\n");
		for(ucc=0;  ucc<NUMBER_OF_DIAG_GROUPS;  ucc++)	//list the groups
			{
			vSERIAL_UI8_2char_out(ucc,' ');
			vSERIAL_rom_sout(": ");
			vSERIAL_rom_sout(cpaGroupName[ucc]);
			vSERIAL_crlf();
			}/* END: for(ucc) */

		vSERIAL_crlf();

		ucGroupAnsIdx = ucFULLDIAG_getChoiceOrAll(
							"Pick one or A for all: ",
							NUMBER_OF_DIAG_GROUPS
							);

		ucGroupIdx = ucGroupAnsIdx;
 		ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_group_bit = 1;

		if(ucGroupAnsIdx == 'A')
			{
			ucGroupIdx = 0;
			ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_group_bit = 0;
			ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit = 1;
			}
		if(ucGroupAnsIdx == 'X')
			{
			break;
			}






		/******  NOW PICK OUT A TEST TO RUN  -- IF NOT RUNNING ALL *********/
		ucTestAnsIdx = 0;						//default
		ucTestNumIdx = 0;						//default
		if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_group_bit)
			{
			vSERIAL_rom_sout("DiagsAre:\r\n");

			for(ucc=0;  ucc<ucaDiagCountPerGroup[ucGroupIdx];  ucc++)	//list the groups
				{
				vSERIAL_UI8_2char_out(ucc,' ');
				vSERIAL_rom_sout(": ");
				vSERIAL_rom_sout((cppGroupsFuncDesc[ucGroupIdx])[ucc]);
				vSERIAL_crlf();
				}/* END: for(ucc) */

			vSERIAL_crlf();

			ucTestAnsIdx = ucFULLDIAG_getChoiceOrAll(
								"Pick one or A for all: ",
								ucaDiagCountPerGroup[ucGroupIdx]
								);

			ucTestNumIdx = ucTestAnsIdx;
			ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_test_bit = 1;

			if(ucTestAnsIdx == 'A')	// all tests
				{
				ucTestNumIdx = 0;
				ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_test_bit = 0;
				ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit = 1;
				}

			if(ucTestAnsIdx == 'X')
				{
				continue;
				}

			}/* END: if() */



//		vSERIAL_rom_sout("FellIntoTsts\r\n");



		/************ AT THIS POINT WE ARE READY TO RUN *******************/
		vSERIAL_rom_sout("RunTsts...\r\n");

		/* GROUP LOOP SETUP */
		ucGroupLpStart = 0;
		ucGroupLpEnd   = NUMBER_OF_DIAG_GROUPS;
		ucGroupLpInc   = 1;
		if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_group_bit) //change setup if only one group
			{
			ucGroupLpStart = ucGroupIdx;
			ucGroupLpInc = 0;
			}

		/* GROUP LOOP */
		for(ucGidx=ucGroupLpStart; ucGidx<ucGroupLpEnd;	ucGidx+=ucGroupLpInc)
			{

			/* TEST LOOP SETUP */
			ucTestLpStart = 0;
			ucTestLpEnd   = ucaDiagCountPerGroup[ucGidx];
			ucTestLpInc   = 1;
			if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_test_bit) //change setup if only one group
				{
				ucTestLpStart = ucTestNumIdx;
				ucTestLpInc = 0;
				}

			/* TEST LOOP */
			for(ucTidx=ucTestLpStart; ucTidx<ucTestLpEnd;  ucTidx+=ucTestLpInc)
				{

				/* REPEAT LOOP SETUP */
				uiRepLpStart = 0;
				uiRepLpEnd   = (uipaGROUPS_testRepCntPtr[ucGidx])[ucTidx];
				uiRepLpInc   = 1;
				if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_test_repeat_looping_bit) //change setup if only one group
					{
					uiRepLpInc = 0;
					}

				/* REPEAT LOOP */
				for(uiRidx=uiRepLpStart; uiRidx<uiRepLpEnd; uiRidx+=uiRepLpInc)
					{
					/* RUN THE TEST AND TEST THE RESULTING FLAG */
					ucTestRet = (pfpGroupPtrArray[ucGidx])[ucTidx]();
					switch(ucTestRet)
						{
						case DIAG_ACTION_KEY_ERR:
							if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_halt_on_error_bit)
								{
								vSERIAL_rom_sout("\r\nHltOnErr\'H\'GotoNxtHlt,\'G\'=GoNoHlt...");
								while(TRUE) //lint !e774
									{
									ucChar = ucSERIAL_bin();
									if((ucChar == 'g') || (ucChar == 'G'))
										{
										ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_halt_on_error_bit = 0;
										vSERIAL_bout(ucChar);
										vSERIAL_crlf();
										break;
										}
									if((ucChar == 'h') || (ucChar == 'H'))
										{
										vSERIAL_bout(ucChar);
										vSERIAL_crlf();
										break;
										}
									}/* END: while() */

								}/* END: if() */
							break;
							
						case DIAG_ACTION_KEY_EXIT:
							ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_top_bit = 1;
							break;	

						default:
							break;

						}/* END: switch() */


					/* CHECK FOR ANY KEY HITS */
					if((ucFULLDIAG_doRunTimeKeyHitChk() > DIAG_ACTION_KEY_PLAIN_KEY) ||
					   (ucTestRet > DIAG_ACTION_KEY_ERR))
						{
						if((ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_top_bit) ||
						   (ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_prev_test_bit) ||
						   (ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_next_test_bit) ||
						   (ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_quit_out_to_return_bit)
						   )
							{
							break;
							}
						
						if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_change_looping_sts_bit)
							{
							ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_change_looping_sts_bit = 0;

							if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_test_repeat_looping_bit)
								{
								ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_test_repeat_looping_bit = 0;
								ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_test_bit = 0;
								uiRepLpInc = 1;
								ucTestLpInc = 1;
								break;
								}
							else
								{
								ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_test_repeat_looping_bit = 1;
								uiRepLpInc = 0;
								continue;
								}
							}

						if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_tell_whats_running_bit)
							{
							vSERIAL_crlf();
							vSERIAL_rom_sout((cppGroupsFuncDesc[ucGidx])[ucTidx]);
							vSERIAL_crlf();
							ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_tell_whats_running_bit = 0;

							#if 0
							vSERIAL_rom_sout("ucGidx= ");
							vSERIAL_UI8out(ucGidx);
							vSERIAL_crlf();
							vSERIAL_rom_sout("ucTidx= ");
							vSERIAL_UI8out(ucTidx);
							vSERIAL_crlf();
							#endif

							continue;
							}

						}/* END: if() */

					}/* END: for(uiRidx) */	/* END: of REPEAT LOOP */



				if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_quit_out_to_return_bit)
					{
					break;
					}

				if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_top_bit)
					{
					ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_test_bit = 0;
					ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_prev_test_bit = 0;
					ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_next_test_bit = 0;
					break;
					}

				if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_prev_test_bit)
					{
					if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_test_bit)
						{
						ucTestLpInc = 0;
						if(ucTidx != 0) ucTidx--;
						}
					else
						{
						ucTestLpInc = 1;
						if(ucTidx != 0) ucTidx--;
						ucTidx--;
						}
					ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_prev_test_bit = 0;
					continue;
					}

				if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_next_test_bit)
					{
					ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_next_test_bit = 0;
					if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_test_bit)
						{
						ucTestLpInc = 0;
						if(ucTidx < (ucaDiagCountPerGroup[ucGidx])-1) ucTidx++;
						}
					else
						{
						ucTestLpInc = 1;
						}
					continue;
					}

				}/* END: for(ucTidx) */  /* END of TEST LOOP */




			if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_quit_out_to_return_bit)
				{
				break;
				}

			if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_top_bit)
				{
				ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_group_bit = 0;
				ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_top_bit = 0;
				break;
				}

			}/* END: for(ucGidx) */  /* END of GROUP LOOP */




		if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_quit_out_to_return_bit)
			{
			break;
			}

		}/* END: while() */  /* END of DIAG LOOP */

	vSERIAL_rom_sout("XitDiag\r\n");

	return;

	}/* END: vFULLDIAG_run_module() */








/*******************  FRAM DIAGNOSTICS HERE  ********************************/


/**********************  ucFULLDIAG_FRAM_0()  ********************************
*
* Routine to test the low level FRAM calls
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

uchar ucFULLDIAG_FRAM_0(
		void
		)
	{
	uchar ucRetVal;
	uchar ucVal;
	uint uiVal;
	USL uslVal;		
	unsigned long ulVal;

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(cpFRAM_FuncDesc[0]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	vFRAM_init();

	/* TEST THE B8 WRITE AND READ ROUTINES */
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_write_B8(FRAM_TEST_ADDR, 0x12);
	ucVal = ucFRAM_read_B8(FRAM_TEST_ADDR);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	if(ucVal != 0x12)
		{
		ucRetVal = 1;						//indicate and error
		vMSG_showStorageErr(
			"FRAM B8 write failure",
			(unsigned long)FRAM_TEST_ADDR,
			(unsigned long)0x12,
			(unsigned long)ucVal
			);

		vBUZ_morrie();
		}
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_write_B8(FRAM_TEST_ADDR, (~0x12 & 0xFF));
	ucVal = ucFRAM_read_B8(FRAM_TEST_ADDR);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	if(ucVal != (~0x12 & 0xFF))
		{
		ucRetVal = 1;						//indicate and error
		vMSG_showStorageErr(
			"FRAM B8 write failure",
			(unsigned long)FRAM_TEST_ADDR,
			(unsigned long)(~0x12 & 0xFF),
			(unsigned long)ucVal
			);

		vBUZ_morrie();
		}




	/* TEST THE B16 WRITE AND READ ROUTINES */
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_write_B16(FRAM_TEST_ADDR, 0x1234);
	uiVal = uiFRAM_read_B16(FRAM_TEST_ADDR);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	if(uiVal != 0x1234)
		{
		ucRetVal = 1;						//indicate and error
		vMSG_showStorageErr(
			"FRAM B16 write failure",
			(unsigned long)FRAM_TEST_ADDR,
			(unsigned long)0x1234,
			(unsigned long)uiVal
			);

		vBUZ_morrie();
		}
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_write_B16(FRAM_TEST_ADDR, (~0x1234 & 0xFFFF));
 	uiVal = uiFRAM_read_B16(FRAM_TEST_ADDR);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	if(uiVal != (~0x1234 & 0xFFFF))
		{
		ucRetVal = 1;						//indicate and error
		vMSG_showStorageErr(
			"FRAM B16 write failure",
			(unsigned long)FRAM_TEST_ADDR,
			(unsigned long)(~0x1234 & 0xFFFF),
			(unsigned long)uiVal
			);

		vBUZ_morrie();
		}



	/* TEST THE B24 WRITE AND READ ROUTINES - removed and not used
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_write_B24(FRAM_TEST_ADDR, 0x123456);
	uslVal = uslFRAM_read_B24(FRAM_TEST_ADDR);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	if(uslVal != 0x123456)
		{
		ucRetVal = 1;						//indicate and error
		vMSG_showStorageErr(
			"FRAM B24 write failure",
			(unsigned long)FRAM_TEST_ADDR,
			(unsigned long)0x123456,
			(unsigned long)uslVal
			);

		vBUZ_morrie();
		}
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_write_B24(FRAM_TEST_ADDR, (~0x123456 & 0xFFFFFF));
	uslVal = uslFRAM_read_B24(FRAM_TEST_ADDR);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	if(uslVal != (~0x123456 & 0xFFFFFF))
		{
		ucRetVal = 1;						//indicate and error
		vMSG_showStorageErr(
			"FRAM B24 write failure",
			(unsigned long)FRAM_TEST_ADDR,
			(unsigned long)(~0x123456 & 0xFFFFFF),
			(unsigned long)uslVal
			);

		vBUZ_morrie();
		}
*/



	/* TEST THE B32 WRITE AND READ ROUTINES */
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_write_B32(FRAM_TEST_ADDR, 0x12345678);
	ulVal = ulFRAM_read_B32(FRAM_TEST_ADDR);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	if(ulVal != 0x12345678)
		{
		ucRetVal = 1;						//indicate and error
		vMSG_showStorageErr(
			"FRAM B32 write failure",
			(unsigned long)FRAM_TEST_ADDR,
			(unsigned long)0x12345678,
			(unsigned long)ulVal
			);

		vBUZ_morrie();
		}
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_write_B32(FRAM_TEST_ADDR, (~0x12345678 & 0xFFFFFFFF));
	ulVal = ulFRAM_read_B32(FRAM_TEST_ADDR);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	if(ulVal != (~0x12345678 & 0xFFFFFFFF))
		{
		ucRetVal = 1;						//indicate and error
		vMSG_showStorageErr(
			"FRAM B32 write failure",
			(unsigned long)FRAM_TEST_ADDR,
			(unsigned long)(~0x12345678 & 0xFFFFFFFF),
			(unsigned long)ulVal
			);

		vBUZ_morrie();
		}


	vFRAM_quit();

	return(ucRetVal);

	}/* END: ucFULLDIAG_FRAM_0() */







/**********************  ucFULLDIAG_FRAM_1()  ********************************
*
* Routine to test the level 2 FRAM calls
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

uchar ucFULLDIAG_FRAM_1(
		void
		)
	{
	uchar ucRetVal;
	uint uiVersionNum;
	
	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(cpFRAM_FuncDesc[1]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* CHECK IF THE FRAM IS THERE */
	if(ucL2FRAM_chk_for_fram(FRAM_CHK_REPORT_MODE) == 0)
		{
		vSERIAL_rom_sout("ChkFramFail\r\n");
		vBUZ_morrie();
		ucRetVal = 1;
		return(ucRetVal);
		}
	vSERIAL_rom_sout("FramDetected\r\n");



	/* WE ARE ABOUT TO CHANGE THE FRAM MEMORY -- CHECK IF IT IS FORMATTED */
	vSERIAL_rom_sout("ChkingForFramFmt\r\n");
	if(uiL2FRAM_chk_for_fram_format())		//are we formatted?
 		{
		/* FRAM IS FORMATED */
		vSERIAL_rom_sout("FramIsFmtted\r\n");


		/* IF THIS IS AN Auto TEST -- LEAVE */
		if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
			{
			vSERIAL_rom_sout(AutoTestExitStr);
			return(ucRetVal);
			}

		/* NO THIS IS A MANUALLY RUN TEST -- ASK FOR AN OVERRIDE */
		vSERIAL_rom_sout("OverwriteFram?(YorN)...");

		/* GET THE CONFIRMATION */
		{
		uchar ucStr[4];
		if(ucSERIAL_getEditedInput(ucStr, 2)) return(DIAG_ACTION_KEY_EXIT);
		if((ucStr[0] != 'Y') && (ucStr[0] != 'y')) return(DIAG_ACTION_KEY_EXIT);
		}

		}/* END: if() */

	vSERIAL_rom_sout("Tsting...\r\n");




	/* IF YOU ARE HERE, FRAM IS EITHER UNFORMATTED, OR HAS AN OPERATOR OVERRIDE */

	/* WIPE THE FRAM  & CHECK THE WIPE */
	vSERIAL_rom_sout("SetFramTo0x99\r\n");
	vFRAM_fillFramBlk(0, 512, 0x99);
	vFRAM_show_fram(0, 24);
	/* CHECK THE FRAM WIPE */
	vSERIAL_rom_sout("ChkingFramSet\r\n");
	if(ucFRAM_chk_fram_blk(0,512,0x99) == 0)
		{
		vSERIAL_rom_sout("FramSetBd\r\n");
		vBUZ_morrie();
		ucRetVal = 1;
		return(ucRetVal);
		}
	else
		{
		vSERIAL_rom_sout("FramSetOK\r\n");
		}


	/* NOW FORMAT THE FRAM */
	vSERIAL_rom_sout("FmtOfFram\r\n");
	vL2FRAM_format_fram();
	vFRAM_show_fram(0, 24);
	/* CHECK THE FRAM FORMAT */
	vSERIAL_rom_sout("ChkingFramFmt\r\n");
	if(uiL2FRAM_chk_for_fram_format() == 0)
		{
		vSERIAL_rom_sout("FramFmtBd\r\n");
		vBUZ_morrie();
		ucRetVal = 1;
		}
	else
		{
		vSERIAL_rom_sout("FramFmtOK\r\n");
		}
		
	/* CHECK THE FORMAT VERSION NUMBER */
	vSERIAL_rom_sout("ChkingFramVer\r\n");
	uiVersionNum = uiL2FRAM_get_version_num();
	vSERIAL_rom_sout("FramVerChk: RdVer= ");
	vSERIAL_HB16out(uiVersionNum);
	vSERIAL_rom_sout(" ShouldBeVer= ");
	vSERIAL_HB16out(FRAM_VERSION);
	vSERIAL_crlf();
	if(uiVersionNum == FRAM_VERSION)
		{
		vSERIAL_rom_sout("FramVerOK\r\n");
		}
	else
		{
		vSERIAL_rom_sout("FramVerBd\r\n");
		vBUZ_morrie();
		ucRetVal = 1;
		}


	/* NOW UNFORMAT THE FRAM */
	vSERIAL_rom_sout("UnfmtOfFram\r\n");
	vL2FRAM_force_fram_unformat();
	vFRAM_show_fram(0, 24);
	/* NOW CHECK THE UNFORMAT */
	vSERIAL_rom_sout("ChkingFramUNfmt\r\n");
	if(uiL2FRAM_chk_for_fram_format() != 0)
		{
		vSERIAL_rom_sout("FramUNfmtBd\r\n");
		vBUZ_morrie();
		ucRetVal = 1;
		}
	else
		{
		vSERIAL_rom_sout("FramUNfmtOK\r\n");
		}


	/* NOW RE-FORMAT THE FRAM */
	vSERIAL_rom_sout("RefmtOfFram\r\n");
	vL2FRAM_format_fram();
	vFRAM_show_fram(0, 24);
	/* CHECK THE FRAM RE-FORMAT */
	vSERIAL_rom_sout("ChkingFramRefmt\r\n");
	if(uiL2FRAM_chk_for_fram_format() == 0)
		{
		vSERIAL_rom_sout("FramRefmtBd\r\n");
		vBUZ_morrie();
		ucRetVal = 1;
		}
	else
		{
		vSERIAL_rom_sout("FramRefmtOK\r\n");
		}

	return(ucRetVal);

	}/* END: ucFULLDIAG_FRAM_1() */






/**********************  ucFULLDIAG_FRAM_2()  ********************************
*
* "FRAM2: Special test to catch the first byte is bad problem"
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

uchar ucFULLDIAG_FRAM_2(
		void
		)
	{
	uchar ucRetVal;
//	uchar ucTmp;
//	uint uii;
	

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(cpFRAM_FuncDesc[2]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* TURN ON THE SD BOARD */
	vSERIAL_rom_sout("1stTstIsWithSDpwred\r\n");
//	if(ucSDCTL_start_SD_and_specify_boot(SD_BOOT_LOADER_DISABLED) == 0)
//		{
//		vSERIAL_rom_sout("BdSDstart\r\n");
//		vBUZ_morrie();
//		ucRetVal = 1;
//		return(ucRetVal);
//		}

	/* CHECK IF THE FRAM IS THERE */
	if(ucL2FRAM_chk_for_fram(FRAM_CHK_REPORT_MODE) == 0)
		{
		vSERIAL_rom_sout("ChkFramFailRet\r\n");
		vBUZ_morrie();
		ucRetVal = 1;
//		vSDCTL_halt_SD_and_power_down();
		return(ucRetVal);
		}
	vSERIAL_rom_sout("FramDetectd\r\n");



	/* WIPE THE FRAM  & CHECK THE WIPE */
	vSERIAL_rom_sout("FramTstLocSetTo0x99\r\n");
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	/* CHECK THE FRAM WIPE */
	vSERIAL_rom_sout("ChkingFramTstArea\r\n");
	if(ucFRAM_chk_fram_blk(FRAM_TEST_ADDR, 4, 0x99) == 0)
		{
		vSERIAL_rom_sout("FramSetBd\r\n");
		vBUZ_morrie();
		ucRetVal = 1;
		}
	else
		{
		vSERIAL_rom_sout("FramSetOK\r\n");
		}


	/* NOW WE ARE GOING TO CLEAR THE FRAM TEST AREA BUT WITH SERIAL PORT INPUT */
	vSERIAL_rom_sout("FramTstLocSetTo0x66AfterCommInput\r\n");

	#if 1
	vDELAY_wait100usTic(20000);				//this causes fails too
	#endif

	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x66);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	/* CHECK THE FRAM WIPE */
	vSERIAL_rom_sout("ChkOfFramSet\r\n");
	if(ucFRAM_chk_fram_blk(FRAM_TEST_ADDR, 4, 0x66) == 0)
		{
		vSERIAL_rom_sout("FramSetBd\r\n");
		vBUZ_morrie();
		ucRetVal = 1;
		}
	else
		{
		vSERIAL_rom_sout("FramSetOK\r\n");
		}


	/* POWER DOWN THE SD BOARD */
//	vSDCTL_halt_SD_and_power_down();


	/* LEAVE SD BOARD POWERED DOWN */
	vSERIAL_rom_sout("2ndPartOfTstWithSDPwredOff\r\n");
	vSERIAL_rom_sout("TstShouldFail,IfNot,SDmayBeBd\r\n");

	/* WIPE THE FRAM  & CHECK THE WIPE */
	vSERIAL_rom_sout("FramTstLocSetTo0x99\r\n");
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);

	/* NOW WE ARE GOING TO CLEAR THE FRAM TEST AREA BUT WITH SERIAL PORT INPUT */
	vSERIAL_rom_sout("FramTstLocSetTo0x66AfterSerialInput\r\n");

	#if 1
	vDELAY_wait100usTic(20000);				//this causes fails too
	#endif

	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x66);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	/* CHECK THE FRAM WIPE */
	vSERIAL_rom_sout("ChkingFramLocForBdSet\r\n");
	if(ucFRAM_chk_fram_blk(FRAM_TEST_ADDR, 4, 0x66) == 1)
		{
		vSERIAL_rom_sout("FramSetBdFail\r\n");
		vBUZ_morrie();
		ucRetVal = 1;
		}
	else
		{
		vSERIAL_rom_sout("FramSetBdOK\r\n");
		}

	return(ucRetVal);

	}/* END: vFULLDIAG_FRAM_2() */






/*********************  FLASH DIAGNOSTICS HERE  *****************************/


/**********************  FULLDIAG_FLASH_0()  ********************************
*
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

uchar ucFULLDIAG_FLASH_0(
		void
		)
	{
	uchar ucRetVal;

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(cpFLASH_FuncDesc[0]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	vDELAY_wait100usTic(5000);		//wait 500ms

	return(ucRetVal);

	}/* END: ucFULLDIAG_FLASH_0() */








/*****************  SD BOARD DIAGNOSTICS HERE  ******************************/


/**********************  ucFULLDIAG_SDBOARD_0()  ****************************
*
* "  SDBOARD_0: Break out of sleep bit follower program  ",
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

uchar ucFULLDIAG_SDBOARD_0(
		void
		)
	{
/*
	unsigned long ull;
	uchar ucRetVal;

	// SHOW THE INTRO //
	vSERIAL_printDashIntro(cpSDBOARD_FuncDesc[0]);

	// ASSUME NO ERRORS //
	ucRetVal = DIAG_ACTION_KEY_NONE;

	// DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC //
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_rom_sout(AutoTestExitStr);
		return(ucRetVal);
		}

	vSERIAL_rom_sout(
		"NOTE:ThisTstOnlyWrksIfSDhasSpecialCode\r\n"
		);

	// TURN ON THE SD BOARD //
//	if(ucSDCTL_start_SD_and_specify_boot(SD_BOOT_LOADER_DISABLED) == 0)
//		{
//		vSERIAL_rom_sout("BdStrtOnSD\r\n");
//		vBUZ_morrie();
//		ucRetVal = 1;
//		return(ucRetVal);
//		}


	// THIS DIAG COPIES THE BUTTON PUSH INTO THE SD_BREAKOUT_OF_SLEEP_BIT //
	vSERIAL_rom_sout("PshingBttnRaisesLineOnSD_BREAK_OUT_OF_SLEEPpin\r\n\n");
	vSERIAL_rom_sout("BttnToutIfNotPushed\r\n\n");
	for(ull=0;  ull<5000000UL;  ull++)
		{

		// This need to change! to reflect the change to mini SD
		if(BUTTON_IN_PORT & BUTTON_BIT)
			PORTBbits.SD_BREAKOUT_OF_SLEEP_BIT = 1;
		else
			PORTBbits.SD_BREAKOUT_OF_SLEEP_BIT = 0;

		if(ucSERIAL_kbhit()) break;

		}// END: for() //

	// POWER DOWN THE SD BOARD //
//	vSDCTL_halt_SD_and_power_down();

	return(ucRetVal);
*/
return(0); // added at port to return no problems with sd board that is not there.
	}/* END: ucFULLDIAG_SDBOARD_0() */






/**********************  ucFULLDIAG_SDBOARD_1()  ****************************
*
* "  SDBOARD_1: Check if the SD board serial number is xmitted correctly  ",
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

uchar ucFULLDIAG_SDBOARD_1(
		void
		)
	{
/*
	uchar ucRetVal;

	// SHOW THE INTRO //
	vSERIAL_printDashIntro(cpSDBOARD_FuncDesc[1]);

	// ASSUME NO ERRORS //
	ucRetVal = DIAG_ACTION_KEY_NONE;


	// POWER THE SD BOARD //
//	if(ucSDCTL_start_SD_and_specify_boot(SD_BOOT_LOADER_DISABLED) == 0)
//		{
//		vSERIAL_rom_sout("SDstartErr\r\n");
//		vBUZ_morrie();
//		ucRetVal = 1;	//error
//		}


	// READ SD MSG VERSION //
	if(ucSDCOM_awaken_SD_and_do_full_msg_transfer(SENSOR_SD_MSG_VERSION) == 0)
		{
		vSERIAL_rom_sout("SDmsgTransferErr\r\n");
		vBUZ_morrie();
		ucRetVal = 1;	//error
		}

	// CHECK THE MSG VERSION //
	if((ucaSDSPI_BUFF[3] != SD_MSG_VERSION_HI) || (ucaSDSPI_BUFF[4] != SD_MSG_VERSION_LO))
		{
		vSERIAL_rom_sout("SDmsgVerOldShouldBe ");
		vSERIAL_HB8out(SD_MSG_VERSION_HI);
		vSERIAL_bout('.');
		vSERIAL_HB8out(SD_MSG_VERSION_LO);
		vSERIAL_rom_sout(" was ");
		vSERIAL_HB8out(ucaSDSPI_BUFF[3]);
		vSERIAL_bout('.');
		vSERIAL_HB8out(ucaSDSPI_BUFF[4]);
		vSERIAL_crlf();
		vBUZ_morrie();
		ucRetVal = 1;	//error
		}

	// POWER DOWN THE SD BOARD //
//	vSDCTL_halt_SD_and_power_down();

	return(ucRetVal);
*/
return(0);
	}/* END: ucFULLDIAG_SDBOARD_1() */









/*****************  RADIO DIAGNOSTICS HERE  ******************************/


/**********************  ucFULLDIAG_RADIO_0()  ****************************
*
* "  RADIO_0: Measure the Transmit power of the radio  ",
*
*
* This routine turns on the radio in XMIT mode and hangs there.
* This is so the radio XMIT current can be measured.
* The signal is steady (not pulsed) so that the full power can be measured.
*
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

uchar ucFULLDIAG_RADIO_0(
		void
		)
	{
	uchar ucRetVal;
	uchar ucChar;

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(cpRADIO_FuncDesc[0]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC */
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_rom_sout(AutoTestExitStr);
		return(ucRetVal);
		}

	/* LOAD THE RADIO */
	vRADIO_init();

	/* FORCE THE RADIO TO HIGHEST TRANSMIT POWER */
	vRADIO_setTransmitPower(XMIT_PWR_HI);


	/* TURN RADIO ON */
	vRADIO_init();					//turn radio on
	vRADIO_xmit_msg();				//radio mode = XMIT


	/* TURN ON THE TRANSMIT BIT */
	vSERIAL_rom_sout("InXmit0Mode\r\n");
	vSERIAL_rom_sout("SpaceCharSwitchsMode,or\'X\'toXit\r\n");
	ucChar = ucSERIAL_bin();
	switch(ucChar)
		{
		case 'x':
		case 'X':
			ucRetVal = DIAG_ACTION_KEY_EXIT;
			break;

		default:
			break;

		}/* END: switch() */

	if(ucRetVal != DIAG_ACTION_KEY_EXIT)
		{
		vSERIAL_rom_sout("Xmit1Mode\r\n");
		vSERIAL_rom_sout("SpaceChrToSwtchMode,or\'X\'toXit\r\n");
		while(!ucSERIAL_kbhit());			//lint !e722
		}

	/* MAKE SURE RADIO IS SHUT OFF */
	vRADIO_quit();

	vSERIAL_rom_sout("Xit\r\n"); 
	return(ucRetVal);

	}/* END: ucFULLDIAG_RADIO_0() */






/**********************  ucFULLDIAG_RADIO_1()  ****************************
*
* "RADIO_1: Characterize the messaging error",
*
* This routine has a sender section and a receiver section.  The sender
* sends out a known message and the receiver receives it and checks it
* for an exact bit for bit match.  If it matches it goes ahead and waits
* for the next message.  If not it prints out an error message and then
* goes ahead and waits for the next message.
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

uchar ucFULLDIAG_RADIO_1(
		void
		)
	{
/*
	uchar ucRetVal;
	uchar ucChar;
	uchar ucChanIdx;


	// SHOW THE INTRO //
	vSERIAL_printDashIntro(cpRADIO_FuncDesc[1]);

	// ASSUME NO ERRORS //
	ucRetVal = DIAG_ACTION_KEY_NONE;

	// DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC //
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_rom_sout(AutoTestExitStr);
		return(ucRetVal);
		}

	// CHOOSE CHANNEL FREQUENCY //
	vSERIAL_rom_sout("PickFreq\'O\'=Op,\'D\'=Discvr,\'T\'=Tst...");
	ucChar = ucSERIAL_bin();
	switch(ucChar)
		{
		case 'd':
		case 'D':
			vSERIAL_rom_sout("Discvr\r\n");
			ucChanIdx = DISCOVERY_CHANNEL_INDEX;
			break;

		case 'o':
		case 'O':
			vSERIAL_rom_sout("Op\r\n");
			ucChanIdx = DATA_CHANNEL_INDEX;
			break;

		case 't':
		case 'T':
			vSERIAL_rom_sout("Tst\r\n");
			ucChanIdx = TEST_CHANNEL_INDEX;
			break;


		default:
			vSERIAL_showXXXmsg();
			ucRetVal = DIAG_ACTION_KEY_EXIT;
			goto Radio_1_exit;

		}// END: switch() //



	// CHOOSE TRANSMITTER OR RECEIVER //
	vSERIAL_rom_sout("Choose\'T\'=Xmit,\'R\'=Rec,\'X\'=Xit...");
	ucChar = ucSERIAL_bin();
	switch(ucChar)
		{
		case 't':
		case 'T':
			vSERIAL_rom_sout("Xmit\r\n");
			ucRetVal = ucDRADIO_continuousXmitTestMsg(ucChanIdx);
			break;

		case 'r':
		case 'R':
			vSERIAL_rom_sout("Rec\r\n");
			ucRetVal = ucDRADIO_continuousReceiveTestMsg(ucChanIdx);
			break;

		default:
			vSERIAL_showXXXmsg();
			ucRetVal = DIAG_ACTION_KEY_EXIT;
			break;

		}// END: switch() //


Radio_1_exit:

	return(ucRetVal);

*/
return(0);
	}// END: ucFULLDIAG_RADIO_1() //






/**********************  ucFULLDIAG_RADIO_2()  ****************************
*
* "RADIO_2: Check the Send/Receive switching DIODE"
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

uchar ucFULLDIAG_RADIO_2(
		void
		)
	{
/*
	uchar ucRetVal;
	uchar ucChar;

	// SHOW THE INTRO //
	vSERIAL_printDashIntro(cpRADIO_FuncDesc[2]);

	// ASSUME NO ERRORS //
	ucRetVal = DIAG_ACTION_KEY_NONE;

	// DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC //
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_rom_sout(AutoTestExitStr);
		return(ucRetVal);
		}

	// LOAD THE RADIO //
	vRADIO_init();

	// This needs to change! to reflect the radio changes

	// TURN RADIO ON //
	PORTJbits.RADIO_MODE_BIT = 0;			//radio mode = XMIT
	PORTJbits.RADIO_STANDBY_BIT = 1;		//turn radio on
	PORTGbits.RADIO_ANTENNA_BIT = 1;		//XMIT antenna


	// NOW SWITCH THE MODE BIT BETWEEN SEND AND RECEIVE //
	vSERIAL_rom_sout("SwchBitBetweenXmt&Rec\r\n");
	vSERIAL_rom_sout("\'X\'toXit\r\n");
	vSERIAL_rom_sout("Start\r\n");
	while(!ucSERIAL_kbhit())
		{
		if(ucSERIAL_kbhit()) break;		//chk for key hit

		vSERIAL_bout('T');
		PORTJbits.RADIO_MODE_BIT = 0;	//radio mode = XMIT
		vDELAY_wait100usTic(10000);		//switch delay

		vSERIAL_bout('R');
		PORTJbits.RADIO_MODE_BIT = 1;	//radio mode = RECEIVE
		vDELAY_wait100usTic(10000);		//switch delay

		}// END: while() //


	// HANDLE THE KEY HIT THAT BROKE OUT OF THE WHILE //
	ucChar = ucSERIAL_bin();
	switch(ucChar)
		{
		case 'x':
		case 'X':
			ucRetVal = DIAG_ACTION_KEY_EXIT;
			break;

		default:
			break;

		}// END: switch() //

	// MAKE SURE RADIO IS SHUT OFF //
	vRADIO_quit();

	vSERIAL_rom_sout("Xit\r\n"); 
	return(ucRetVal);
*/
return(0);
	}// END: ucFULLDIAG_RADIO_2() //






/**********************  ucFULLDIAG_RADIO_3()  ****************************
*
* "RADIO_3: Radio Message Sniffer",
*
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

uchar ucFULLDIAG_RADIO_3(
		void
		)
	{
/*
	uchar ucRetVal;
	uchar ucChar;
	uchar ucChanIdx;


	// SHOW THE INTRO //
	vSERIAL_printDashIntro(cpRADIO_FuncDesc[3]);

	// ASSUME NO ERRORS //
	ucRetVal = DIAG_ACTION_KEY_NONE;

	// DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC //
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_rom_sout(AutoTestExitStr);
		return(ucRetVal);
		}

	// CHOOSE CHANNEL FREQUENCY //
	vSERIAL_rom_sout("PickFreq\'O\'=Op,\'D\'=Discvr,\'T\'=Tst...");
	ucChar = ucSERIAL_bin();
	switch(ucChar)
		{
		case 'd':
		case 'D':
			vSERIAL_rom_sout("Discvr\r\n");
			ucChanIdx = DISCOVERY_CHANNEL_INDEX;
			break;

		case 'o':
		case 'O':
			vSERIAL_rom_sout("Op\r\n");
			ucChanIdx = DATA_CHANNEL_INDEX;
			break;

		case 't':
		case 'T':
			vSERIAL_rom_sout("Tst\r\n");
			ucChanIdx = TEST_CHANNEL_INDEX;
			break;

		default:
			vSERIAL_showXXXmsg();
			ucRetVal = DIAG_ACTION_KEY_EXIT;
			goto Radio_3_exit;

		}// END: switch() //

	vSERIAL_rom_sout("Sniff On\r\n");
	ucRetVal = ucDRADIO_run_sniffer(ucChanIdx);

Radio_3_exit:
	return(ucRetVal);
*/
return(0);

	}/* END: ucFULLDIAG_RADIO_3() */





/*****************  SENSOR DIAGNOSTICS HERE  ******************************/


/**********************  ucFULLDIAG_SENSOR_0()  ****************************
*
* "SENSOR_0: Light sensor readings from all light sensors",
*
* This routine reads Light 1,2,3,4 repeatedly.
*
*****************************************************************************/

uchar ucFULLDIAG_SENSOR_0(
		void
		)
	{
	uchar ucRetVal;
	uchar ucii;
	uchar ucjj;
	uint uiVal;
	uchar ucSensorID[4];

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(cpSENSOR_FuncDesc[0]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC */
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_rom_sout(AutoTestExitStr);
		return(ucRetVal);
		}

	for(ucii=0; ucii<4;  ucii++)
		{
		ucSensorID[ucii] = SENSOR_LIGHT_1 + ucii;
		}/* END: for() */

	for(ucii=0;  ;  ucii++)
		{
		/* SHOW HEADER LINE */
		if((ucii %16) == 0)
			{
			vSERIAL_rom_sout("\r\n   L1     L2     L3     L4\r\n");
			}

	    vACTION_read_sensors_into_buffer(ucSensorID);

	    for(ucjj=0; ucjj<4; ucjj++)
		   	{			
			/* SHOW THE LIGHT SENSOR VALUES */
			uiVal = uiMISC_buildUintFromBytes(
				(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI +(3 * ucjj)],
				NO_NOINT
				);
			vSERIAL_UI16out(uiVal);
			vSERIAL_rom_sout("  ");

	        }// END: for(ucjj = 0)

		vSERIAL_crlf(); //print out a new line of records

		if(ucSERIAL_kbhit()) break;

        }/* END: while() */

	return(ucRetVal);

	}/* END: ucFULLDIAG_SENSOR_0() */






/**********************  ucFULLDIAG_SENSOR_1()  ****************************
*
* "SENSOR_1: Soil Moisture readings from all moisture sensors"
*
*****************************************************************************/

uchar ucFULLDIAG_SENSOR_1(
		void
		)
	{
	uchar ucRetVal;
	uchar ucc;
	uchar ucii;
	uint uiVal;
	uchar ucSensorID[4];
//	long lVal;
//	long lReading[2];

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(cpSENSOR_FuncDesc[1]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC */
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_rom_sout(AutoTestExitStr);
		return(ucRetVal);
		}

	ucSensorID[0] = SENSOR_SOIL_MOISTURE_1;
	ucSensorID[1] = SENSOR_SOIL_MOISTURE_2;
	ucSensorID[2] = 0;
	ucSensorID[3] = 0;

	for(ucc=0;  ;  ucc++)
		{
		/* SHOW HEADER LINE */
		if((ucc %16) == 0)
			{
			vSERIAL_rom_sout("\r\nSOIL1  SOIL2\r\n");
			}

	    vACTION_read_sensors_into_buffer(ucSensorID);

		/* SHOW THE SENSOR VALUES */
		for(ucii=0; ucii<2;  ucii++)
			{
			uiVal = uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI +(3 * ucii)],
				 	NO_NOINT
				 	);

			//lReading[ucii] = (long)uiVal;

			vSERIAL_UI16out(uiVal);
			vSERIAL_rom_sout("  ");

			}/* END: for(ucii) */

		vSERIAL_crlf();

		if(ucSERIAL_kbhit()) break;

		vDELAY_wait100usTic(5000);				//wait 1/2 sec

		}/* END: for(ucc) */

	return(ucRetVal);

	}/* END: ucFULLDIAG_SENSOR_1() */





/********************  vFULLDIAG_showTCgroupOf3()  **************************
*
* This routine shows the Vactual, OwireColdJctTemp, and the Kun Converted value
*
*****************************************************************************/

void vFULLDIAG_showTCgroupOf3(
		int iV_actual,
		int iT_coldJct_in_C
		)
	{
	int iT_in_C;
	int iT_in_F;

	vSERIAL_I16outFormat(iV_actual,4);
	vSERIAL_bout(' ');

	vSERIAL_I16outFormat(iT_coldJct_in_C, 4);
	vSERIAL_bout(' ');

	iT_in_C = iTHERMO_computeTfromTC(iV_actual, iT_coldJct_in_C);
	iT_in_F = iTHERMO_convertOwire_C_to_F(iT_in_C);
	vTHERMO_showOwireValue(iT_in_F);
	vSERIAL_bout(' ');

	return;

	}/* END: vFULLDIAG_showTCgroupOf3() */





/**********************  ucFULLDIAG_SENSOR_2()  ****************************
*
* "SENSOR_2: Thermocouple reading from all thermocouples"
*
*****************************************************************************/

uchar ucFULLDIAG_SENSOR_2(
		void
		)
	{
	uchar ucRetVal;
	uchar ucii;
	uchar ucjj;
	int iV_actual;
	int iT_coldJct_in_C;
	uchar ucSensorID1[4];
	uchar ucSensorID2[4];

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(cpSENSOR_FuncDesc[2]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC */
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_rom_sout(AutoTestExitStr);
		return(ucRetVal);
		}

	ucSensorID1[0] = SENSOR_TC_1;
	ucSensorID1[1] = SENSOR_ONEWIRE_0;
	ucSensorID1[2] = SENSOR_TC_2;
	ucSensorID1[3] = SENSOR_ONEWIRE_0;

	ucSensorID2[0] = SENSOR_TC_3;
	ucSensorID2[1] = SENSOR_ONEWIRE_0;
	ucSensorID2[2] = SENSOR_TC_4;
	ucSensorID2[3] = SENSOR_ONEWIRE_0;

	for(ucii=0; ; ucii++)
		{

	    vACTION_read_sensors_into_buffer(ucSensorID1);

		/* SHOW THE HEADER PERIODICALLY */
		if((ucii % 24) == 0)
			vSERIAL_rom_sout("\r\n TC1 OWIRE Deg_F    TC2 OWIRE Deg_F    TC3 OWIRE Deg_F    TC4 OWIRE Deg_F\r\n");

#if 0
 vSERIAL_rom_sout("\r\nTC1 OWIRE Deg_F    TC2 OWIRE Deg_F    TC3 OWIRE Deg_F    TC4 OWIRE Deg_F\r\n");
//xxxx +xxx -xx.yy   xxxx +xxx -xx.yy   xxxx +xxx -xx.yy   xxxx +xxx -xx.yy    
#endif
		/* SHOW THE SENSOR VALUES */
		for(ucjj=0; ucjj<2;  ucjj++)
			{
			iV_actual = (int)uiMISC_buildUintFromBytes(
						(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI +(6 * ucjj)],
						NO_NOINT
						);
			iT_coldJct_in_C = (int)uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI+3 +(6 * ucjj)],
				 	NO_NOINT
				 	);
			vFULLDIAG_showTCgroupOf3(iV_actual, iT_coldJct_in_C);

			vSERIAL_rom_sout("  ");

			}/* END: for(ucjj) */

		/* GET DATA FOR TC 3&4  */
	    vACTION_read_sensors_into_buffer(ucSensorID2);

		/* SHOW THE SENSOR VALUES */
		for(ucjj=0; ucjj<2;  ucjj++)
			{

			iV_actual = (int)uiMISC_buildUintFromBytes(
						(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI +(6 * ucjj)],
						NO_NOINT
						);
			iT_coldJct_in_C = (int)uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI+3 +(6 * ucjj)],
				 	NO_NOINT
				 	);
			vFULLDIAG_showTCgroupOf3(iV_actual, iT_coldJct_in_C);

			vSERIAL_rom_sout("  ");

			}/* END: for(ucjj) */

		vSERIAL_crlf();

		if(ucSERIAL_kbhit()) break;

		}/* END: for(ucii) */

	return(ucRetVal);

	}/* END: ucFULLDIAG_SENSOR_2() */





/**********************  ucFULLDIAG_SENSOR_3()  ****************************
*
* "SENSOR_3: Vaisala Ave Wind Speed & Direction",
*
*****************************************************************************/

uchar ucFULLDIAG_SENSOR_3(
		void
		)
	{
	uchar ucRetVal;
	uchar ucii;
	uchar ucSensorNum;
	uint uiVal1;
	uint uiVal2;
	uchar ucSensorID[4];

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(cpSENSOR_FuncDesc[2]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC */
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_rom_sout(AutoTestExitStr);
		return(ucRetVal);
		}

	ucSensorID[0] = SENSOR_WIND_SPEED_AVE_WHOLE_VS;
	ucSensorID[1] = SENSOR_WIND_SPEED_AVE_FRAC_VS;
	ucSensorID[2] = SENSOR_WIND_DIR_AVE_WHOLE_VS;
	ucSensorID[3] = SENSOR_WIND_DIR_AVE_FRAC_VS;

	while(TRUE) //lint !e774
		{
	    vACTION_read_sensors_into_buffer(ucSensorID);

		/* SHOW THE SENSOR VALUES */
		for(ucii=0; ucii<2;  ucii++)
			{
			ucSensorNum = ucaMSG_BUFF[OM2_IDX_DATA_0_SENSOR_NUM +(6 * ucii)];
			uiVal1 = uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI +(6 * ucii)],
				 	NO_NOINT
				 	);
			uiVal2 = uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI+3 +(6 * ucii)],
				 	NO_NOINT
				 	);
			vSENSOR_showSensorName(ucSensorNum, L_JUSTIFY);
			vSERIAL_rom_sout(" = ");
			vSERIAL_HBV16out(uiVal1);
			vSERIAL_bout('.');
			vSERIAL_HB16out(uiVal2);
			vSERIAL_crlf();

			}/* END: for(ucii) */

		if(ucSERIAL_kbhit()) break;

		}/* END: while() */

	return(ucRetVal);

	}/* END: ucFULLDIAG_SENSOR_3() */







/**********************  ucFULLDIAG_SENSOR_4()  ****************************
*
* "SENSOR_4: Vaisala Air Pressure & Air Temperature",
*
*****************************************************************************/

uchar ucFULLDIAG_SENSOR_4(
		void
		)
	{
	uchar ucRetVal;
	uchar ucii;
	uchar ucSensorNum;
	uint uiVal1;
	uint uiVal2;
	uchar ucSensorID[4];

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(cpSENSOR_FuncDesc[2]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC */
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_rom_sout(AutoTestExitStr);
		return(ucRetVal);
		}

	ucSensorID[0] = SENSOR_AIR_PRESS_WHOLE_VS;
	ucSensorID[1] = SENSOR_AIR_PRESS_FRAC_VS;
	ucSensorID[2] = SENSOR_AIR_TEMP_WHOLE_VS;
	ucSensorID[3] = SENSOR_AIR_TEMP_FRAC_VS;

	while(TRUE) //lint !e774
		{
	    vACTION_read_sensors_into_buffer(ucSensorID);

		/* SHOW THE SENSOR VALUES */
		for(ucii=0; ucii<2;  ucii++)
			{
			ucSensorNum = ucaMSG_BUFF[OM2_IDX_DATA_0_SENSOR_NUM +(6 * ucii)];
			uiVal1 = uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI +(6 * ucii)],
				 	NO_NOINT
				 	);
			uiVal2 = uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI+3 +(6 * ucii)],
				 	NO_NOINT
				 	);
			vSENSOR_showSensorName(ucSensorNum, L_JUSTIFY);
			vSERIAL_rom_sout(" = ");
			vSERIAL_HBV16out(uiVal1);
			vSERIAL_bout('.');
			vSERIAL_HB16out(uiVal2);
			vSERIAL_crlf();

			}/* END: for(ucii) */

		if(ucSERIAL_kbhit()) break;

		}/* END: while() */

	return(ucRetVal);

	}/* END: ucFULLDIAG_SENSOR_4() */







/**********************  ucFULLDIAG_SENSOR_5()  ****************************
*
* "SENSOR_5: Vaisala Relative Humidity & Rain Accumulation"
*
*****************************************************************************/

uchar ucFULLDIAG_SENSOR_5(
		void
		)
	{
	uchar ucRetVal;
	uchar ucii;
	uchar ucSensorNum;
	uint uiVal1;
	uint uiVal2;
	uchar ucSensorID[4];

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(cpSENSOR_FuncDesc[2]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC */
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_rom_sout(AutoTestExitStr);
		return(ucRetVal);
		}

	ucSensorID[0] = SENSOR_REL_HUMID_WHOLE_VS;
	ucSensorID[1] = SENSOR_REL_HUMID_FRAC_VS;
	ucSensorID[2] = SENSOR_RAIN_ACC_WHOLE_VS;
	ucSensorID[3] = SENSOR_RAIN_ACC_FRAC_VS;

	while(TRUE) //lint !e774
		{
	    vACTION_read_sensors_into_buffer(ucSensorID);

		/* SHOW THE SENSOR VALUES */
		for(ucii=0; ucii<2;  ucii++)
			{
			ucSensorNum = ucaMSG_BUFF[OM2_IDX_DATA_0_SENSOR_NUM +(6 * ucii)];
			uiVal1 = uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI +(6 * ucii)],
				 	NO_NOINT
				 	);
			uiVal2 = uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI+3 +(6 * ucii)],
				 	NO_NOINT
				 	);
			vSENSOR_showSensorName(ucSensorNum, L_JUSTIFY);
			vSERIAL_rom_sout(" = ");
			vSERIAL_HBV16out(uiVal1);
			vSERIAL_bout('.');
			vSERIAL_HB16out(uiVal2);
			vSERIAL_crlf();

			}/* END: for(ucii) */

		if(ucSERIAL_kbhit()) break;

		}/* END: while() */

	return(ucRetVal);

	}/* END: ucFULLDIAG_SENSOR_5() */







/**********************  ucFULLDIAG_BUZ_0()  *********************************
*
* "BUZ_0: Let operator hear sounds
*
*****************************************************************************/

uchar ucFULLDIAG_BUZ_0(
		void
		)
	{
	uchar ucRetVal;
	uchar ucReply;

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(cpBUZ_FuncDesc[0]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC */
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_rom_sout(AutoTestExitStr);
		return(ucRetVal);
		}

	ucRetVal = DIAG_ACTION_KEY_EXIT;


Buz_0_1:
	vSERIAL_rom_sout("SysStart\r\n");
	vBUZ_tune_imperial();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_1;		//repeat

Buz_0_2:
	vSERIAL_rom_sout("NewRdioLnk\r\n");
	vBUZ_tune_TaDah_TaDah();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_2;		//repeat

Buz_0_3:
	vSERIAL_rom_sout("BattLo\r\n");
	vBUZ_tune_bad_news_1();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_3;		//repeat

Buz_0_4:
	vSERIAL_rom_sout("NoFramFmt\r\n");
	vBUZ_tune_bad_news_2();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_4;		//repeat

Buz_0_5:
	vSERIAL_rom_sout("FramFail\r\n");
	vBUZ_morrie();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_5;		//repeat

Buz_0_6:
	vSERIAL_rom_sout("SlntRdio\r\n");
	vBUZ_raspberry();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_6;		//repeat

Buz_0_7:
	vSERIAL_rom_sout("NoRdioBrd\r\n");
	vBUZ_raygun_up();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_7;		//repeat

Buz_0_8:
	vSERIAL_rom_sout("FlshFail\r\n");
	vBUZ_raygun_down();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_8;		//repeat

Buz_0_9:
	vSERIAL_rom_sout("SD-Fail\r\n");
	vBUZ_scale_down();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_9;		//repeat


Buz_0_x:
	return(ucRetVal);

	}/* END: ucFULLDIAG_BUZ_0() */







/**************** ucFULLDIAG_exit_next_repeat() ******************************
*
* RET: 	0 = exit
*		1 = next
*		2 = repeat
*		everything else = next
*
******************************************************************************/
uchar ucFULLDIAG_exit_next_repeat(
		void
		)
	{
	uchar ucChar;

	vSERIAL_rom_sout("X-it,N-ext,R-epeat...");
	ucChar = ucSERIAL_bin();
	vSERIAL_bout(ucChar);
	vSERIAL_crlf();
	ucChar = ucSERIAL_toupper(ucChar);
	if(ucChar == 'X') return(0);
	if(ucChar == 'Q') return(0);
	if(ucChar == 'R') return(2);

	return(1);

	}/*END: ucFULLDIAG_exit_next_repeat() */





/*-------------------------------  MODULE END  ------------------------------*/

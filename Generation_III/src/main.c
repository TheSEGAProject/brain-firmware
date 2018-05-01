
/**************************  MAIN.C  ******************************************
*
* Main routine for wizard.
*
******************************************************************************/

#define THIS_MAIN_VERSION 0x04
#define THIS_SUB_VERSION  0x00

#define THIS_VERSION_SUBCODE	' '		//debug codes
//#define THIS_VERSION_SUBCODE	'a'		//debug codes
//#define THIS_VERSION_SUBCODE	'b'		//debug codes
//#define THIS_VERSION_SUBCODE	'c'		//debug codes
//#define THIS_VERSION_SUBCODE	'd'		//debug codes


/*lint -e526 */		/* function not defined */
/*lint -e563 */		/* label not referencecd */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e716 */		/* while(1) ... */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


#include "std.h"			//std defines
#include "diag.h"			//Diagnostic package header
//#include "fulldiag.h"		//full system diagnostic routines
#include "config.h"			//system configuration definitions
#include <msp430x54x.h>		//register and ram definition file
#include "main.h"			//main defines
#include "radio.h"			//radio C portions
//#include "ad.h"  			//internal A/D
#include "delay.h"			//approx delay routine
#include "misc.h"			//homeless routines
//#include "sram.h"			//static ram routines
#include "l2sram.h"			//SRAM message storage handler
#include "rts.h"			//Real Time Sched routines
#include "rand.h"			//random number generator
#include "action.h"			//event action module
#include "key.h"			//keyboard handler
#include "serial.h"			//serial port
//#include "discover.h"		//discovery routines
#include "time.h"			//System Time routines
#include "daytime.h"		//Daytime routines
//#include "flash.h"  		//FLASH memory handler routines
#include "buz.h"			//Buzzer routines
#include "button.h" 		//Button routines
#include "sdcom.h"  		//SD communication routines
#include "sdctl.h"			//SD board power control
#include "sdspi.h"  		//MASTER COMMUNICATION CODE
//#include "spi.h"			//SPI handler routines
//#include "fram.h"			//FRAM handler routines
#include "l2fram.h"			//Level 2 FRAM handler routines
#include "l2flash.h"		//Level 2 FLASH handler routines
#include "report.h"			//Reporting routines
#include "msg.h"			//msg helper routines
#include "gid.h"			//Group ID routines
#include "stbl.h"			//Schedule table routines
#include "sensor.h"			//sensor routines
#include "pick.h"			//Trigger routines
#include "MODOPT.h"			//Modify Options routines
#include "LNKBLK.h"			//radio link handler routines


#ifdef DEBUG_DISPATCH_TIME
 #include "t0.h"			//Timer T0 routines
#endif

#include "crc.h"			//CRC calculator routine

#ifdef ESPORT_ENABLED				//defined in diag.h
 #include "esport.h"			//external serial port
#endif



/****************************  DEFINES  **************************************/



//#define KILL_ALL_CHECKS TRUE
#ifndef KILL_ALL_CHECKS
  #define KILL_ALL_CHECKS FALSE
#endif


//#define DEBUG_DISPATCH_TIME 1




/**********************  VOLATILE  GLOBALS  **********************************/

 volatile uchar ucaMSG_BUFF[ MAX_RESERVED_MSG_SIZE ];

 volatile uchar ucaX0FLD[ MSG_XFLDSIZE ];
 volatile uchar ucaX1FLD[ MSG_XFLDSIZE ];

 volatile uchar ucALARM_TIME[4];
 volatile uchar ucCLK_TIME[4];
 volatile uchar ucCLK2_TIME[4];
 volatile uchar ucBUTTON_COUNT[4];

#ifdef INC_ESPORT						//defined on Cmd line
 volatile uchar ucESPORT_TX_Byte;		//transmit byte ram
 volatile uchar ucESPORT_RX_Byte;		//receive byte ram
 volatile uchar ucESPORT_BitCounter;	//
 volatile uchar ucESPORT_TimeCounter;	//
#endif /* END: INC_ESPORT */

 volatile uchar ucaSDSPI_BUFF[ MAX_SDSPI_SIZE ];

 volatile uchar ucaBigMinuend[6];
 volatile uchar ucaBigSubtrahend[6];
 volatile uchar ucaBigDiff[6];

 volatile uchar ucRAND_NUM[RAND_NUM_SIZE];

 volatile uchar ucaCommQ[COMM_Q_SIZE];

 volatile uchar ucB3TEMP;			//used in ONEWIREA & RANDA

 volatile uchar ucINT_TEMP1;
 volatile uchar ucINT_TEMP2;
 volatile uchar ucINT_TEMP3;
 volatile uchar ucINT_TEMP4;

 volatile uchar ucQonIdx_LUL;
 volatile uchar ucQoffIdx_LUL;
 volatile uchar ucQcount;

 volatile union				//ucFLAG0_BYTE
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
												//		In a Hub when it is reset.
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

#define FLAG0_INIT_VAL	    0x00		//0000 0000



 volatile union							//ucFLAG1_BYTE
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

#define FLAG1_INIT_VAL	  0x00  	//0000 0000



 volatile union							//ucFLAG2_BYTE
  	{
  	uchar byte;

  	struct
  	 {
  	 unsigned FLG2_T3_ALARM_MCH_BIT:1;		//bit 0 ;1=T3 Alarm, 0=no alarm
  	 unsigned FLG2_T1_ALARM_MCH_BIT:1;		//bit 1 ;1=T1 Alarm, 0=no alarm
  	 unsigned FLG2_BUTTON_INT_BIT:1;		//bit 2 ;1=XMIT, 0=RECEIVE
  	 unsigned FLG2_CLK_INT_BIT:1;			//bit 3	;1=clk ticked, 0=not
  	 unsigned FLG2_X_FROM_MSG_BUFF_BIT:1;	//bit 4
  	 unsigned FLG2_R_BUSY_BIT:1;			//bit 5 ;int: 1=REC BUSY, 0=IDLE
  	 unsigned FLG2_R_BARKER_ODD_EVEN_BIT:1;	//bit 6 ;int: 1=odd, 0=even
  	 unsigned FLG2_R_BITVAL_BIT:1;			//bit 7 ;int: 
  	 }FLAG2_STRUCT;

  	}ucFLAG2_BYTE;

#define FLAG2_INIT_VAL	0x00	//00000000




 volatile union						//ucGLOB_diagFlagByte1
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

#define DIAG_FLAG_BYTE_1_INIT_VAL 0x00	//00000000


  volatile union						//ucGLOB_diagFlagByte2
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

#define DIAG_FLAG_BYTE_2_INIT_VAL 0x00	//00000000




/*---------  END of VOLATILES  -----------------------------*/







 /***********************  NON-VOLATILE GLOBALS  *****************************/

 uchar ucGLOB_myLevel;				//senders level +1

 long lGLOB_initialStartupTime;		//Time used to compute uptime

 long lGLOB_lastAwakeTime;			//Nearest thing to slot time
 long lGLOB_opUpTimeInSec;			//Nearest thing to cur operational up time
 long lGLOB_lastAwakeLinearSlot;	//Nearest thing to cur linear slot a
 long lGLOB_lastAwakeFrame;			//Nearest thing to cur frame
 uchar ucGLOB_lastAwakeSlot;		//Nearest thing to cur slot
 uchar ucGLOB_lastAwakeNSTtblNum;	//Nearest thing to cur NST tbl
 uchar ucGLOB_lastAwakeStblIdx;		//Nearest thing to cur sched idx 

 long lGLOB_lastScheduledFrame;		//last scheduled frame number
 uchar ucGLOB_lastScheduledSchedSlot;	//slot for sched in last scheduled frame

 uchar ucGLOB_SDC4StblIdx;		//Sched tbl idx for SDC4  function
 uchar ucGLOB_RDC4StblIdx;		//Sched tbl idx for RDC4  function
 uchar ucGLOB_TC12StblIdx;		//DEBUG: Sched tbl idx for TC12
 uchar ucGLOB_TC34StblIdx;		//DEBUG: Sched tbl idx for TC34
 uchar ucGLOB_LT12StblIdx;		//DEBUG: Sched tbl idx for LT12
 uchar ucGLOB_LT34StblIdx;		//DEBUG: Sched tbl idx for LT34

 long lGLOB_OpMode0_inSec;			//Start of Opmode

// ulong ulGLOB_sramTblPtr_LUL;		//grows up,  init = end to sched tbl section
// uint  uiGLOB_sramMsgCount;

 usl uslGLOB_sramQon_NFL;
 usl uslGLOB_sramQoff;
 uint uiGLOB_sramQcnt;

 uchar ucGLOB_curMsgSeqNum;

 int iGLOB_Hr0_to_SysTim0_inSec;	//dist from SysTim0 to Hr0
//uint uiGLOB_WrldStartHr;			//Starting hour in WorldTime

 uint uiGLOB_grpID;					//group ID for this group
 uchar ucGLOB_StblIdx_NFL;			//next free loc in the sched tables

 uchar ucGLOB_lineCharPosition;		//line position for computing tabulation

 int iGLOB_completeSysLFactor;		//entire Signed LFactor quantity.

 
// uchar ucGLOB_msgSysLFactor;		//Message (byte size) unsigned Load Factor
 uchar ucGLOB_msgSysLnkReq;			//Msg Linkup Req


 uchar ucGLOB_radioChannel;			//Current radio channel number (0 - 127)


 /*----------  DEBUG RAM LOCATIONS  ---------*/
 uchar ucGLOB_testByte;				//counts thermocouple onewire dropouts
 uchar ucGLOB_testByte2;			//counts button return type 2 errors

 uint uiGLOB_bad_flash_CRC_count;	//count of bad CRC's on flash msgs

 uint uiGLOB_lostROM2connections;	//total lost ROM2's, Zro on startup
 uint uiGLOB_lostSOM2connections;	//total lost SOM2's, Zro on startup

 uint uiGLOB_ROM2attempts;			//total ROM2 attempts, Zro on startup
 uint uiGLOB_SOM2attempts;			//total SOM2 attempts, Zro on startup

 uint uiGLOB_TotalSDC4trys;			//total SDC4 attempts, Zro on startup
 uint uiGLOB_TotalRDC4trys;			//total RDC4 attempts, Zro on startup

 union			//ucGLOB_debugBits1
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

 #define DEBUG_BITS_1_INIT_VAL 0x00	//00000000


 /* RAM COPY OF FRAM OPTION BIT ARRAY */
 uchar ucaGLOB_optionBytes[OPTION_BYTE_COUNT];




//extern const ulong ulaSenseActVal[E_ACTN_MAX_NUM_IDX];


/******************************  DECLARATIONS  *******************************/


static void vMAIN_sendRadioStartupMsg(
		void
		);

void vMAIN_computeDispatchTiming(
		void
		);


#ifdef RUN_NOTHING
static void vMAIN_run_nothing(
		void
		);
#endif



void vMAIN_showCompilerAnomalies(
		const char *cMsg
		);




/*******************************  CODE  **************************************/




/*******************************  MAIN  *************************************
*
*
*
*
******************************************************************************/

void main(
		void
		)
	{
	WDTCTL = WDTPW+WDTHOLD;					// Stop WDT
	__bic_SR_register(GIE);					// disable global interrupts

	// SET INITIAL VALUE ON PORTS 
	PAOUT = PAOUT_INIT_VAL;
	PBOUT = PBOUT_INIT_VAL;
	PCOUT = PCOUT_INIT_VAL;
	PDOUT = PDOUT_INIT_VAL;
	PEOUT = PEOUT_INIT_VAL;
	PFOUT = PFOUT_INIT_VAL;

	// SET PORT DIRECTIONS
	PADIR = PADIR_INIT_VAL;
	PBDIR = PBDIR_INIT_VAL;
	PCDIR = PCDIR_INIT_VAL;
	PDDIR = PDDIR_INIT_VAL;
	PEDIR = PEDIR_INIT_VAL;
	PFDIR = PFDIR_INIT_VAL;

	// SET PORT PULL UP/DOWN RESISTOR 
	PAREN = PAREN_INIT_VAL;
	PBREN = PBREN_INIT_VAL;
	PCREN = PCREN_INIT_VAL;
	PDREN = PDREN_INIT_VAL;
	PEREN = PEREN_INIT_VAL;
	PFREN = PFREN_INIT_VAL;
	
	// SET PORT FUNCTION I/O OR PERIPHERAL
	PASEL = PASEL_INIT_VAL;
	PBSEL = PBSEL_INIT_VAL;
	PCSEL = PCSEL_INIT_VAL;
	PDSEL = PDSEL_INIT_VAL;
	PESEL = PESEL_INIT_VAL;
	PFSEL = PFSEL_INIT_VAL;
	
	// SET PORTA INTERRUPT ENABLES
	PAIE = PAIE_INIT_VAL;
	
	// SET PORTA INTERRUPT EDGE (0 = RISING EDGE, 1 = FALLING)
	PAIES = PAIES_INIT_VAL;

	// Setup XT1, MCLK, SMCLK, ACLK
	UCSCTL6 &= ~(XT1OFF);					// XT1 On
	UCSCTL6 |= XCAP_3;						// Internal load cap
	// Loop until XT1,XT2 & DCO stabilizes
	do
	{
		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);	// Clear XT2,XT1,DCO fault flags
		SFRIFG1 &= ~OFIFG;					// Clear fault flags
	}while (SFRIFG1&OFIFG);					// Test oscillator fault flag
	UCSCTL6 &= ~(XT1DRIVE_3);				// XT1 is now stable, reduce drive strength
	
	UCSCTL1 = DCORSEL_16MHZ;				// Select DCO range
	UCSCTL2 = DCO_MULT_16MHZ;				// Set DCO Multiplier
	UCSCTL4 = SELM__DCOCLK + SELS__DCOCLK + SELA__XT1CLK;	// Select sources MCLK, SMCLK, ACLK
	UCSCTL5 = DIVS__4;						// Select source divide MCLK = 0, SMCLK = 4, ACLK = 0
	
	LED_PORT |= LED_BIT;					// init debug bit	

	__bis_SR_register(GIE);					// enable global interrupts
	
	/*----------------  RAM MEMORY INIT  ------------------------------------*/
	ucFLAG0_BYTE.byte = FLAG0_INIT_VAL;
	ucFLAG1_BYTE.byte = FLAG1_INIT_VAL;
	ucFLAG2_BYTE.byte = FLAG2_INIT_VAL;
	ucGLOB_curMsgSeqNum = 1;
	ucGLOB_diagFlagByte1.byte = DIAG_FLAG_BYTE_1_INIT_VAL;
	ucGLOB_diagFlagByte2.byte = DIAG_FLAG_BYTE_2_INIT_VAL;
	ucGLOB_lineCharPosition = 0;
	iGLOB_completeSysLFactor = 0;
//	ucGLOB_msgSysLFactor = 0;
	ucGLOB_StblIdx_NFL = 0;
	ucGLOB_radioChannel = ucRADIO_getChanAssignment(ILLEGAL_CHANNEL_INDEX);

	ucGLOB_testByte = 0;			//counts thermocouple onewire dropouts
  	ucGLOB_testByte2 = 0;			//counts button push type 2 errors

	uiGLOB_bad_flash_CRC_count = 0;	//count of bad CRC's on flash retrieve

	uiGLOB_lostROM2connections = 0;	//counts lost ROM2's
	uiGLOB_lostSOM2connections = 0;	//counts lost SOM2's

	uiGLOB_ROM2attempts = 0;		//count number of ROM2 attempts
	uiGLOB_SOM2attempts = 0;		//count number of SOM2 attempts

	uiGLOB_TotalSDC4trys = 0;		//counts number of SDC4 attempts
	uiGLOB_TotalRDC4trys = 0;		//counts number of RDC4 attempts

	ucGLOB_debugBits1.byte = 0;		//debug flags

	/* INIT THE TABLE ENTRYS TO ILLEGAL VALUES */
	ucGLOB_SDC4StblIdx = 255; 	//Sched tbl idx for SDC4  function
	ucGLOB_RDC4StblIdx = 255; 	//Sched tbl idx for RDC4  function
	ucGLOB_TC12StblIdx = 255; 	//DEBUG: Sched tbl idx for TC12
	ucGLOB_TC34StblIdx = 255; 	//DEBUG: Sched tbl idx for TC34
	ucGLOB_LT12StblIdx = 255; 	//DEBUG: Sched tbl idx for LT12
	ucGLOB_LT34StblIdx = 255; 	//DEBUG: Sched tbl idx for LT34

	/* SETUP THE ucaGLOB_roleByte[]	ARRAY */
	vMODOPT_copyAllFramOptionsToRamOptions();

	/*----------------  END: RAM MEMORY INIT  ------------------------------------*/



	/*------------------- START: FUNCTION INIT  -----------------------------*/
	vBUTTON_init();					//setup the button

	#ifdef RUN_NOTHING
	vMAIN_run_nothing();			//runs before serial_init to avoid delay
	#endif

	vSERIAL_init();					//setup the serial port

	vSERIAL_rom_sout("\r\n0,");	/*------------------ 0 -----------------------*/

	/*--------------------  REPORT ANOMALIES  -------------------------------*/

	#ifndef SERIAL_Q_OUTPUT_ENABLED
	vMAIN_showCompilerAnomalies("  SERIAL PORT IS POLLED  ");
	#endif

	#if 1
	if(ucTIME_getECLKsts(YES_SKP_ECLK_RAM_FLAG) >= 3)
		{
		vSERIAL_rom_sout("\r\nECLKfnd\r\n");
		}
	vTIME_init();					//setup the clk
	#else
	vMAIN_showCompilerAnomalies("  TIME_INIT() IS COMMENTED OUT  ");
	#endif

	#ifdef ESPORT_ENABLED				//defined in diag.h
	vMAIN_showCompilerAnomalies("  ESPORT ENABLED  ");
	#endif


	/*--------------------  INIT ESPORT  ------------------------------------*/
	#ifdef ESPORT_ENABLED				//defined in diag.h
	vESPORT_init();
	#endif

	/*--------------------  CHECK FOR BROWN OUT -----------------------------*/
	while(TRUE)
		{
		if(uiMISC_doCompensatedBattRead() > 3800) break;	//above 3.8V is good

		vSERIAL_rom_sout("BattLow= ");
		vACTION_showBattReading();
		vSERIAL_crlf();

		vBUZ_tune_bad_news_1();	//Battery Low Sound   -  sounds should change!

		vTIME_setAlarmFromLong(lTIME_getSysTimeAsLong()+4L);
		ucMISC_sleep_until_button_or_clk(SLEEP_MODE);		//lint !e534

		}/* END: while() */

	vSERIAL_rom_sout("Batt= ");
	vACTION_showBattReading();
	vSERIAL_crlf();


	vSERIAL_rom_sout("\r\n1,");	/*------------------ 1 ----------------------*/
	vL2SRAM_init();

	#ifdef DEBUG_DISPATCH_TIME
	vT0_init();
	#endif

	/* SET THE LINK LEVEL */
	ucGLOB_myLevel = LEVEL_MAX_VAL;
	if(ucL2FRAM_isHub())						//Hub init for level & group
		{
		/* INIT THE LEVEL FOR THE HUB */
		ucGLOB_myLevel = 0;						//init level

		/* INIT GID-SELECTOR */
		vGID_init();
		}

	#if 0		//DEBUG
	vSERIAL_rom_sout("\r\nTEST ROUTINES\r\n");
	if(ucL2FRAM_isHub()) vSERIAL_rom_sout("IS HUB\r\n");
	if(ucL2FRAM_isSender()) vSERIAL_rom_sout("IS SENDER\r\n");
	if(ucL2FRAM_isSampler()) vSERIAL_rom_sout("IS SAMPLER\r\n");
	if(ucL2FRAM_isReceiver()) vSERIAL_rom_sout("IS RECEIVER\r\n");
	#endif		//END: DEBUG 



	vSERIAL_rom_sout("2,");		/*------------------- 2 ---------------------*/


	/* INIT THE SD BOARD */
	if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_SD_BD_IS_ATTCHD))
		{
		vSERIAL_rom_sout("SDstart\r\n");
		ucSDCTL_start_SD_and_specify_boot(SD_BOOT_LOADER_DISABLED); //lint !e534

		} 
		/* END: if(OPTPAIR_SD_BD_IS_ATTCHD) */



	vSERIAL_rom_sout("3,");		/*------------------- 3 ---------------------*/



   	/* INCREMENT THE STARUP COUNTER */
	vL2FRAM_incRebootCount();			//must be after SD init



	vSERIAL_rom_sout("4,");		/*------------------- 4 ---------------------*/



	/* RUN EARLY DIAGNOSTICS */
//	vDIAG_run_diagnostics();     // This was commented out before port began


	vSERIAL_rom_sout("5,");		/*------------------- 5 ---------------------*/


	/* TELL THAT SYS IS RUNNING */
	vMAIN_printIntro();

	/* BLINK TO LET THEM KNOW WE ARE ALIVE */
	vMISC_blinkLED(1);
	vBUZ_blink_buzzer(1);


	vSERIAL_rom_sout("6,");		/*-------------------- 6 --------------------*/



	vSERIAL_rom_sout("7,");		/*-------------------- 7 --------------------*/




#if 0		//special amptest  this section will need change! if you want to use it.
	{
	uchar ucLEDval;

	vBUZ_buggs();
	vSDCTL_set_SD_power_bit(SD_POWER_ON);
	vTIME_setAlarmFromLong(0x70000000);		//set sleep = forever

	ucLEDval = LED_ON;
	while(TRUE)			//lint !e774
		{
		#if 1
		//TRISFbits.E_CLK_OWIRE_DATA_BIT = 0;	//force onewire to output
		TRISFbits.E_CLK_OWIRE_DATA_BIT = 1;	//force onewire to input
		#endif

		#if 0
		vSERIAL_bout('+');
		#endif

		if(ucLEDval == LED_ON) ucLEDval = LED_OFF; else ucLEDval = LED_ON;
		#if 0
		vMISC_setLED(ucLEDval);
		#endif

		#if 0
		continue;							//RUN pwr usage
		#endif

		#if 0								//SLEEP pwr usage
		ucMISC_sleep_until_button_or_clk(SLEEP_MODE); //lint !e534
		continue;
		#endif

											//HIBERNATE pwr usage
		ucMISC_sleep_until_button_or_clk(HIBERNATE_MODE); //lint !e534

		}/* END: while(TRUE) */

	}/* END: special amptest */
#endif		//special amptest



	/* CHECK IF WE ARE SUPPOSED TO HIBERNATE */

	if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_DO_HBRNAT_CHK))
		{
		uchar ucHibernateFlag;
		long lStartTime;

		/* BLINK TWICE TO ASK FOR HIBERNATION BUTTON PUSH */
		/* WAIT 3/4 SEC TO MAKE THE BEEPS CLEAR */

		vDELAY_wait100usTic(7500);
		vMISC_blinkLED(2);
		vBUZ_blink_buzzer(2);
	
		/* NOW WAIT FOR 5 SECONDS -- IF NO BUTTON PUSH THEN START SYS CHECKS */
		lStartTime = lTIME_getSysTimeAsLong();
		ucHibernateFlag = 0;					//assume no hibernation

		while((lTIME_getSysTimeAsLong() - lStartTime) < 5)
			{
			if(ucBUTTON_checkForLongButtonPush())
				{
				ucHibernateFlag = 1;
				break;
				}
			}/* END: while() */

		if(ucHibernateFlag)
			{
			if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_ALLOW_BRAIN_STUP_HBRNAT))
				{

			    /* TELL THE OPERATOR THAT WE ARE HIBERNATING */
				vMISC_blinkLED(3);
				vBUZ_blink_buzzer(3);

				/* ANNOUNCE WHAT IS GOING ON */
				vSERIAL_rom_sout("Hibrnat...\r\n");	
				vTIME_setAlarmFromLong(0x70000000);		//sleep until button push


				/* WAIT HALF A SECOND TO MAKE SURE THE BUTTON IS OFF */
				vDELAY_wait100usTic(5000);

				/* HIBERNATE */

				vSERIAL_rom_sout("EntHibrnat..\r\n");	
				ucMISC_sleep_until_button_or_clk(HIBERNATE_MODE); //lint !e534

				vSERIAL_rom_sout("BTTN\r\n"); 
				ulBUTTON_readAndClrButtonCount();	//lint !e534

				}/* END: if(OPTPAIR_ALLOW_BRAIN_STUP_HBRNAT) */
				
			}/* END: if(ucHibernateFlag) */

		}/* END: if(OPTPAIR_DO_HBRNAT_CHK) */






	vSERIAL_rom_sout("8,");		/*-------------------- 8 --------------------*/









#if (KILL_ALL_CHECKS == FALSE)

/******************  SUB-SYSTEM CHECKS  ************************************/

	vSERIAL_rom_sout("9,");		/*-------------------- 9 --------------------*/


	/* POWER UP THE SD BOARD */
	/* NOTE: This has to be outside the CHECK_FOR_SD_ON_STARTUP define */
	/* 		 because the check for flash and fram require the SD to be up */


	if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_SD_BD_IS_ATTCHD))
		{
		vSERIAL_rom_sout("10,");	/*---------------- 10 -------------------*/

//		vDIAG_run_diagnostics();		//fails

		if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_CHK_FOR_SD_ON_STUP))
			{
			/* READ SD MSG VERSION */
			while(TRUE)			//lint !e774
				{
				if(ucSDCOM_awaken_SD_and_do_full_msg_transfer(SENSOR_SD_MSG_VERSION) == 0)
					{
					vSERIAL_rom_sout("BdSDmsgTransfer\r\n");
					vBUZ_scale_down();	//SD Failed
					vSERIAL_rom_sout("Rtry\r\n");
					continue;
					}

				if((ucaSDSPI_BUFF[3] != SD_MSG_VERSION_HI) || (ucaSDSPI_BUFF[4] != SD_MSG_VERSION_LO))
					{
					vSERIAL_rom_sout("WrongSDmsgVer\r\n");
					vBUZ_scale_down();  //SD Bad Ver
					vSERIAL_rom_sout("Rtry\r\n");
					continue;
					}
				break;

				}/* END: while() */

			}/* END: if(OPTPAIR_CHK_FOR_SD_ON_STUP) */

		}/* END: if(OPTPAIR_SD_BD_IS_ATTCHD) */






	vSERIAL_rom_sout("11,");	/*------------------ 11 ---------------------*/

	/* CHECK IF THE FRAM IS ONBOARD AND WORKING */
	if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_CHK_FOR_FRAM_ON_STUP))
		{
		while(!ucL2FRAM_chk_for_fram(FRAM_CHK_REPORT_MODE))
			{
			vSERIAL_rom_sout("FramFail\r\n");
			vBUZ_morrie();  //Fram Failed 
			vSERIAL_rom_sout("Rtry\r\n");

			}/* END: while() */

		}/* END: if(OPTPAIR_CHK_FOR_FRAM_ON_STUP) */






	vSERIAL_rom_sout("12,");	/*------------------- 12 --------------------*/

	/* CHECK IF THE FRAM IS FORMATTED */

//	vL2FRAM_force_fram_unformat();			//debug

	if(!uiL2FRAM_chk_for_fram_format())
		{
		long lBegTime;
		uchar ucChar;

		/* WUPS FRAM IS NOT FORMATTED -- CHECK IF WE SHOULD FORMAT IT*/
		ucChar = 0;
		vBUZ_tune_bad_news_2(); //Fram not formatted
		vSERIAL_rom_sout("\r\nFramNotFmted--LeaveIt? [YorN] ");
		lBegTime = lTIME_getSysTimeAsLong();
		while((lTIME_getSysTimeAsLong() - lBegTime) < 10)
			{
			if(ucSERIAL_kbhit())
				{
				ucChar = ucSERIAL_bin();
				vSERIAL_bout(ucChar);
				break;
				}
			}/* END: while() */

		vSERIAL_crlf();

		/* FORMAT THE FRAM */
		if((ucChar != 'Y') && (ucChar != 'y'))
			{
			vSERIAL_rom_sout("Fmting\r\n");
			vL2FRAM_format_fram();
			}

		}/* END: if() */

	vSERIAL_rom_sout("12a,");	/*----------------- 12a ---------------------*/
	vL2FRAM_showStBlkTbl();





	vSERIAL_rom_sout("13,");	/*------------------ 13 ---------------------*/

	/* CHECK IF FLASH IS ONBOARD AND WORKING */
	if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_SD_BD_IS_ATTCHD))
		{
		while(!ucL2FLASH_chkForFlash(FLASH_CHK_REPORT_MODE, YES_INITQUIT))
			{
			vSERIAL_rom_sout("FlshFail\r\n");
			vBUZ_raygun_down(); //Flash Failed
			vSERIAL_rom_sout("Rtry\r\n");
			}
		}/* END: if(OPTPAIR_SD_BD_IS_ATTCHD) */




	vSERIAL_rom_sout("14,");	/*------------------- 14 --------------------*/

	/* CHECK IF THE RADIO BOARD IS CONNECTED */
	if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_CHK_FOR_RDIO_BD))
		{
		while(!ucRADIO_chk_for_radio_board())
			{
			vSERIAL_rom_sout("NoRdioBrd\r\n");
			vBUZ_raygun_up(); //No Radio Board
			vSERIAL_rom_sout("Rtry\r\n");
			}/* END: while() */

		} /* END: if() */


	vSERIAL_rom_sout("15,");	/*------------------- 15 --------------------*/


#endif /* KILL_ALL_CHECKS */





/****************  START RUNNING THE INITIAL SYSTEM  *************************/


	/* NOW SETUP THE RADIO AND THEN SHUT IT DOWN */
	vRADIO_init();

	/* INT RANDOM NUMBERS */
	uslRAND_getNewSeed();	//lint !e534 //stuff a new seed into rand array

	#if 0
	vSERIAL_rom_sout("Rand= ");
	vSERIAL_HB24out(uslRAND_getFullSysSeed());
	vSERIAL_crlf();
	#endif


	vSERIAL_rom_sout("16,");	/*------------------- 16 --------------------*/


	/* REPORT THE SYSTEM RESTARTING */
	vREPORT_buildReportAndLogIt(
			SENSOR_BR_BOARD_MSG,			//Sensor 0 num
			BRAIN_RESTART,					//Sensor 0 Data
			0,								//Sensor 1 num
			0,								//Sensor 1 Data
			OPTPAIR_RPT_SYS_RSTRTS_TO_RDIO,	//Radio OptionPairIdx
			OPTPAIR_RPT_SYS_RSTRTS_TO_FLSH 	//Flash OptionPairIdx
			);


	/* TELL THE WORLD THAT WE ARE ALIVE */
	vMISC_blinkLED(5);

	vBUZ_tune_imperial();

	vMAIN_printIntro();


	vSERIAL_rom_sout("17,");	/*------------------- 17 --------------------*/


	if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_SND_RDIO_STUP_MSG))
		{
		/* SEND THE STARTUP PACKET SO LISTENERS WILL KNOW WE'RE UP */
		uchar uc;

		for(uc=0;  uc<3; uc++)
			{
			vMAIN_sendRadioStartupMsg();

			vDELAY_wait100usTic(2000);				//wait 200ms

			}/* END: for() */
		}


	vSERIAL_rom_sout("18,");	/*------------------- 18 --------------------*/


	/* RUN THE DIAGNOSTICS AT THIS POINT */
	vDIAG_run_diagnostics();


	/* SHOW THE CURRENT ROLE PARAMETERS */
	vMODOPT_showAllRamOptionBits();



	/************ OPMODE INIT  ***********/
	{
	/* SETUP THE SCHEDULER TABLES */
	vSTBL_setupInitialStbls();

	/* CLEAR THE LNKBLK TABLE */
	vLNKBLK_zeroEntireLnkBlkTbl();

	/* SETUP THE TRIGGER TABLES */
	vPICK_initSSPtbls();

	/* CLEAR BOTH NST TBLS */
	vRTS_clrNSTtbl(0);
	vRTS_clrNSTtbl(1);

	/* INIT THE CURRENT FRAME COUNTER */
	lGLOB_OpMode0_inSec = 3;	//first frame start = 4, 2nd frame st = 260

	lGLOB_lastAwakeTime = lTIME_getSysTimeAsLong();
	lGLOB_opUpTimeInSec = lGLOB_lastAwakeTime - lGLOB_OpMode0_inSec;
	lGLOB_lastAwakeLinearSlot = lGLOB_opUpTimeInSec / SECS_PER_SLOT_L;
	lGLOB_lastAwakeFrame = lGLOB_lastAwakeLinearSlot / SLOTS_PER_FRAME_I;
	ucGLOB_lastAwakeSlot = (uchar)(lGLOB_lastAwakeLinearSlot % SLOTS_PER_FRAME_I);
	ucGLOB_lastAwakeNSTtblNum = (uchar)(lGLOB_lastAwakeFrame % 2);

	#if 1
	vSERIAL_rom_sout("lGLOB_OpMode0_inSec= ");
	vSERIAL_IV32out(lGLOB_OpMode0_inSec);
	vSERIAL_crlf();

	vSERIAL_rom_sout("lGLOB_lastAwakeTime= ");
	vSERIAL_IV32out(lGLOB_lastAwakeTime);
	vSERIAL_crlf();

	vSERIAL_rom_sout("lGLOB_opUpTimeInSec= ");
	vSERIAL_IV32out(lGLOB_opUpTimeInSec);
	vSERIAL_crlf();

	vSERIAL_rom_sout("lGLOB_lastAwakeLinearSlot= ");
	vSERIAL_IV32out(lGLOB_lastAwakeLinearSlot);
	vSERIAL_crlf();

	vSERIAL_rom_sout("lGLOB_lastAwakeFrame= ");
	vSERIAL_IV32out(lGLOB_lastAwakeFrame);
	vSERIAL_crlf();

	vSERIAL_rom_sout("ucGLOB_lastAwakeSlot= ");
	vSERIAL_UIV8out(ucGLOB_lastAwakeSlot);
	vSERIAL_crlf();

	vSERIAL_rom_sout("ucGLOB_lastAwakeNSTtblNum= ");
	vSERIAL_UIV8out(ucGLOB_lastAwakeNSTtblNum);
	vSERIAL_crlf();
	vSERIAL_crlf();

	#endif


	/* SCHEDULE THE CURRENT NST */
	vRTS_scheduleNSTtbl(lGLOB_lastAwakeFrame);

	#if 1
	vRTS_showAllNSTentrys(0,YES_SHOW_STBL);
	vRTS_showAllNSTentrys(1,YES_SHOW_STBL);
	#endif


	vSTBL_showStatsOnLFactor();				//debug this routine


//	/* SET THE ALARM TIME TO WAKE UP WHEN OP-MODE STARTS */
//	vTIME_setAlarmFromLong(lGLOB_OpMode0_inSec);
//
//	#if  1
//	vSERIAL_rom_sout("SleepTilOp..\r\n");	
//	#endif
//
//	/* NOW SLEEP UNTIL OPERATION MODE ARRIVES */
//	if(ucMISC_sleep_until_button_or_clk(SLEEP_MODE) == 1)
//		{
//		ucKEY_doKeyboardCmdInput();		//lint !e534
//		}
//

	/************  MAIN LOOP  *****************/

	#if  1
	/* SHOW THE DEBUG HEADER */
	vSERIAL_printDashIntro("OP MODE");	
	vTIME_showWholeSysTimeInDuS(YES_CRLF);
	#endif


	while(TRUE)			//lint !e716 !e774  //loop by slot
		{
	   #ifdef DEBUG_DISPATCH_TIME
		uchar ucaDispatchBegTime[6];
		uchar ucaDispatchEndTime[6];
		uchar ucaDispatchDiff[6];
		long lClkTime_inLtics;
		long lT0Time_inLtics;
		long lClk_T0_diff;
		long lAbsDiff;
	   #endif


		/*------------   COMPUTE THE DISPATCHER TIMING ----------------------*/
		vMAIN_computeDispatchTiming();


		#if 0
		/* SHOW THE DEBUG HEADER */
		vSERIAL_rom_sout("AfterDispatchTiming ");	
		vTIME_ShowWholeSysTimeInDuS(YES_CRLF);
		#endif



		#ifdef DEBUG_DISPATCH_TIME
		/* GET THE START TIME */
		vTIME_copyWholeSysTimeToBytesInDtics(&ucaDispatchBegTime[0]); //get the time before dispatch
//		vT0_start_T0_timer();
		#endif


		/*-------------  DISPATCH TO THE EVENT ------------------------------*/

		/* HANDLE THE DISPATCH */
		vACTION_dispatch_to_event(
							ucGLOB_lastAwakeNSTtblNum,
							ucGLOB_lastAwakeSlot
							);


		/*------------  CHECK FOR A BUTTON PUSH  ----------------------------*/
		if(ucBUTTON_isButtonFlgSet())
			{
			ulBUTTON_readAndClrButtonCount();	//lint !e534

		  #if 0
			if(ucSERIAL_kbhit()) ucSERIAL_bin();		//clr any key hit

			vSERIAL_rom_sout("HangHereChkingRdio\r\n");
			while(!ucSERIAL_kbhit());	//lint !e722
			ucSERIAL_bin();

			vSERIAL_rom_sout("DoingRdioInit\r\n");
			vRADIO_init();
			while(!ucSERIAL_kbhit());	//lint !e722
			ucSERIAL_bin();


			vSERIAL_rom_sout("RdioOff\r\n");
			vRADIO_quit();
			while(!ucSERIAL_kbhit());	//lint !e722
			ucSERIAL_bin();

			vSERIAL_rom_sout("cont..\r\n");
		  #endif

			ucKEY_doKeyboardCmdInput();		//lint !e534
			}


		#ifdef DEBUG_DISPATCH_TIME
		/* GET THE RETURN TIME */
		vTIME_copyWholeSysTimeToBytesInDtics(&ucaDispatchEndTime[0]); //get the time after dispatch
//		vT0_stop_T0_timer();

		//vTIME_showWholeTimeInDuS(&ucaDispatchBegTime[0], NO_CRLF);
		//vSERIAL_rom_sout(" -> ");
		//vTIME_showWholeTimeInDuS(&ucaDispatchEndTime[0], NO_CRLF);
		//vSERIAL_rom_sout(" = ");
		//vTIME_showWholeTimeDiffInDuS(&ucaDispatchBegTime[0], &ucaDispatchEndTime[0], YES_CRLF);

		/* COMPUTE CLK TIME DIFF IN LTICS */
		lClkTime_inLtics = (long)ulTIME_computeTimeDiffInLtics(
									&ucaDispatchBegTime[0],
									&ucaDispatchEndTime[0],
									&ucaDispatchDiff[0]
									);
		lT0Time_inLtics = (long)ulT0_get_val_in_Ltics();

		lClk_T0_diff = lClkTime_inLtics - lT0Time_inLtics;
		lAbsDiff = lClk_T0_diff;
		if(lAbsDiff < 0) lAbsDiff = -lAbsDiff;
		if(ucGLOB_lastAwakeStblIdx != SLEEP_STBL_IDX)
			{
			if(lAbsDiff > 100)
				{
				vSERIAL_rom_sout("TimJmpBetweenClk&T0\r\n");
				}
			vSERIAL_rom_sout("CLK= ");
			vSERIAL_IV32out(lClkTime_inLtics);
			vSERIAL_rom_sout("   T0= ");
			vSERIAL_IV32out(lT0Time_inLtics);
			vSERIAL_rom_sout("  = ");
			vSERIAL_IV32out(lClk_T0_diff);
			vSERIAL_crlf();
			}
		#endif


		}/* END: while(TRUE) */


	}/* END: MAIN_OPMODE_PROGRAM */


	vSERIAL_rom_sout("\r\nPROG XIT\r\n"); //lint !e527
	vSERIAL_quit();

Hang_here:
	goto Hang_here;

  }/* END: main() */








/***********************  vMAIN_showVersionNum()  *****************************
*
*
*
*
******************************************************************************/

void vMAIN_showVersionNum(
		void
		)
	{

	vSERIAL_bout('V');
	vSERIAL_HB4out(THIS_MAIN_VERSION);
	vSERIAL_bout('.');
	vSERIAL_HB8out(THIS_SUB_VERSION);
	#if (THIS_VERSION_SUBCODE != ' ')
	vSERIAL_bout(THIS_VERSION_SUBCODE);
	#endif

	return;

	}/* END: vMAIN_showVersionNum() */







/************************  vMAIN_printIntro()  ************************************
*
* Print out our introduction message
*
******************************************************************************/

void vMAIN_printIntro(
		void
		)
	{
	uchar ucECLKsts;

	/* ISSUE THE INTRO MESSAGE */
	vSERIAL_crlf();
	vDAYTIME_convertSysTimeToShowDateAndTime(TEXT_FORM);

	vSERIAL_rom_sout("\r\nWiz ");
	vSERIAL_rom_sout("("RUN_CODE_NAME") ");

	vMAIN_showVersionNum();
	vSERIAL_bout(' ');

	vMODOPT_showCurRole();
	vSERIAL_bout(':');
	vL2FRAM_showSysID();

	vSERIAL_rom_sout(" (SD-");

	if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_SD_BD_IS_ATTCHD))
		{
		if(ucSDCOM_awaken_SD_and_do_full_msg_transfer(SENSOR_SD_CODE_VERSION) == 0)
			{
			vSERIAL_rom_sout("NA)");
			}
		else
			{
			vSERIAL_rom_sout("V");
//			vSERIAL_UIV8out(ucaSDSPI_BUFF[3]);	//lint !e527 //SD MAIN Version Num
			vSERIAL_bout('.');
//			vSERIAL_HB8out(ucaSDSPI_BUFF[4]); //SD MAIN Sub Version Num
			vSERIAL_bout(')');

			/* SHOW THE SD MESSAGE VERSION */
			vSERIAL_rom_sout("(SDmsg-");

			if(ucSDCOM_awaken_SD_and_do_full_msg_transfer(SENSOR_SD_MSG_VERSION) == 0)
				{
				vSERIAL_rom_sout("NA)");
				}
			else
				{
				vSERIAL_rom_sout("V");
//				vSERIAL_UIV8out(ucaSDSPI_BUFF[3]);	//lint !e527 //Version Num
				vSERIAL_bout('.');
//				vSERIAL_HB8out(ucaSDSPI_BUFF[4]); 	//Sub Version Num
				vSERIAL_bout(')');
				}/* END: else if() */

			}/* END: else if() */

		}/* END: if(OPTPAIR_SD_BD_IS_ATTCHD) */
	else
		{
		vSERIAL_rom_sout("OFF)");

		}/* END: else (OPTPAIR_SD_BD_IS_ATTCHD) */



	/* TELL ABOUT THE FRAM */
	vSERIAL_rom_sout("(FRAM-");
	if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_CHK_FOR_FRAM_ON_STUP))
		{
		uint uiVal;

		uiVal = uiL2FRAM_get_version_num();
		vSERIAL_rom_sout("V");
		vSERIAL_UIV8out((uchar)(uiVal>>8));	//Version Num
		vSERIAL_bout('.');
		vSERIAL_HB8out((uchar)(uiVal & 0xFF)); //Sub Version Num
		vSERIAL_bout(')');
		}
	else
		{
		vSERIAL_rom_sout("OFF)");

		}/* END: else(OPTPAIR_CHK_FOR_FRAM_ON_STUP) */



	vSERIAL_rom_sout("(ECLK-");
	ucECLKsts = ucTIME_getECLKsts(YES_SKP_ECLK_RAM_FLAG);
	switch(ucECLKsts)
		{
		case 0:
		case 1:
			vSERIAL_rom_sout("NONE)");
			break;

		case 2:
			vSERIAL_rom_sout("NoTick)");
			break;

		case 3:
			vSERIAL_rom_sout("OK)");
			break;

		default:
			vSERIAL_rom_sout("???");
			vSERIAL_HB8out(ucECLKsts);
			vSERIAL_rom_sout(")");
			break;

		}/* END: switch() */


	vSERIAL_crlf();

	/* SHOW MESSAGE COUNT */
	vSERIAL_rom_sout("M");					//SRAM msg count
	vSERIAL_UIV16out(uiL2SRAM_getMsgCount());

	/* SHOW FLASH MESSAGE COUNT */
	vSERIAL_rom_sout(" F");
	vSERIAL_UIV24out((usl)lL2FRAM_getFlashUnreadMsgCount()); //lint !e13

	vSERIAL_crlf();
	vSERIAL_crlf();

	return;

	} /* END: vMAIN_printIntro() */






/*************************  vMAIN_sendRadioStartupMsg()  *********************
*
*
******************************************************************************/

static void vMAIN_sendRadioStartupMsg(
		void
		)
	{
	/* SET THE FREQUENCY */
	vRADIO_setBothFrequencys(
				ucRADIO_getChanAssignment(DATA_CHANNEL_INDEX),
				NO_FORCE_FREQ
				);

	vMSG_buildMsgHdr_GENERIC( //HDR= Len,Type,Group,Src,Dest
			ST1_MSG_LAST_BYTE_NUM_UC,		//Len
			MSG_TYPE_STARTUP,				//Type
			0								//Dest
			);

	/* COMPUTE THE CRC */
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534 //compute the CRC

	/* SHIP IT */
  	vRADIO_xmit_msg();

	/* * SHUT RADIO OFF */
	vRADIO_quit();		

	return;

	}/* END: vMAIN_sendRadioStartupMsg() */







/******************  vMAIN_computeDispatchTiming()  **************************
*
* This routine computes the dispatcher timing and then returns.
*
******************************************************************************/

void vMAIN_computeDispatchTiming(
		void
		)
	{
	long lThisTime;
	long lThisSlotEndTime;
	long lOpUpTimeInSec;
	long lThisLinearSlot;
	long lThisSlotRemainder;
	long lThisFrameNum;
	uchar ucThisSlotNum;
	long lii;
	uchar ucT3RetVal;


	#if 0
	vSERIAL_rom_sout("D\r\n");
	#endif

	/* CHECK IF WE HAVE HAD A TIME RESET */
	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_RESET_ALL_TIME_BIT)
		{
		#if 1
		vSERIAL_rom_sout("TimSet\r\n");
		#endif

		lGLOB_lastAwakeTime = lTIME_getSysTimeAsLong();
		lOpUpTimeInSec = lGLOB_lastAwakeTime - lGLOB_OpMode0_inSec;
		lGLOB_lastAwakeLinearSlot = lOpUpTimeInSec / SECS_PER_SLOT_L;
		lThisSlotEndTime = ((lGLOB_lastAwakeLinearSlot+1) * SECS_PER_SLOT_L) +
							lGLOB_OpMode0_inSec;
		lThisSlotRemainder = lOpUpTimeInSec % SECS_PER_SLOT_L;
		lGLOB_lastAwakeFrame = lGLOB_lastAwakeLinearSlot / SLOTS_PER_FRAME_I;
		ucGLOB_lastAwakeSlot = (uchar)(lGLOB_lastAwakeLinearSlot % SLOTS_PER_FRAME_I);
		ucGLOB_lastAwakeNSTtblNum = (uchar)(lGLOB_lastAwakeFrame % 2);
		ucGLOB_lastAwakeStblIdx = ucRTS_getNSTentry(
										ucGLOB_lastAwakeNSTtblNum,
										ucGLOB_lastAwakeSlot
										);
		lGLOB_lastScheduledFrame = lGLOB_lastAwakeFrame;	  //= this frame

		#if 0
		vSERIAL_rom_sout("C:FindNSTslot\r\n");
		#endif

		ucGLOB_lastScheduledSchedSlot = 0x3F;
//		ucGLOB_lastScheduledSchedSlot = ucRTS_findNSTslotNumOfSched(ucGLOB_lastAwakeNSTtblNum);

		#if 0
		vSERIAL_rom_sout("R:FindNSTslot\r\n");
		#endif

		/* IF THE SCHED HAS NOT ALREADY RUN IN THIS TIME THEN FORCE IT TO BE RUN */
		if(ucGLOB_lastAwakeSlot > ucGLOB_lastScheduledSchedSlot)
			{
			#if 1
			vSERIAL_rom_sout("C:ForceNxtNSTsched\r\n");
			#endif

			vRTS_scheduleNSTtbl(lGLOB_lastAwakeFrame+1L);	//sched next NST

			#if 1
			vSERIAL_rom_sout("R:ForceNxtNSTsched\r\n");
			#endif
			}

		/* CLR THE RESET TIME BIT */
		ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_RESET_ALL_TIME_BIT = 0;	//shut flag off

		#if 0
		vSERIAL_rom_sout("TimSet\r\n");
		#endif

		}

	/* WAIT TIL TIME IS POSITIVE */
	if(lGLOB_lastAwakeTime > lTIME_getSysTimeAsLong())
		{
		vSERIAL_rom_sout("WaitingForTimePositive\r\n");
		while(lGLOB_lastAwakeTime > lTIME_getSysTimeAsLong()); //lint !e722
		}

	/* COMPUTE THE OPERATIONAL TIME IN LINEAR SLOTS */
	lThisTime = lTIME_getSysTimeAsLong();
	lOpUpTimeInSec = lThisTime - lGLOB_OpMode0_inSec;
	lThisLinearSlot = lOpUpTimeInSec / SECS_PER_SLOT_L;
	lThisSlotEndTime = ((lThisLinearSlot+1) * SECS_PER_SLOT_L) +
					lGLOB_OpMode0_inSec;
	lThisSlotRemainder = lOpUpTimeInSec % SECS_PER_SLOT_L;
	lThisFrameNum = lThisLinearSlot / SLOTS_PER_FRAME_I; //nxt frame num
	ucThisSlotNum = (uchar)(lThisLinearSlot % SLOTS_PER_FRAME_I);

	/* IF STILL IN THE LAST SLOT -- WAIT FOR NEXT SLOT */
	if(lThisLinearSlot == lGLOB_lastAwakeLinearSlot)
		{
		/* SLEEP IF POSSIBLE */
		if(lThisSlotRemainder > 0)
			{
			if(ucTIME_setT3AlarmToSecMinus200ms(lThisSlotEndTime) == 0) 
				{
				#if 0
				vSERIAL_rom_sout("E:ShortSlp\r\n");
				#endif
				
				ucMISC_sleep_until_button_or_clk(SLEEP_MODE); //lint !e534

				#if 0
				vSERIAL_rom_sout("X:ShortSlp\r\n");
				#endif

				}
			}
		else
			{

			/* WAIT OUT THE REST HERE */
			#if 0
			vSERIAL_rom_sout("E:WhileWait\r\n");
			#endif

			while(lThisSlotEndTime > lTIME_getSysTimeAsLong()); //lint !e722

			#if 0
			vSERIAL_rom_sout("X:WhileWait\r\n");
			#endif

			}

		}/* END: if() */

	/* IF ITS IN THE LAST_SLOT+1 -- GO START IT */
	if(lThisLinearSlot == lGLOB_lastAwakeLinearSlot+1L)	goto Update_and_leave;




	/*------ IF WE ARE HERE THEN TIME SKIPPED FORWARD MORE THAN A SLOT -------*/


	/* IF WE SKIPPED FRAMES -- CATCH UP THE SCHEDULED FRAMES */
	if(lThisFrameNum > lGLOB_lastScheduledFrame)
		{
		#if 1
		vSERIAL_rom_sout("FRAMEskpFrom ");
		vSERIAL_HBV32out((ulong)lGLOB_lastScheduledFrame);
		vSERIAL_rom_sout(" to ");
		vSERIAL_HBV32out((ulong)lThisFrameNum);
		vSERIAL_crlf();
		#endif

		/* SCHEDULER DID NOT RUN FOR THIS FRAME -- CATCH IT UP */
		for(lii=lGLOB_lastScheduledFrame+1;  lii<=lThisFrameNum; lii++)
			{
			vRTS_scheduleNSTtbl(lii);

			}/* END: for() */

		/* CHECK IF THE SCHED SLOT FOR THIS FRAME HAS BEEN PASSED */
		if(ucThisSlotNum > ucGLOB_lastScheduledSchedSlot)
			{
			#if 1
			vSERIAL_rom_sout("Frame#");
			vSERIAL_HBV32out((ulong)lThisFrameNum);
			vSERIAL_rom_sout(" Frame&SlotSkpOverSchedFrom ");
			vSERIAL_HB8out(ucGLOB_lastScheduledSchedSlot);
			vSERIAL_rom_sout(" to ");
			vSERIAL_HB8out(ucThisSlotNum);
			vSERIAL_crlf();
			#endif
			/* YES--ROLL ONE MORE */
			vRTS_scheduleNSTtbl(lThisFrameNum+1L);
			}
		}
	else
		{
		/* CHECK IF THE SCHED SLOT FOR THIS FRAME HAS BEEN PASSED */
		if((ucGLOB_lastAwakeSlot < ucGLOB_lastScheduledSchedSlot) &&
		   (ucThisSlotNum > ucGLOB_lastScheduledSchedSlot))
			{
			#if 1
			vSERIAL_rom_sout("Frame#");
			vSERIAL_HBV32out((ulong)lThisFrameNum);
			vSERIAL_rom_sout(" SlotOnlySkpFrm ");
			vSERIAL_HB8out(ucGLOB_lastAwakeSlot);
			vSERIAL_rom_sout(" to ");
			vSERIAL_HB8out(ucThisSlotNum);
			vSERIAL_rom_sout(" skpingSchedSlotAt ");
			vSERIAL_HB8out(ucGLOB_lastScheduledSchedSlot);
			vSERIAL_crlf();
			#endif
			/* YES--ROLL ONE MORE */
			vRTS_scheduleNSTtbl(lThisFrameNum+1L);
			}
		}







	/* WAIT FOR A SLOT START */
	/* SLEEP IF POSSIBLE */
	if(lThisSlotRemainder > 0)
		{
		if(!ucTIME_setT3AlarmToSecMinus200ms(lThisSlotEndTime))
			{
			ucMISC_sleep_until_button_or_clk(SLEEP_MODE); //lint !e534
			}
		}

	/* FINISH WAITING HERE */
	while(lThisSlotEndTime > lTIME_getSysTimeAsLong()); //lint !e722




Update_and_leave:

	/* WE ARE CAUGHT UP -- SO UPDATE THE CURRENT FRAME AND SLOT */
	lGLOB_lastAwakeTime = lTIME_getSysTimeAsLong();
	lOpUpTimeInSec = lGLOB_lastAwakeTime - lGLOB_OpMode0_inSec;
	lGLOB_lastAwakeLinearSlot = lOpUpTimeInSec / SECS_PER_SLOT_L;
	lThisSlotEndTime = ((lGLOB_lastAwakeLinearSlot+1) * SECS_PER_SLOT_L) +
						lGLOB_OpMode0_inSec;
	lThisSlotRemainder = lOpUpTimeInSec % SECS_PER_SLOT_L;
	lGLOB_lastAwakeFrame = lGLOB_lastAwakeLinearSlot / SLOTS_PER_FRAME_I;
	ucGLOB_lastAwakeSlot = (uchar)(lGLOB_lastAwakeLinearSlot % SLOTS_PER_FRAME_I);
	ucGLOB_lastAwakeNSTtblNum = (uchar)(lGLOB_lastAwakeFrame % 2);


	/* SET SYSTIME ALARM TO NEXT SLOT START */
	vTIME_setAlarmFromLong(lThisSlotEndTime);

	/* SET T3 ALARM TO THIS SLOT END */
	ucT3RetVal = ucTIME_setT3AlarmToSecMinus200ms(lThisSlotEndTime);
	if(ucT3RetVal != 0)
		{
		#if 1
		vSERIAL_rom_sout("T3AlarmFail= ");
		vSERIAL_HB8out(ucT3RetVal);
		vSERIAL_crlf();
		#endif
		}

	#if 0
	vSERIAL_rom_sout("X\r\n");
	#endif

	vSERIAL_crlf();

	return;

	}/* END: vMAIN_computeDispatchTiming() */





/********************  vMAIN_showCompilerAnomalies()  *************************
*
* Run nothing in the brain so the SD can perform.
*
*
*******************************************************************************/

void vMAIN_showCompilerAnomalies(
		const char *cMsg
		)
	{
	vBUZ_blink_buzzer(5);

	vSERIAL_crlf();
	vSERIAL_dash(20);
	vSERIAL_rom_sout(cMsg);
	vSERIAL_dash(20);
	vSERIAL_crlf();

	vBUZ_blink_buzzer(5);

	return;

	}/* END: vMAIN_showCompilerAnomalies() */





#ifdef RUN_NOTHING
/**************************  vMAIN_run_nothing()  ****************************
*
* Run nothing in the brain so the SD can perform.
*
*
*****************************************************************************/

void vMAIN_run_nothing(
		void
		)
	{
	ucSDCTL_start_SD_and_specify_boot(SD_BOOT_LOADER_ENABLED); //lint !e534

	vSERIAL_init();
	vSERIAL_rom_sout("NOTHING:\r\n");

	vBUZ_blink_buzzer(1);

HANG:
	goto HANG;


	}/* END: vMAIN_run_nothing() */
#endif



/* -----------------------  END OF MODULE  ------------------------------- */

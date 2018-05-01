
/**************************  OPMODE.C  *****************************************
*
* Routines to perform OPMODEs during events
*
*
* V1.00 10/04/2003 wzr
*		Started
*
******************************************************************************/

#include "diag.h"			//Diagnostic package

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
///*lint -e714 */ 	/* symbol not referenced */
/*lint -e716 */		/* while(1) ... */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
///*lint -e757 */ 	/* global declarator not referenced */
///*lint -e752 */ 	/* local declarator not referenced */
///*lint -e758 */ 	/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


//#include <msp430x54x.h>		//processor reg description */
#include "std.h"			//standard defines
#include "config.h" 		//system configuration description file
//#include "main.h"			//
#include "radio.h"			//radio routines
#include "misc.h"			//homeless functions
//#include "reading.h"		//sensor reading module
//#include "ad.h" 			//AD module
#include "crc.h"			//CRC calculation module
#include "l2sram.h"  		//disk storage module
#include "serial.h"			//serial IO port stuff
#include "l2fram.h" 		//level 2 fram routines
//#include "sram.h"			//static ram routines
//#include "sdctl.h"  		//SD board control routines
//#include "sdcom.h" 		//SD communication package
//#include "l2flash.h"		//level 2 flash routines
#include "time.h"			//Time routines
#include "action.h" 		//Event action routines
#include "msg.h"			//MSG generator routines
//#include "delay.h"  		//Delay routines
#include "rts.h"			//Real time Scheduler
#include "stbl.h"			//Schedule table routines
#include "gid.h"			//group ID
#include "opmode.h"			//operational mode module
//#include "t0.h" 			//Timer 0 routines
#include "rad40.h"	   		//Radix 40 routines
#include "sensor.h"
#include "report.h"			//error reporting mechanisms
#include "MODOPT.h"			//Modify Options routines
#include "LNKBLK.h"			//Link Byte routines

/*********************  EXTERNS  ********************************************/

extern volatile uchar ucaMSG_BUFF[ MAX_RESERVED_MSG_SIZE ];

//extern uchar ucGLOB_CurMsgSeqNum;
extern long lGLOB_lastAwakeLinearSlot;	//Nearest thing to cur linear slot
extern long lGLOB_OpMode0_inSec;		//Start of OP mode
extern uchar ucGLOB_lastAwakeStblIdx;	//Nearest thing to cur sched idx 
//extern uchar ucGLOB_lastAwakeNSTtblNum;	//Nearest thing to cur NST tbl

extern int iGLOB_completeSysLFactor;	//entire Signed LFactor quantity.


#if 0
extern volatile union							//ucFLAG1_BYTE
		{
		unsigned char byte;

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

extern volatile union							//ucFLAG2_BYTE
		{
		unsigned char byte;

		struct
		 {
	  	 unsigned FLG2_T3_ALARM_MCH_BIT:1;		//bit 0 ;1=Alarm, 0=no alarm
		 unsigned FLG2_T1_ALARM_MCH_BIT:1;		//bit 1
		 unsigned FLG2_BUTTON_INT_BIT:1;		//bit 2 ;1=XMIT, 0=RECEIVE
		 unsigned FLG2_CLK_INT_BIT:1;			//bit 3	;1=clk ticked, 0=no tic
		 unsigned FLG2_X_FROM_MSG_BUFF_BIT:1;	//bit 4
		 unsigned FLG2_R_BUSY_BIT:1;			//bit 5 ;int: 1=REC BUSY, 0=IDLE
		 unsigned FLG2_R_BARKER_ODD_EVEN_BIT:1;	//bit 6 ;int: 1=odd, 0=even
		 unsigned FLG2_R_BITVAL_BIT:1;			//bit 7 ;int: 
		 }FLAG2_STRUCT;

		}ucFLAG2_BYTE;



extern const uchar ucaStsTblNum[2];

extern uint uiGLOB_lostROM2connections;	//counts lost ROM2's
extern uint uiGLOB_lostSOM2connections;	//counts lost SOM2's

extern uint uiGLOB_ROM2attempts;		//count ROM2 attempts
extern uint uiGLOB_SOM2attempts;		//count SOM2 attempts



/*************************  DEFINES  *****************************************/

#define TOP_OM2_MSG_COUNT  6

#define RETRIEVE_MSG_SEQ_MISMATCH_BIT	0x20  // 0010 0000





/***********************  INTERNAL DECLARATIONS  *****************************/


static void vOPMODE_zroMissedMsgCnt(
		void
		);

uchar ucOPMODE_getOM2LinkByteVal(
		void
		);




/*****************************  CODE STARTS HERE  ****************************/




/************************  vOPMODE_buildMsg_OM1() ***************************
*
* This routine builds a OM1 msg header
*
******************************************************************************/

static void vOPMODE_buildMsg_OM1(
		uint uiDestSN,				//Dest serial num
		long lSyncTimeSec,			//the Sync time Sec value
		uchar ucMsgVacancy			//Vacancy number
		)
	{
	/* BUILD THE MSG */
	vMSG_buildMsgHdr_GENERIC(
				OM1_MSG_LAST_BYTE_NUM_UC,	//last byte num
				MSG_TYPE_OM1,				//msg num
				uiDestSN					//dest SN
				);

	/* STUFF THE SYNC TIME */
	vMISC_copyUlongIntoBytes(
				(ulong)lSyncTimeSec,
				(uchar *) &ucaMSG_BUFF[OM1_IDX_SYNC_TIME_XI],
				NO_NOINT
				);
	vMISC_copyUintIntoBytes(
				OM1_SYNC_IN_CLKS_UI,
				(uchar *) &ucaMSG_BUFF[OM1_IDX_SYNC_TIME_SUB_HI],
				NO_NOINT
				);

	/* STUFF THE SECONDS TO END OF SLOT */ 
	ucaMSG_BUFF[OM1_IDX_DELTA_SEC_TO_SLOT_END] = SECS_PER_SLOT_I -1;

	/* STUFF SRC VACANCY */
	ucaMSG_BUFF[OM1_IDX_AVAIL_MSG_SPACE] = ucMsgVacancy;

	/* COMPUTE THE CRC */
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534 //compute the CRC

	return;

	}/* END: vOPMODE_buildMsg_OM1() */






#ifdef SOM2_DIAG_ENABLED
/***********************  vOPMODE_recFake_OM1()  ****************************
*
* Receive a Fake OM1 message
*
******************************************************************************/

static void vOPMODE_recFake_OM1(
		void
		)
	{
	uint uiSrcSN;
	uint uiDestSN;
	long lSyncTimeSec;
	uchar ucMsgVacancy;


	/* FILL THE MSG PARAMETERS FIRST */
	uiSrcSN = (uint)ulRTS_getSchedEntry(SCHED_SN_TBL_NUM,
									ucGLOB_lastAwakeStblIdx
									);
	uiDestSN = uiL2FRAM_getSnumLo16AsUint();

	lSyncTimeSec = lTIME_getSysTimeAsLong();	
	ucMsgVacancy = 15;


	/* BUILD THE MSG */
	vOPMODE_buildMsg_OM1(uiDestSN,
						lSyncTimeSec,
						ucMsgVacancy
						);


	/* CHANGE THE SOURCE SN TO BE FROM DEST SN */
	vMISC_copyUintIntoBytes(
				uiSrcSN,									//uint
				(uchar *)&ucaMSG_BUFF[OM1_IDX_SRC_SN_HI],	//to
				NO_NOINT
				);

	/* RE-COMPUTE THE CRC */
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534


	/* FAKE THE RECEIVED FLAG */
	ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT = 1;


	vDELAY_wait100usTic(4000);	//200ms faked response time

	return;

	}/* END: vOPMODE_recFake_OM1() */

#endif /* SOM2_DIAG_ENABLED */







/*************  vOPMODE_stuffDataPairToOM2()  **********************************
*
* This routine adds a data pair to an existing OM2 and increments the
* End of Msg by 3.
*
*****************************************************************************/

void vOPMODE_stuffDataPairToOM2(
		uchar ucSensorNum,		//sensor number
		uint uiSensorData,		//sensor Data
		uchar ucPairStIdx		//Starting Msg Idx for Data Pair
		)
	{
	#if 0
	vSERIAL_rom_sout("LdingSensr ");
	vSERIAL_UIV8out(ucSensorNum);
	vSERIAL_rom_sout(" = ");
	vSERIAL_UIV16out(uiSensorData);
	vSERIAL_rom_sout(" in MSG+");
	vSERIAL_UIV8out(ucPairStIdx);
	vSERIAL_crlf();
	#endif

	/* STUFF THE SENSOR NUMBER */
	ucaMSG_BUFF[ucPairStIdx++] = ucSensorNum;

	/* STUFF THE SENSOR DATA */
	vMISC_copyUintIntoBytes(
				uiSensorData,
				(uchar *)&ucaMSG_BUFF[ucPairStIdx],
				NO_NOINT
				);

	return;

	}/* END: vOPMODE_stuffDataPairToOM2() */






/*********************  vOPMODE_buildMsg_OM2() *******************************
*
* This routine builds a OM2 msg & computes the msg CRC
*
******************************************************************************/

void vOPMODE_buildMsg_OM2(
		uchar ucByte0_pktFlags,		//LAST_PKT_BIT & NO_DATA_PKT_BIT
		uint uiDestSN,				//Dest serial num
		uchar ucMsgSeqNum,			//Msg seq number
		uchar ucGenericOM2LinkByte,	//Link Byte Value
		uint uiAgentSN,				//Data Sampler Agent Num
		long lCollectionTime,		//Data Sample time
		uchar ucDataCount,			//Cnt of valid sensor data entries (1 - 4)
		uchar ucSensorNum_0,		//Data Sensor 1 number
		uint uiSensorData_0,		//data from Sensor 1
		uchar ucSensorNum_1,		//Data Sensor 1 number
		uint uiSensorData_1,		//data from Sensor 1
		uchar ucSensorNum_2,		//Data Sensor 2 number
		uint uiSensorData_2,		//data from Sensor 2
		uchar ucSensorNum_3,		//Data Sensor 3 number
		uint uiSensorData_3			//data from Sensor 3
		)
	{
	uchar ucc;

	#if 0
	vSERIAL_rom_sout("\r\nOP:BldOM2Agnt= ");
	//vSERIAL_UIV16out(uiAgentSN);
	vRAD40_showRad40(uiAgentSN);
	vSERIAL_crlf();
	#endif




	/* BUILD THE MSG */
	vMSG_buildMsgHdr_GENERIC(
				OM2_MSG_LAST_BYTE_NUM_UC,	//last byte num
				MSG_TYPE_OM2,				//msg num
				uiDestSN					//dest SN
				);

	/* STUFF THE MSG SEQ NUM */
	ucaMSG_BUFF[OM2_IDX_MSG_SEQ_NUM] = ucMsgSeqNum;

	/* STUFF THE LOADING FACTOR */
	ucaMSG_BUFF[OM2_IDX_GENERIC_LINK_BYTE] = ucGenericOM2LinkByte;

	/* STUFF THE AGENT NUMBER */
	vMISC_copyUintIntoBytes(
					uiAgentSN,				//Data Agent serial number
					(uchar *)&ucaMSG_BUFF[OM2_IDX_AGENT_NUM_HI],
					NO_NOINT
					);

	/* STUFF THE COLLECTION TIME */
	vMISC_copyUlongIntoBytes(
				(ulong)lCollectionTime,
				(uchar *) &ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_XI],
				NO_NOINT
				);

	/* CHECK THE SENSOR COUNT */
	if(ucDataCount > OM2_MAX_DATA_ENTRY_COUNT)
		{
		#if 1
		vSERIAL_rom_sout("OP:SnsrCntOver\r\n");
		#endif
		/* FORCE IT TO MAX */
		ucDataCount = OM2_MAX_DATA_ENTRY_COUNT;
		}

	

	/* STUFF THE SENSOR READINGS */
	if(ucDataCount != 0)
		{
		for(ucc=1;  ucc<=ucDataCount;  ucc++)
			{
			uchar ucBaseIdx;
			uchar ucSensor;
			uint uiSensorDat;

			ucBaseIdx = (uchar)(OM2_IDX_DATA_0_SENSOR_NUM + ((ucc-1) * 3));

			switch(ucc)
				{
				case 1:
					ucSensor = ucSensorNum_0;
					uiSensorDat = uiSensorData_0;
					break;

				case 2:
					ucSensor = ucSensorNum_1;
					uiSensorDat = uiSensorData_1;
					break;

				case 3:
					ucSensor = ucSensorNum_2;
					uiSensorDat = uiSensorData_2;
					break;

				case 4:
					ucSensor = ucSensorNum_3;
					uiSensorDat = uiSensorData_3;
					break;

				default:								//lint !e616
					ucSensor = 0;
					uiSensorDat = 0;
					break;

				}/* END: switch() */

			vOPMODE_stuffDataPairToOM2(ucSensor, uiSensorDat, ucBaseIdx);

			}/* END: for(ucc) */

		}/* END: if() */



	/* SET THE CORRECT MSG SIZE IDX */
										//  Hdr       DataCount     CRC Idx
										// Size         Size	   Size	Adj
	ucaMSG_BUFF[OM2_IDX_EOM_IDX] = (uchar)(16 + (ucDataCount * 3) + 2 -1);
	ucaMSG_BUFF[OM2_IDX_EOM_IDX] |= ucByte0_pktFlags;

	#if 1
	if(ucByte0_pktFlags & OM2_NODATA_BIT)
		{
		vSERIAL_rom_sout("BldOM2:BldEmptyDatPkt\r\n");
		}
	#endif


	/* COMPUTE THE CRC */
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534 //compute the CRC

	return;

	}/* END: vOPMODE_buildMsg_OM2() */






/************************  ucOPMODE_RetrieveMsg_OM2() ***************************
*
* This routine retrieves a previously built OM2 msg from SRAM
*
* RET:	Yes	0 = OK
*		Not 0 = FLAG BYTE (below)
*
*    7        6        5        4        3         2       1        0
*ÚÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄ¿
*³  Last  ³ No Data³        ³        ³        ³        ³        ³        ³
*³  Pkt   ³  Pkt   ³        ³        ³        ³        ³        ³        ³
*³        ³        ³        ³        ³        ³        ³        ³        ³
*ÀÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÙ
*
******************************************************************************/

static uchar ucOPMODE_RetrieveMsg_OM2(
		uint uiDestSN,				//dest Serial num
		uchar ucNewMsgFlags,		//New flags for this next pkt
		uchar ucGenericOM2LinkByte	//New link Byte value
		)
	{
	uchar ucMsgSizeMinusOne;
	uint uiMsgCnt;


	/* COPY NEW MSG INTO BUFFER */
	ucMsgSizeMinusOne = ucL2SRAM_getCopyOfCurMsg();

	/* MAKE SURE MSG HAS SOMETHING IN IT */
	if(ucMsgSizeMinusOne)
		{
		/* CLR ANY SPURIOUS LAST_PKT_BIT FLAGS FROM MSG SIZE */
		ucaMSG_BUFF[OM2_IDX_EOM_IDX] &= (MAX_MSG_SIZE_MASK | OM2_NODATA_BIT);

		#if 1
		/* REPORT AN EMPTY PACKET STORED IN THE SRAM */
		if(ucaMSG_BUFF[OM2_IDX_EOM_IDX] & OM2_NODATA_BIT)
			{
			vSERIAL_rom_sout("OP:SRAM-hasEmptyPkt\r\n");
			}
		#endif

		/* CLR ANY POSSIBLE SPURIOUS BITS FROM MSG TYPE */
		ucaMSG_BUFF[OM2_IDX_MSG_TYPE] &= OM2_MSG_TYPE_MASK;


		/* CHK FOR NEXT MSG IN THE SRAM -- IF NONE SET LAST PKT BIT */
		uiMsgCnt = uiL2SRAM_getMsgCount();
		if(uiMsgCnt == 1)
			{
			ucaMSG_BUFF[OM2_IDX_EOM_IDX] |= OM2_LAST_PKT_BIT;
			}

		#if 0
		/* DEBUG: REPORT WHAT THE MSG COUNT IS */
		vSERIAL_rom_sout("M=");
		vSERIAL_UIV16out(uiMsgCnt);
		vSERIAL_crlf();
		#endif

		goto OPRM_common_jct;		//skip down to common function

		}/* END: if() */


	/**********   IF WE ARE HERE THE SRAM IS EMPTY ***************************/

	/* BUILD AN EMPTY MSG */
	#if 1
	vSERIAL_rom_sout("OP:NoDatPktBlt\r\n");
	#endif

	/* BUILD A EMPTY DATA PACKET */
	vOPMODE_buildMsg_OM2(
		OM2_LAST_PKT_BIT | OM2_NODATA_BIT,	//LAST_PKT_BIT & NO_DATA_PKT_BIT
		uiDestSN,						//Dest Serial num
		ucMSG_incMsgSeqNum(),			//Seq Num
		ucGenericOM2LinkByte,			//Message Sys Load Factor
		0,								//Agent
		0L,								//Collection time
		0,								//Data Count
		0,								//Sensor Num 0
		0,								//Sensor Data 0
		0,								//Sensor Num 1
		0,								//Sensor Data 1
		0,								//Sensor Num 2
		0,								//Sensor Data 2
		0,								//Sensor Num 3
		0								//Sensor Data 3
		);


	/*-------------------------  FAKE MSG  ----------------------------------*/
	#ifdef SOM2_DIAG_ENABLED

	#if 0
	vSERIAL_rom_sout("OP:BltFakMsg\r\n");
	#endif

	/* BUILD A FAKE OM2 */
	vOPMODE_buildMsg_OM2(
			uiDestSN,						//Dest Serial num
			ucMSG_incMsgSeqNum(),			//Msg Seq Num
			ucGenericOM2LinkByte,			//Generic Linkup Byte
			0x1150,							//Agent SN
			0x00001500L,					//Collection Time
			2,								//Data Count
			SENSOR_LIGHT_1,					//Sensor 0 number
			0x0150,							//Sensor 0 data
			SENSOR_LIGHT_2,					//Sensor 1 number
			0x0150,							//Sensor 1 data
			0,								//Sensor 2 number
			0,								//Sensor 2 data
			0,								//Sensor 3 number
			0								//Sensor 3 data
			);
	#endif
	/*-----------------------  END: FAKE MSG  -------------------------------*/


OPRM_common_jct:

	#if  0
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);
	#endif

	/* BUILD OM2 MSG FOR THIS RADIO HOP */
	ucaMSG_BUFF[OM2_IDX_EOM_IDX] |= ucNewMsgFlags;//set NoData & LastPkt (maybe)

	ucaMSG_BUFF[OM2_IDX_MSG_TYPE] = MSG_TYPE_OM2;	//insure msg type

	vGID_copyWholeSysGidToBytes((uchar *)&ucaMSG_BUFF[GMH_IDX_GID_HI]); //set this grp ID

	vL2FRAM_copySnumLo16ToBytes((uchar *)&ucaMSG_BUFF[OM2_IDX_SRC_SN_HI]); //set SENDER ID

	vMISC_copyUintIntoBytes(uiDestSN,				//set DEST ID
							(uchar *)&ucaMSG_BUFF[OM2_IDX_DEST_SN_HI],
							NO_NOINT
							);

	ucaMSG_BUFF[OM2_IDX_MSG_SEQ_NUM] = ucMSG_incMsgSeqNum(); //set seq num


	/* ALG1: Load Factor,  ALG2:Link Req */
	ucaMSG_BUFF[OM2_IDX_GENERIC_LINK_BYTE] = ucGenericOM2LinkByte; //set load factor


	/* COMPUTE THE CRC */
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534 //compute the CRC

	#if  0
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED); //show OM2
	#endif

	return(ucaMSG_BUFF[OM2_IDX_EOM_IDX] & ~MAX_MSG_SIZE_MASK);

	}/* END: ucOPMODE_RetrieveMsg_OM2() */








#ifdef ROM2_DIAG_ENABLED
/***********************  vOPMODE_recFake_OM2()  ****************************
*
*
*
******************************************************************************/

static void vOPMODE_recFake_OM2(
		void
		)
	{
	/* BUILD THE MSG */
	vOPMODE_buildMsg_OM2(
			uiL2FRAM_getSnumLo16AsUint(),	//Dest SN
			ucMSG_incMsgSeqNum(),			//Msg Seq Num
			0,								//Load Factor
			0x1150,							//Agent SN
			0x00001500L,					//Collection Time
			2,								//Sensor data count
			SENSOR_LIGHT_1,					//Sensor 1 number
			0x0150,							//Sensor 1 data
			SENSOR_LIGHT_2,					//Sensor 2 number
			0x0150,							//Sensor 2 data
			0,								//Sensor 3 number
			0								//Sensor 3 data
			);


	/* CHANGE THE SOURCE SN TO BE FROM UNIT 1 */
	vMISC_copyUintIntoBytes(
				1,									//uint
				(uchar *)&ucaMSG_BUFF[OM2_IDX_SRC_SN_HI],	//to
				NO_NOINT
				);

	/* RE-COMPUTE THE CRC */
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534


	/* FAKE THE RECEIVED FLAG */
	ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT = 1;


	vDELAY_wait100usTic(4000);	//400ms faked response time


	return;

	}/* END: vOPMODE_recFake_OM2() */
#endif








/************************  vOPMODE_buildMsg_OM3() ***************************
*
* This routine builds a OM3 msg header
*
******************************************************************************/

static void vOPMODE_buildMsg_OM3(
		uchar ucByte0_pktFlags,	//
		uchar ucByte1_pktFlags,	//
		uint uiDestSN,	  		//Dest serial num
		uchar ucMsgNum,	  		//Msg seq number		
		uchar ucGenericOM3LinkRet //Ret ALG1:LoadFactor | ALG2:LinkConfirmation
		)
	{
	/* BUILD THE MSG */
	vMSG_buildMsgHdr_GENERIC(
				OM3_MSG_LAST_BYTE_NUM_UC,	//last byte num
				MSG_TYPE_OM3,				//msg num
				uiDestSN					//dest SN
				);

	/* SET THE BYTE 0 PACKET FLAGS */
	ucaMSG_BUFF[OM3_IDX_EOM_IDX] |= ucByte0_pktFlags;

	/* SET THE BYTE 1 PACKET FLAGS */
	ucaMSG_BUFF[OM3_IDX_MSG_TYPE] |= ucByte1_pktFlags;

	/* STUFF THE MSG SEQ NUM */
	ucaMSG_BUFF[OM3_IDX_ACKED_SEQ_NUM] = ucMsgNum;

	/* STUFF THE GENERIC LINK RETURN VALUE */
	/* ALG1:LinkupDcnt | ALG2:LinkupConfirmation */
	ucaMSG_BUFF[OM3_IDX_GENERIC_LINK_RETURN] = ucGenericOM3LinkRet;
	
	/* COMPUTE THE CRC */
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534 //compute the CRC

	return;

	}/* END: vOPMODE_buildMsg_OM3() */






#ifdef SOM2_DIAG_ENABLED
/***********************  vOPMODE_recFake_OM3()  *****************************
*
*
*
******************************************************************************/

static void vOPMODE_recFake_OM3(
		uint uiSrcSN,
		uint uiDestSN,
		uchar ucMsgNum
		)
	{

	/* BUILD THE OM3 */
	vOPMODE_buildMsg_OM3(0,0,uiDestSN, ucMsgNum, 0x09); //Link Req



	/* CHANGE THE SOURCE SN TO BE FROM UNIT 1 */
	vMISC_copyUintIntoBytes(
				uiSrcSN,
				(uchar *)&ucaMSG_BUFF[OM3_IDX_SRC_SN_HI],	//to
				NO_NOINT
				);

	/* RE-COMPUTE THE CRC */
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534


	/* FAKE THE RECEIVED FLAG */
	ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT = 1;


	vDELAY_wait100usTic(4000);	//200ms faked response time

	return;

	}/* END: vOPMODE_recFake_OM3() */

#endif /* SOM2_DIAG_ENABLED */






/****************  ucOPMODE_incLnkBrkCntAndChkForDeadOM2()  *****************
*
* This routine is called:
*		1. when a first OM1 is missed during SOM2.
*		2. when a first OM2 is missed during ROM2.
*
* RET:	1 = This link has been closed
*		0 = This link is still open
*
*****************************************************************************/

uchar ucOPMODE_incLnkBrkCntAndChkForDeadOM2(//RET: 1=Lnk Broken, 0=Lnk still open
		void
		)
	{
	uint uiStsVal;
	uchar ucActionNum;
	uchar ucMissedMsgCnt;

	/* GET THE CURRENT STS TBL ENTRY */
	uiStsVal = (uint)ulL2SRAM_getStblEntry(
						ucaStsTblNum[ucRTS_lastScheduledNSTnum()],	//Tbl num
						ucGLOB_lastAwakeStblIdx					//Tbl Idx
						);

	/* INC THE MISSED MSG COUNT */
	if((uiStsVal & 0x0700) != 0x0700) uiStsVal += 0x0100;
	ucMissedMsgCnt = (uchar)((uiStsVal & 0x0700) >> 8);	//save out count


	/* WRITE CURRENT STS BACK */
	vL2SRAM_putStblEntry(ucaStsTblNum[ucRTS_lastScheduledNSTnum()],	//Tbl num
						 ucGLOB_lastAwakeStblIdx,					//Tbl Idx
						 (ulong)uiStsVal
						 );

	#if 1
	vSERIAL_rom_sout("OM2failCnt= ");
	vSERIAL_UIV8out(ucMissedMsgCnt);
	vSERIAL_crlf();
	#endif

	/* IF NODE IS STILL CONNECTED -- LEAVE NOW */
	if((ucMissedMsgCnt) < OM2_BROKEN_LINK_MAX_COUNT) return(0);
	

	/*----- IF WE ARE HERE THE LINK IS CONSIDERED DEAD --------------------*/

	/* INC THE GLOBAL DEBUG COUNTERS */
	ucActionNum =(uchar)ulL2SRAM_getStblEntry(
							SCHED_ACTN_TBL_NUM,			//Tbl num
							ucGLOB_lastAwakeStblIdx		//Tbl Idx
							);
	if(ucActionNum == E_ACTN_ROM2) uiGLOB_lostROM2connections++;
	if(ucActionNum == E_ACTN_SOM2) uiGLOB_lostSOM2connections++;


	#if 1
	/* REPORT TO THE CONSOLE */
	vSERIAL_rom_sout("OM2>");
	vRAD40_showRad40((uint)ulL2SRAM_getStblEntry(
					 		SCHED_SN_TBL_NUM,		 //Tbl num
					 		ucGLOB_lastAwakeStblIdx //Tbl Idx
					 		));
	vSERIAL_rom_sout(" LkClose,ROM2Lost=");
	vSERIAL_UIV16out(uiGLOB_lostROM2connections);
	vSERIAL_rom_sout(", SOM2Lost=");
	vSERIAL_UIV16out(uiGLOB_lostSOM2connections);
	vSERIAL_crlf();
	#endif

	/* DELETE THIS SCHED TBL ENTRY */
	vSTBL_deleteStblEntry(ucGLOB_lastAwakeStblIdx);	//Tbl Idx

	/* CLEAR ANY RESIDUE IN THE LINK BLK TBL */
	vLNKBLK_clrSingleLnkBlk(ucGLOB_lastAwakeStblIdx);

	/* SHOW UPDATED SCHED TBL ENTRY */
	vSTBL_showSingleStblEntry(ucGLOB_lastAwakeStblIdx,NO_HDR,YES_CRLF);

	return(1);	//ret link broken flag

	}/* END: ucOPMODE_incLnkBrkCntAndChkForDeadOM2() */






/*************************  vOPMODE_SOM2()  **********************************
*
* Wait for OM1 and send off an OM2
*
******************************************************************************/

void vOPMODE_SOM2(
		void
		)
	{
	uchar ucc;
	uint uiOtherGuysSN;
	uint uiMySN;
	uchar ucMsgSeqNum;
	uchar ucIntegrityRetVal;
	uchar ucVacancy;
	uchar ucMaxLpCnt;
	uchar ucRetrieveFlagVal;
	uchar ucGenericOM2LinkByte;
	uchar ucGenericOM3LinkRetByte;
	uchar ucNewMsgFlags;
	uint uiLinkFailReason;

//	vT0_start_T0_timer();				//timing debug

	/* INIT */
	uiLinkFailReason = SOM2_BLANK_REASON;

	/* SET THE FREQUENCY */
	vRADIO_setBothFrequencys(
				ucRADIO_getChanAssignment(DATA_CHANNEL_INDEX),
				NO_FORCE_FREQ
				);

	/* START RECEIVER UP TO CAPTURE AN OM1 -- BEFORE ANYTHING ELSE */
	#ifndef SOM2_DIAG_ENABLED
	/* START RECEIVER FOR OM1 */
	vRADIO_start_receiver();
	#endif

	/* INC THE ATTEMPT COUNT */
	uiGLOB_SOM2attempts++;


	#ifdef SOM2_DIAG_ENABLED
	vOPMODE_recFake_OM1();
	#endif


	/* GET THE OTHER LINK'S SERIAL NUM */
	uiOtherGuysSN = (uint)ulL2SRAM_getStblEntry(
							SCHED_SN_TBL_NUM,			//Tbl num
							ucGLOB_lastAwakeStblIdx		//Tbl Idx
							);
	#if 1
	/* REPORT TO CONSOLE THE OTHER LINK SN */
	vSERIAL_rom_sout(">");
	vRAD40_showRad40(uiOtherGuysSN);
	vSERIAL_crlf();
	#endif


	/* GET MY SERIAL NUMBER */
	uiMySN = uiL2FRAM_getSnumLo16AsUint();


	/* SETUP A DEFAULT LINK BYTE AND STASH IT IN CASE OF A LINKUP FAILURE */
	ucGenericOM3LinkRetByte = LNKREQ_1FRAME_1LNK;
	vLNKBLK_fillLnkBlkFromMultipleLnkReq( ucGLOB_lastAwakeStblIdx,	//Tbl Idx
										  ucGenericOM3LinkRetByte,	//Lnk Confirm
										  lTIME_getSysTimeAsLong()
										  );


	/* WAIT FOR OM1 MSG */
	while(TRUE) //lint !e716 !e774
		{

		/* WAIT FOR OM1 OR TIMEOUT */
		if(!ucMSG_waitForMsgOrTimeout())
			{
			/* TIMED OUT */
			#if  1
			/* REPORT TO THE CONSOLE */
			vSERIAL_rom_sout("ROM1Tout\r\n");
			vTIME_showWholeSysTimeInDuS(YES_CRLF);
			#endif

			uiLinkFailReason = SOM2_OM1_MISSED;				//assume missed OM1

			if(ucOPMODE_incLnkBrkCntAndChkForDeadOM2())
				{
				uiLinkFailReason = SOM2_OM1_MISSED_LINK_BROKEN; //change reason
				}

			goto Exit_SOM2;		//leave
			}

		/* CHECK THE OM1 INTEGRITY */
		ucIntegrityRetVal = ucMSG_chkMsgIntegrity(  //RET: Bit Err Mask, 0 if OK
			CHKBIT_CRC+CHKBIT_MSG_TYPE+CHKBIT_GRP_SEL+CHKBIT_GID+CHKBIT_DEST_SN+CHKBIT_SRC_SN,
			CHKBIT_CRC+CHKBIT_MSG_TYPE+CHKBIT_GRP_SEL+CHKBIT_GID+CHKBIT_DEST_SN+CHKBIT_SRC_SN,
			MSG_TYPE_OM1,	//Msg Type
			uiOtherGuysSN,	//SRC SN
			uiMySN			//Dst SN
			);

		/* IF OM1 IS GOOD -- BREAKOUT */
		if(!ucIntegrityRetVal) break;

		/* OM1 WAS BAD */
		#if  1
		vSERIAL_rom_sout("ROM1:Bad,Retry\r\n");
		#endif

		/* RESTART THE RADIO & KEEP LOOKING FOR A GOOD OM1 */
		#ifdef SOM2_DIAG_ENABLED
		vOPMODE_recFake_OM1();			//fake a good OM1
		#else
		vRADIO_start_receiver();		//re-start the radio
		#endif

		}/* END: while() */



	/* OM1 IS GOOD */

	/* SAVE THE NEW OM1 TIME IN CLK2 -- SO ALARM TIME (CLK1) IS STILL GOOD */
	vTIME_setWholeClk2FromBytes((uchar *)&ucaMSG_BUFF[OM1_IDX_SYNC_TIME_XI]);

	/* SAVE THE OM1 VACANCY NUMBER */
	ucVacancy = ucaMSG_BUFF[OM1_IDX_AVAIL_MSG_SPACE];

	/* ZRO OUT THE MISSED MSG CNT IN SCHED STAT TBL */
	vOPMODE_zroMissedMsgCnt();

	#if  1
	/* REPORT TO THE CONSOLE THAT ROM1 HAS BEEN RECEIVED */
	vRAD40_showRad40(uiOtherGuysSN);
	vSERIAL_rom_sout(">ROM1\r\n");
	#endif

	/* SETUP TO COUNT THE OM2 MSGS */
	ucMaxLpCnt = TOP_OM2_MSG_COUNT;
	if(ucMaxLpCnt > ucVacancy) ucMaxLpCnt = ucVacancy;
//	ucMaxLpCnt = 0;		//debug:  force only a single message

	#if 0
	vSERIAL_rom_sout("LpMsgCnt= ");
	vSERIAL_HB4out(ucMaxLpCnt);
	vSERIAL_crlf();
	#endif


	/*-----------  SEND-OM2/RECEIVE-OM3  ---------------------------*/

	for(ucc=0;  ucc<=ucMaxLpCnt;  ucc++)
		{
		/* COMPUTE THE LAST_PKT_BIT FOR THIS TRANSMISSION */
		ucNewMsgFlags = 0;
		if(ucc == ucMaxLpCnt)
			{
			ucNewMsgFlags = OM2_LAST_PKT_BIT;
			}

		/* LOAD THE OM2 LINK BYTE */
		ucGenericOM2LinkByte = ucOPMODE_getOM2LinkByteVal();

		/* BUILD THE OM2 MSG */
		ucRetrieveFlagVal = ucOPMODE_RetrieveMsg_OM2(
									uiOtherGuysSN,
									ucNewMsgFlags,
									ucGenericOM2LinkByte
									);

		/* SAVE A LOCAL COPY OF THE OM2 SEQ NUM */
		ucMsgSeqNum = ucaMSG_BUFF[OM2_IDX_MSG_SEQ_NUM];

		/* QUIT NOW IF NO TIME IS LEFT */
		if(ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT)
			{
			#if 1
			vSERIAL_rom_sout("SOM2:ToutX\r\n");
			#endif

			/* CHECK IF WE SENT AT LEAST ONE OM2 */
			if(ucc == 0)
				{
				/* WUPS -- WE NEVER SENT A SINGLE OM2 */
				uiLinkFailReason = SOM2_TIMEOUT_NONE_SENT;
				goto Update_and_exit_SOM2;
				}

			break;
			}

		/* CHECK IF WE HAVE ENOUGH TIME TO DO THE NEXT ONE */
		if(uiTIME_getT3AsUint() > (65535 - 2458))
			{
			#if 1
			vSERIAL_rom_sout("SOM2:300ms-X\r\n");
			#endif

			/* CHECK IF WE SENT AT LEAST ONE OM2 */
			if(ucc == 0)
				{
				/* WUPS -- WE NEVER SENT A SINGLE OM2 */
				uiLinkFailReason = SOM2_TIMEOUT_BELOW_300MS_LIMIT;
				goto Update_and_exit_SOM2;
				}

			break;
			}


		/* SEND THE SOM2 MSG */
		vRADIO_xmit_msg();

		/* CHECK IF WE ARE DONE (NO OM3 TO BE RECEIVED) */
		if(ucRetrieveFlagVal & OM2_NODATA_BIT)
			{
			#if 1
			vSERIAL_rom_sout("SOM2:NoDatX\r\n");
			#endif
			break;
			}
		if(ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT)
			{
			#if 1
			vSERIAL_rom_sout("SOM2:T3ToutNoOM3-X\r\n");
			#endif
			break;
			}

		#ifndef SOM2_DIAG_ENABLED
		/* START RECEIVER IMMEDIATELY FOR OM3 */
		vRADIO_start_receiver();
		#endif

		#if  1
		/* REPORT ON SENDING THE OM2 MSG */
		vSERIAL_UIV8out(ucc);			//msg num
		vSERIAL_rom_sout(":SOM2>");
		vRAD40_showRad40(uiOtherGuysSN);
		vSERIAL_crlf();
		#endif

		/* WAIT FOR OM3 */
		while(TRUE)		//lint !e716 !e774
			{

			#ifdef SOM2_DIAG_ENABLED
			vOPMODE_recFake_OM3(uiOtherGuysSN,		//Src SN
								uiMySN,				//Dest SN
								ucMsgSeqNum			//Msg num
								);
			#endif

			/* WAIT FOR RECEIVED OM3 OR TIMEOUT */
			if(!ucMSG_waitForMsgOrTimeout())
				{
				#if  1
				vSERIAL_rom_sout("ROM3:ToutX\r\n");
				#endif
				uiLinkFailReason = SOM2_OM3_MISSED;
				goto Update_and_exit_SOM2;
				}

			/* CHK THE OM3 MSG INTEGRITY */
			ucIntegrityRetVal = ucMSG_chkMsgIntegrity(  //RET: Bit Err Mask, 0 if OK
				CHKBIT_CRC+CHKBIT_MSG_TYPE+CHKBIT_GRP_SEL+CHKBIT_GID+CHKBIT_DEST_SN+CHKBIT_SRC_SN,
				CHKBIT_CRC+CHKBIT_MSG_TYPE+CHKBIT_GRP_SEL+CHKBIT_GID+CHKBIT_DEST_SN+CHKBIT_SRC_SN,
				MSG_TYPE_OM3,	//MSG TYPE
				uiOtherGuysSN,	//src SN
				uiMySN			//Dst SN
				);

			/* CHK IF THE OM3 MSG SEQ NUM MATCHES THE SENT OM2 SEQ NUM */
			if(ucaMSG_BUFF[OM3_IDX_ACKED_SEQ_NUM] != ucMsgSeqNum)
				{
				#if  1
				vSERIAL_rom_sout("ROM3:SeqMiss\r\n");
				#endif
				ucIntegrityRetVal |= RETRIEVE_MSG_SEQ_MISMATCH_BIT;
				}

			/* IF OM3 IS GOOD -- BREAK OUT */
			if(!ucIntegrityRetVal)
				{
				/* IF FIRST REPLY THEN SAVE THE OM3 LINK REPLY */
				if(ucc == 0)
					{
					/* SAVE THE OM3 LINKUP REPLY */
					ucGenericOM3LinkRetByte = ucaMSG_BUFF[OM3_IDX_GENERIC_LINK_RETURN];

					vLNKBLK_fillLnkBlkFromMultipleLnkReq(
							ucGLOB_lastAwakeStblIdx,	//Tbl Idx
							ucGenericOM3LinkRetByte,	//Lnk Confirm
							lTIME_getSysTimeAsLong()
							);

					}/* END: if(ucc==0) */

				break;	//leave the loop, OM3 was good
				}


			/* OM3 WAS BAD */

			#if  1
			vSERIAL_rom_sout("ROM3:MsgRjt\r\n");
			#endif

			#ifndef SOM2_DIAG_ENABLED
			/* START RECEIVER IMMEDIATELY FOR OM3 */
			vRADIO_start_receiver();
			#endif

			}/* END: while() */


		/* OM3 WAS GOOD DELETE THE OM2 MSG FROM SRAM */
		vL2SRAM_delCurMsg();

		#if 1
		/* REPORT TO CONSOLE ON RECEIVING THE OM3 MSG */
		vSERIAL_rom_sout("ROM3:OK:LkRq=");
		vLNKBLK_showLnkReq(ucGenericOM3LinkRetByte);

		vSERIAL_crlf();
		#endif


		if(ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT)	break;
		if(ucRetrieveFlagVal & OM2_LAST_PKT_BIT) break;

		}/* END: for() */

Update_and_exit_SOM2:	//comes here if we HAVE received an OM1

	/* PUT RADIO ASLEEP */
	vRADIO_quit();

	/* UPDATE THE TIME */
	vTIME_setSysTimeFromClk2();

Exit_SOM2:				//comes here if we HAVE-NOT received an OM1

	/* PUT RADIO ASLEEP */
	vRADIO_quit();

	if(uiLinkFailReason != SOM2_BLANK_REASON)
		{
		/* REPORT THE LINK FAIL */
		vREPORT_buildReportAndLogIt(
			SENSOR_SOM2_LINK_INFO,			//ROM2 INFO ID	(Sensor #0)
			uiOtherGuysSN,					//DEST SN		(Sensor Data #0)
			SENSOR_SOM2_INFO_REASON,		//REASON ID		(Sensor #1)
			uiLinkFailReason,				//REASON		(Sensor Data #1)
			OPTPAIR_RPT_SOM2_FAILS_TO_RDIO,	//Radio Option Pair Idx
			OPTPAIR_RPT_SOM2_FAILS_TO_FLSH	//Flash Option Pair Idx
			);

		}

	#if  1
	vSTBL_showSingleStblEntry(ucGLOB_lastAwakeStblIdx, NO_HDR, YES_CRLF);
	vSERIAL_rom_sout("X\r\n");	
	#endif

	#if 0
	vT0_stop_T0_timer();
	vSERIAL_rom_sout("T0= ");
	vT0_show_T0_in_uS();
	vSERIAL_crlf();
	#endif

	return;

	}/* END: vOPMODE_SOM2() */







/*************************  vOPMODE_ROM2()  **********************************
*
* Send an OM1 to indicate wakeup, and receive the OM2 packets.
*
******************************************************************************/

void vOPMODE_ROM2(
		void
		)
	{
	uchar ucIntegrityRetVal;
	long lCurSec;
	long lSendMsgSec;
	uint uiOtherGuysSN;
	uchar ucaSendOM2Time[6];
	uchar ucXmitRetVal;
	uchar ucVacancyNum;
	uchar ucNoDataFlag;
	uchar ucLastPktFlag;
	uchar ucMsgNum;
	uchar ucMsgCount;
	uchar ucGenericOM2LinkByte;
	uchar ucGenericOM3LinkRetByte;
	uint uiLinkFailReason;

//	vT0_start_T0_timer();				//debug Timer

	/* INIT */
	uiLinkFailReason = ROM2_BLANK_REASON;

	/* SET THE FREQUENCY */
	vRADIO_setBothFrequencys(
				ucRADIO_getChanAssignment(DATA_CHANNEL_INDEX),
				NO_FORCE_FREQ
				);

	/* INC THE ATTEMPT COUNT */
	uiGLOB_ROM2attempts++;

	/* GET THE SERIAL NUMBER FOR DEST (FOREIGN SN) */
	uiOtherGuysSN = (uint)ulL2SRAM_getStblEntry(SCHED_SN_TBL_NUM,ucGLOB_lastAwakeStblIdx);

	#if 1
	vSERIAL_rom_sout("<");
	vRAD40_showRad40(uiOtherGuysSN);
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
		vSERIAL_rom_sout("ROM2:TimSlip,Abrt\r\n");
		#endif
		uiLinkFailReason = ROM2_OM1_START_TIME_MISMATCH;
		goto Exit_ROM2;
		}

	/* GET THE VACANCY NUMBER */
	ucVacancyNum = (uchar)uiL2SRAM_getVacantMsgCount();
	if(ucVacancyNum > 15) ucVacancyNum = 15; //limit the hi range


	/* PACK THE OM1 MSG INTO THE BUFFER */
	vOPMODE_buildMsg_OM1(
			uiOtherGuysSN,			//dest serial num
			lCurSec,				//current time in sec
			ucVacancyNum			//room in host
			);


	/* SAVE TIME INTO WHOLE TIME ARRAY */
	vMISC_copyUlongIntoBytes(
				(ulong)lSendMsgSec,
				&ucaSendOM2Time[0],
				NO_NOINT
				);
	vMISC_copyUintIntoBytes(
				MSG_ST_DELAY_IN_CLKS_UI,
				&ucaSendOM2Time[4],
				NO_NOINT
				);

	#if 0
	/* SHOW THE ACTUAL OM1 REQUIRED START TIME */
	vSERIAL_rom_sout("OM1snd= ");
	vTIME_showWholeTimeInDuS(&ucaSendOM2Time[0], YES_CRLF);
	vSERIAL_rom_sout("OM1sync= ");
	vTIME_showWholeTimeInDuS((uchar *)&ucaMSG_BUFF[OM1_IDX_SYNC_TIME_XI], YES_CRLF);
	#endif


	/* SETUP A DEFAULT LINK BYTE IN CASE OF FAILURE */
	ucGenericOM3LinkRetByte = LNKREQ_1FRAME_1LNK;
	vLNKBLK_fillLnkBlkFromMultipleLnkReq( ucGLOB_lastAwakeStblIdx,	//Tbl Idx
										  ucGenericOM3LinkRetByte,	//Lnk Confirm
										  lTIME_getSysTimeAsLong()
										  );


	/* SEND THE OM1 */
	#ifdef ROM2_DIAG_ENABLED
	ucXmitRetVal = ucMSG_doSubSecXmit(&ucaSendOM2Time[0], USE_CLK1, NO_RECEIVER_START);
	#else
	ucXmitRetVal = ucMSG_doSubSecXmit(&ucaSendOM2Time[0], USE_CLK1, YES_RECEIVER_START);
	#endif

	/* CHECK IF IT WAS TOO LATE TO START AN OM1 */
	if(ucXmitRetVal == 0)
		{
		#if 1
		vSERIAL_rom_sout("OM1:XmtLate\r\n");
		#endif
		uiLinkFailReason = ROM2_OM1_TOO_LATE_TO_SEND;
		goto Exit_ROM2;		//too late -- exit the slot
		}

	#if 1
	vSERIAL_rom_sout("SOM1>");
	vRAD40_showRad40(uiOtherGuysSN);
	vSERIAL_crlf();
	#endif


	/* RECEIVE OM2 & MAKE OM3 REPLYS */
	for(ucMsgCount = 0;  ;  ucMsgCount++)
		{
		#ifdef ROM2_DIAG_ENABLED
		vOPMODE_recFake_OM2();
		#endif


		/* WAIT FOR OM2 OR TIMEOUT */
		if(!ucMSG_waitForMsgOrTimeout())
			{
			/* ROM2 TIMEOUT */
			#if  1
			vSERIAL_rom_sout("ROM2:Tout\r\n");
			#endif

			/* IF THIS WAS THE FIRST OM2 -- BUMP LNK FAIL CNT */
			if(ucMsgCount == 0)
				{

				uiLinkFailReason = ROM2_MISSED;	//assume its a simple miss

				if(ucOPMODE_incLnkBrkCntAndChkForDeadOM2())
					{
					uiLinkFailReason = ROM2_LINK_BROKEN;	//wups this is the big fail
					}

				}/* END: if(ucMsgCount==0) */

			goto Exit_ROM2;
			}

		#if 0
		vSERIAL_rom_sout("M-");
		#endif

		/* CHECK THE INTEGRITY OF THE OM2 */
		ucIntegrityRetVal = ucMSG_chkMsgIntegrity(  //RET: BitMask if BAD,  0 if GOOD
			CHKBIT_CRC+CHKBIT_MSG_TYPE+CHKBIT_GRP_SEL+CHKBIT_GID+CHKBIT_DEST_SN+CHKBIT_SRC_SN,
			CHKBIT_CRC+CHKBIT_MSG_TYPE+CHKBIT_GRP_SEL+CHKBIT_GID+CHKBIT_DEST_SN+CHKBIT_SRC_SN,
			MSG_TYPE_OM2,
			uiOtherGuysSN,					//SRC SN
			uiL2FRAM_getSnumLo16AsUint()	//DEST SN
			);

		/* IF OM2 WAS BAD -- RETRY */
		if(ucIntegrityRetVal)
			{
			#ifndef ROM2_DIAG_ENABLED
			/* RESTART THE RADIO */
			vRADIO_start_receiver();
			#endif

			#if 1
			vSERIAL_rom_sout("ROM2:Bd\r\n");
			#endif

			continue;
			}

		/* OM2 WAS GOOD */


		/* SAVE THE OM2 MSG FLAGS */

		/* SAVE THE OM2 NO PKT FLAG */
		ucNoDataFlag = 0;
		if(ucaMSG_BUFF[OM2_IDX_EOM_IDX] & OM2_NODATA_BIT)
			{
			ucNoDataFlag = 1;
			}

		/* SAVE THE OM2 LAST PKT FLAG */
		ucLastPktFlag = 0;
		if(ucaMSG_BUFF[OM2_IDX_EOM_IDX] & OM2_LAST_PKT_BIT)
			{
			ucLastPktFlag = 1;
			}

		/* SAVE THE OM2 MSG SEQUENCE NUMBER */
		ucMsgNum = ucaMSG_BUFF[OM2_IDX_MSG_SEQ_NUM];

		/* STASH A LOCAL COPY OF OM2 LINK BYTE */
		ucGenericOM2LinkByte = ucaMSG_BUFF[OM2_IDX_GENERIC_LINK_BYTE];

	    #if  1
		/* REPORT ON CONSOLE THAT WE GOT AN OM2 MSG */
		vSERIAL_UIV8out(ucMsgCount);
		vSERIAL_rom_sout(":ROM2<");
		vRAD40_showRad40(uiOtherGuysSN);
		vSERIAL_rom_sout("=OK.");
		if(ucNoDataFlag)	vSERIAL_rom_sout("NoDat,");
		if(ucLastPktFlag)   vSERIAL_rom_sout("LastPkt");
		vSERIAL_crlf();
		#endif


		/* SAVE THIS OM2 PACKET IF IT HAS DATA IN IT */
		if(!ucNoDataFlag)
			{
			/* STASH THE OM2 MSG */
			vL2SRAM_storeMsgToSram();

			}/* END: if(NOT (NO_DATA)) */


		/* IF WE ARE TOO CLOSE TO END OF TIME SLOT -- LEAVE */
		if(ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT)
			{
			#if 1
			vSERIAL_rom_sout("SOM3:Tout\r\n");
			#endif
			break;
			}


		/* GET A LINKUP DCNT BASED ON OM2 LFACTOR */
		ucGenericOM3LinkRetByte = ucOPMODE_computeLnkRetByte(
										ucGenericOM2LinkByte
										);


		/* BUILD THE OM3 REPLY*/
		vOPMODE_buildMsg_OM3(
					OM3_MSG_EMPTY_PKT_BIT,			//Byte 0 Flags
					0,								//Byte 1 Flags
					uiOtherGuysSN,					//Dest serial num
					ucMsgNum,						//Msg seq number
					ucGenericOM3LinkRetByte			//reply load factor
					);

		/* SEND OM3 */
		vRADIO_xmit_msg();

		/* ZRO THE STS MISSED MSG COUNT */
		vOPMODE_zroMissedMsgCnt();

		/* STASH NEW LINK REQ */
		if(ucMsgCount == 0)
			{
			/* STASH THE NEW OM3 LNKREQ */
			vLNKBLK_fillLnkBlkFromMultipleLnkReq(
					ucGLOB_lastAwakeStblIdx,	//Tbl Idx
					ucGenericOM3LinkRetByte,	//Lnk Confirm
					lTIME_getSysTimeAsLong()
					);

			}/* END: if(ucMsgCount==0) */

		/* IF WE ARE AT THE END OF SLOT -- LEAVE */
		if(ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT)
			{
			#if 1
			vSERIAL_rom_sout("SOM3:Tout-norml\r\n");
			#endif
			break;
			}

		#ifndef ROM2_DIAG_ENABLED
		/* RESTART THE RADIO */
		vRADIO_start_receiver();
		#endif

		#if 1
		/* REPORT TO CONSOLE OM3 WAS SENT */
		vSERIAL_rom_sout("SOM3>");
		//vSERIAL_UIV16out(uiOtherGuysSN);
		vRAD40_showRad40(uiOtherGuysSN);

		vSERIAL_rom_sout(" Lk=");
		vLNKBLK_showLnkReq(ucGenericOM3LinkRetByte);

		vSERIAL_crlf();
		#endif

		if(ucLastPktFlag || ucNoDataFlag)
			{
			#if 1
			vSERIAL_rom_sout("ROM2:X\r\n");
			#endif
			break;
			}

		}/* END: for(ucMsgCount) */

Exit_ROM2:

	vRADIO_quit();			//go into sleep

	if(uiLinkFailReason != ROM2_BLANK_REASON)
		{
		vREPORT_buildReportAndLogIt(
			SENSOR_ROM2_LINK_INFO,			//ROM2 INFO ID	(Sensor #0)
			uiOtherGuysSN,					//DEST SN		(Sensor Data #0)
			SENSOR_ROM2_INFO_REASON,		//REASON ID		(Sensor #1)
			uiLinkFailReason,				//REASON		(Sensor Data #1)
			OPTPAIR_RPT_ROM2_FAILS_TO_RDIO,	//Radio OptionPairIdx
			OPTPAIR_RPT_ROM2_FAILS_TO_FLSH	//Flash OptionPairIdx
			);
		}

	#if 0
	/* REPORT TO CONSOLE THE TIME T0 DATA */
	vT0_stop_T0_timer();
	vSERIAL_rom_sout("T0= ");
	vT0_show_T0_in_uS();
	vSERIAL_crlf();
	#endif

	#if  0
	/* REPORT TO CONSOLE THE UPDATED TBL ENTRY */
	vSTBL_showSingleStblEntry(ucGLOB_lastAwakeStblIdx, NO_HDR, YES_CRLF);
	vSERIAL_rom_sout("X\r\n");	
	#endif

	return;

	}/* END: vOPMODE_ROM2() */






/*********************  vOPMODE_zroMissedMsgCnt()  ****************************
*
*
*
******************************************************************************/

static void vOPMODE_zroMissedMsgCnt(
		void
		)
	{
	uint uiStsVal;

	/* IF SCHEDULER HAS RUN WE UPDATE THE NEXT STS TBL */
	/* IF SCHEDULER HAS NOT RUN WE UPDATE THE CUR STS TBL */

	/* READ THE STATUS VALUE */
	uiStsVal = (uint)ulL2SRAM_getStblEntry(
							ucaStsTblNum[ucRTS_lastScheduledNSTnum()],
							ucGLOB_lastAwakeStblIdx
							);

	/* CLEAR THE MISSED MSG COUNT */
	uiStsVal = (uiStsVal & 0x00FF);

	/* WRITE THE STATUS VALUE BACK */
	vL2SRAM_putStblEntry(
						ucaStsTblNum[ucRTS_lastScheduledNSTnum()],
						ucGLOB_lastAwakeStblIdx,
						(ulong)uiStsVal
						);


	return;

	}/* END: vOPMODE_zroMissedMsgCnt() */




/******************  ucOPMODE_getOM2LinkByteVal()  ***************************
*
*
*
*****************************************************************************/
uchar ucOPMODE_getOM2LinkByteVal(
		void
		)
	{
	uchar ucOM2LinkVal;

	ucOM2LinkVal = ucLNKBLK_computeMultipleLnkReqFromSysLoad(
						iGLOB_completeSysLFactor
						);

	return(ucOM2LinkVal);

	}/* END: ucOPMODE_getOM2LinkByteVal() */






/******************  ucOPMODE_computeLnkRetByte()  ***************************
*
*
*
******************************************************************************/

uchar ucOPMODE_computeLnkRetByte(
		uchar ucGenericOM2LinkByte
		)
	{

	return(ucGenericOM2LinkByte);				//for now -- just say OK


	}/* END: ucOPMODE_computeLnkRetByte() */




/*-------------------------------  MODULE END  ------------------------------*/

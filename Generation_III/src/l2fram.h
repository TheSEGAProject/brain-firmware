
/***************************  L2FRAM.H  **************************************
*
* Header for L2FRAM routine pkg
*
*
* V2.07 12/20/2006 wzr
*	Added SENSE_ACT bytes to FRAM STARTUP BLKS.
*
* V2.06 04/28/2006 wzr
*	Renamed FRAM_SYS_FLAGS_ADDR to be FRAM_CONFIG_BYTE_ADDR to reflect the
*	fact that this location contains a copy of the loaded (ROM) config byte.
*	Added FRAM_SYS_FLAGS_ADDR to save value of data streaming.
*
* V1.00 12/28/2004 wzr
*	started
*
******************************************************************************/

#ifndef L2FRAM_H_INCLUDED
 #define L2FRAM_H_INCLUDED

 #define YES_ABBREVIATE		1
 #define  NO_ABBREVIATE		0


/* NOTE: defines have been moved inside L2FRAM.C to reduce define count */


// #define FRAM_ID_ADDR_XI						0	//4 bytes
// #define FRAM_ID_ADDR_HI						1
// #define FRAM_ID_ADDR_MD						2
// #define FRAM_ID_ADDR_LO						3
//	#define FRAM_ID_VAL_XI							'F'
//	#define FRAM_ID_VAL_HI							'R'
//	#define FRAM_ID_VAL_MD							'A'
//	#define FRAM_ID_VAL_LO							'M'

// #define FRAM_VER_ADDR						4	//2 bytes 
// #define FRAM_VER_ADDR_HI						4
// #define FRAM_VER_ADDR_LO						5
	 #define FRAM_VERSION_HI		0x02
	 #define FRAM_VERSION_LO		0x09
	 #define FRAM_VERSION (((uint)FRAM_VERSION_HI<<8) | ((uint)FRAM_VERSION_LO))

 #define FRAM_TEST_ADDR 						6	//4 bytes
	#define FRAM_TEST_ADDR_XI		6
	#define FRAM_TEST_ADDR_HI		7
	#define FRAM_TEST_ADDR_MD		8
	#define FRAM_TEST_ADDR_LO		9

//	#define FRAM_TEST_VAL_POSITIVE_UL	0xCC118855
//	#define FRAM_TEST_VAL_NEGATIVE_UL	(~FRAM_TEST_VAL_POSITIVE_UL)

 #define FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR		10	//4 bytes
 #define FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR		14	//4 bytes
 #define FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR	18	//4 bytes
 #define FRAM_FLSH_REFRESH_PAGE_NFL_ADDR		22	//2 bytes

 #define FRAM_TIME_SAVE_AREA_ADDR				24	//4 bytes
 #define FRAM_REBOOT_COUNT_ADDR					28	//2 bytes
 #define FRAM_USER_ID_ADDR						30	//2 bytes


 #define FRAM_OPTION_BYTE_0_ADDR				32	// Option byte array
 #define FRAM_OPTION_BYTE_1_ADDR				33	
 #define FRAM_OPTION_BYTE_2_ADDR				34
 #define FRAM_OPTION_BYTE_3_ADDR				35
 #define FRAM_OPTION_BYTE_4_ADDR				36


 #define FRAM_OPTION_IDX_ADDR					39

 /*------------------------------------*/
 /* NOTE: GAP HERE FOR MORE PARAMETERS */
 /*------------------------------------*/


 #define FRAM_ST_BLK_COUNT_ADDR					40	//1 byte

 #define FRAM_ST_BLK_0_ADDR						41	//Blk = 10 bytes long
//	#define FRAM_ST_BLK_0_ACTION_ADDR			41	//1 byte
//	#define FRAM_ST_BLK_0_FLAGS_ADDR			42	//1 byte
//	#define FRAM_ST_BLK_0_HR_ST_ADDR			43	//2 bytes
//	#define FRAM_ST_BLK_0_INTERVAL_ADDR			45	//2 bytes

	#define FRAM_ST_BLK_ACTION_IDX				 0	//idx from blk st (1 byte)
	#define FRAM_ST_BLK_FLAGS_IDX				 1	//(1 byte)
	#define FRAM_ST_BLK_HR_ST_IDX				 2	//(2 bytes)
	#define FRAM_ST_BLK_INTERVAL_IDX			 4	//(2 bytes)
	#define FRAM_ST_BLK_SENSE_ACT_IDX			 6	//(4 bytes)

    #define FRAM_ST_BLK_SIZE				10




  #define FRAM_LAST_ST_BLK_ADDR				351


/*************************  NOTE  ********************************************
*
* The trigger areas is indexed by device number.  At the time of this writing
* there were 76 devices.  The trigger area is allocated for 80 devices. 
* Each device takes 2 bytes of storage.
*
******************************************************************************/

  #define FRAM_Y_TRIG_AREA_BEG_ADDR			352

  #define FRAM_Y_TRIG_AREA_END_ADDR			511





 #define FRAM_CHK_REPORT_MODE	1
 #define FRAM_CHK_SILENT_MODE	0



 /* ROUTINE DEFINITIONS */

/*--------------------------------*/

 uchar ucL2FRAM_chk_for_fram(
		uchar ucReportMode	//FRAM_CHK_SILENT_MODE, FRAM_CHK_REPORT_MODE
		);

 void vL2FRAM_format_fram(
 		void
 		);

 void vL2FRAM_initFramFlashPtrs(
		void
		);

 uint uiL2FRAM_chk_for_fram_format(
 		void
		);

 void vL2FRAM_force_fram_unformat(
 		void
		);

 void vL2FRAM_writeDefaultStartBlkList(
		void
		);


/*--------------------------------*/

 uint uiL2FRAM_get_version_num(
 		void
		);

/*--------------------------------*/

 long lL2FRAM_getLinearFlashPtr(
		uint uiFlashPtrFRAMaddr
		);

 void vL2FRAM_putLinearFlashPtr(
		uint uiFlashPtrFRAMaddr,
		long lFlashPtrToStore
		);

 void vL2FRAM_showLinearFlashPtr(
		long lFlashPtrToShow
		);

 void vL2FRAM_incFlashOnPtr(
 		void
		);

 void vL2FRAM_incFlashOffPtr(
 		void
		);

 void vL2FRAM_incFlashReadPtr(
 		void
		);

/*--------------------------------*/

 int iL2FRAM_getFlashRefreshPtr(
 		void
		);

 void vL2FRAM_incFlashRefreshPtr(
 		void
		);

/*--------------------------------*/


 void vL2FRAM_expungeAllUploadedFlashMsgs(
 		void
		);

 void vL2FRAM_undeleteAllUploadedFlashMsgs(
 		void
		);

 long lL2FRAM_getFlashUnreadMsgCount(
 		void
		);

 long lL2FRAM_getFlashReadMsgCount(
 		void
		);

 long lL2FRAM_getFlashFreeMsgCount(
 		void
		);

/*--------------------------------*/

 void vL2FRAM_stuffSavedTime(
		ulong ulSavedTimeVal
		);

 ulong ulL2FRAM_getSavedTime(
 		void
		);

/*--------------------------------*/

 void vL2FRAM_writeRebootCount(
		uint uiNewRebootCountVal
		);

 uint uiL2FRAM_getRebootCount(
 		void
		);

 void vL2FRAM_incRebootCount(
 		void
		);

/*--------------------------------*/

uchar ucL2FRAM_isHub(
 		void
		);

uchar ucL2FRAM_isSender(
		void
		);

uchar ucL2FRAM_isSampler(
		void
		);

uchar ucL2FRAM_isReceiver(
		void
		);

/*--------------------------------*/

 uint uiL2FRAM_getSnumLo16AsUint(
 		void
		);

 void vL2FRAM_copySnumLo16ToBytes(
		uchar *ucpToPtr
		);

 uchar ucL2FRAM_getSnumMd8(
 		void
		);

 uchar ucL2FRAM_getSnumLo8(
 		void
		);

 void vL2FRAM_setSysID(
		uint uiSysID
		);

 void vL2FRAM_showSysID(
		void
		);

/*--------------------------------*/

uchar ucL2FRAM_getNextFreeStBlk(	//Ret: next free StBlk Num
		void
		);

uchar ucL2FRAM_getStBlkTblCount(
		void
		);

void vL2FRAM_setStBlkTblCount(
		uchar ucBlkCount
		);


/*--------------------------------*/


ulong ulL2FRAM_getStBlkEntryVal(	//RET: StBlk Entry Val
		uchar ucStBlkNum,			//blk number
		uchar ucStBlkEntryIdx		//index into the blk
		);

void vL2FRAM_putStBlkEntryVal(		//Stuff the Value int the StBlk
		uchar ucStBlkNum,			//blk number
		uchar ucStBlkEntryIdx,		//index into the blk
		ulong ulVal					//value to put
		);

void vL2FRAM_stuffStBlk(
		uchar ucStBlkNum,
		uchar ucActionNum,
		uchar ucFlagVal,
		uint  uiHrStartVal,
		uint  uiSampleIntervalVal,
		ulong ulSenseActVal
		);


uchar ucL2FRAM_findStBlkAction(		//RET: StBlk num,  255 if none
 		uchar ucDesiredActionNum	//action number to search for
		);

uchar ucL2FRAM_addRuntimeStBlk(		//RET: StBlk num of new entry
 		uchar ucDesiredActionNum	//action number to search for
		);

void vL2FRAM_deleteStBlk(			//remove an entry from StBLk
 		uchar ucStBlkNum			//StBlk Num
		);

void vL2FRAM_showStBlkTbl(
 		void
		);

/*--------------------------------*/

void vL2FRAM_putYtriggerVal(	//Stuff Val into FRAM trigger Area
		uchar ucSensorNum,
		uint uiYtriggerVal
		);

uint uiL2FRAM_getYtriggerVal(	//ret trigger Val from FRAM
		uchar ucSensorNum
		);

void vL2FRAM_showYtriggerTbl(
		void
		);

/*--------------------------------*/


#endif /* L2FRAM_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */

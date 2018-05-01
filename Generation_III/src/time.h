
/***************************  TIME.H  ****************************************
*
* Header for TIME routine pkg
*
* V1.00 3/3/2004 wzr
*	started
*
******************************************************************************/

/* NOTE: Read overview in TIME.c */

#ifndef TIME_H_INCLUDED
	#define TIME_H_INCLUDED



	#define FRAME_SLOT_TIME		1
	#define HEX_SEC_TIME		2
	#define INT_SEC_TIME		3


	#define FILL_LEAD_WITH_SPACES	TRUE
	#define FILL_LEAD_WITH_ZEROS	FALSE

	#define YES_RESET_TIME 1
	#define  NO_RESET_TIME 0

	#define YES_SKP_ECLK_RAM_FLAG	1
	#define NO_SKP_ECLK_RAM_FLAG	0

	#define DEFAULT_HR_OFFSET_IN_SEC 0			//default hour offset


	/* ROUTINE DEFINITIONS */

 void vTIME_init(
		void
		);

 long lTIME_getSysTimeAsLong(
		void
		);

 void vTIME_copySysTimeToBytes(
		uchar *ucpToPtr				//4 bytes
		);

 void vTIME_setSysTimeFromLong(
		long lNewSysTime
		);

 void vTIME_setSysTimeFromClk2(
		void
		);

/*---------------------------------*/

uchar ucTIME_getECLKsts(//0=noPing, 1=OfflineFlg, 2=notTicking, 3=OK
		uchar ucSkpRamFlag		//YES_SKP_ECLK_RAM_FLAG, NO_SKP_ECLK_RAM_FLAG
		);

long lTIME_getECLKtimeAsLong(
 		void
		);

void vTIME_setECLKtimeFromLong(
 		long lNewECLKtime
		);

/*---------------------------------*/

 uint uiTIME_getSubSecAsUint(
		void
		);

 void vTIME_copySubSecToBytes(
		uchar *ucpToPtr
		);

 void vTIME_setSubSecFromUint(
		uint uiNewSubSec
		);

 void vTIME_setSubSecFromBytes(
		uchar *ucpFromPtr
		);

/*---------------------------------*/

 void vTIME_copyWholeSysTimeToBytesInDtics(
		uchar *ucpToPtr		//6 bytes (Ret = Disjoint Tics)
		);

 void vTIME_copyWholeSysTimeToBytesInLtics(
		uchar *ucpToPtr		//6 bytes (Ret = Linear Tics)
		);

 void vTIME_setWholeSysTimeFromBytesInDtics(
		uchar *ucpFromPtr	//6 bytes ptr (Src = Disjoint Tics)
		);

 void vTIME_showWholeSysTimeInDuS(	//shown in decimal uS form
		uchar ucCRLF_termFlag	//YES_CRLF, NO_CRLF
		);

/*---------------------------------*/

 void vTIME_showWholeTimeInDuS(	//shown in Decimal uS form
		uchar *ucpTimeArray,	//6 bytes (in Disjoint time format)
		uchar ucCRLF_termFlag	//YES_CRLF, NO_CRLF
		);

 void vTIME_showWholeTimeDiffInDuS(	//shown in Decimal uS form
		uchar *ucpBegTimeArray,	//6 bytes ptr (Disjoint time format)
		uchar *ucpEndTimeArray,	//6 bytes ptr (Disjoint time format)
		uchar ucCRLF_termFlag  	//YES_CRLF, NO_CRLF
		);

 ulong ulTIME_computeTimeDiffInLtics(
		uchar *ucpBegTime_inDtics,	//6 bytes (Disjoint time format)
		uchar *ucpEndTime_inDtics,	//6 bytes (Disjoint time format)
		uchar *ucpDiffTime_inLtics  //6 bytes (Linear Time format)
		);

 void vTIME_convertDticsToLtics(
 		uchar *ucpSrcTimeArray,	  //6 bytes (in Disjoint time format)
		uchar *ucpLinearTimeArray //6 bytes (in Linear time format)
		);

/*---------------------------------*/

 long lTIME_getAlarmAsLong(
		void
		);

 void vTIME_copyAlarmToBytes(
		uchar *ucpToPtr
		);

 void vTIME_setAlarmFromLong(
		long lNewAlarm
		);

 void vTIME_setAlarmFromBytes(
		uchar *ucpFromPtr
		);

/*---------------------------------*/

 int iTIME_getHr0_to_sysTim0_asInt(
		void
		);

 void vTIME_copyHr0_to_sysTim0_toBytes(
		uchar *ucpToPtr
		);

/*---------------------------------*/

 long lTIME_get_Hr_From_SysTim_InSecs(
		long lUpTimeSecs
		);

 int iTIME_get_HrOffset_From_SysTim_InSecs(
		long lUpTimeSecs
		);

/*---------------------------------*/

 long lTIME_getClk2AsLong(
		void
		);

 void vTIME_copyClk2ToBytes(
		uchar *ucpToPtr
		);

 void vTIME_setClk2FromLong(
		long lNewClk2
		);

 void vTIME_setClk2FromBytes(
		uchar *ucpFromPtr
		);

/*---------------------------------*/

 void vTIME_copyWholeClk2ToBytes(
		uchar *ucpToPtr				//6 bytes (Ret = Disjoint time format)
		);

 void vTIME_setWholeClk2FromBytes(
		uchar *ucpFromPtr			//6 bytes ptr (Src = Disjoint time format)
		);

 void vTIME_showWholeClk2(			//shown in uS decimal form
		uchar ucCRLF_termFlag		//YES_CRLF, NO_CRLF
		);

/*---------------------------------*/


 uint uiTIME_getT3AsUint(
		void
		);

 uchar ucTIME_setT3AlarmToSecMinus200ms(
 		// RET:0=OK, 1=Too Late, 2=Too Close, 3=Too Early
		long lSlotEndTime
		);

/*---------------------------------*/

long lTIME_convertTimeToLinearSlotNum(
		long lTime
		);

long lTIME_getFrameNumFromTime(
		long lTime
		);

long lTIME_getSlotNumFromTime(
		long lTime
		);

void vTIME_showTime(
		long lTime,
		uchar ucTimeFormFlag,	//FRAME_SLOT_TIME, HEX_SEC_TIME, INT_SEC_TIME
		uchar ucCRLF_flag		//NO_CRLF,YES_CRLF
		);


/*---------------------------------*/



#endif /* TIME_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */

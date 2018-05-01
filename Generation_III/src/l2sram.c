
/**************************  L2SRAM.C  *****************************************
*
* Routines to handle the static ram store and forward data area
*
*
******************************************************************************/

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e716 */		/* while(1) */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */



#include "STD.h"			//standard defines
//#include "DIAG.h"			//diagnostic defines
#include "config.h"			//configuration parameters 
//#include "MAIN.H"			//
//#include "MISC.H"			//homeless functions
//#include "SENSOR.h"  		//Sensor names routines
#include "SRAM.H"			//level 1 sram memory routines
#include "L2SRAM.h"			//level 2 sram memory routines
#include "SERIAL.h"			//serial port routines
#include "MSG.h"			//message routines
//#include "RTS.h"			//real time sched
#include "MODOPT.h"			//Modify Options routines



/**********************  EXTERNS  ********************************************/

extern volatile uchar ucaMSG_BUFF[ MAX_RESERVED_MSG_SIZE ];

extern usl   uslGLOB_sramQon_NFL;
extern usl   uslGLOB_sramQoff;
extern uint  uiGLOB_sramQcnt;

extern const USL uslStblBaseAddr[MAX_SCHED_TBL_NUM];
extern const uchar ucStblByteWidth[MAX_SCHED_TBL_NUM];




/**********************  TABLES  ********************************************/

#define MAX_TBL_NUM 20


const char *cpaSRAMtblNames[MAX_TBL_NUM] =
	{
	"P_FUNC",				// 0
	"SN",					// 1
	"RND_1",				// 2
	"RND_2",				// 3
	"DCNT_1",				// 4
	"DCNT_2",				// 5
	"S_TIM",				// 6
	"INTVL",				// 7
	"FLGS",					// 8
	"STS_1",				// 9
	"STS_2",				//10
	"ACTN",					//11
	"SNSACT",				//12
	"NST_1",				//13
	"NST_2",				//14
	"SSP_Y",				//15
	"SSP_T",				//16
	"Y_TRIG",				//17
	"T_TRIG",				//18
	"STOR_FWD"				//19
	}; /* END: cpaSRAMtblNames[] */


const uint uiaSRAMtblBaseAddr[MAX_TBL_NUM] =
	{								  
	SCHED_PRIORITY_FUNC_TBL_BASE_ADDR,		//"P_FUNC"	// 0
	SCHED_SN_TBL_BASE_ADDR,					//"SN",		// 1
	SCHED_RAND_TBL_1_BASE_ADDR,				//"RND_1",	// 2
	SCHED_RAND_TBL_2_BASE_ADDR,				//"RND_2",	// 3
	SCHED_DCNT_TBL_1_BASE_ADDR,				//"DCNT_1",	// 4
	SCHED_DCNT_TBL_2_BASE_ADDR,				//"DCNT_2",	// 5
	SCHED_ST_TBL_BASE_ADDR,					//"S_TIM",	// 6
	SCHED_INTRVL_TBL_BASE_ADDR,				//"INTVL",	// 7
	SCHED_FLAG_TBL_BASE_ADDR,				//"FLGS",	// 8
	SCHED_STAT_TBL_1_BASE_ADDR,				//"STS_1",	// 9
	SCHED_STAT_TBL_2_BASE_ADDR,				//"STS_2",	//10
	SCHED_ACTN_TBL_BASE_ADDR,				//"ACTN",	//11
	SCHED_SENSE_ACT_TBL_BASE_ADDR,			//"SNSACT",	//12
	NST_1_TBL_BASE_ADDR,					//"NST_1",	//13
	NST_2_TBL_BASE_ADDR,					//"NST_2",	//14
	SSP_Y_TBL_BASE_ADDR,					//"SSP_Y",	//15
	SSP_T_TBL_BASE_ADDR,					//"SSP_T",	//16
	SSP_DELTA_Y_TRIG_TBL_BASE_ADDR,			//"Y_TRIG",	//17
	SSP_DELTA_T_TRIG_TBL_BASE_ADDR,			//"T_TRIG",	//18
	L2SRAM_BASE_ADDR						//"STOR_FWD"//19
	}; /* END: cpaSRAMtblNames[] */









/************************  DECLARATIONS  *************************************/

static void vL2SRAM_incQptr(
		uchar ucQ_ID		//Q indicator L2SRAM_Q_ON_ID or L2SRAM_Q_OFF_ID
		);


/************************  CODE  *********************************************/


/**********************  vL2SRAM_init() ***************************************
*
* Initialize the L2SRAM data structure in SRAM
*		
*
******************************************************************************/

void vL2SRAM_init(
		void
		)
	{
	#if 0
	vSERIAL_rom_sout("E:L2SRAM_init\r\n");
	#endif

	uslGLOB_sramQon_NFL = L2SRAM_MSG_Q_BEG_UL;
	uslGLOB_sramQoff = L2SRAM_MSG_Q_BEG_UL;
	uiGLOB_sramQcnt = 0;

	#if 0
	vSERIAL_rom_sout("L2SRM:Qon=");
	vSERIAL_HB24out(uslGLOB_sramQon_NFL);
	vSERIAL_rom_sout("  Qoff=");
	vSERIAL_HB24out(uslGLOB_sramQoff);
	vSERIAL_rom_sout(" Qcnt=");
	vSERIAL_UI16out(uiGLOB_sramQcnt);
	vSERIAL_crlf();
	#endif

	return;

	}/* END: vL2SRAM_init() */





/****************  ulL2SRAM_getGenericTblEntry()  ***************************
*
*
*
******************************************************************************/

ulong ulL2SRAM_getGenericTblEntry(
		usl uslTblBaseAddr,		//Tbl base addr
		usl uslTblBaseMax,		//Tbl base max (LUL)
		uchar ucTblIdx,			//Tbl index
		uchar ucTblIdxMax,		//Tbl index Max (LUL)
		uchar ucTblByteWidth	//Tbl byte width
		)
	{
	USL uslAbsTblAddr;
	USL uslTblIdx;
	USL uslTblByteWidth;
	ulong ulRetVal;

	ulRetVal = 0;


	/* REPORT OUT OF RANGE SCHED TBL BASE */
	if(uslTblBaseAddr > uslTblBaseMax)
		{
		#if 1
		vSERIAL_rom_sout("L2SRM:RdAbove ");
		vL2SRAM_showTblName(uslTblBaseMax);
		vSERIAL_rom_sout(" AddrMax:\r\n");

		vSERIAL_rom_sout("TblBase= ");
		vSERIAL_UIV24out(uslTblBaseAddr);
		vSERIAL_rom_sout("  MaxBase= ");
		vSERIAL_UIV24out(uslTblBaseMax);
		vSERIAL_rom_sout("  TblIdx = ");
		vSERIAL_UIV8out(ucTblIdx);
		vSERIAL_rom_sout("  MaxIdx= ");
		vSERIAL_UIV8out(ucTblIdxMax);
		vSERIAL_crlf();
		#endif
		goto getGenericTblEntry_exit;
		}


	/* REPORT OUT OF RANGE SCHED TBL IDX */
	if(ucTblIdx > ucTblIdxMax)
		{
		#if 1
		vSERIAL_rom_sout("L2SRM:RdAbove ");
		vL2SRAM_showTblName(uslTblBaseMax);
		vSERIAL_rom_sout(" IdxMax:\r\n");

		vSERIAL_rom_sout("TblBase= ");
		vSERIAL_UIV24out(uslTblBaseAddr);
		vSERIAL_rom_sout("  MaxBase= ");
		vSERIAL_UIV24out(uslTblBaseMax);
		vSERIAL_rom_sout("  TblIdx= ");
		vSERIAL_UIV8out(ucTblIdx);
		vSERIAL_rom_sout("  MaxIdx= ");
		vSERIAL_UIV8out(ucTblIdxMax);
		vSERIAL_crlf();
		#endif
		goto getGenericTblEntry_exit;
		}


	uslTblIdx = ucTblIdx;
	uslTblByteWidth = ucTblByteWidth;
	uslAbsTblAddr = (uslTblBaseAddr + (uslTblIdx * uslTblByteWidth));

	switch(ucTblByteWidth)
		{
		case 1:
			ulRetVal = (ulong) ucSRAM_read_B8(uslAbsTblAddr);
			break;
		case 2:
			ulRetVal = (ulong) uiSRAM_read_B16(uslAbsTblAddr);
			break;
//		case 3:
//			ulRetVal = (ulong) uslSRAM_read_B24(uslAbsTblAddr);
//			break;
		case 4:
			ulRetVal = ulSRAM_read_B32(uslAbsTblAddr);
			break;
		default:
			#if 1
			vSERIAL_rom_sout("\r\nL2SRAM:OutOfRangeRdByteWidth= ");
			vSERIAL_UIV8out(ucTblByteWidth);
			vSERIAL_crlf();
			#endif
			ulRetVal = 0L;
			break;			

		}/* END: switch() */


	#if 0
	vSERIAL_rom_sout("L2SRM:RD[");
	vSERIAL_HBV32out((ulong) uslAbsTblAddr);
	vSERIAL_rom_sout("]=");
	vSERIAL_HBV32out(ulRetVal);
	vSERIAL_rom_sout("  Base=");
	vSERIAL_HBV32out((ulong)uslTblBaseAddr);
	vSERIAL_rom_sout("  Idx=");
	vSERIAL_HBV32out((ulong)uslTblIdx);
	vSERIAL_rom_sout("  Width=");
	vSERIAL_HBV32out((ulong)uslTblByteWidth);
	vSERIAL_crlf();
	#endif

getGenericTblEntry_exit:

	return(ulRetVal);

	}/* END: ulL2SRAM_getGenericTblEntry() */







/***********************  vL2SRAM_putGenericTblEntry()  ************************
*
*
*
******************************************************************************/

void vL2SRAM_putGenericTblEntry(
		USL uslTblBaseAddr,		//Tbl base addr
		usl uslTblBaseMax,		//Tbl base max (LUL)
		uchar ucTblIdx,			//Tbl index
		uchar ucTblIdxMax,		//Tbl index Max (LUL)
		uchar ucTblByteWidth,	//Tbl byte width
		ulong ulEntryVal		//Tbl entry value
		)
	{
	USL uslAbsTblAddr;
	USL uslTblIdx;
	USL uslTblByteWidth;

	/* REPORT OUT OF RANGE SCHED TBL BASE */
	if(uslTblBaseAddr > uslTblBaseMax)
		{
		#if 1
		vSERIAL_rom_sout("L2SRM:WriteAbove ");
		vL2SRAM_showTblName(uslTblBaseMax);
		vSERIAL_rom_sout(" AddrMax:\r\n");

		vSERIAL_rom_sout("TblBase= ");
		vSERIAL_UIV24out(uslTblBaseAddr);
		vSERIAL_rom_sout("  MaxBase= ");
		vSERIAL_UIV24out(uslTblBaseMax);
		vSERIAL_rom_sout("  TblIdx = ");
		vSERIAL_UIV8out(ucTblIdx);
		vSERIAL_rom_sout("  MaxIdx= ");
		vSERIAL_UIV8out(ucTblIdxMax);
		vSERIAL_crlf();
		#endif
		goto putGenericTblEntry_exit;
		}


	/* REPORT OUT OF RANGE SCHED TBL IDX */
	if(ucTblIdx > ucTblIdxMax)
		{
		#if 1
		vSERIAL_rom_sout("L2SRM:WriteAbove ");
		vL2SRAM_showTblName(uslTblBaseMax);
		vSERIAL_rom_sout(" IdxMax:\r\n");

		vSERIAL_rom_sout("TblBase= ");
		vSERIAL_UIV24out(uslTblBaseAddr);
		vSERIAL_rom_sout("  MaxBase= ");
		vSERIAL_UIV24out(uslTblBaseMax);
		vSERIAL_rom_sout("  TblIdx= ");
		vSERIAL_UIV8out(ucTblIdx);
		vSERIAL_rom_sout("  MaxIdx= ");
		vSERIAL_UIV8out(ucTblIdxMax);
		vSERIAL_crlf();
		#endif
		goto putGenericTblEntry_exit;
		}

	uslTblIdx = ucTblIdx;
	uslTblByteWidth = ucTblByteWidth;
	uslAbsTblAddr = (uslTblBaseAddr + (uslTblIdx * uslTblByteWidth));

	switch(ucTblByteWidth)
		{
		case 1:
			vSRAM_write_B8(uslAbsTblAddr, (uchar)ulEntryVal);
			break;
		case 2:
			vSRAM_write_B16(uslAbsTblAddr, (uint)ulEntryVal);
			break;
//		case 3:
//			vSRAM_write_B24(uslAbsTblAddr, (USL)ulEntryVal);
//			break;
		case 4:
			vSRAM_write_B32(uslAbsTblAddr, ulEntryVal);
			break;
		default:
			#if 1
			vSERIAL_rom_sout("\r\nL2SRM:BdRdByteWdth= ");
			vSERIAL_UIV8out(ucTblByteWidth);
			vSERIAL_crlf();
			#endif
			break;			

		}/* END: switch() */

	#if 0
	vSERIAL_rom_sout("L2SRM:Write[");
	vSERIAL_HBV32out((ulong) uslAbsTblAddr);
	vSERIAL_rom_sout("] ");
	vL2SRAM_showTblName((ulong)uslAbsTblAddr);
	vSERIAL_rom_sout(" =");
	vSERIAL_HBV32out(ulEntryVal);
	vSERIAL_crlf();
	#endif

putGenericTblEntry_exit:

	return;

	}/* END: ulL2SRAM_putGenericTblEntry() */








/***********************  ulL2SRAM_getStblEntry()  ***************************
*
*
*
******************************************************************************/

ulong ulL2SRAM_getStblEntry(
		uchar ucTblNum,			//Tbl Number
		uchar ucTblIdx			//Tbl index
		)
	{
	USL uslTblBaseAddr;		//Tbl base addr
	uchar ucTblByteWidth;	//Tbl byte width
	ulong ulRetVal;


	ulRetVal = 0;

	/* CHECK THE TABLE NUMBER RANGE */
	if(ucTblNum > LAST_SCHED_TBL_NUM)
		{
		#if 1
		vSERIAL_rom_sout("L2SRM:BdTblnum\r\n");
		vSERIAL_rom_sout("TblNum = ");
		vSERIAL_UIV8out(ucTblNum);
		vSERIAL_rom_sout("  ReqTblIdx= ");
		vSERIAL_UIV8out(ucTblIdx);
		vSERIAL_crlf();
		#endif
		goto GetStblEntry_exit;
		}

	if(ucTblIdx >= GENERIC_SCHED_MAX_IDX)
		{
		vSERIAL_rom_sout("L2SRM:GSE:MaxIdxErr= ");
		vSERIAL_UIV8out(ucTblIdx);
		vSERIAL_rom_sout("  Max= ");
		vSERIAL_UIV8out(GENERIC_SCHED_MAX_IDX);
		vSERIAL_crlf();
		goto GetStblEntry_exit;
		}

	/* GET THE TABLE BASE AND WIDTH */
	uslTblBaseAddr = uslStblBaseAddr[ucTblNum];
	ucTblByteWidth = ucStblByteWidth[ucTblNum];

	/* CALL THE GENERIC TABLE ACCESS ROUTINE */
	ulRetVal = ulL2SRAM_getGenericTblEntry(
					uslTblBaseAddr,					//Tbl Base addr
					(usl)SCHED_LAST_TBL_BASE_ADDR,	//Tbl Base Max (LUL)
					ucTblIdx,						//Tbl Index
					GENERIC_SCHED_LAST_IDX_VAL,		//Tbl Index Max (LUL)
					ucTblByteWidth					//Tbl data width
					);

GetStblEntry_exit:

	return(ulRetVal);

	}/* END: ulL2SRAM_getStblEntry() */








/***********************  vL2SRAM_putStblEntry()  ************************
*
*
*
******************************************************************************/

void vL2SRAM_putStblEntry(
		uchar ucTblNum,			//Tbl Number
		uchar ucTblIdx,			//Tbl index
		ulong ulEntryVal		//Tbl entry value
		)
	{
	USL uslTblBaseAddr;			//Tbl base addr
	uchar ucTblByteWidth;		//Tbl byte width

	/* CHECK THE TABLE NUMBER RANGE */
	if(ucTblNum > LAST_SCHED_TBL_NUM)
		{
		#if 1
		vSERIAL_rom_sout("L2SRM:BadWriteTblnum\r\n");
		vSERIAL_rom_sout("TblNum= ");
		vSERIAL_UIV8out(ucTblNum);
		vSERIAL_rom_sout("  ReqTblIdx= ");
		vSERIAL_UIV8out(ucTblIdx);
		vSERIAL_crlf();
		#endif
		goto PutStblEntry_exit;
		}

	if(ucTblIdx >= GENERIC_SCHED_MAX_IDX)
		{
		vSERIAL_rom_sout("RTS:PSE:MaxIdxValErr\r\n");
		goto PutStblEntry_exit;
		}

	/* GET THE TABLE BASE AND WIDTH */
	uslTblBaseAddr = uslStblBaseAddr[ucTblNum];
	ucTblByteWidth = ucStblByteWidth[ucTblNum];

	/* CALL THE GENERIC TABLE ACCESS ROUTINE */
	vL2SRAM_putGenericTblEntry(
					uslTblBaseAddr,					//Tbl Base addr
					(usl)SCHED_LAST_TBL_BASE_ADDR,	//Tbl Base Max (LUL)
					ucTblIdx,						//Tbl Index
					GENERIC_SCHED_LAST_IDX_VAL,		//Tbl Index Max (LUL)
					ucTblByteWidth,					//Tbl data width
					ulEntryVal						//Tbl data
					);

PutStblEntry_exit:

	return;

	}/* END: vL2SRAM_putStblEntry() */








/**********************  uiL2SRAM_getMsgCount() *******************************
*
* RET:	number of messages on disk.
*
******************************************************************************/

uint uiL2SRAM_getMsgCount(
		void
		)
	{
	return(uiGLOB_sramQcnt);

	}/* END: uiL2SRAM_getMsgCount() */





/*****************  uiL2SRAM_getVacantMsgCount() *****************************
*
* RET:	Estimated number of vacant messages on disk.
*
******************************************************************************/

uint uiL2SRAM_getVacantMsgCount(
		void
		)
	{
	ulong ulMsgVacancy;

	ulMsgVacancy = L2SRAM_MSG_BUFF_COUNT_UL - uiGLOB_sramQcnt;

	return((uint)ulMsgVacancy);

	}/* END: uiL2SRAM_getVacantMsgCount() */





/**********************  vL2SRAM_incQptr()  ***********************************
*
* Inc the Q pointer indicated
*
******************************************************************************/

static void vL2SRAM_incQptr(
		uchar ucQ_ID		//Q indicator L2SRAM_Q_ON_ID or L2SRAM_Q_OFF_ID
		)
	{
	usl uslGenericQptr;
	
	/* ASSUME WHAT THE Q PTR VALUE IS */
	uslGenericQptr = uslGLOB_sramQon_NFL;
	/* CHANGE IT IF IT WAS A BAD ASSUMPTION */
	if(ucQ_ID == L2SRAM_Q_OFF_ID) uslGenericQptr = uslGLOB_sramQoff;

	/* INCREMENT THE Q POINTER */
	uslGenericQptr += MAX_MSG_SIZE;
	/* WRAP IF REQUIRED */
	if(uslGenericQptr >= L2SRAM_MSG_Q_END_UL) uslGenericQptr = L2SRAM_MSG_Q_BEG_UL;

	/* UPDATE THE VALUE OF THE POINTER */
	if(ucQ_ID == L2SRAM_Q_ON_ID)
		{
		uslGLOB_sramQon_NFL = uslGenericQptr;
		return;
		}

	uslGLOB_sramQoff = uslGenericQptr;
	return;

	}/* END: vL2SRAM_incQptr() */








/************************  vL2SRAM_storeMsgToSram() *****************************
*
* add a new message to the SRAM storage
*
* NOTE: This routine does the actual function of stuffing the msg buffer into
*		the SRAM, It does not check to see if its the right thing to do.
*		do not put guards in this code.
*
******************************************************************************/

void vL2SRAM_storeMsgToSram(
		void
		)
	{
	uchar ucii;

	/* COPY MSG TO SRAM */
	for(ucii=0;  ucii<MAX_MSG_SIZE;  ucii++)
		{
		vSRAM_write_B8(uslGLOB_sramQon_NFL+ucii, ucaMSG_BUFF[ucii]);
		}/* END: for(ui) */


	/* CHECK TO SEE IF WE ARE PASSING THE OFF Q PTR -- IF SO INC IT ALSO */
	if((uiGLOB_sramQcnt != 0) && (uslGLOB_sramQon_NFL == uslGLOB_sramQoff))
		{
		vL2SRAM_incQptr(L2SRAM_Q_OFF_ID);
		vL2SRAM_incQptr(L2SRAM_Q_ON_ID);
		/* DONT BUMP THE COUNT ITS A WASH */
		return;
		}

	/* OTHERWISE ADD A DATA ITEM TO THE COUNT */
	vL2SRAM_incQptr(L2SRAM_Q_ON_ID);
	uiGLOB_sramQcnt++;

	return;

	}/* END: vL2SRAM_storeMsgToSram() */






/*******************  vL2SRAM_storeMsgToSramIfAllowed() *************************
*
* store a msg to SRAM if we are storing to SRAM.
*
******************************************************************************/

void vL2SRAM_storeMsgToSramIfAllowed(
		void
		)
	{
	/* WE ARE NOT STORING TO SRAM IF WE ARE YES-SAMPLING AND NOT-SENDING */
	if((ucMODOPT_readSingleRamOptionBit(OPTPAIR_WIZ_SAMPLES)) &&
	   (!ucMODOPT_readSingleRamOptionBit(OPTPAIR_WIZ_SENDS))) return;

	/* OTHERWISE SEND TO SRAM */
	vL2SRAM_storeMsgToSram();

	return;

	}/* END: vL2SRAM_storeMsgToSramIfAllowed() */




/**********************  ucL2SRAM_getCopyOfCurMsg() ****************************
*
* copy the current message from SRAM.
*
*
* RET:	== 0  nothing to send (we are empty)
*		>  0  EOM_idx (end of msg idx) = SIZE-1 (high bits have been stripped)
*
******************************************************************************/

uchar ucL2SRAM_getCopyOfCurMsg(
		void
		)
	{
	uchar ucii;

	/* CHECK IF WE HAVE ANYTHING TO COPY */
	if(uiGLOB_sramQcnt == 0)
		{
		#if 0
		vSERIAL_rom_sout("NoMsgToCopy\r\n");
		#endif
		return(0);
		}

	/* COPY SRAM TO MSG BUFFER */
	for(ucii=0;  ucii<MAX_MSG_SIZE;  ucii++)
		{
		ucaMSG_BUFF[ucii] = ucSRAM_read_B8(uslGLOB_sramQoff+ucii);

		}/* END: for(ui) */
	
	return(ucaMSG_BUFF[OM2_IDX_EOM_IDX] & MAX_MSG_SIZE_MASK);

	}/* END: ucL2SRAM_getCopyOfCurMsg() */






/************************  vL2SRAM_delCurMsg() *******************************
*
* remove cur message from the SRAM storage
*
******************************************************************************/

void vL2SRAM_delCurMsg(
		void
		)

	{
	/* CHECK IF WE HAVE ANYTHING TO DELETE */
	if(uiGLOB_sramQcnt == 0)
		{
		#if 1
		vSERIAL_rom_sout("L2SRM:NoMsgToDel\r\n");
		#endif
		return;
		}

	vL2SRAM_incQptr(L2SRAM_Q_OFF_ID);

	uiGLOB_sramQcnt--;

	return;

	}/* END: vL2SRAM_delCurMsg() */





/**********************  vL2SRAM_showTblName() *******************************
*
* Tells what table an address is in
*
*****************************************************************************/
void vL2SRAM_showTblName( //locate an SRAM addr in an SRAM table & show it
		ulong ulAddr
		)
	{
	uchar ucii;
	ulong ulTblBase0;
	ulong ulTblBase1;

	/* SEARCH FOR THE ADDRESS HIT */
	for(ucii=0;  ucii<MAX_TBL_NUM;  ucii++)
		{
		ulTblBase0 = uiaSRAMtblBaseAddr[ucii];
		ulTblBase1 = MAX_SRAM_ADDR_UL;
		if(ucii != MAX_TBL_NUM-1)  ulTblBase1 = uiaSRAMtblBaseAddr[ucii+1];

		if(ulAddr > ulTblBase1) continue;
		if((ulAddr < ulTblBase1) && (ulAddr >= ulTblBase0)) break;

		}/* END: for(ucii) */
	if(ucii >= MAX_TBL_NUM) ucii = MAX_TBL_NUM-1;

	vSERIAL_rom_sout(cpaSRAMtblNames[ucii]);
	vSERIAL_rom_sout(" TBL");

	return;

	}/* END: vL2SRAM_showTblName() */





#if 0
/**********************  vL2SRAM_dumpTblBases() *******************************
*
* debug
*
*****************************************************************************/
void vL2SRAM_dumpTblBases(
		void
		)
	{
	uchar ucii;
	ulong ulTblBase0;

	/* SEARCH FOR THE ADDRESS HIT */
	for(ucii=0;  ucii<MAX_TBL_NUM;  ucii++)
		{
		ulTblBase0 = uiaSRAMtblBaseAddr[ucii];
		vSERIAL_rom_sout("SysTbl: ");
		vL2SRAM_showTblName(ulTblBase0);
		vSERIAL_rom_sout(" Base= ");
		vSERIAL_UI32out(ulTblBase0);
		vSERIAL_crlf();
		}/* END: for(ucii) */

	return;

	}/* END: vL2SRAM_dumpTblBases() */

#endif




/*-------------------------------  MODULE END  ------------------------------*/

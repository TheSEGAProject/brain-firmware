
/***************************  RTS.H  ****************************************
*
* Header for RTS (REAL TIME SCHEDULER) routine pkg
*
*
* V1.00 03/02/2005 wzr
*	started
*
******************************************************************************/

#ifndef RTS_H_INCLUDED
 #define RTS_H_INCLUDED

#define YES_SHOW_STBL	1
#define  NO_SHOW_STBL	0



/* SCHEDULER PRIORITYS, 7 = LOWEST, 0 = HIGHEST */
#define PRIORITY_0	0x00	//00000000
#define PRIORITY_1	0x20	//00100000
#define PRIORITY_2	0x40	//01000000
#define PRIORITY_3	0x60	//01100000
#define PRIORITY_4	0x80	//10000000
#define PRIORITY_5	0xA0	//10100000
#define PRIORITY_6	0xC0	//11000000
#define PRIORITY_7	0xE0	//11100000

#define PRIORITY_MAX_VAL	0xE0		//11100000
#define PRIORITY_MASK		0xE0		//11100000
#define PRIORITY_INC_VAL	0x20		//00100000


#define SCHED_FUNC_DORMANT_SLOT		0
#define SCHED_FUNC_SCHEDULER_SLOT	1
#define SCHED_FUNC_SDC4_SLOT		2
#define SCHED_FUNC_RDC4_SLOT		3
#define SCHED_FUNC_ALL_SLOT			4
#define SCHED_FUNC_OM2_SLOT			5
#define SCHED_FUNC_INTERVAL_SLOT	6
#define SCHED_FUNC_LOADBASED_SLOT	7


#define SCHED_FUNC_MAX_COUNT		8
#define SCHED_FUNC_MASK				0x1F		//00011111

/*************  NOTE: WHEN ADDING A NEW FUNCTION *****************************

1. Add a new define
1. Be sure to inc the max count
2. Write the new function
3. Add the declaration to the function declarations in RTS.c
4. Add a new entry in fpaSchedFunctArray[] in RTS.c

******************************************************************************/


/*--------------------------------------------------------------------------*/


 /* ROUTINE DEFINITIONS */
void vRTS_putNSTentry(
		uchar ucNST_tblNum,		//NST tbl (0 or 1)
		uchar ucNST_slot,		//NST slot number
		uchar ucNST_val			//NST value
		);

uchar ucRTS_getNSTentry(
		uchar ucNST_tblNum,		//NST tbl (0 or 1)
		uchar ucNST_slot		//NST slot number
		);

#if 0
void vRTS_showSingleNSTentry(
		uchar ucNST_tblNum,		//0 or 1
		uchar ucNST_slot,		//NST slot num
		uchar ucShowTitleFlag,	//YES_TITLE, NO_TITLE
		uchar ucShowHdrFlag,	//YES_HDR, NO_HDR
		uchar ucCRLF_termFlag	//YES_CRLF, NO_CRLF
		);
#endif

void vRTS_showAllNSTentrys(
		uchar ucNST_tblNum,		//NST tbl num (0 or 1)
		uchar ucShowStblFlag	//YES_SHOW_STBL, NO_SHOW_STBL
		);

void vRTS_convertAllRDC4slotsToSleep(
		void
		);

void vRTS_scheduleNSTtbl(
		long lNextFrameNumber	//Frame we are filling NST for
		);

uchar ucRTS_findNearestNSTslot(
		uchar ucNST_tblNum,		//NST table (0 or 1)
		uchar ucDesiredSlot		//NST slot num
		);

void vRTS_clrNSTtbl(
		uchar ucNST_tblNum		//NST table (0 or 1)
		);

void vRTS_runScheduler(
		void
		);

void vRTS_showActionHdrLine(
		uchar ucCRLF_termFlg	//YES_CRLF, NO_CRLF
		);

uchar ucRTS_thisNSTnum(
		void
		);

uchar ucRTS_nextNSTnum(
		void
		);

#if 0
uchar ucRTS_hasSchedRun(
		void
		);
#endif

uchar ucRTS_lastScheduledNSTnum(
		void
		);

void vRTS_showStatsOnLFactor(
		void
		);

#if 0
uchar ucRTS_findNSTslotNumOfSched(
		uchar ucNSTtblNum
		);
#endif


#endif /* RTS_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */


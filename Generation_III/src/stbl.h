
/***************************  STBL.H  ****************************************
*
* Header for STBL (REAL TIME SCHEDULER) routine pkg
*
*
* V1.00 03/02/2005 wzr
*	started
*
******************************************************************************/

#ifndef STBL_H_INCLUDED
 #define STBL_H_INCLUDED


#define SLEEP_STBL_IDX		0		//sleep Stbl idx is always 0



 /* ROUTINE DEFINITIONS */

/*--------------------------------*/

void vSTBL_clrSingleStblRow(
		uchar ucStblEntryNum		//table index
		);

void vSTBL_clrAllStblRows(
		void
		);

/*--------------------------------*/

void vSTBL_setupInitialStbls(
		void
		);

void vSTBL_stuffSingleStblEntry(
		uchar ucStblEntryNum,			//Tbl Index
		uchar ucUseVal,					//Tbl  0 USE entry
		uint uiSN,						//Tbl  1 SERIAL NUMBER entry
		USL uslRandNum0,				//Tbl  2 RANDOM NUMBER 0 entry
		USL uslRandNum1,				//Tbl  3 RANDOM NUMBER 1 entry
		uint uiLoadNum0,				//Tbl  4 DOWNCOUNT VAL 0 entry
		uint uiLoadNum1,				//Tbl  5 DOWNCOUNT VAL 1 entry
		uint uiEventStartTime,			//Tbl  6 EVENT START TIME entry
		uint uiEventIntervalTime,		//Tbl  7 EVENT INTERVAL TIME entry
		uchar ucFlagVal,				//Tbl  8 EVENT FLAG entry
		uint uiStatVal1, 				//Tbl  9 EVENT STAT 1 entry
		uint uiStatVal2, 				//Tbl  9 EVENT STAT 2 entry
		uchar ucActionNum,				//Tbl 10 ACTION NUM entry
		uchar ucLnkBlkIdx,				//Tbl 12 LNKBLK IDX entry
		ulong ulSenseActVal				//Tbl 13 SENSE ACT entry
		);

void vSTBL_deleteStblEntry(
		uchar ucStblEntryNum		//Sched Idx
		);

void vSTBL_addNewActionToStbl(	//Add a new action to the Sched Tbl
		uchar ucActionNum,	  	//Action number to enter
		uchar ucStBlkNum,	  	//StBlk num
		uint uiEventSN		  	//Serial Number to assign
		);

void vSTBL_showSingleStblEntry(
		uchar ucStblEntryNum,		//Sched Tbl Entry num
		uchar ucHDRflag,			//YES_HDR, NO_HDR
		uchar ucCRLF_termFlag		//YES_CRLF, NO_CRLF
		);

void vSTBL_showAllStblEntrys(
		void
		);

void vSTBL_showBriefStblEntrys(
		void
		);

/*--------------------------------*/

uchar ucSTBL_searchStblsForMatch( //Ret: Mch=StblIdx, NoMch=Out of range Sched Idx
		uint uiSearch_SN,				//serial number
		uchar ucSearch_actionNum		//action number
		);

void vSTBL_showTblBaseName(
		usl uslBaseAddr
		);

/*--------------------------------*/

void vSTBL_deleteAllROM2slots(
		void
		);

uint uiSTBL_countSOM2andROM2entrys(
		void
		);

void vSTBL_showAllROM2Entrys(
		void
		);

void vSTBL_showAllSOM2Entrys(
		void
		);

void vSTBL_showSOM2andROM2counts(
		uchar ucCRLF_termFlag	//YES_CRLF, NO_CRLF
		);

void vSTBL_showRDC4andSDC4counts(
		uchar ucCRLF_termFlag	//YES_CRLF, NO_CRLF
		);

/*--------------------------------*/

uchar ucSTBL_getNewStblIdx(
		void
		);

/*--------------------------------*/

void vSTBL_computeSysLFactor(
		void
		);

void vSTBL_showStatsOnLFactor(
		void
		);

void vSTBL_stuffSingleAction_LFactor(
		uchar ucNSTtblNum,			//NST tbl num (0 or 1)
		uchar ucStblIdx			//index of event being evaluated
		);

/*--------------------------------*/

void vSTBL_showLinkupSchedule(
		void
		);

uint uiSTBL_handleLoadDowncnt(
		uint uiOldLoadVal
		);

void vSTBL_showAllActionEntrys(
		uchar ucActionToShow
		);

void vSTBL_showLnkStats(
		void
		);

void vSTBL_showPFuncName(
		uchar ucPFuncVal	//Priority function number
		);

void vSTBL_showPFuncPriority(
		uchar ucPFuncVal	//Priority function number
		);



#endif /* STBL_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */


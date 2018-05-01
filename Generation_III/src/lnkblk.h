
/**************************  LNKBLK.H  ***************************************
*
* Header for LNKBLK radio linkup routine pkg
*
*
* V1.00 06/29/2008 wzr
*	started
*
******************************************************************************/

#ifndef LNKBLK_H_INCLUDED
 #define LNKBLK_H_INCLUDED



#define LNKREQ_1FRAME_1LNK  ((1<<3) | (1)) 	//1-Frame out, 1-Link
											//Used as LnkReq & LnkConfirm value
#define LNKREQ_2FRAME_1LNK  ((2<<3) | (1)) 	//2-Frame out, 1-Link
											//Used as Force value in sched



#define MAX_LNK_DIST_IN_FRAMES   7				//7-Frames out
#define MAX_LNK_DIST_IN_FRAMES_L ((long)MAX_LNK_DIST_IN_FRAMES)

#define MAX_LNK_DIST_IN_SEC_L ((long)MAX_LNK_DIST_IN_FRAMES_L * SECS_PER_FRAME_L)

#define MAX_LNKREQ  ((MAX_LNK_DIST_IN_FRAMES<<3) | (1))

#define MIN_LNK_DIST_IN_SEC_L ((long)SECS_PER_FRAME_L / ENTRYS_PER_LNKBLK_BLK_L)
#define MIN_LNKREQ  ((1<<3) | (ENTRYS_PER_LNKBLK_BLK -1))



#define LNK_MSG_TRANSFER_THRESHOLD    10		//number of msgs 
#define LNK_MSG_TRANSFER_THRESHOLD_L ((long)LNK_MSG_TRANSFER_THRESHOLD)

/*--------------------------------*/

long lLNKBLK_readSingleLnkBlkEntry(
		uchar ucStblIdx,			//Sys Tbl Idx
		uchar ucLnkBlkEntryNum		//entry number in the LnkBlk
		);

void vLNKBLK_writeSingleLnkBlkEntry(
		uchar ucStblIdx,			//Sys Tbl Idx
		uchar ucLnkBlkEntryNum,		//entry number in the LnkBlk
		long lLnkBlkVal 			//value to write into the LnkBlk
		);


/*--------------------------------*/

void vLNKBLK_clrSingleLnkBlk(
		uchar ucStblIdx			//Sys Tbl Idx
		);

void vLNKBLK_zeroEntireLnkBlkTbl(
		void
		);


/*--------------------------------*/

void vLNKBLK_showSingleLnkBlk(
		uchar ucStblIdx,			//Sys Tbl Idx
		uchar ucTimeFormFlag,		//FRAME_SLOT_TIME,HEX_SEC_TIME,INT_SEC_TIME
		uchar ucShowZroEntryFlag,	//YES_SHOW_ZRO_ENTRYS, NO_SHOW_ZRO_ENTRYS
		uchar ucCRLF_flag			//YES_CRLF, NO_CRLF
		);

void vLNKBLK_showAllLnkBlkTblEntrys(
		void
		);


/*--------------------------------*/


long lLNKBLK_computeTimeForSingleLnk(
		uchar ucStblIdx,		//Sys Tbl index of the lnk
		uchar ucSpecificLnkReq,	//Link Req = 5bits,,3bits => deltaFrame,,delta Link
		long lBaseTime			//Base time to compute the next lnk from
		);

void vLNKBLK_fillLnkBlkFromMultipleLnkReq(
		uchar ucStblIdx,	   	//Sys Tbl index
		uchar ucMultipleLnkReq,	//Lnk Req = 5bits,,3bits => deltaFrame,,delta Link
		long lBaseTime		    //Base Time to compute the lnks from
		);


/*--------------------------------*/

uchar ucLNKBLK_computeMultipleLnkReqFromSysLoad(
		int iSysLoad	//system load in msgs/hr
		);

void vLNKBLK_showLnkReq(
		uchar ucLnkReq
		);


/*--------------------------------*/


#endif /* LNKBLK_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */

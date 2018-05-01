
/***************************  CRC.H  ****************************************
*
* CRC calculator header file
*
* V1.00 10/07/2002 wzr
*	started
*
******************************************************************************/

#ifndef CRC_H_INCLUDED
  #define CRC_H_INCLUDED


#define CRC_FOR_MSG_TO_SEND 1
#define CRC_FOR_MSG_TO_REC  0



/* ROUTINE DEFINITIONS */

/***********************  ucCRC16_compute_msg_CRC()  ****************************
*
* This routine calculates the CRC for both the send msg and the receive msg.
*
* NOTE:	For a msg that you are sending you must leave an extra 2 bytes for
*		the CRC, and it MUST BE (THAT'S MUST BE) included in the size,
*		that is passed into this routine.
*
* NOTE: This set of CRC routines is hardwired (for speed) to only calculate
*		the CRC in the msg buffer.  We have not implemented a generalized
*		CRC routine here.
*
* RET:	1 = CRC is OK
*		0 = CRC mismatch
*
******************************************************************************/

unsigned char ucCRC16_compute_msg_CRC( /* RET: 1=CRC OK, 0=CRC mismatch */
		unsigned char ucMsgFlag				//send msg or receive msg flag
		);


#endif /* CRC_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */

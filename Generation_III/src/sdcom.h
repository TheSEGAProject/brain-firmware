
/***************************  SDCOM.H  ****************************************
*
* Header for SDCOM routine pkg
*
* Will likely need change! to make room for new mini SD's
*
* V1.00 10/27/2004 wzr
*	started
*
******************************************************************************/

#ifndef SDCOM_H_INCLUDED
	#define SDCOM_H_INCLUDED


	/* POWER UP/DOWN CONTROLS */
	#define YES_POWER_CONTROL   1
	#define  NO_POWER_CONTROL   0

	/* SD SLEEP CONTROLS */
	#define SD_UP_FOR_MESSAGING			1
	#define SD_UP_FOR_BRAIN_SPI_WORK	2

	#define MIN_USED_SPI_MSG_SIZE		5
	#define MAX_USED_SPI_MSG_SIZE		7



	/* ROUTINE DEFINITIONS */

	uchar ucSDCOM_awaken_SD_and_do_full_msg_transfer(
		unsigned char ucSensorNum
		);

	uchar ucSDCOM_send_a_burst_msg_to_SD(
		void
		);

	uchar ucSDCOM_receive_a_burst_from_SD(
		void
		);


#endif /* SDCOM_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */



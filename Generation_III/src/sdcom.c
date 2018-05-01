
/**************************  SDCOM.C  *******************************************
*
* Routines to handle SD board control.
* 
* needs change!
*
*
* V1.00 10/27/2004 wzr
*		Started
*
******************************************************************************/

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */



#include "DIAG.h"			//diagnostic defines
//#include <msp430x54x.h>		//processor reg description */
#include "STD.h"			//standard definitions
#include "CONFIG.h" 		//system configuration description file
//#include "MAIN.h"			//main definitions
//#include "SERIAL.h"		//serial IO port stuff
//#include "SDCOM.h"		//SD control routines
//#include "DELAY.h"  		//Wait routines
#include "SDSPI.h"			//SPI routines to handle SD communication
#include "SDCTL.h"			//SD board control routines
#include "SERIAL.h"			//serial port controller
#include "SDCOM.h"			//SD communication routines
#include "SENSOR.h"			//Sensor name module

extern volatile unsigned char ucaSDSPI_BUFF[ MAX_SDSPI_SIZE ];



/******************  ucSDCOM_send_a_burst_msg_to_SD()  *********************
*
* NOTE: This routine assumes that the SD board is powered up already.
* NOTE:	This routine does not power off the SD board
*
* This routine performs 3 parts:
*		1. Turn on SD SPI
*		2. send the message
*		3. Turn off SD SPI
*
*
* RET	1 = successful
*		0 = unsuccessful
*
******************************************************************************/

unsigned char ucSDCOM_send_a_burst_msg_to_SD(
		void
		)
	{
	uchar uc;

	/* WAIT FOR THE SD BOARD TO SAY ITS OK TO TURN ON SPI */
	if(!ucSDSPI_wait_for_slave_ready(500000L))
		{
		#if 0
		vSERIAL_rom_sout("SDCOM:ToutSlvRdy\r\n");
		#endif
		return(0);	//timed out
		}

	/* BRING UP THE SPI FOR THE SD BOARD */
	vSDSPI_init();							//make sure messaging is on

	/* SEND THE MESSAGE */
	for(uc=0;  uc<ucaSDSPI_BUFF[0];  uc++)
		{
		vSDSPI_bout(ucaSDSPI_BUFF[uc]);
		}/* END: for() */

	/* SHUT OFF THE SPI FOR THE SD BOARD */
	vSDSPI_quit();							//make sure messaging is off

	return(1);

	}/* END: ucSDCOM_send_a_burst_msg_to_SD() */







/******************  ucSDCOM_receive_a_burst_from_SD()  *********************
*
* NOTE: This routine assumes that the SD board is powered up already.
* NOTE:	This routine does not power off the SD board
*
* This routine performs 3 parts:
*		1. Turn on SD SPI
*		2. receive the message
*		3. Shut off SD SPI
*
*
* RET:	1 = successful
*		0 = error
*
******************************************************************************/

unsigned char ucSDCOM_receive_a_burst_from_SD(
		void
		)
	{
	uchar uc;
	uchar ucRetVal;

	ucRetVal = 0; 	//assume an error

	/* WAIT FOR THE SD BOARD TO SAY ITS OK TO TURN ON SPI */
	if(!ucSDSPI_wait_for_slave_ready(400000L))
		{
		#if 1
		vSERIAL_rom_sout("SDCOM:Rec:SlvNotRdy\r\n");
		#endif
		return(ucRetVal);
		}

	/* BRING UP THE SPI FOR THE SD BOARD */
	vSDSPI_init();							//make sure messaging is on

	/* GET THE MESSAGE */
	ucaSDSPI_BUFF[0] = ucSDSPI_bin();

	/* CHECK THE MSG SIZE */
	if(ucaSDSPI_BUFF[0] < MIN_USED_SPI_MSG_SIZE)
		{
		vSERIAL_rom_sout("SDCOM:MsgSizTooLO\r\n");
		goto SDCOM_rabsSD_exit;
		}
	if(ucaSDSPI_BUFF[0] > MAX_USED_SPI_MSG_SIZE)
		{
		vSERIAL_rom_sout("SDCOM:MsgSiz\r\n");
		goto SDCOM_rabsSD_exit;
		}

	if(ucaSDSPI_BUFF[0] == 6)
		{
		vSERIAL_rom_sout("SDCOM:MsgSiz=6\r\n");
		goto SDCOM_rabsSD_exit;
		}

	/* GET THE REST OF THE SPI MESSAGE */
	for(uc=1;  uc<ucaSDSPI_BUFF[0];  uc++)
		{
		ucaSDSPI_BUFF[uc] = ucSDSPI_bin();
		}/* END: for() */

	#if 1
	if(ucaSDSPI_BUFF[0] == 0)
		{
		vSERIAL_rom_sout("\r\nSDCOM:MsgSiz=0\r\n");
//		vSERIAL_bout('Z');			//short err version
		}
	#endif


	/* CHECK MESSAGE TYPE */
	if(ucaSDSPI_BUFF[1] >= 4)
		{
		vSERIAL_rom_sout("SDSPI:MsgTyp>=4\r\n");
		goto SDCOM_rabsSD_exit;
		}

	/* CHECK SENSOR NUMBER */
	if(ucaSDSPI_BUFF[2] == 0)
		{
		vSERIAL_rom_sout("SDSPI:MsgSnsr=0\r\n");
		goto SDCOM_rabsSD_exit;
		}
	if(ucaSDSPI_BUFF[2] >= SENSOR_MAX_VALUE)
		{
		vSERIAL_rom_sout("SDSPI:MsgSnsr>Mx\r\n");
		goto SDCOM_rabsSD_exit;
		}

	/* IF YOU GET HERE THE MESSAGE IS GOOD */
	ucRetVal = 1;		

SDCOM_rabsSD_exit:

	/* SHUT OFF THE SPI FOR THE SD BOARD */
	vSDSPI_quit();							//make sure messaging is off

	return(ucRetVal);

	}/* END: ucSDCOM_receive_a_burst_from_SD() */








/*****************  ucSDCOM_awaken_SD_and_do_full_msg_transfer()  **************
*
* This routine powers up the SD board, 
*	then messages to is to read a sensor,
*	then waits for the SD board to do the reading
*	then reads back the message from the SD board
*	then powers down the SD board.
*
* The returned message is in the ucaSDSPI_BUFF array.
*
*
* RET:	1 = successful
*		0 = failure
*
******************************************************************************/

uchar ucSDCOM_awaken_SD_and_do_full_msg_transfer(
		unsigned char ucSensorNum
		)
	{
//	#define TURN_ON_ALL_SDCOM_AWAKEN_REPORTING  1
	uchar uc;


	/* CLEAR THE SD MSG BUFFER */
	for(uc=0;  uc<MAX_SDSPI_SIZE;  uc++)
		{
		ucaSDSPI_BUFF[uc] = 0;
		}/* END: for(uc) */

	/* SETUP THE MSG REQUEST */
	ucaSDSPI_BUFF[CMD_MSG_SIZE_IDX] = 3;
	ucaSDSPI_BUFF[CMD_MSG_CMD_IDX]  = CMD_TYPE_READ_SENSOR;
	ucaSDSPI_BUFF[CMD_MSG_SENSOR_NUM_IDX]  = ucSensorNum;

	#ifdef TURN_ON_ALL_SDCOM_AWAKEN_REPORTING
	/* SHOW THE MESSAGE BEFORE WE SEND IT */
	vSERIAL_rom_sout("SntSD= ");
	for(uc=0;  uc<7;  uc++)
		{
		vSERIAL_HB8out(ucaSDSPI_BUFF[uc]);
		vSERIAL_bout(',');
		}/* END: for(uc) */
	vSERIAL_crlf();
	#endif

	/* LET THE SD START RUNNING */
	vSDCTL_break_SD_out_of_sleep();

	/* SEND THE MESSAGE TO THE SD BOARD */
	if(ucSDCOM_send_a_burst_msg_to_SD() == 0)
		{
		#if 0
		vSERIAL_rom_sout("SDCOM:SndBrstFail\r\n");
		#endif
		return(0);
		}

	/* CLEAR THE MESSAGE BUFFER */
	for(uc=0;  uc<MAX_SDSPI_SIZE;  uc++)
		{
		ucaSDSPI_BUFF[uc] = 0;
		}/* END: for(uc) */

	/* GET RET MSG FROM SD BOARD */
	if(ucSDCOM_receive_a_burst_from_SD() == 0)
		{
		#if 0
		vSERIAL_rom_sout("SDCOM:RecBrstFail\r\n");
		#endif
		return(0);
		}


	#ifdef TURN_ON_ALL_SDCOM_AWAKEN_REPORTING
	/* SHOW THE MESSAGE WE RECEIVED */
	vSERIAL_rom_sout("RecSD= ");
	for(uc=0;  uc<MAX_SDSPI_SIZE;  uc++)
		{
		vSERIAL_HB8out(ucaSDSPI_BUFF[uc]);
		vSERIAL_bout(',');
		}/* END: for(uc) */
	vSERIAL_crlf();
	vSERIAL_crlf();
	#endif

	return(1);

	}/* END: ucSDCOM_awaken_SD_and_do_full_msg_transfer() */







/*-------------------------------  MODULE END  ------------------------------*/

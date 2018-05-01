
/**************************  SDSPI.C  *****************************************
*
* Routines to handle SDSPI (MASTER COMMUNICATION OVER SPI).
*
* needs change!
*
* V1.00 6/14/2004 wzr
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


#include "STD.h"			//standard definitions
#include "CONFIG.h" 		//system configuration description file
#include <msp430x54x.h>		//processor reg description */
#include "SERIAL.h" 		//serial IO port stuff
//#include "delay.h"  		//delay timer routines
#include "SPI.h"			//spi port routines
#include "SDSPI.h"			//SD special SPI functions


//#define TIMEOUT_ON_SD_READY  40000		//bad
//#define TIMEOUT_ON_SD_READY 400000		//good
#define TIMEOUT_ON_SD_READY 200000		//good
//#define TIMEOUT_ON_SD_READY 100000		//good
//#define TIMEOUT_ON_SD_READY  50000		//bad




/******************  vSDSPI_init()  *******************************************
*
* Setup the MASTER COM LINK
*
******************************************************************************/

void vSDSPI_init(
		void
		)
	{
/*
	// SET TO INPUT THE SLAVE READY LINE //
	TRISBbits.SD_SPI_SLAVE_READY_BIT = 1; 	//INPUT = slave ready

	vSPI_master_init(SPI_MODE_0);
*/
	return;

	}/* END: vSDSPI_init() */







/******************  vSDSPI_quit()  *******************************************
*
* Quit the SPI port as a MASTER
*
******************************************************************************/

void vSDSPI_quit(
		void
		)
	{
//	vSPI_master_quit();
	return;

	}/* END: vSDSPI_quit() */




/******************  ucSDSPI_wait_for_slave_ready()  **************************
*
* Wait for the slave ready bit to come up (or timeout)
*
* RET:	1 = slave is ready
*		0 = slave not ready (timeout)
*
******************************************************************************/

uchar ucSDSPI_wait_for_slave_ready(
		ulong ulWaitTime
		)
	{
	ulong ull;
/*
	// WAIT FOR THE SD BOARD TO INDICATE ITS READY //
	for(ull=0;  ull<ulWaitTime; ull++)
		{
		// WAIT FOR THE SD BOARD TO INDICATE ITS READY //
		if(PORTBbits.SD_SPI_SLAVE_READY_BIT)
			{
			return(1);		//slave is ready -- return
			}// END: if() //

		}// END: for() //

	// FELL OUT OF LOOP -- TIMEOUT REPORT IT //
//	vSERIAL_rom_sout("SDSPI_0:SlvTout\r\n");

	return(0);
*/
return(1);

	}/* END: ucSDSPI_wait_for_slave_ready() */






/*******************  ucSDSPI_bin()  ******************************************
*
* Inputs a single byte from the SPI pprt using a dummy write.
*
******************************************************************************/

uchar ucSDSPI_bin(
		void
		)
	{
	uchar ucChar;
/*
	if(ucSDSPI_wait_for_slave_ready(TIMEOUT_ON_SD_READY))
		{
		ucChar = ucSPI_bin();
		}
	else
		{
		#if 1
		vSERIAL_rom_sout("SDSPI_1:SlvTout\r\n");
		#endif
		ucChar = 0xFE;
		}
*/
	return(ucChar);

	}/* END: ucSDSPI_bin() */







/*********************  vSDSPI_bout()  *****************************************
*
* Outputs a single byte to the SPI pprt and ignores the return input.
*
******************************************************************************/

void vSDSPI_bout(
		uchar ucOutByte
		)
	{
/*
	if(ucSDSPI_wait_for_slave_ready(TIMEOUT_ON_SD_READY))
		{
		vSPI_bout(ucOutByte);
		}
	else
		{
		#if 1
		vSERIAL_rom_sout("SDSPI_2:SlvTout\r\n");
		#endif
		}
*/
	return;

	}/* END: vSDSPI_bout() */









/************************  vSDSPI_rom_sout()  *******************************
*
*
******************************************************************************/

void vSDSPI_rom_sout(
		const char *cStrPtr		//lint !e125
		) 
	{	
/*										
   	while(*cStrPtr)
		{
		vSDSPI_bout((uchar)*cStrPtr++);
		}
*/
	return;

	}/* END: vSDSPI_rom_sout() */




/*-------------------------------  MODULE END  ------------------------------*/

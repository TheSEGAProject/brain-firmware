
/**************************  SPI.C  *****************************************
*
* Routines to handle low level SPI control.
*
*
* V1.00 10/14/2003 wzr
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



#include <msp430x54x.h>		//processor reg description */
#include "STD.h"			//standard definitions
#include "CONFIG.h"			//system configuration description file
#include "SERIAL.h"			//serial IO port stuff



/******************  vSPI_master_init()  **************************************
*
* Setup the SPI port as a master, 
*
*			1. CLK DATA ON RISING_EDGE
*			2. CLK REST STATE IS LO
*			3. Read data at middle of sample
*			4. CLK RATE = FOSC/4
*
*
******************************************************************************/

void vSPI_master_init(
		unsigned char ucMode
		)
	{

	/* SET THE SPI PIN DIRECTIONS */
	SPI_OUT_DIR_PORT |= SPI_OUT_BIT;
	SPI_INCLK_DIR_PORT |= SPI_CLK_BIT;
	SPI_INCLK_DIR_PORT &= ~SPI_IN_BIT;
	SPI_OUT_SEL_PORT |= SPI_OUT_BIT;
	SPI_INCLK_SEL_PORT |= (SPI_IN_BIT + SPI_CLK_BIT);
	
	SPI_CTL1 |= UCSWRST;			// **Put state machine in reset**
	
	switch(ucMode)
		{
		case 0: 
			SPI_CTL0 = 0xA9;			// MSB first, 8-bit, 3-pin SPI, Master, 
										// clk polarity low, synchronous, FALLINGEdge
			break;

		case 1:
			SPI_CTL0 = 0x29;			// MSB first, 8-bit, 3-pin SPI, Master, 
										// clk polarity low, synchronous, RISINGEdge
			break;

		case 2:
			SPI_CTL0 = 0x69;			// MSB first, 8-bit, 3-pin SPI, Master, 
										// clk polarity high, synchronous, FALLINGEdge
			break;

		case 3:
			SPI_CTL0 = 0xE9;			// MSB first, 8-bit, 3-pin SPI, Master, 
										// clk polarity high, synchronous, RISINGEdge
			break;

		default:

			vSERIAL_rom_sout("SPI_initCallBad\r\n");
			break;

		}/* END: switch() */
		SPI_CTL1 |= UCSSEL__SMCLK;	// SMCLK used for BRCLK
		SPI_CLK_DIVIDER = 0x0001;	// UCBR1 - 16-bit
									// SPI CLK runs at freq(BRCLK)/UCBR1
		SPI_CTL1 &= ~UCSWRST;		// **Initialize USCI state machine**
	return;

	}/* END: vSPI_master_init() */






/******************  vSPI_master_quit()  **************************************
*
* Quit the SPI port as a master
*
*
******************************************************************************/

void vSPI_master_quit(
		void
		)
	{
	SPI_CTL1 |= UCSWRST;	// **Put state machine in reset**
	
	SPI_OUT_DIR_PORT &= ~SPI_OUT_BIT;
	SPI_INCLK_DIR_PORT &= ~SPI_CLK_BIT;
	
	return;

	}/* END: vSPI_master_quit() */






/******************  ucSPI_IO_Transaction()  *********************************
*
* Outputs a single byte to the SPI pprt and returns an input byte
*
******************************************************************************/

unsigned char ucSPI_IO_Transaction(
		unsigned char ucOutByte
		)
	{
	#define WAITCOUNT_UL  500000UL
	unsigned long ulCounter;

	SPI_INTFLAG_REG = 0x00;						//clr the done flags

	SPI_TX_BUF = ucOutByte;						//stuff the output byte

	ulCounter = WAITCOUNT_UL;
	while(--ulCounter)
		{
		if(SPI_INTFLAG_REG & UCTXIFG) break;	//wait for cmd to complete
		}

	if(ulCounter == 0)
		{
		vSERIAL_rom_sout("SPItout\r\n");
		}

	return(SPI_RX_BUF);

	}/* END: ucSPI_IO_Transaction() */







/*******************  ucSPI_bin()  ************************************
*
* MASTER Inputs a single byte from the SPI pprt
* using a dummy write.
*
*
******************************************************************************/

unsigned char ucSPI_bin(
		void
		)
	{
	return(ucSPI_IO_Transaction(0));

	}/* END: ucSPI_bin() */







/*********************  vSPI_bout()  **********************************
*
* MASTER Outputs a single byte to the SPI pprt
* and ignores the return input.
*
******************************************************************************/

void vSPI_bout(
		unsigned char ucOutByte
		)
	{

	ucSPI_IO_Transaction(ucOutByte);

	return;

	}/* END: vSPI_bout() */




/*-------------------------------  MODULE END  ------------------------------*/

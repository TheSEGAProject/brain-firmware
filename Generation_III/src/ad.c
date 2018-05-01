
/*******************************  AD.C  **************************************
*
* This is the module to read the system A/D.
*
* V1.00 09/23/2002 wzr
*		Started.
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



#include <msp430x54x.h>		//processor reg definitions
//#include "std.h"
#include "config.h" 		//system configuration definitions
#include "DELAY.h"
//#include "MISC.H"
#include "AD.h"



/***************************  vAD_init()  *************************************
*
*
*
*
******************************************************************************/

void vAD_init(
		void
		)
	{
		
	// SET ALL ADC INPUT PINS TO PERIPHERAL FROM I/O
	P6SEL |= ADC_INPUTS_PORT6;
	P7SEL |= ADC_INPUTS_PORT7;

	// CONFIGURE THE ANALOG PINS AND VOLTAGE REF
	ADC12CTL0 = ADC12ON+ADC12SHT02+ADC12REFON+ADC12REF2_5V;
				// Turn on, sample/hold, 2.5 ref voltage on
	
	ADC12CTL1 = ADC12SHP;
				// Use sampling timer, ADC12OSC(5MHz) clk, not divided, single channel conversion
	
	ADC12CTL1 = ADC12TCOFF+ADC12RES_2+ADC12SR;
				// Not divided, temp sensor off, 12-bit, ref output off, reduced power/sampling speed
	

	// TURN ON THE AD 
	ADC12CTL0 |= ADC12ENC; 


	// CLEAR THE INTERRUPT FLAGS 
	ADC12IFG = 0;

	return;

	}/* END: vAD_init() */





/*********************  vAD_setup_analog_bits()  *****************************
*
*
*
*
******************************************************************************/

void vAD_setup_analog_bits(
		unsigned char ucChanNum
		)
	{
	ucChanNum &= 0x0F; 				// scrub off spurious bits 


	switch(ucChanNum)
		{
		case BATT_AD_CHAN:							// BATTERY
			BAT_VIN_PORT_DIR &= ~BAT_VIN_BIT;		// voltage pin input
			BAT_SEN_EN_PORT_DIR |= BAT_SEN_EN_BIT;	// batt sense enable pin output
			BAT_SEN_EN_PORT_OUT |= BAT_SEN_EN_BIT;	// enable batt sense
			break;

		default:
			break;

		}/* END: switch() */

	return;

	}/* END: vAD_setup_analog_bits() */






/********************  vAD_select_chan() *************************************
*
*
*
******************************************************************************/

void vAD_select_chan(
		unsigned char ucChanNum,		// chan num (0-15)
		unsigned char ucWaitType		// YES_WAIT, NO_WAIT
		)
	{
	/* SELECT THE CHANNEL */
	ADC12MCTL0 &= 0xF0;					// scrub off chan bits
	ucChanNum &= 0x0F;
	ADC12MCTL0 |= ucChanNum;			// set the channel

	if(ucWaitType) vDELAY_wait100usTic(1);

	return;								// leave

	}/* END: vAD_select_chan() */








/*********************  vAD_clearout_analog_bits()  **************************
*
*
*
*
******************************************************************************/

void vAD_clearout_analog_bits(
		unsigned char ucChanNum
		)
	{
	ucChanNum &= 0x0F; 									// scrub off spurious bits 

	switch(ucChanNum)
		{
		case BATT_AD_CHAN:								//BATTERY
			BAT_SEN_EN_PORT_OUT &= ~BAT_SEN_EN_BIT;		//disable
			break;

		default:
			break;

		}/* END: switch() */

	return;

	}/* END: vAD_clearout_analog_bits() */








/***************************  uiAD_read() ************************************
*
*
*
******************************************************************************/

unsigned int uiAD_read(
		unsigned char ucChanNum,		//(0 - 15)
		unsigned char ucChanSelectFlag	//(YES_SELECT_CHAN, NO_SELECT_CHAN)
		)
	{
	unsigned int uiAD_val;

	/* SELECT THE CHANNEL IF REQUESTED */
	if(ucChanSelectFlag) vAD_select_chan(ucChanNum, YES_WAIT);

	/* STARTUP A CONVERSION */
	ADC12CTL0 |= ADC12SC;

	/* WAIT FOR CONVERSION */
	while(!(ADC12IFG & BIT0));

	/* READ THE VALUE */
	uiAD_val = ADC12MEM0;

	return(uiAD_val);					//leave

	}/* END: uiAD_read() */




/******************  uiAD_full_init_setup_read_and_shutdown() ****************
*
* This is the lazy mans AD read (the way its supposed to be)
*
******************************************************************************/

unsigned int uiAD_full_init_setup_read_and_shutdown(
		unsigned char ucChanNum		//(0 - 15)
		)
	{
	unsigned int uiAD_val;

	/* INIT */
	vAD_init();

	/* SETUP THE ANALOG BITS */
	vAD_setup_analog_bits(ucChanNum);

	/* READ THE VALUE */
	uiAD_val = uiAD_read(ucChanNum, YES_SELECT_CHAN);

	/* DESELECT THE ANALOG BITS */
	vAD_clearout_analog_bits(ucChanNum);

	/* QUIT THE AD */
	vAD_quit();

	return(uiAD_val);					//leave

	}/* END: uiAD_full_init_setup_read_and_shutdown() */






/***************************  vAD_quit()  *************************************
*
*
*
*
******************************************************************************/

void vAD_quit(
		void
		)
	{

	/* TURN OFF A/D */
	ADC12CTL0 &= ~ADC12ON;				//shut off A/D
	P6SEL &= ~ADC_INPUTS_PORT6;			//turn port 6 back to digital
	P7SEL &= ~ADC_INPUTS_PORT7;			//turn port 7 back to digital

	}/* END: vAD_quit() */








/**************************  MODULE END  *************************************/

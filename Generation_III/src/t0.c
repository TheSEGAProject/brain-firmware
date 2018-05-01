
/**************************  T0.C  ******************************************
*
* T0 routines here, used for debugging - for now we are using the TB0. 
* This has not yet been fully tested and may conflict with the normal use of TB0.
* may need to change! the entire design of T0.
*
* V1.00 11/02/2005 wzr
*		Started
*
******************************************************************************/

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* function not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */
/*lint -e768 */		/* global struct member not referenced */



#include "DIAG.h"			//diagnostic defines
#include <msp430x54x.h>		//processor register description
#include "std.h"			//common definitions
#include "config.h" 		//system configuration definitions
//#include "main.h"			//mains defines
//#include "MISC.H"			//misc functions
//#include "DELAY.h" 		//delay routines
#include "SERIAL.H" 		//serial port IO pkg
#include "T0.h"				//timer routines
//#include "MISC.h"			//misc utility routines
//#include "BIGSUB.h"			//big subtract routines
//#include "L2FRAM.h"			//level 2 Ferro ram
//#include "OPMODE.h"			//opmode msg routines
//#include "ACTION.h"			//action routines
//#include "REPORT.h"			//msg reporting routines



/*****************************  CODE  ***************************************/



/**********************  vT0_init() ****************************************
*
* Init the T0 timer
*
*****************************************************************************/

void vT0_init(
		void
		)
	{

	/* SETUP TIMER 0 */
	T0_CTL = 0x0180;		// 16-bit, OFF, ACLK input, non-interrupt, clr int flag
	T0_COUNTER = 0;			// clear count

	return;

	}/* END: vT0_init() */





/**********************  vT0_quit() ****************************************
*
* Init the T0 timer
*
*****************************************************************************/

void vT0_quit(
		void
		)
	{

	/* CLR TIMER 0 */
	T0_CTL = 0x0180;		//shut it off
	T0_COUNTER = 0;			// clear count

	return;

	}/* END: vT0_quit() */




/**********************  vT0_start_T0_timer()  *******************************
*
* Clears the timer count and starts the timer
*
******************************************************************************/
void vT0_start_T0_timer(
		void
		)
	{
	T0_COUNTER = 0;				//clr count
	T0_CTL = 0x0190;			//turn it on up-mode and clear int flag

	return;

	}/* END: vT0_start_T0_timer() */





/**********************  vT0_continue_T0_timer()  ********************************
*
* Turns the timer on without clearing it
*
***************************************************************************/
void vT0_continue_T0_timer(
		void
		)
	{

	T0_CTL = 0x0190;			//turn it on up-mode and clear int flag

	return;

	}/* END: vT0_continue_T0_timer() */



		 
/**********************  vT0_stop_T0_timer()  ********************************
*
*
*
***************************************************************************/
void vT0_stop_T0_timer(
		void
		)
	{
	T0_CTL = 0x0180;			//turn it off and don't clear count

	return;

	}/* END: vT0_stop_T0_timer() */





/**********************  uiT0_get_val_in_tics() *****************************
*
* read the T0 timer 
*
*****************************************************************************/

uint uiT0_get_val_in_tics(
		void
		)
	{
	uint uiVal;

	uiVal = T0_COUNTER;

	return(uiVal);

	}/* END: uiT0_get_val_in_tics() */






/**********************  ulT0_get_val_in_Ltics() *****************************
*
* read the T0 timer
*
*****************************************************************************/

ulong ulT0_get_val_in_Ltics(
		void
		)
	{
	ulong ulRawRead;
	ulong ulTotal_Ltics;

 	ulRawRead = uiT0_get_val_in_tics();
	ulTotal_Ltics = (ulRawRead * RATIO_LTICS_TO_T0TICS);;

	return(ulTotal_Ltics);

	}/* END: ulT0_get_val_in_Ltics() */







/**********************  ulT0_get_val_in_uS() *****************************
*
* read the T0 timer
*
****************************************************************************/

ulong ulT0_get_val_in_uS(
		void
		)
	{
	ulong ulRawRead;
	ulong ulTotal_uS;

 	ulRawRead = uiT0_get_val_in_tics();
	ulTotal_uS = ulRawRead * T0_uS_PER_TIC_UL;

	return(ulTotal_uS);

	}/* END: uiT0_get_val_in_uS() */




/**********************  uiT0_set_val() ************************************
*
* set the T0 timer
*
*****************************************************************************/

void vT0_set_val(
		uint uiTicVal				//in Tics
		)
	{

	T0_COUNTER = uiTicVal;
	
	T0_CTL &= ~T0_INT_FLG_BIT;		//clr flag

	return;

	}/* END: vT0_set_val() */




/**********************  vT0_clr_val() ************************************
*
* clear the T0 timer
*
*****************************************************************************/

void vT0_clr_val(
		void
		)
	{

	T0_COUNTER = 0;

	T0_CTL &= ~T0_INT_FLG_BIT;		//clr flag

	return;

	}/* END: vT0_clr_val() */






/**********************  vT0_show_T0_in_uS() ************************************
*
* show the T0 timer in us
*
*****************************************************************************/

void vT0_show_T0_in_uS(
		void
		)
	{
	uint uiWholeSec;
	ulong ulSubSecTics;
	ulong ulSubSec_nS;
	ulong ulSubSec_uS;
	uint uiRawRead;

	uiRawRead = uiT0_get_val_in_tics();
	uiWholeSec = uiRawRead / T0_TICS_PER_SEC_UI;			//convert to full sec
	ulSubSecTics = (ulong)(uiRawRead % T0_TICS_PER_SEC_UI);	//convert to sub sec tics
	ulSubSec_nS = ulSubSecTics * T0_nS_PER_TIC_UL;			//convert to nS
	ulSubSec_uS = ulSubSec_nS / 1000UL;						//convert to uS

	vSERIAL_UIV16out(uiWholeSec);
	vSERIAL_bout('.');
	vSERIAL_UI32MicroDecOut((long)ulSubSec_uS); 			//in uS.

	return;

	}/* END: vT0_show_T0_in_uS() */




/* --------------------------  END of MODULE  ------------------------------- */

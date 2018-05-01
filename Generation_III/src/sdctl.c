
/**************************  SDCTL.C  *******************************************
*
* Routines to handle SD board control.
*
* needs change!
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



#include <msp430x54x.h>		//processor reg description */
#include "STD.h"			//standard definitions
#include "CONFIG.h" 		//system configuration description file
#include "SERIAL.h" 		//serial IO port stuff
//#include "SPI.h"			//SPI low level control routines
#include "SDCTL.h"			//SD control routines
#include "DELAY.h"			//Wait routines
//#include "MISC.h"			//misc routines
#include "BUTTON.h"			//button handler




/******************  vSDCTL_set_SD_power_bit()  ****************************************
*
* Routine to power ON/OFF the SD board
*
******************************************************************************/

void vSDCTL_set_SD_power_bit(
		unsigned ucPowerFlag	//SD_POWER_ON  or SD_POWER_OFF
		)
	{
/*
	// MAKE SURE POWER BIT IS AN OUTPUT //
	TRISBbits.SD_POWER_BIT = 0;		//SD power line = output

	// TURN SD POWER ON //
	if(ucPowerFlag)
		{
		// DRAIN THE SPI_SLAVE_READY BEFORE WE POWER ON //
		PORTBbits.SD_SPI_SLAVE_READY_BIT = 0;	//set to zro
		TRISBbits.SD_SPI_SLAVE_READY_BIT = 0;	//set to output (DO THE DRAIN)
		_asm NOP _endasm;						//delay
		TRISBbits.SD_SPI_SLAVE_READY_BIT = 1;	//set to input

		// TURN ON THE POWER //
		PORTBbits.SD_POWER_BIT = 0;		//SD = POWER ON
		}
	else
		{
		// TURN OFF THE POWER //
		PORTBbits.SD_POWER_BIT = 1;		//SD = POWER OFF

		// DRAIN THE SPI_SLAVE_READY //
		PORTBbits.SD_SPI_SLAVE_READY_BIT = 0;	//set to zro
		TRISBbits.SD_SPI_SLAVE_READY_BIT = 0;	//set to output (DO THE DRAIN)
		_asm NOP _endasm;
		TRISBbits.SD_SPI_SLAVE_READY_BIT = 1;	//set to input
		}

	vDELAY_wait4us();					//SD wait -- prevent losing bits
*/
	return;

	}// END: vSDCTL_set_SD_power_bit() //







/******************  ucSDCTL_test_SD_power_bit()  ************************************
*
* Routine indicate if SD board is powered up
*
* RET:	1 = SD power is on
*		0 = SD power is off
* 
* might need to change! for testing if the return value needs be zero in some places
*
******************************************************************************/

unsigned char ucSDCTL_test_SD_power_bit(
		void
		)
	{
/*
	TRISBbits.SD_POWER_BIT = 0;		//SD power line = output

	if(PORTBbits.SD_POWER_BIT) return(0);
*/
	return(1);

	}// END: ucSDCTL_test_SD_power_bit() //








/******************  vSDCTL_set_SD_reset_bit()  ***********************************
*
* Routine to hold the reset on for the SD board
*
*
******************************************************************************/

void vSDCTL_set_SD_reset_bit(
		unsigned char ucResetFlag	//SD_RESET_MODE or SD_RUN_MODE
		)
	{
/*
	// MAKE SURE RESET BIT IS AN OUTPUT //
	TRISFbits.SD_RESET_BIT = 0;		//SD reset line = output

	if(ucResetFlag)
		PORTFbits.SD_RESET_BIT = 0;		//SD = RESET
	else
		PORTFbits.SD_RESET_BIT = 1;		//SD = RUN (NO RESET)
*/
	return;

	}// END: vSDCTL_set_SD_reset_bit() //






/******************  ucSDCTL_test_SD_reset_bit()  ************************************
*
* Routine to detect if the SD board is reset
*
* RET:	1 if SD is reset
*		0 if SD if not reset
*
*  might need to change! for testing if the return value needs be zero in some places
*
******************************************************************************/

unsigned char ucSDCTL_test_SD_reset_bit(
		void
		)
	{
/*
	TRISFbits.SD_RESET_BIT = 0;		//SD reset line = output

	if(PORTFbits.SD_RESET_BIT) return(0);
*/
	return(1);

	}// END: ucSDCTL_test_SD_reset_bit() //






/******************  ucSDCTL_test_SD_running_bits()  ************************************
*
* Routine indicate if SD board is powered up and is not in reset
*
* RET:	1 if SD_POWER = YES & SD_RESET = NO
*		0 otherwise
*
*  might need to change! for testing if the return value needs be zero in some places
*
******************************************************************************/

unsigned char ucSDCTL_test_SD_running_bits(
		void
		)
	{
/*
	if(ucSDCTL_test_SD_power_bit() == 0) return(0);
	if(ucSDCTL_test_SD_reset_bit() == 1) return(0);
*/
	return(1);

	}// END: ucSDCTL_test_SD_running_bits() //






/*****************  ucSDCTL_detect_SD_power_up_pulse()  **************************
*
* This routine detects if the SD_pwr_up_flag has flopped no less than 5 times
* to indicate whether the SD is really up for business.
*
* Ret:	1 = yup SD is up and it flopped the line at least 5 times
*		0 = no SD is not up / or SD is not programmed to respond to brain msgs.
* 
******************************************************************************/

uchar ucSDCTL_detect_SD_power_up_pulse(
		void
		)
	{
/*
	uchar uci;
	uchar ucj;

	ulong uli;

	#define FIRST_HI_PULSE_WAIT_COUNT 200000UL
	#define FIRST_LO_PULSE_WAIT_COUNT 15000UL

	TRISFbits.SD_PWR_UP_FLAG_BIT = 1;	//input

	// WAIT FOR THE FIRST HI PULSE TO COME ALONG //
	for(uli=0; uli<FIRST_HI_PULSE_WAIT_COUNT;  uli++)
		{
		if(uli == FIRST_HI_PULSE_WAIT_COUNT-1)
			{
			vSERIAL_rom_sout("SDCTL:1stHiSDpulseTout\r\n");
			return(0);	//err exit
			}

		}// END: for() //


	// WAIT FOR THE FIRST LO PULSE TO COME ALONG //
	for(uli=0; uli<FIRST_LO_PULSE_WAIT_COUNT;  uli++)
		{
		if(uli == FIRST_LO_PULSE_WAIT_COUNT-1)
			{
			vSERIAL_rom_sout("SDCTL:1stLoSDpulseTout\r\n");
			return(0);	//err exit
			}

		}// END: for() //


	// GET THE REST OF THE PULSES //
	for(uci=0;  uci<4;  uci++)
		{

		// CHECK FOR HI PULSE //
		ucj=0;
		while(PORTFbits.SD_PWR_UP_FLAG_BIT)
			{
			ucj++;
			if(ucj > 250) return(0);	//err exit

			}// END: while() //


		// CHECK FOR LO PULSE //
		ucj=0;
		while(!PORTFbits.SD_PWR_UP_FLAG_BIT)
			{
			ucj++;
			if(ucj > 250) return(0);	//err exit
			}// END: while() //

		}// END: for(uci) //
*/
	return(1);							// yup -- SD is there //

	}// END: ucSDCTL_detect_SD_power_up_pulse() //






/******************  vSDCTL_set_SD_boot_load_bit()  ************************************
*
* Routine to BLOCK / UNBLOCK the boot loader bit on the SD board
*
*
******************************************************************************/

void vSDCTL_set_SD_boot_load_bit(
		unsigned ucBootLoaderFlag	//SD_BOOT_LOADER_ENABLED  or SD_BOOT_LOADER_DISABLED
		)
	{
/*
	// MAKE SURE BOOT LOADER BIT IS AN OUTPUT //
	TRISFbits.SD_BT_LDR_DISABLE_BIT = 0; //SD loader line = output

	// ENABLE/DISABLE SD BOOT LOADER //
	if(ucBootLoaderFlag)
		PORTFbits.SD_BT_LDR_DISABLE_BIT = 0; //SD BOOT LOADER = ENABLED
	else
		PORTFbits.SD_BT_LDR_DISABLE_BIT = 1; //SD BOOT LOADER = DISABLED
*/
	return;

	}/* END: vSDCTL_set_SD_boot_load_bit() */






/******************  ucSDCTL_wait_for_SD_boot_done_flag()  ****************************
*
* Routine to wait for the boot ldr to finish
* Timeout if not finished.
*
* RET:	1 = SD bootldr completed
*		0 = SD bootldr timed out
*
******************************************************************************/

unsigned char ucSDCTL_wait_for_SD_boot_done_flag(
		unsigned long ulWaitCount_ms 			//timeout limit in ms
		)
	{
/*
	unsigned long ul;

	for(ul=0;  ul<ulWaitCount_ms;  ul++)
		{
		// wait for done or button push override //
		if((PORTFbits.SD_BT_LDR_DONE_BIT) || (ucBUTTON_buttonHit()))
			return(1);

		vDELAY_wait100usTic(10);					//1ms

		}// END: for() //

	return(0);
*/
return(1);

	}/* END: ucSDCTL_wait_for_SD_boot_done_flag() */







/******************  ucSDCTL_start_SD_and_specify_boot()  **********************
*
* Routine to BLOCK / UNBLOCK the boot loader bit on the SD board
*
* RET:	1 = successful SD is up
*		0 = unsuccessful SD in not up
*
******************************************************************************/

uchar ucSDCTL_start_SD_and_specify_boot(//1=SD up, 0=SD not up
		uchar ucBootLoaderFlag	//SD_BOOT_LOADER_ENABLED  or SD_BOOT_LOADER_DISABLED
		)
	{
/*
	// MAKE SURE SD IS IN RESET //
	vSDCTL_set_SD_reset_bit(SD_RESET_MODE);
	
	// TURN ON POWER //
	vSDCTL_set_SD_power_bit(SD_POWER_ON);		//SD sleeps now

	// SETUP THE BOOT LOADER //
	if(ucBootLoaderFlag)
		vSDCTL_set_SD_boot_load_bit(SD_BOOT_LOADER_ENABLED);
	else
		vSDCTL_set_SD_boot_load_bit(SD_BOOT_LOADER_DISABLED);

	// LET THE SD START RUNNING //
	vSDCTL_set_SD_reset_bit(SD_RUN_MODE);


	// CHECK IF THE SD IS REALLY UP //
	#if 0
	if(!ucSDCTL_detect_SD_power_up_pulse())
		{
		vSERIAL_rom_sout("SDCTL:NoSDstartPulse\r\n");
		return(0);
		}
	#endif

	vDELAY_wait100usTic(300);	//wait
*/
	return(1);

	}/* END: ucSDCTL_start_SD_and_specify_boot() */








/******************  vSDCTL_halt_SD_and_power_down()  ************************
*
* Routine to turn off the SD board
*
*
******************************************************************************/

void vSDCTL_halt_SD_and_power_down(
		void
		)
	{

#if 0
	// PUT THE SD BOARD BACK INTO RESET //
	PORTFbits.SD_RESET_BIT = 0;					//reset

	// POWER OFF THE SD BOARD //
	PORTBbits.SD_POWER_BIT = 1;					//power off

	// SHUT OFF THE SD LOADER FLAG //
	PORTFbits.SD_BT_LDR_DISABLE_BIT = 0;		 //SD boot loader = disabled
#endif

	return;

	}// END: vSDCTL_halt_SD_and_power_down() //







/******************  vSDCTL_break_SD_out_of_sleep()  ***********************************
*
* Routine to handle the breakout_of_sleep control
*
******************************************************************************/

void vSDCTL_break_SD_out_of_sleep(
		void
		)
	{
/*
	// MAKE SURE RESET BIT IS AN OUTPUT //
	TRISBbits.SD_BREAKOUT_OF_SLEEP_BIT = 0;		//output

	// PULSE THE BREAKOUT FLAG TO RELEASE THE SD //
	PORTBbits.SD_BREAKOUT_OF_SLEEP_BIT = 1;	//SD = run
	_asm
		NOP		//lint !e40 !e522
		NOP		//lint !e40 !e522
	_endasm
	PORTBbits.SD_BREAKOUT_OF_SLEEP_BIT = 0;	//SD = back to sleep after msg
*/
	return;

	}/* END: vSDCTL_break_SD_out_of_sleep() */





/*-------------------------------  MODULE END  ------------------------------*/

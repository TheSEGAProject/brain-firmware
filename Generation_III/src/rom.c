
/**************************  ROM.C  ******************************************
*
* Rom Config Byte and Rom Serial Number retrievial.
*
* V1.00 12/11/2003 wzr
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


#include "STD.h"			//standard defines
//#include "CONFIG.h"		//configuration values
//#include "SERIAL.H"		//serial port handler
#include "ROM.h"				//serial number handler
//#include "MISC.h"			//misc utility routines




/*************************  SpecialDataSection  ******************************
*
* Special area set asside for configuration data
* This area is filled by the loader
*
*****************************************************************************/

 


static unsigned char ucSN_XI = 0x00;		//0x20 - these are old memory locations
static unsigned char ucSN_HI = 0x00;		//0x21
static unsigned char ucSN_MD = 0x06;		//0x22
static unsigned char ucSN_LO = 0x69;		//0x23

static unsigned char ucCONFIG_byte = 0x00; 	//0x24





/****************************  CODE  *****************************************/


#if 0 //currently not used

/************************  ucROM_getRomConfigByte()  *****************************
*
* Returns ROM config byte value
*
******************************************************************************/

unsigned char ucROM_getRomConfigByte(
		void
		)
	{

	return(ucCONFIG_byte);

	}/* END: ucROM_getRomConfigByte() */
#endif







/*******************  uiROM_getRomConfigSnumAsUint()  *************************
*
* Returns low 2 bytes of ROM serial number
*
******************************************************************************/

uint uiROM_getRomConfigSnumAsUint(
		void
		)
	{
	uint uiSN;

	uiSN = ucSN_MD;
	uiSN <<= 8;
	uiSN |= (uint) ucSN_LO; 

	return(uiSN);

	}/* END: uiROM_getRomConfigSnumAsUint() */




/* --------------------------  END of MODULE  ------------------------------- */

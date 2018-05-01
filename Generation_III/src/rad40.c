

/**************************  RAD40.C  ******************************************
*
* Radix 40 conversions and display routines
*
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
#include "CONFIG.h" 		//configuration values
#include "SERIAL.H" 		//serial port handler
//#include "RAD40.h"  		//radix 40 routines
//#include "MISC.h"			//misc utility routines



/****************************  rom tables  ***********************************/

const char *cpRad40Tbl = ".ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789- $";





/****************************  CODE  *****************************************/




/************************  ucRAD40_isRad40()  ********************************
*
* Ret  1 if the char is a rad40 char
*	   0 if the char is not a rad40 char
*
******************************************************************************/

uchar ucRAD40_isRad40(		//Tell if a char is a legal rad40 char
		uchar ucByte,		//char to check 
		uchar ucCaseFlag	//CASE_INSENSITIVE, or CASE_SENSITIVE
		)
	{
	uchar ucii;
	uchar ucRadByte;

	/* FORCE THE BYTE TO BE UPPERCASE */
	if(ucCaseFlag == CASE_INSENSITIVE)
		ucByte = ucSERIAL_toupper(ucByte);

	/* SEARCH FOR THE BYTE */
	for(ucii=0;  ucii<40;  ucii++)
		{
		ucRadByte = (uchar)*(cpRad40Tbl+ucii);

		if(ucByte == ucRadByte) return(1);
		}

	return(0);

	}/* END: ucRAD40_isRad40() */







/***********************  ucRAD40_convertByteToRad40()  **********************
*
* Ret Rad40 value of a text char
*
******************************************************************************/

uchar ucRAD40_convertByteToRad40(
		uchar ucByte
		)
	{
	uchar ucii;
	uchar ucRadByte;

	/* FORCE THE BYTE TO BE UPPERCASE */
	ucByte = ucSERIAL_toupper(ucByte);

	/* SEARCH FOR THE BYTE */
	for(ucii=0;  ucii<40;  ucii++)
		{
		ucRadByte = (uchar)*(cpRad40Tbl+ucii);

		if(ucByte == ucRadByte) return(ucii);
		}

	return(0);

	}/* END: ucRAD40_convertByteToRad40() */








/***********************  ucRAD40_convertRad40ToByte()  **********************
*
* Ret Rad40 value of a text char
*
******************************************************************************/

uchar ucRAD40_convertRad40ToByte(
		uchar ucRad40Val
		)
	{
	uchar ucByte;

	ucByte = (uchar)*(cpRad40Tbl+ucRad40Val);
	if(ucByte == 0) ucByte=' ';					//convert null to space

	return(ucByte);

	}/* END: ucRAD40_convertRad40ToByte() */






/************************  uiRAD40_ram_convertStrToRad40()  **********************
*
* Ret 16bit packed Rad40 value from a 3 byte string.
*
******************************************************************************/

uint uiRAD40_ram_convertStrToRad40(
		char *cStrPtr
		)
	{
	uchar ucii;
	uint uiVal;
	uchar ucRad40Byte;
	uchar ucZroFlg;

	/* CONVERT THE STRING TO RAD40 */
	uiVal = 0;
	ucZroFlg = FALSE;
	for(ucii=0;  ucii<3;  ucii++)
		{
		uiVal *= 40;
		ucRad40Byte = (uchar)*(cStrPtr+ucii);

		#if 0
		vSERIAL_bout('(');
		vSERIAL_UIV8out(ucRad40Byte);
		vSERIAL_rom_sout("),");
		#endif

		if(ucRad40Byte == 0) ucZroFlg = TRUE;
		if(ucZroFlg) continue;

		uiVal += ucRAD40_convertByteToRad40(ucRad40Byte);
		}

	return(uiVal);

	}/* END: uiRAD40_ram_convertStrToRad40() */







/**********************  uiRAD40_rom_convertStrToRad40()  ********************
*
* Ret 16bit packed Rad40 value from a 3 byte string.
*
******************************************************************************/

uint uiRAD40_rom_convertStrToRad40(
		const char *cStrPtr
		)
	{
	uchar ucii;
	uint uiVal;
	uchar ucRad40Byte;
	uchar ucZroFlg;

	/* CONVERT THE STRING TO RAD40 */
	uiVal = 0;
	ucZroFlg = FALSE;
	for(ucii=0;  ucii<3;  ucii++)
		{
		uiVal *= 40;
		ucRad40Byte = (uchar)*(cStrPtr+ucii);

		#if 0
		vSERIAL_bout('(');
		vSERIAL_UIV8out(ucRad40Byte);
		vSERIAL_rom_sout("),");
		#endif

		if(ucRad40Byte == 0) ucZroFlg = TRUE;
		if(ucZroFlg) continue;

		uiVal += ucRAD40_convertByteToRad40(ucRad40Byte);
		}

	return(uiVal);

	}/* END: uiRAD40_rom_convertStrToRad40() */






/************************  vRAD40_convertRad40ToStr()  ***********************
*
*
*
******************************************************************************/

void vRAD40_convertRad40ToStr(
		uint uiRad40Val,
		char cStrPtr[]			//str ptr must pt to str at least 4 chars
		)
	{
	uchar ucii;
	uchar ucRemainder;

	cStrPtr[3] = 0;
	for(ucii=0;  ucii<3;  ucii++)
		{
		ucRemainder =  (uchar)(uiRad40Val % 40);
		cStrPtr[2-ucii] = (char)ucRAD40_convertRad40ToByte(ucRemainder);

		uiRad40Val /= 40;

		}/* END: for(ucii) */

	return;

	}/* END: vRAD40_convertRad40ToStr() */









/************************  vRAD40_showRad40()  *******************************
*
*
*
******************************************************************************/

void vRAD40_showRad40(
		uint uiRad40Val
		)
	{
	uchar ucii;
	char caStrArray[4];

	vRAD40_convertRad40ToStr(uiRad40Val, &caStrArray[0]);

	for(ucii=0; ucii<3;  ucii++)
		{
		vSERIAL_bout((uchar)caStrArray[ucii]);
		}
			
	return;


	}/* END: vRAD40_showRad40str() */








/* --------------------------  END of MODULE  ------------------------------- */


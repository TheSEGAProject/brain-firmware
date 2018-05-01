
/***************************  RAD40.H  ****************************************
*
* Radix 40 conversions and display routines
*
*
* V1.00 12/11/2005 wzr
*	started
*
******************************************************************************/

#ifndef RAD40_H_INCLUDED
 #define RAD40_H_INCLUDED



 uchar ucRAD40_isRad40(		//Tell if a char is a legal rad40 char
		uchar ucByte,		//char to check 
		uchar ucCaseFlag	//CASE_INSENSITIVE, or CASE_SENSITIVE
		);

 uchar ucRAD40_convertByteToRad40(
		uchar ucByte
		);

 uchar ucRAD40_convertRad40ToByte(
		uchar ucRad40Val
		);

 uint uiRAD40_ram_convertStrToRad40(
		char *cStrPtr
		);

 uint uiRAD40_rom_convertStrToRad40(
		const char *cStrPtr
		);

 void vRAD40_convertRad40ToStr(
		uint uiRad40Val,
		char cStrPtr[]			//str ptr must pt to str at least 4 chars
		);

 void vRAD40_showRad40(
		uint uiRad40Val
		);

#endif /* RAD40_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */

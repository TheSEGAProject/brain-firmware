

/**************************  NUMCMD.C  ******************************************
*
* Numbered command matcher
*
* V1.00 12/11/2003 wzr
*		Started
*
******************************************************************************/



/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e716 */		/* while(1) */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


#include "std.h"		//standard defines
#include "config.h"		//configuration values
#include "serial.h"		//serial port handler
#include "NUMCMD.h"		//command matcher
//#include "misc.h"		//misc utility routines







/***********************  DECLARATIONS  **************************************/

void vNUMCMD_showCmdName(	//Show the command name
		char cCmdNum,					 //number of the cmd chosen
		const char *cpaCmdStrArray[], //ptr to cmd str array (11 char max)
		uchar ucArraySize				//size of the cmd str array
		);



/***********************  CODE STARTS HERE  **********************************/




/***************  vNUMCMD_doCmdInputAndDispatch()  ***************************************
*
*
******************************************************************************/

void vNUMCMD_doCmdInputAndDispatch( //RET: cmd num, (-1 if Escape)
		uchar ucPromptChar,				  //Prompt to display for user
		const char *cpaCmdStrArray[], //ptr to cmd str array (11 char max)
		const GENERIC_CMD_FUNC *vCmdFunction, //ptrs to cmd functions
		uchar ucArraySize,				  //size of the cmd str array
		uchar ucInitialPromptFlag		  //NO_SHOW_INITIAL_PROMPT, YES_SHOW_INITIAL_PROMPT
		)
	{
	char cNumRetVal;
	uchar ucii;

	for(ucii=0;  ;  ucii++)				//lint !e716 !e774
		{
		/* OUTPUT COMMAND PROMPT */
		if((ucii != 0) || (ucInitialPromptFlag == YES_SHOW_INITIAL_PROMPT))
			{
			vSERIAL_bout(ucPromptChar);
			}


		/* GET A COMMAND */
		cNumRetVal = cNUMCMD_getCmdNum(
						ucArraySize
						);

		if(cNumRetVal >= 0)
			{
			vNUMCMD_showCmdName(
						cNumRetVal,			//Cmd Num
						cpaCmdStrArray,		//Array of Cmd names
						ucArraySize			//Array Size
						);
			}

		/* CHECK FOR A QUIT */
		if(cNumRetVal == 0)	goto DCIAD_xit;

		/* IF NOT ESCAPE THE DO THE COMMAND */
		if(cNumRetVal >= 0)
			{
			/* DISPATCH TO THE COMMAND */
			vCmdFunction[(uchar)cNumRetVal]();
			}

		vSERIAL_crlf();

		}/* END: while(TRUE) */

DCIAD_xit:

	return;

	}/* END: vNUMCMD_doCmdInputAndDispatch() */








/***************  vNUMCMD_showCmdName()  ***************************************
*
* RET:	-1 = ESCAPE
*		 0 = EXIT
*		>0 = CMD
*
******************************************************************************/

void vNUMCMD_showCmdName(	//Show the command name
		char cCmdNum,					 //number of the cmd chosen
		const char *cpaCmdStrArray[], //ptr to cmd str array (11 char max)
		uchar ucArraySize				//size of the cmd str array
		)
	{
	uchar ucii;

	/* CHK FOR A BAD NUMBER */
	if(cCmdNum >= ucArraySize)
		{
		vSERIAL_rom_sout("???");
		return;
		}/* END: if() */


	/* ECHO THE COMMAND */
	for(ucii=0;  ;  ucii++)
		{
		if(cpaCmdStrArray[cCmdNum][ucii] == ' ') break;
		if(cpaCmdStrArray[cCmdNum][ucii] ==  0 ) break;
		vSERIAL_bout((uchar)cpaCmdStrArray[cCmdNum][ucii]);
		}
	vSERIAL_crlf();

	return;

	}/* END: vNUMCMD_showCmdName() */






/***************  cNUMCMD_getCmdNum()  ***************************************
*
* This routine returns a (char) command number from 0 to 127
* or else will return a negative value if escape is hit.
*
* RET:	-1 = ESCAPE
*		 0 = EXIT
*		>0 = CMD
*
******************************************************************************/

char cNUMCMD_getCmdNum(		//RET: cmd num, (-1 if Escape)
		uchar ucUpperRangeLimit	//Upper limit for range checking
		)
	{
	ulong ulVal;
	uchar ucStr[4];

	while(TRUE)										//lint !e774
		{
		/* CHECK FOR ESCAPE */
		if(ucSERIAL_getEditedInput(ucStr, 3))
			{
			vSERIAL_showXXXmsg();					//escape was hit
			return(-1);
			}

		/* CHECK FOR AT LEAST 1 NUMERIC */
		if((ucStr[0] < '0') || (ucStr[0] > '9'))
			{
			vSERIAL_rom_sout("MustBeNumeric,TryAgain..");
			continue;
			}

		/* CONVERT THE TEXT INTO A NUMBER */
		ulVal = (ulong)lSERIAL_AsciiToNum(ucStr, UNSIGNED, DECIMAL);

		/* CHK FOR A BAD NUMBER */
		if(ulVal >= ucUpperRangeLimit)
			{
			vSERIAL_rom_sout("OutOfRange,TryAgain..");
			continue;

			}/* END: if() */

		break;

		}/* END: while() */

	return((char)ulVal);

	}/* END: cNUMCMD_getCmdNum() */






/***********************  vNUMCMD_showCmdList()  *********************************
*
* This is a help message displayer
*
* It shows the	NUMBER,
*				COMMAND (first cmd must be Exit cmd)
*				EXPLANATION
* commands in columns.
*
* NOTE: In the wisard we have an artificial limit of 0 - 15 commands
*		with 0 = EXIT so that it will fit 2 commands nicely into a
*		byte for outbound messaging.
*
*******************************************************************************/

void vNUMCMD_showCmdList(	//show the command list to user
		const char *cpaTextStrArray[],	//ptr to the cmd text explanation
		uchar ucCmdArraySize				//size of cmd str array (above)
		)
	{

	uchar ucii;

	vSERIAL_rom_sout("NUM  CMD\r\n");

	for(ucii=0;  ucii<ucCmdArraySize;  ucii++)
		{
		vSERIAL_UI8_2char_out(ucii, ' ');
		vSERIAL_rom_sout(": ");
		vSERIAL_rom_sout(cpaTextStrArray[ucii]);

		vSERIAL_crlf();
		}

	return;

	}/* END: vNUMCMD_showCmdList() */




/* --------------------------  END of MODULE  ------------------------------- */


/*******************************  DELAY.C  ***********************************
*
* This is the module to perform delays  
* 
* will need to change! to actually reflect the amount of time expected
*
*
* V1.00 10/01/2002 wzr
*		Started.
*		
******************************************************************************/



/*lint -e714 */		/* symbol not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e768 */		/* global struct member not referenced */




//#include "std.h"			//standard include
//#include "config.h" 		//system configuration definitions
//#include "serial.h"
//#include "DELAY.h"


void vDELAY_wait4us(
		void
		)
	{

	__delay_cycles(56);

	return;

	}/* END: vDELAY_wait4us() */



void vDELAY_wait100us(
		void
		)
	{
	__delay_cycles(1600);

	return;

	}/* END: vDELAY_wait100us() */




void vDELAY_wait100usTic(
		unsigned int uiCount
		)
	{

	for( ;uiCount>0 ;uiCount--)
		{
		vDELAY_wait100us();
		}

	return;

	}/* END: vDELAY_wait100usTic() */


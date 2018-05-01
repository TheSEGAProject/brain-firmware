


/***************************  THERMO.H  ****************************************
*
* THERMO port header file
*
*
* V1.00 04/29/2002 wzr
*	started
*
******************************************************************************/

#ifndef THERMO_H_INCLUDED
 #define THERMO_H_INCLUDED


int iTHERMO_computeTfromTC( //RET: 16bit signed val 12bit whole,,4bit fraction
		int iVact,		//Vact = Vreading - Voffset (signed 16 bit number)
		int iTcold		//16bit signed val 12bit whole,,4bit fraction
		);

void vTHERMO_showOwireValue(
		int iTemp	//16bit signed val 12bit whole,,4bit fraction
		);

int iTHERMO_convertOwire_C_to_F(
		int iT_in_C		//16bit signed val 12bit whole,,4bit fraction
		);



#endif /* THERMO_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */

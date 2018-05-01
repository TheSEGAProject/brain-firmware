
/****************************  THERMO.C  *******************************************
*
*  Calculate the temperature in SD board
*
*
*
*  V1.11 01/30/2006 wzr
*		Added: vTHERMO_showOwireValue()
*		Added: iTHERMO_convertOwire_C_to_F()
*
*  V1.10 11/19/2004 kun xia
*
*  V1.00 10/27/2004 kun xia
*		Started
*
* 
*********************************************************************************************/


//#define DEBUG_PRINTOUT_ENABLED 1
#ifndef DEBUG_PRINTOUT_ENABLED
	#define DEBUG_PRINTOUT_ENABLED 0
#endif


#include "std.h"		//standard defines
#include "config.h"		//processor configuration
#include "serial.h"		//serial port control routines





/***************************  ROM LOOKUP TABLE  *******************************
*
* index = temperature
* table value = voltage in Volts
*
* Values taken from "THE TEMPERATURE HANDBOOK" published by OMEGA.
* Page Z-205 Revised Thermocouple Reference Tables. Type T Reference
* Tables N.I.S.T. Monograph 175 Revised to ITS-90.
* 
*
*******************************************************************************/

const int iaTtoV[16] = 
	{
	-1816,		//-50C = -58.0F	//Table = -1.819 mV
	-1545,		//-42C = -43.6F	//Table = -1.545 mV
	-1264,		//-34C = -29.2F	//Table = -1.264 mV
	-976,		//-26C = -14.8F	//Table = -0.976 mV
	-683,		//-18C = - 0.4F	//Table = -0.683 mV
	-382,		//-10C =  14.0F	//Table = -0.383 mV
	-77,		//- 2C =  28.4F	//Table = -0.077 mV
	234,		//  6C =  42.8F	//Table =  0.234 mV
	549,		// 14C =  57.2F	//Table =  0.549 mV
	870,		// 22C =  72.6F	//Table =  0.870 mV
	1196,		// 30C =  86.0F	//Table =  1.196 mV
	1528,		// 38C = 100.4F	//Table =  1.528 mV
	1865,		// 46C = 114.8F	//Table =  1.865 mV
	2207,		// 54C = 129.2F	//Table =  2.208 mV
	2556,		// 62C = 143.6F	//Table =  2.556 mV
	2908		// 70C = 158.0F	//Table =  2.909 mV
	};



/**************************  DEFINES  ****************************************/





/***************************  CODE  *****************************************/


/************  iTHERMO_computeTfromTC() ***************************
*
*  Calculate the Real Temperature of the SD board  
*
*  Values
*  
*  iVact  : the actual voltage (micro )
*  iTcold : coldjunction temperature
*  iVcj   : the voltage of coldjunction temperature map to
*  iVreal : Vact+Vcj which is the real voltage 
*  iTreal : the temperature of the real voltage map to
*
*  Algorithm
*  
*  There is a mapping from Temperature to Voltage, and Voltage to Temperature
*  there is f(temperature)=voltage and f(-1)(voltage)=temperature      
*  from -50C to 70C the graph of f(temperature)=voltage is nearly linear
*
*  In this algorithm there are 16 interpolations of the f(temperature)=voltage
*  from temperature -50,-42,-34....62 70
*
*  1 f(iTcold)--------->iVcj
*  2 iVcj+iVact-------->iVreal
*  3 f(1)(iVreal)------>iTreal
*
* NOTE:
* Readings are scaled by 256 to handle the fractional part as an integer value.
*
*
* RET:	16 bit signed value (12 bits whole,,4 bits fractional)
*
*************************************************************************************************************************/


int iTHERMO_computeTfromTC( //RET:	16bit signed val 12bit whole,,4bit fraction
		int iVact,		//Vact = Vreading - Voffset (signed 16 bit number)
		int iTcold		//16bit signed val 12bit whole,,4bit fraction
		)
	{
	int  iVcj;
	int  iVreal;
	int  iTreal;
	int  iVbase;
	int  iVnext;
	int  iIndex;
	int  iIndex_df;
	uint uiDistance;
	int  iI;

	#if (DEBUG_PRINTOUT_ENABLED == 1)
	vSERIAL_rom_sout("Vact= ");
	vSERIAL_IV16out(Vact);
	vSERIAL_crlf();
	#endif
	

	/* ASSUME TEMPERATURE BELOW END OF TABLE */
	iVcj=-1816+(iTcold+800)*2; //calculate Vcj if Temperature below -50C

	/* CHECK FOR TEMPERATURE ABOVE END OF TABLE */
	if(iTcold>=1120)
		{
		iVcj=2908+(iTcold-1120)*3;   //calculate Vcj if Temperature over 70c
		}
	else
		{
		/* USE THE TABLE TO CALCULATE THE COLD JCT VOLTAGE WHEN TEMPERATURE BETWEEN -50c AND 70c*/
		if((iTcold > -800))					       
			{
			/* CALCULATE THE VCJ USING LINEAR APPROXIMATION  */

			/* GET THE NEAREST SMALL POINT */
			iIndex = (iTcold+800)>>7;

			/* GET THE DISTANCE BETWEN THE NEAREST SMALL POINT AND THE TCOLD */
			iIndex_df = iTcold+800-(iIndex<<7);	 //per degree was scaled by mulitipy256

			/*LINEAR APPROXIMATION */
			iVcj=(((long)(iaTtoV[iIndex+1]-iaTtoV[iIndex])*(long)iIndex_df)>>7)+iaTtoV[iIndex];

			#if (DEBUG_PRINTOUT_ENABLED == 1)
			vSERIAL_rom_sout("Idx= ");
			vSERIAL_IV16out(index);
			vSERIAL_crlf();
			vSERIAL_rom_sout("Idx_df= ");
			vSERIAL_IV16out(index_df);
			vSERIAL_crlf();
			#endif			   	

		}/* END: if() */

	}/* END: if() */
 

	#if (DEBUG_PRINTOUT_ENABLED == 1)
	vSERIAL_rom_sout("Vcj= ");
	vSERIAL_IV16out(Vcj);
	vSERIAL_crlf();
	#endif

	/* CALCULATE the REAL TEMPERATURE */
	iVreal = iVact + iVcj;					      

	#if (DEBUG_PRINTOUT_ENABLED == 1)
	vSERIAL_rom_sout("Vreal= ");
	vSERIAL_IV16out(iVreal);
	vSERIAL_crlf();
	#endif
 
	/* CHECK FOR TEMPERATURE BELOW END OF TABLE */
	iTreal=(1816+iVreal)/2-800;  


	/* CHECK FOR TEMPERATURE ABOVE END OF TABLE */      
	if(iVreal >= 2908)  
		{
		iTreal=(iVreal-2908)/3+1118;
		}  	
	else
		{
		/* FROM THE TABLE TO GET TREAL BY REAL VOLTAGE, TEMPERATURE BETWEEN -50C AND 70C*/
		if(iVreal > -1816) 			    
 			{
			/* SEARCH VREAL IN TABLE */
			iI = 1;

			while(iaTtoV[iI] <= iVreal)	iI++;
			 
			iVbase = iaTtoV[iI-1];
			iVnext = iaTtoV[iI];
		  
			/* LINEAR APPROX TO CACULATE POSITION BETWEEN 2 PTS (MULTIPLY 160)*/
			uiDistance=(int)(((long)(iVreal-iVbase)<<11)/(iVnext-iVbase));     

			/* CALCULATE REAL TEMPERATURE */
			iTreal  = (int)((iI-1)<<7)+(uiDistance>>4)-800;				  
			if(iVreal < 0) iTreal++;
	 
			#if (DEBUG_PRINTOUT_ENABLED == 1) 
			vSERIAL_rom_sout("i= ");
		   	vSERIAL_IV16out(i);
	 		vSERIAL_crlf();
			vSERIAL_rom_sout("dist= ");
			vSERIAL_IV16out(distance);
			vSERIAL_crlf();
			#endif

			}
			
		}/*END if()   */ 

	return(iTreal);

	}/* END: iTHERMO_computeTfromTC() */






/**********************  vTHERMO_showOwireValue()  *********************************
*
*
*
*
******************************************************************************/

void vTHERMO_showOwireValue(
		int iTemp	//16bit signed val 12bit whole,,4bit fraction
		)
	{

	int iWhole;
	int iAbsTemp;
	uint uiScaledFraction;
	uint uiFraction;

	#if 0
	vSERIAL_I16out(iTemp);
	vSERIAL_rom_sout(" = ");
	vSERIAL_HB16out((uint)iTemp);
	vSERIAL_rom_sout(" = ");
	#endif

	iWhole = iTemp / 16;

	iAbsTemp = iTemp;
	if(iTemp < 0) iAbsTemp = -iAbsTemp;
	uiScaledFraction = (uint)((iAbsTemp & 0x000F) << 12);	//scale by 4096

	uiFraction = uiScaledFraction / 655;		//divide by (16 * 4096)/100
	if((uiScaledFraction % 655) > 327) uiFraction++;	//round
	
	/* NOTE: we know that uiFraction is a 2 digit number between 99 and 0 */


	vSERIAL_I16outFormat(iWhole, 3);
	vSERIAL_bout('.');
	vSERIAL_UI8_2char_out((uchar)uiFraction, '0');

	return;

	}/* END: vTHERMO_showOwireValue() */






/******************  iTHERMO_convertOwire_C_to_F()  ***************************
*
*
*
******************************************************************************/
int iTHERMO_convertOwire_C_to_F(
		int iT_in_C		//16bit signed val 12bit whole,,4bit fraction
		)
	{

	int iT_in_F;

	iT_in_F = ((9 * iT_in_C) / 5) + 512;

	return(iT_in_F);

	}/* END: iTHERMO_convertOwire_C_to_F() */




/* --------------------------  END of MODULE  ------------------------------- */

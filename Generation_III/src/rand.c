
/**************************  RAND.C  *****************************************
*
* Routines to read and write the static ram
*
*
*
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



#include "DIAG.h"			//diagnostic definitions
//#include <msp430x54x.h>		//processor reg description */
#include "STD.H"			//standard defines
//#include "CONFIG.H" 		//configuration defines
//#include "MAIN.H"			//
//#include "MISC.H"			//homeless functions
//#include "DISK0.H"
//#include "SERIAL.H"
#include "RAND.H"			//random number header
#include "L2FRAM.h"			//level 2 fram routines
#include "RADIO.h"			//radio handler
#include "MODOPT.h"			//Modify Options routines





// RAND_NUM_SIZE may need to change!  short longs are not used.
extern volatile unsigned char ucRAND_NUM[RAND_NUM_SIZE];




/***********************  uslRAND_getNewSeed()  ********************************
*
* This routine stuffs a new seed into the rand area
*
*****************************************************************************/

usl uslRAND_getNewSeed(	//stuff a new seed into rand array (and ret val)
		void
		)
	{
	uint uii;
	uint uiLoopMax;

	if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_USE_RDIO_FOR_RAND_SEED))
		{
		vRAND_stuffFullSysSeed(uslRADIO_getRandomNoise());
		vRADIO_quit();
		}
	else
		{
		vRAND_stuffFullSysSeed(0x8336F3);
		}


	/* ROLL THE RANDOM NUMBER THE SERIAL NUMBER OF TIMES */
	uiLoopMax = uiL2FRAM_getSnumLo16AsUint();
	for(uii=0;  uii<uiLoopMax;  uii++)
		{
		ucRAND_getRolledMidSysSeed();  //lint !e534
		}

	return(uslRAND_getFullSysSeed());

	}/* END: uslRAND_getNewSeed() */





/**********************  vRAND_stuffFullSysSeed() ***********************************
*
* stuff the random seed
*
******************************************************************************/

void vRAND_stuffFullSysSeed(	//Stuff a full seed value
		USL uslRandNum	
		)
	{
	ucRAND_NUM[RAND_HI] = (unsigned char)(uslRandNum>>16);
	ucRAND_NUM[RAND_MD] = (unsigned char)(uslRandNum>>8);
	ucRAND_NUM[RAND_LO] = (unsigned char)(uslRandNum);

	return;

	}/* END: vRAND_stuffFullSysSeed() */




/**********************  uslRAND_getFullSysSeed() ****************************
*
* read the random seed
*
******************************************************************************/

USL uslRAND_getFullSysSeed( //RET: unchanged seed from mem.
		void
		)
	{
	USL uslTmp;  

	uslTmp =  (USL)ucRAND_NUM[RAND_HI]; 
	uslTmp <<= 8;
	uslTmp |= ucRAND_NUM[RAND_MD];
	uslTmp <<= 8;
	uslTmp |= ucRAND_NUM[RAND_LO];

	return(uslTmp);

	}/* END: uslRAND_getFullSysSeed() */



/**********************  vRAND_getNextNum() *********************************
*
* 
*		
*
******************************************************************************/

void vRAND_getNextNum(
		void
		)
	{
	
	// needs change! when decided on how to handle this.
	
	return;

	}/* END: vRAND_getNextNum() */




/**********************  uslRAND_getRolledFullSysSeed() **********************
*
* 
*		
*
******************************************************************************/

USL uslRAND_getRolledFullSysSeed( //RET: rolls cur seed & rets new seed
		void
		)
	{

	vRAND_getNextNum();

	return(uslRAND_getFullSysSeed());

	}/* END: uslRAND_getRolledFullSysSeed() */






/**********************  ucRAND_getRolledMidSysSeed() *********************************
*
* YES ROLL, RET MID BYTE
*
******************************************************************************/

unsigned char ucRAND_getRolledMidSysSeed(
		//RET: rolls the current seed value and returns the middle byte
		void
		)
	{
	USL uslTmp; 

	uslTmp = uslRAND_getRolledFullSysSeed();

	return((unsigned char) (uslTmp >> 8));

	}/* END: ucRAND_getRolledMidSysSeed() */





/**********************  ucRAND_getMidSysSeed()  ********************************
*
* NO ROLL, RET MID BYTE
*
*****************************************************************************/

unsigned char ucRAND_getMidSysSeed( //RET: middle seed byte without rolling
		void
		)
	{

	return(ucRAND_NUM[RAND_MD]);

	}/* END: ucRAND_getMidSysSeed() */







/*******************  uslRAND_getRolledFullForeignSeed() **********************
*
* RET: NEST ROLL OF THE PASSED VALUE, and RESTORE THE SYSTEM SEED
*
******************************************************************************/

USL uslRAND_getRolledFullForeignSeed( //RET: ROLL THIS NUM, RET FULL VALUE
		usl uslForeignRandSeed
		)
	{
	usl uslRolledForeignSeed;  
	usl uslSavedSysSeed;

	/* SAVE THE SYSTEM SEED */
	uslSavedSysSeed = uslRAND_getFullSysSeed();

	/* STUFF THE NEW SEED */
	vRAND_stuffFullSysSeed(uslForeignRandSeed);

	/* ROLL THE NEW SEED AND GET THE RESULT */
	uslRolledForeignSeed = uslRAND_getRolledFullSysSeed();

	/* NOW RESTORE THE SYSTEM SEED */
	vRAND_stuffFullSysSeed(uslSavedSysSeed);

	return(uslRolledForeignSeed);

	}/* END: uslRAND_getRolledFullForeignSeed() */







/*******************  uslRAND_getMidForeignSeed() **********************
*
* RET: NEST ROLL OF THE PASSED VALUE, and RESTORE THE SYSTEM SEED
*
******************************************************************************/

uchar ucRAND_getMidForeignSeed( //RET: ROLL THIS NUM, RET FULL VALUE
		usl uslForeignRandSeed
		)
	{
	usl uslRolledForeignSeed;  

	uslRolledForeignSeed = uslRAND_getRolledFullForeignSeed(uslForeignRandSeed);

	return((uchar)(uslRolledForeignSeed >> 8));

	}/* END: uslRAND_getMidForeignSeed() */







/*-------------------------------  MODULE END  ------------------------------*/

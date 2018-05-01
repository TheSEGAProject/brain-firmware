
/***************************  SENSEACT.H  ****************************************
*
* Header for SENSEACT routine pkg
*
*
*
* V1.00 12/21/2006 wzr
*	started
*
******************************************************************************/

#ifndef SENSEACT_H_INCLUDED
	#define SENSEACT_H_INCLUDED


uchar ucSENSEACT_getRawSensorDataSizeTblEntry(
		uchar ucSensorNum
		);

uchar ucSENSEACT_getSensorUserFlag(
		uchar ucSensorNum
		);

uchar ucSENSEACT_getSensorSDfuncFlag(
		uchar ucSensorNum
		);

uchar ucSENSEACT_getSensorDataSize(
		uchar ucSensorNum
		);

void vSENSEACT_showSensorShortName(
		uchar ucSensorNum
		);

void vSENSEACT_showSenseActWordInText(
		ulong ulSenseActEntry
		);

ulong ulSENSEACT_correctSenseActWord(
		ulong ulSenseActWord
		);


#endif /* SENSEACT_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */


/***************************  REPORT.H  ****************************************
*
* Header for REPORT routine pkg
*
*
* V1.00 01/28/2005 wzr
*	started
*
******************************************************************************/

#ifndef REPORT_H_INCLUDED
 #define REPORT_H_INCLUDED

/* COPY TO SRAM FLAGS */
#define YES_COPY_TO_SRAM 1
#define  NO_COPY_TO_SRAM 0


/* ROUTINE DEFINITIONS */

void vREPORT_logReport( //Log the OM2 already built in the buffer 
		uchar ucReportRadioIdxPair,		//Radio OptionPairIdx
		uchar ucReportFlashIdxPair		//Flash OptionPairIdx
		);

void vREPORT_buildReportAndLogIt(
		uchar ucSensorNum0,				//Sensor num  0
		uint uiReportNum0,				//Sensor Data 0
		uchar ucSensorNum1,				//Sensor num  1
		uint uiReportNum1,				//Sensor Data 1
		uchar ucReportRadioIdxPair,		//Radio OptionPairIdx
		uchar ucReportFlashIdxPair		//Flash OptionPairIdx
		);

void vREPORT_buildEmptyReportHull(
		void
		);

uchar ucREPORT_addSensorDataToExistingReport(
		uchar ucSensor0,			//Sensor num  0
		uint uiData0,				//Sensor Data 0
		uchar ucSensor1,			//Sensor num  1 (0 if not to be added)
		uint uiData1				//Sensor Data 1
		);

void vREPORT_doRegularReport(
		void
		);


#endif /* REPORT_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */


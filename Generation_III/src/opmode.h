
/***************************  OPMODE.H  ****************************************
*
* Header for OPMODE routine pkg
*
*
* V1.00 01/28/2005 wzr
*	started
*
******************************************************************************/

#ifndef OPMODE_H_INCLUDED
	#define OPMODE_H_INCLUDED


//#define OM2_BROKEN_LINK_MAX_COUNT		1
#define OM2_BROKEN_LINK_MAX_COUNT		3

//#define OM2_BROKEN_LINK_LIMIT 		(OM2_BROKEN_LINK_MAX_COUNT << 8)



//#define STATUS_FLAG_OM3_MISSED		0xFF



/*---------------   ROUTINE DEFINITIONS  -----------------------------------*/

void vOPMODE_stuffDataPairToOM2(
		uchar ucSensorNum,		//sensor number
		uint uiSensorData,		//sensor Data
		uchar ucPairStIdx		//Starting Msg Idx for Data Pair
		);

void vOPMODE_buildMsg_OM2(
		uchar ucPktFlags,			//LAST_PKT_BIT & NO_DATA_PKT_BIT
		uint uiDestSN,				//Dest serial num
		uchar ucMsgSeqNum,			//Msg seq number
		uchar ucLoadFactor,			//Loading factor
		uint uiAgentSN,				//Data Sampler Agent Num
		long lCollectionTime,		//Data Sample time
		uchar ucDataCount,			//Cnt of valid sensor data entries (1 - 4)
		uchar ucSensorNum_0,		//Data Sensor 1 number
		uint uiSensorData_0,		//data from Sensor 1
		uchar ucSensorNum_1,		//Data Sensor 1 number
		uint uiSensorData_1,		//data from Sensor 1
		uchar ucSensorNum_2,		//Data Sensor 2 number
		uint uiSensorData_2,		//data from Sensor 2
		uchar ucSensorNum_3,		//Data Sensor 3 number
		uint uiSensorData_3			//data from Sensor 3
		);

void vOPMODE_SOM2(
		void
		);

void vOPMODE_ROM2(
		void
		);

void vOPMODE_evaluate_SOM2(
		void
		);

uchar ucOPMODE_computeLnkRetByte(
		uchar ucGenericOM2LinkByte
		);



#endif /* OPMODE_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */

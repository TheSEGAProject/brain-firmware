
/***************************  ACTION.H  ****************************************
*
* Header for ACTION routine pkg
*
* NOTE: See Main.h for the disk layout map. 
*
*
*
* V1.00 10/04/2003 wzr
*	started
*
******************************************************************************/

#ifndef ACTION_H_INCLUDED
	#define ACTION_H_INCLUDED

 #define BATT_CHK_REPORT_MODE 1	//do batt chk and report from routine
 #define BATT_CHK_SILENT_MODE 0	//do batt chk and say nothing

/* DEFINE THE WHO_CAN_RUN BITS HERE */

#define RBIT_OFF		0x01	//0000 0001		//0
#define RBIT_SENDONLY	0x02	//0000 0010		//1
#define RBIT_HUB		0x04	//0000 0100		//2
#define RBIT_RELAY		0x08	//0000 1000		//3
#define RBIT_STANDALONE 0x10	//0001 0000		//4
#define RBIT_TERMINUS	0x20	//0010 0000		//5
#define RBIT_SAMPLEHUB  0x40	//0100 0000		//6
#define RBIT_SPOKE		0x80	//1000 0000		//7

#define RBIT_ALL		0xFF	//1111 1111
#define RBIT_SAMPLERS	(RBIT_STANDALONE| RBIT_TERMINUS| RBIT_SAMPLEHUB| RBIT_SPOKE)
#define RBIT_SENDERS	(RBIT_SENDONLY| RBIT_RELAY| RBIT_TERMINUS| RBIT_SPOKE)
#define RBIT_RECEIVERS	(RBIT_HUB| RBIT_RELAY| RBIT_SAMPLEHUB| RBIT_SPOKE)

/*--------------------  EVENT ACTION LIST  ----------------------------------*/


/**************************  ACTION 0  ***************************************/
 #define E_ACTN_SLEEP				0
  #define E_ACTN_SLEEP_UL			((ulong) E_ACTN_SLEEP)

  #define STS_LFACT_SLEEP			0

  #define USE_FLAGS_SLEEP  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2)

  #define SENSE_ACT_SLEEP_VAL		0UL

  #define SLEEP_WHO_CAN_RUN			RBIT_ALL


/**************************  ACTION 1  ***************************************/
 #define E_ACTN_TEST_MSG			1

   /* DEFAULT SAMPLE INTERVAL */
   #ifndef SAMPLE_INTERVAL_TEST_MSG_I
//   #define SAMPLE_INTERVAL_TEST_MSG_I	 ((int)   30)	//30 Sec
//   #define SAMPLE_INTERVAL_TEST_MSG_I	 ((int)   60)	// 1 Min
//	 #define SAMPLE_INTERVAL_TEST_MSG_I	 ((int)  256)	// 1 Frame
//	 #define SAMPLE_INTERVAL_TEST_MSG_I	 ((int)  540)	// 9 Min
//	 #define SAMPLE_INTERVAL_TEST_MSG_I	 ((int)  900)	//15 Min
	 #define SAMPLE_INTERVAL_TEST_MSG_I	 ((int) 1800)	//30 Min
   #endif

   #define STS_LFACT_TEST_MSG (3600 / SAMPLE_INTERVAL_TEST_MSG_I)

   #define HR0_TO_SAMPLE0_TEST_MSG_IN_SEC ( 0 )

   #define USE_FLAGS_TEST_MSG  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | \
				 				F_USE_MAKE_OM2)

  #define SENSE_ACT_TEST_MSG_VAL ((ulong)(0))

  #define TEST_MSG_WHO_CAN_RUN		RBIT_ALL



/**************************  ACTION 2  ***************************************/
 #define E_ACTN_FAKE_SD_DATA	2

	/* DEFAULT SAMPLE INTERVAL */
  #ifndef SAMPLE_INTERVAL_FAKE_SD_DATA_I
//	    #define SAMPLE_INTERVAL_FAKE_SD_DATA_I	 ((int)   30)	//30 Sec
//		#define SAMPLE_INTERVAL_FAKE_SD_DATA_I	 ((int)   60)	// 1 Min
		#define SAMPLE_INTERVAL_FAKE_SD_DATA_I	 ((int)  256)	// 1 Frame
//		#define SAMPLE_INTERVAL_FAKE_SD_DATA_I	 ((int)  540)	// 9 Min
//		#define SAMPLE_INTERVAL_FAKE_SD_DATA_I	 ((int)  900)	//15 Min
//		#define SAMPLE_INTERVAL_FAKE_SD_DATA_I	 ((int) 1800)	//30 Min
  #endif

  #define STS_LFACT_FAKE_SD_DATA (3600 / SAMPLE_INTERVAL_FAKE_SD_DATA_I)

  #define HR0_TO_SAMPLE0_FAKE_SD_DATA_IN_SEC ( 0 )

  #define USE_FLAGS_FAKE_SD_DATA   (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | \
									F_USE_MAKE_OM2)

  #define SENSE_ACT_FAKE_SD_DATA_VAL ((ulong)(0))

  #define FAKE_SD_DATA_WHO_CAN_RUN	RBIT_SAMPLERS



/**************************  ACTION 3  ***************************************/
 #define E_ACTN_ROLL_YOUR_OWN			3	//ROLL YOUR OWN SAMPLING FUNCTION

	/* DEFAULT SAMPLE INTERVAL */
  #ifndef SAMPLE_INTERVAL_ROLL_YOUR_OWN_I
	    #define SAMPLE_INTERVAL_ROLL_YOUR_OWN_I	 ((int) 1800)	//30 Min
  #endif

  #define STS_LFACT_ROLL_YOUR_OWN (3600 / SAMPLE_INTERVAL_ROLL_YOUR_OWN_I)

  #define HR0_TO_SAMPLE0_ROLL_YOUR_OWN_IN_SEC ( 0 )

  #define USE_FLAGS_ROLL_YOUR_OWN  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | \
				 					F_USE_MAKE_OM2)

  #define SENSE_ACT_ROLL_YOUR_OWN ((ulong)(0))

  #define ROLL_YOUR_OWN_WHO_CAN_RUN		RBIT_SAMPLERS



/**************************  ACTION 4  ***************************************/
 #define E_ACTN_BATT_SENSE			4	//sample the battery

  #ifdef COMPILE_FOR_WIZ2_DUKETEST
		#define SAMPLE_INTERVAL_BATT_SENSE_I	 ((int)  256) // 1 frame
  #endif

	/* DEFAULT SAMPLE INTERVAL */
  #ifndef SAMPLE_INTERVAL_BATT_SENSE_I
//		#define SAMPLE_INTERVAL_BATT_SENSE_I	 ((int)   30)	//30 Sec
//		#define SAMPLE_INTERVAL_BATT_SENSE_I	 ((int)   60)	// 1 Min
//		#define SAMPLE_INTERVAL_BATT_SENSE_I	 ((int)  256)	// 1 Frame
//		#define SAMPLE_INTERVAL_BATT_SENSE_I	 ((int)  540)	// 9 Min
//		#define SAMPLE_INTERVAL_BATT_SENSE_I	 ((int)  900)	//15 Min
//		#define SAMPLE_INTERVAL_BATT_SENSE_I	 ((int) 1800)	//30 Min
		#define SAMPLE_INTERVAL_BATT_SENSE_I	 ((int) 5400)	//1.5 Hrs
  #endif

  #define STS_LFACT_BATT_SENSE (3600 / SAMPLE_INTERVAL_BATT_SENSE_I)

  #define HR0_TO_SAMPLE0_BATT_SENSE_IN_SEC ( 0 )
//#define HR0_TO_SAMPLE0_BATT_SENSE_IN_SEC (600)	//10 MIN

  #define USE_FLAGS_BATT_SENSE  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2)

  #define SA_BATT_1 ((ulong) SENSOR_BATT_VOLTAGE)

  #define SENSE_ACT_BATT_VAL (SA_BATT_1<<24)

  #define BATT_SENSE_WHO_CAN_RUN	RBIT_ALL



/**************************  ACTION 5  ***************************************/
 #define E_ACTN_TC_12_SAMPLE		5	//thermocouple 1 & 2

  #ifdef COMPILE_FOR_WIZ2_DUKETEST
		#define SAMPLE_INTERVAL_TC_12_I	 ((int)  256)	//1 frame
  #endif

  #ifdef COMPILE_FOR_MARK1234
//		#define SAMPLE_INTERVAL_TC_12_I	 ((int) 1800)	//30 Min
		#define SAMPLE_INTERVAL_TC_12_I	 ((int)   30)	//30 Sec
  #endif

	/* DEFAULT SAMPLE INTERVAL */
  #ifndef SAMPLE_INTERVAL_TC_12_I
//		#define SAMPLE_INTERVAL_TC_12_I	 ((int)   30)	//30 Sec
//		#define SAMPLE_INTERVAL_TC_12_I	 ((int)   60)	// 1 Min
//		#define SAMPLE_INTERVAL_TC_12_I	 ((int)  256)	// 1 Frame
//		#define SAMPLE_INTERVAL_TC_12_I	 ((int)  512)	// 2 Frames
//		#define SAMPLE_INTERVAL_TC_12_I	 ((int)  540)	// 9 Min
//		#define SAMPLE_INTERVAL_TC_12_I	 ((int)  900)	//15 Min
		#define SAMPLE_INTERVAL_TC_12_I	 ((int) 1800)	//30 Min
  #endif

  #define STS_LFACT_TC_12 (3600 / SAMPLE_INTERVAL_TC_12_I)

  #define HR0_TO_SAMPLE0_TC_12_IN_SEC ( 0 )
//#define HR0_TO_SAMPLE0_TC_12_IN_SEC (600)	//10 MIN

  #define USE_FLAGS_TC_12  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2)

  #define SA_TC12_1 ((ulong) SENSOR_TC_1)
  #define SA_TC12_2 ((ulong) SENSOR_ONEWIRE_0)
  #define SA_TC12_3 ((ulong) SENSOR_TC_2)
  #define SA_TC12_4 ((ulong) SENSOR_ONEWIRE_0)

  #define SENSE_ACT_TC_12_VAL ((SA_TC12_1<<24) | (SA_TC12_2<<16) | (SA_TC12_3<<8)  | (SA_TC12_4))

  #define TC12_WHO_CAN_RUN		RBIT_SAMPLERS




/**************************  ACTION 6  ***************************************/
 #define E_ACTN_TC_34_SAMPLE		6	//thermocouple 3 & 4

	#ifdef COMPILE_FOR_WIZ2_DUKETEST
		#define SAMPLE_INTERVAL_TC_34_I	 ((int)  256)	//1 frame
	#endif

	#ifdef COMPILE_FOR_MARK1234
		#define SAMPLE_INTERVAL_TC_34_I	 ((int)   30)	//30 Sec
	#endif

	/* DEFAULT SAMPLE INTERVAL */
	#ifndef SAMPLE_INTERVAL_TC_34_I
//		#define SAMPLE_INTERVAL_TC_34_I	 ((int)   30)	//30 Sec
//		#define SAMPLE_INTERVAL_TC_34_I	 ((int)   60)	// 1 Min
//		#define SAMPLE_INTERVAL_TC_34_I	 ((int)  256)	// 1 Frame
//		#define SAMPLE_INTERVAL_TC_34_I	 ((int)  512)	// 2 Frames
//		#define SAMPLE_INTERVAL_TC_34_I	 ((int)  540)	// 9 Min
//		#define SAMPLE_INTERVAL_TC_34_I	 ((int)  900)	//15 Min
		#define SAMPLE_INTERVAL_TC_34_I	 ((int) 1800)	//30 Min
	#endif

	#define STS_LFACT_TC_34 (3600 / SAMPLE_INTERVAL_TC_34_I)

	#define HR0_TO_SAMPLE0_TC_34_IN_SEC ( 0 )
//	#define HR0_TO_SAMPLE0_TC_34_IN_SEC (600)	//10 MIN



    #define USE_FLAGS_TC_34  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2)

	#define SA_TC34_1 ((ulong) SENSOR_TC_3)
	#define SA_TC34_2 ((ulong) SENSOR_ONEWIRE_0)
	#define SA_TC34_3 ((ulong) SENSOR_TC_4)
	#define SA_TC34_4 ((ulong) SENSOR_ONEWIRE_0)

	#define SENSE_ACT_TC_34_VAL ((SA_TC34_1<<24) | (SA_TC34_2<<16) | (SA_TC34_3<<8)  | (SA_TC34_4))

    #define TC34_WHO_CAN_RUN		RBIT_SAMPLERS



/**************************  ACTION 7  ***************************************/
  #define E_ACTN_LT_12_SAMPLE		7	//light sensors 1 & 2

  #ifdef COMPILE_FOR_WIZ2_DUKETEST
		#define SAMPLE_INTERVAL_LT_12_I	 ((int)  256)	//1 frame
  #endif

  #ifdef COMPILE_FOR_MARK1234
		#define SAMPLE_INTERVAL_LT_12_I	 ((int)   30)	// 30 Sec
  #endif

  /* DEFAULT SAMPLE INTERVAL */
  #ifndef SAMPLE_INTERVAL_LT_12_I
//		#define SAMPLE_INTERVAL_LT_12_I	 ((int)   30)	//30 Sec
//		#define SAMPLE_INTERVAL_LT_12_I	 ((int)   60)	// 1 Min
//		#define SAMPLE_INTERVAL_LT_12_I	 ((int)  256)	// 1 Frame
//		#define SAMPLE_INTERVAL_LT_12_I	 ((int)  512)	// 2 Frames
//		#define SAMPLE_INTERVAL_LT_12_I	 ((int)  540)	// 9 Min
		#define SAMPLE_INTERVAL_LT_12_I	 ((int)  900)	//15 Min
//		#define SAMPLE_INTERVAL_LT_12_I	 ((int) 1800)	//30 Min
  #endif

  #define STS_LFACT_LT_12 (3600 / SAMPLE_INTERVAL_LT_12_I)


  #define HR0_TO_SAMPLE0_LT_12_IN_SEC ( 0 )
//#define HR0_TO_SAMPLE0_LT_12_IN_SEC (600)	//10 MIN

  #define USE_FLAGS_LT_12  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2)

  #define SA_LT12_1 ((ulong) SENSOR_LIGHT_1)
  #define SA_LT12_2 ((ulong) SENSOR_LIGHT_2)

  #define SENSE_ACT_LT_12_VAL ((SA_LT12_1<<24) | (SA_LT12_2<<16))

  #define LT12_WHO_CAN_RUN	RBIT_SAMPLERS




/**************************  ACTION 8  ***************************************/
 #define E_ACTN_SL_12_SAMPLE		8	//soil moisture 1 & 2

	#ifdef COMPILE_FOR_WIZ2_DUKETEST
		#define SAMPLE_INTERVAL_SL_12_I	 ((int)  256)	// 1 frame
	#endif

	/* DEFAULT SAMPLE INTERVAL */
	#ifndef SAMPLE_INTERVAL_SL_12_I
//		#define SAMPLE_INTERVAL_SL_12_I	 ((int)   30)	//30 Sec
//		#define SAMPLE_INTERVAL_SL_12_I	 ((int)   60)	// 1 Min
//		#define SAMPLE_INTERVAL_SL_12_I	 ((int)  256)	// 1 Frame
//		#define SAMPLE_INTERVAL_SL_12_I	 ((int)  512)	// 2 Frames
//		#define SAMPLE_INTERVAL_SL_12_I	 ((int)  540)	// 9 Min
//		#define SAMPLE_INTERVAL_SL_12_I	 ((int)  900)	//15 Min
//		#define SAMPLE_INTERVAL_SL_12_I	 ((int) 1800)	//30 Min
		#define SAMPLE_INTERVAL_SL_12_I	 ((int) 7200)	// 2 Hours
	#endif

	#define STS_LFACT_SL_12 (3600 / SAMPLE_INTERVAL_SL_12_I)

	#define HR0_TO_SAMPLE0_SL_12_IN_SEC ( 0 )
//	#define HR0_TO_SAMPLE0_SL_12_IN_SEC (600)	//10 MIN



  #define USE_FLAGS_SL_12  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2)


  #define SA_SL12_1 ((ulong) SENSOR_SOIL_MOISTURE_1)
  #define SA_SL12_2 ((ulong) SENSOR_SOIL_MOISTURE_2)

  #define SENSE_ACT_SL_12_VAL ((SA_SL12_1<<24) | (SA_SL12_2<<16))

  #define SL12_WHO_CAN_RUN 	RBIT_SAMPLERS



/**************************  ACTION 9  ***************************************/
 #define E_ACTN_RUN_SCHEDULER		9	//Run the scheduler

  #define STS_LFACT_SCHEDULER		0

  #define USE_FLAGS_SCHEDULER  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2)

  #define SENSE_ACT_SCHEDULER_VAL ((ulong)(0))

  #define SCHEDULER_WHO_CAN_RUN	RBIT_ALL



/**************************  ACTION 10  ***************************************/
 #define E_ACTN_SOM2				10	//Send the OM2 packets

  #define STS_LFACT_SOM2			0

  #define USE_FLAGS_SOM2   (F_USE_EXACT_SLOT | F_USE_THIS_RAND | \
							F_USE_NO_MAKE_OM2)

  #define SENSE_ACT_SOM2_VAL ((ulong)(0))

  #define SOM2_WHO_CAN_RUN	RBIT_SENDERS




/**************************  ACTION 11  ***************************************/
 #define E_ACTN_ROM2				11	//Receive the OM2 packets

  #define STS_LFACT_ROM2			0

  #define USE_FLAGS_ROM2  (F_USE_EXACT_SLOT | F_USE_THIS_RAND | F_USE_NO_MAKE_OM2)

  #define SENSE_ACT_ROM2_VAL ((ulong)(0))

  #define ROM2_WHO_CAN_RUN	RBIT_RECEIVERS



/**************************  ACTION 12  *************************************/
 #define E_ACTN_DO_MOVE_SRAM_TO_FLASH		12	//Move_SRAM_to_FLASH
 												//(usually masters only)
 												//copies SRAM to FLASH & 
												//deletes SRAM

//	#define SAMPLE_INTERVAL_MOVE_SRAM_TO_FLASH_I	((int)128) // 2 times/frame 
	#define SAMPLE_INTERVAL_MOVE_SRAM_TO_FLASH_I	((int)256) // 1 time per frame 

	#define STS_LFACT_MOVE_SRAM_TO_FLASH (3600 / SAMPLE_INTERVAL_MOVE_SRAM_TO_FLASH_I)

	#define HR0_TO_SAMPLE0_MOVE_SRAM_TO_FLASH_IN_SEC	 0


    #define USE_FLAGS_MOVE_SRAM_TO_FLASH  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2)
    
	#define SENSE_ACT_MOVE_SRAM_TO_FLASH_VAL ((ulong)(0))

	#define MOVE_SRAM_TO_FLASH_WHO_CAN_RUN	(RBIT_HUB| RBIT_SAMPLEHUB)



/**************************  ACTION 13  *************************************/
 #define E_ACTN_SDC4				13	//send DC4 packets

  #define STS_LFACT_SDC4			0

  #define USE_FLAGS_SDC4 (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2)

  #define SENSE_ACT_SDC4_VAL ((ulong)(0))

  #define SDC4_WHO_CAN_RUN	RBIT_RECEIVERS



/**************************  ACTION 14  *************************************/
 #define E_ACTN_RDC4				14	//Receive DC4 packets

  #define STS_LFACT_RDC4			0


  #define USE_FLAGS_RDC4  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2)

  #define SENSE_ACT_RDC4_VAL ((ulong)(0))

  #define RDC4_WHO_CAN_RUN	RBIT_SENDERS



/**************************  ACTION 15  *************************************/
 #define E_ACTN_REPORT				15	//report

  #ifdef COMPILE_FOR_WIZ2_DUKETEST
		#define SAMPLE_INTERVAL_REPORT_I	 ((int)  256)	//1 frame
  #endif

  /* DEFAULT SAMPLE INTERVAL */
  #ifndef SAMPLE_INTERVAL_REPORT_I
//		#define SAMPLE_INTERVAL_REPORT_I	 ((int)   30)	//30 Sec
//		#define SAMPLE_INTERVAL_REPORT_I	 ((int)   60)	// 1 Min
//		#define SAMPLE_INTERVAL_REPORT_I	 ((int)  256)	// 1 Frame
//		#define SAMPLE_INTERVAL_REPORT_I	 ((int)  540)	// 9 Min
//		#define SAMPLE_INTERVAL_REPORT_I	 ((int)  900)	//15 Min
		#define SAMPLE_INTERVAL_REPORT_I	 ((int) 1800)	//30 Min
  #endif

  #define STS_LFACT_REPORT (3600 / SAMPLE_INTERVAL_REPORT_I)
	
  #define HR0_TO_SAMPLE0_REPORT_IN_SEC ( 0 )


  #define USE_FLAGS_REPORT  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2)

  #define SENSE_ACT_REPORT_VAL ((ulong)(0))

  #define REPORT_WHO_CAN_RUN	RBIT_ALL



/**************************  ACTION 16  ***************************************/
 #define E_ACTN_LT_34_SAMPLE	16	//light sensors 3 & 4

	#ifdef COMPILE_FOR_WIZ2_DUKETEST
		#define SAMPLE_INTERVAL_LT_34_I	 ((int)  256)	//1 frame
	#endif

	#ifdef COMPILE_FOR_MARK1234
		#define SAMPLE_INTERVAL_LT_34_I	 ((int)   30)	// 30 Sec
	#endif

	/* DEFAULT SAMPLE INTERVAL */
	#ifndef SAMPLE_INTERVAL_LT_34_I
//		#define SAMPLE_INTERVAL_LT_34_I	 ((int)   30)	//30 Sec
//		#define SAMPLE_INTERVAL_LT_34_I	 ((int)   60)	// 1 Min
//		#define SAMPLE_INTERVAL_LT_34_I	 ((int)  256)	// 1 Frame
//		#define SAMPLE_INTERVAL_LT_34_I	 ((int)  512)	// 2 Frames
//		#define SAMPLE_INTERVAL_LT_34_I	 ((int)  540)	// 9 Min
		#define SAMPLE_INTERVAL_LT_34_I	 ((int)  900)	//15 Min
//		#define SAMPLE_INTERVAL_LT_34_I	 ((int) 1800)	//30 Min
	#endif

	#define STS_LFACT_LT_34 (3600 / SAMPLE_INTERVAL_LT_34_I)

	#define HR0_TO_SAMPLE0_LT_34_IN_SEC ( 0 )
//	#define HR0_TO_SAMPLE0_LT_34_IN_SEC (600)	//10 MIN

  #define USE_FLAGS_LT_34  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2)

  #define SA_LT34_1 ((ulong) SENSOR_LIGHT_3)
  #define SA_LT34_2 ((ulong) SENSOR_LIGHT_4)

  #define SENSE_ACT_LT_34_VAL ((SA_LT34_1<<24) | (SA_LT34_2<<16))

  #define LT34_WHO_CAN_RUN	RBIT_SAMPLERS




/**************************  ACTION 17  ***************************************/
 #define E_ACTN_LT_1234_SAMPLE	17	//light sensors 1,2,3,4

  #ifdef COMPILE_FOR_WIZ2_DUKETEST
		#define SAMPLE_INTERVAL_LT_1234_I	 ((int)  256)	//1 frame
  #endif

  #ifdef COMPILE_FOR_MARK1234
		#define SAMPLE_INTERVAL_LT_1234_I	 ((int)   30)	// 30 Sec
  #endif

	/* DEFAULT SAMPLE INTERVAL */
  #ifndef SAMPLE_INTERVAL_LT_1234_I
//		#define SAMPLE_INTERVAL_LT_1234_I	 ((int)   30)	//30 Sec
//		#define SAMPLE_INTERVAL_LT_1234_I	 ((int)   60)	// 1 Min
//		#define SAMPLE_INTERVAL_LT_1234_I	 ((int)  256)	// 1 Frame
//		#define SAMPLE_INTERVAL_LT_1234_I	 ((int)  512)	// 2 Frames
//		#define SAMPLE_INTERVAL_LT_1234_I	 ((int)  540)	// 9 Min
		#define SAMPLE_INTERVAL_LT_1234_I	 ((int)  900)	//15 Min
//		#define SAMPLE_INTERVAL_LT_1234_I	 ((int) 1800)	//30 Min
  #endif

  #define STS_LFACT_LT_1234 (3600 / SAMPLE_INTERVAL_LT_1234_I)

  #define HR0_TO_SAMPLE0_LT_1234_IN_SEC ( 0 )
//#define HR0_TO_SAMPLE0_LT_1234_IN_SEC (600)	//10 MIN



  #define USE_FLAGS_LT_1234  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2)

  #define SA_LT1234_1 ((ulong) SENSOR_LIGHT_1)
  #define SA_LT1234_2 ((ulong) SENSOR_LIGHT_2)
  #define SA_LT1234_3 ((ulong) SENSOR_LIGHT_3)
  #define SA_LT1234_4 ((ulong) SENSOR_LIGHT_4)

  #define SENSE_ACT_LT_1234_VAL ((SA_LT1234_1<<24) | (SA_LT1234_2<<16) | (SA_LT1234_3<<8)  | (SA_LT1234_4))

  #define LT1234_WHO_CAN_RUN	RBIT_SAMPLERS



/**************************  ACTION 18  ***************************************/
 #define E_ACTN_LT_1111_SAMPLE	18	//light sensors 1,1,1,1

  #ifdef COMPILE_FOR_WIZ2_DUKETEST
		#define SAMPLE_INTERVAL_LT_1111_I	 ((int)  256)	//1 frame
  #endif

	/* DEFAULT SAMPLE INTERVAL */
  #ifndef SAMPLE_INTERVAL_LT_1111_I
//		#define SAMPLE_INTERVAL_LT_1111_I	 ((int)   30)	//30 Sec
//		#define SAMPLE_INTERVAL_LT_1111_I	 ((int)   60)	// 1 Min
//		#define SAMPLE_INTERVAL_LT_1111_I	 ((int)  256)	// 1 Frame
//		#define SAMPLE_INTERVAL_LT_1111_I	 ((int)  512)	// 2 Frames
//		#define SAMPLE_INTERVAL_LT_1111_I	 ((int)  540)	// 9 Min
//		#define SAMPLE_INTERVAL_LT_1111_I	 ((int)  900)	//15 Min
		#define SAMPLE_INTERVAL_LT_1111_I	 ((int) 1800)	//30 Min
  #endif

  #define STS_LFACT_LT_1111 (3600 / SAMPLE_INTERVAL_LT_1111_I)

  #define HR0_TO_SAMPLE0_LT_1111_IN_SEC ( 0 )
//#define HR0_TO_SAMPLE0_LT_1111_IN_SEC (600)	//10 MIN


  #define USE_FLAGS_LT_1111  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2)

  #define SA_LT1111_1 ((ulong) SENSOR_LIGHT_1)
  #define SA_LT1111_2 ((ulong) SENSOR_LIGHT_1)
  #define SA_LT1111_3 ((ulong) SENSOR_LIGHT_1)
  #define SA_LT1111_4 ((ulong) SENSOR_LIGHT_1)

  #define SENSE_ACT_LT_1111_VAL ((SA_LT1111_1<<24) | (SA_LT1111_2<<16) | (SA_LT1111_3<<8)  | (SA_LT1111_4))

  #define LT1111_WHO_CAN_RUN	RBIT_SAMPLERS



/**************************  ACTION 19  ***************************************/
 #define E_ACTN_VS_AVE_WSPEED_DIR_SAMPLE 19	//Vaisala Ave Wind Speed & Dir

  #ifdef COMPILE_TEST_YES_V_TREE_HUMBOLDT
		#define SAMPLE_INTERVAL_VS_AVE_WSPEED_DIR_I	 ((int) 256)	// 1 Frame
  #endif

	/* DEFAULT SAMPLE INTERVAL */
  #ifndef SAMPLE_INTERVAL_VS_AVE_WSPEED_DIR_I
//		#define SAMPLE_INTERVAL_VS_AVE_WSPEED_DIR_I	 ((int)   30)	//30 Sec
//		#define SAMPLE_INTERVAL_VS_AVE_WSPEED_DIR_I	 ((int)   60)	// 1 Min
//		#define SAMPLE_INTERVAL_VS_AVE_WSPEED_DIR_I	 ((int)  256)	// 1 Frame
//		#define SAMPLE_INTERVAL_VS_AVE_WSPEED_DIR_I	 ((int)  512)	// 2 Frames
//		#define SAMPLE_INTERVAL_VS_AVE_WSPEED_DIR_I	 ((int)  540)	// 9 Min
//		#define SAMPLE_INTERVAL_VS_AVE_WSPEED_DIR_I	 ((int)  900)	//15 Min
		#define SAMPLE_INTERVAL_VS_AVE_WSPEED_DIR_I	 ((int) 1800)	//30 Min
  #endif

  #define STS_LFACT_VS_AVE_WSPEED_DIR (3600 / SAMPLE_INTERVAL_VS_AVE_WSPEED_DIR_I)

  #define HR0_TO_SAMPLE0_VS_AVE_WSPEED_DIR_IN_SEC ( 0 )
//#define HR0_TO_SAMPLE0_VS_AVE_WSPEED_DIR_IN_SEC (600)	//10 MIN


  #define USE_FLAGS_VS_AVE_WSPEED_DIR  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2)

  #define SA_WS_1 ((ulong) SENSOR_WIND_SPEED_AVE_WHOLE_VS)
  #define SA_WS_2 ((ulong) SENSOR_WIND_SPEED_AVE_FRAC_VS)
  #define SA_WS_3 ((ulong) SENSOR_WIND_DIR_AVE_WHOLE_VS)
  #define SA_WS_4 ((ulong) SENSOR_WIND_DIR_AVE_FRAC_VS)

  #define SENSE_ACT_VS_AVE_WSPEED_DIR_VAL ((SA_WS_1<<24) | (SA_WS_2<<16) | (SA_WS_3<<8)  | (SA_WS_4))


  #define VS_AVE_WSPEED_DIR_WHO_CAN_RUN		RBIT_SAMPLERS



/**************************  ACTION 20  ***************************************/
 #define E_ACTN_VS_AIR_PRESS_TEMP_SAMPLE	20	//Vaisala Air Press & Temp

  /* DEFAULT SAMPLE INTERVAL */
  #ifndef SAMPLE_INTERVAL_VS_AIR_PRESS_TEMP_I
//		#define SAMPLE_INTERVAL_VS_AIR_PRESS_TEMP_I	 ((int)   30)	//30 Sec
//		#define SAMPLE_INTERVAL_VS_AIR_PRESS_TEMP_I	 ((int)   60)	// 1 Min
//		#define SAMPLE_INTERVAL_VS_AIR_PRESS_TEMP_I	 ((int)  256)	// 1 Frame
//		#define SAMPLE_INTERVAL_VS_AIR_PRESS_TEMP_I	 ((int)  512)	// 2 Frames
//		#define SAMPLE_INTERVAL_VS_AIR_PRESS_TEMP_I	 ((int)  540)	// 9 Min
//		#define SAMPLE_INTERVAL_VS_AIR_PRESS_TEMP_I	 ((int)  900)	//15 Min
		#define SAMPLE_INTERVAL_VS_AIR_PRESS_TEMP_I	 ((int) 1800)	//30 Min
  #endif

  #define STS_LFACT_VS_AIR_PRESS_TEMP (3600 / SAMPLE_INTERVAL_VS_AIR_PRESS_TEMP_I)

  #define HR0_TO_SAMPLE0_VS_AIR_PRESS_TEMP_IN_SEC ( 0 )
//#define HR0_TO_SAMPLE0_VS_AIR_PRESS_TEMP_IN_SEC (600)	//10 MIN



  #define USE_FLAGS_VS_AIR_PRESS_TEMP  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2)

  #define SA_AP_1 ((ulong) SENSOR_AIR_PRESS_WHOLE_VS)
  #define SA_AP_2 ((ulong) SENSOR_AIR_PRESS_FRAC_VS)
  #define SA_AP_3 ((ulong) SENSOR_AIR_TEMP_WHOLE_VS)
  #define SA_AP_4 ((ulong) SENSOR_AIR_TEMP_FRAC_VS)

  #define SENSE_ACT_VS_AIR_PRESS_TEMP_VAL ((SA_AP_1<<24) | (SA_AP_2<<16) | (SA_AP_3<<8)  | (SA_AP_4))


  #define VS_AIR_PRESS_TEMP_WHO_CAN_RUN		RBIT_SAMPLERS




/**************************  ACTION 21  ***************************************/
 #define E_ACTN_VS_REL_HUMID_RAIN_SAMPLE 21	//Vaisala Rel Humidity, and Rain

	/* DEFAULT SAMPLE INTERVAL */
	#ifndef SAMPLE_INTERVAL_VS_REL_HUMID_RAIN_I
//		#define SAMPLE_INTERVAL_VS_REL_HUMID_RAIN_I	 ((int)   30)	//30 Sec
//		#define SAMPLE_INTERVAL_VS_REL_HUMID_RAIN_I	 ((int)   60)	// 1 Min
//		#define SAMPLE_INTERVAL_VS_REL_HUMID_RAIN_I	 ((int)  256)	// 1 Frame
//		#define SAMPLE_INTERVAL_VS_REL_HUMID_RAIN_I	 ((int)  512)	// 2 Frames
//		#define SAMPLE_INTERVAL_VS_REL_HUMID_RAIN_I	 ((int)  540)	// 9 Min
//		#define SAMPLE_INTERVAL_VS_REL_HUMID_RAIN_I	 ((int)  900)	//15 Min
		#define SAMPLE_INTERVAL_VS_REL_HUMID_RAIN_I	 ((int) 1800)	//30 Min
	#endif

	#define STS_LFACT_VS_REL_HUMID_RAIN (3600 / SAMPLE_INTERVAL_VS_REL_HUMID_RAIN_I)

	#define HR0_TO_SAMPLE0_VS_REL_HUMID_RAIN_IN_SEC ( 0 )
//	#define HR0_TO_SAMPLE0_VS_REL_HUMID_RAIN_IN_SEC (600)	//10 MIN


  #define USE_FLAGS_VS_REL_HUMID_RAIN  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2)

	#define SA_RH_1 ((ulong) SENSOR_REL_HUMID_WHOLE_VS)
	#define SA_RH_2 ((ulong) SENSOR_REL_HUMID_FRAC_VS)
	#define SA_RH_3 ((ulong) SENSOR_RAIN_ACC_WHOLE_VS)
	#define SA_RH_4 ((ulong) SENSOR_RAIN_ACC_FRAC_VS)

	#define SENSE_ACT_VS_REL_HUMID_RAIN_VAL ((SA_RH_1<<24) | (SA_RH_2<<16) | (SA_RH_3<<8)  | (SA_RH_4))

  #define VS_REL_HUMID_RAIN_WHO_CAN_RUN		RBIT_SAMPLERS




/**************************  ACTION 22  ***************************************/
 #define E_ACTN_SAP_12_SAMPLE		22	//SAP FLOW 1 & 2

  #ifdef COMPILE_FOR_WIZ2_DUKETEST
		#define SAMPLE_INTERVAL_SAP_12_I	 ((int)  256)	//1 frame
  #endif

  /* DEFAULT SAMPLE INTERVAL */
  #ifndef SAMPLE_INTERVAL_SAP_12_I
//		#define SAMPLE_INTERVAL_SAP_12_I	 ((int)   30)	//30 Sec
//		#define SAMPLE_INTERVAL_SAP_12_I	 ((int)   60)	// 1 Min
//		#define SAMPLE_INTERVAL_SAP_12_I	 ((int)  256)	// 1 Frame
//		#define SAMPLE_INTERVAL_SAP_12_I	 ((int)  512)	// 2 Frames
//		#define SAMPLE_INTERVAL_SAP_12_I	 ((int)  540)	// 9 Min
//		#define SAMPLE_INTERVAL_SAP_12_I	 ((int)  900)	//15 Min
		#define SAMPLE_INTERVAL_SAP_12_I	 ((int) 1800)	//30 Min
  #endif

  #define STS_LFACT_SAP_12 (3600 / SAMPLE_INTERVAL_SAP_12_I)

  #define HR0_TO_SAMPLE0_SAP_12_IN_SEC ( 0 )
//#define HR0_TO_SAMPLE0_SAP_12_IN_SEC (600)	//10 MIN


  #define USE_FLAGS_SAP_12  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2)

  #define SA_SAP12_1 ((ulong) SENSOR_SAP_1)
  #define SA_SAP12_2 ((ulong) SENSOR_SAP_2)

  #define SENSE_ACT_SAP_12_VAL ((SA_SAP12_1<<24) | (SA_SAP12_2<<16)) 

  #define SAP12_WHO_CAN_RUN		RBIT_SAMPLERS






/**************************  ACTION 23  ***************************************/
 #define E_ACTN_SAP_34_SAMPLE		23	//SAP FLOW 3 & 4


	#ifdef COMPILE_FOR_WIZ2_DUKETEST
	  #define SAMPLE_INTERVAL_SAP_34_I	 ((int)  256)	//1 frame
	#endif

	#ifdef COMPILE_FOR_MARK1234
	  #define SAMPLE_INTERVAL_SAP_34_I	 ((int)   30)	//30 Sec
	#endif

	/* DEFAULT SAMPLE INTERVAL */
	#ifndef SAMPLE_INTERVAL_SAP_34_I
//	  #define SAMPLE_INTERVAL_SAP_34_I	 ((int)   30)	//30 Sec
//	  #define SAMPLE_INTERVAL_SAP_34_I	 ((int)   60)	// 1 Min
//	  #define SAMPLE_INTERVAL_SAP_34_I	 ((int)  256)	// 1 Frame
//	  #define SAMPLE_INTERVAL_SAP_34_I	 ((int)  512)	// 2 Frames
//	  #define SAMPLE_INTERVAL_SAP_34_I	 ((int)  540)	// 9 Min
//	  #define SAMPLE_INTERVAL_SAP_34_I	 ((int)  900)	//15 Min
	  #define SAMPLE_INTERVAL_SAP_34_I	 ((int) 1800)	//30 Min
	#endif

	#define STS_LFACT_SAP_34 (3600 / SAMPLE_INTERVAL_SAP_34_I)

	#define HR0_TO_SAMPLE0_SAP_34_IN_SEC ( 0 )
//	#define HR0_TO_SAMPLE0_SAP_34_IN_SEC (600)	//10 MIN


  #define USE_FLAGS_SAP_34  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2)

  #define SA_SAP34_1 ((ulong) SENSOR_SAP_3)
  #define SA_SAP34_2 ((ulong) SENSOR_SAP_4)

  #define SENSE_ACT_SAP_34_VAL ((SA_SAP34_1<<24) | (SA_SAP34_2<<16)) 

  #define SAP34_WHO_CAN_RUN		RBIT_SAMPLERS


 #define E_ACTN_MAX_NUM_IDX			(23+1)


/********************************  NOTE  *************************************
*
* NOTE: WHEN ADDING A NEW ACTION, YOU MUST:
*
* 1. Write routine to perform the event (this ".C" module).
* 2. Provide defines above for the action (this ".H" module).
* 3. Add new entry in ACTION vector tbl vEventPtrArray[] (this ".C" module).
* 4. Add new entry in RTS table cpActionName[] (this ".C" module).
* 5. Add new init action in vSTBL_setupInitialSchedTbls() (STBL ".C" module).
*		
*
******************************************************************************/


 /* ROUTINE DEFINITIONS */

void vACTION_dispatch_to_event(
		unsigned char ucNSTtblNum,		//table num (0 or 1)
		unsigned char ucNSTslotNum		//slot num in table
		);

void vACTION_read_sensors_into_buffer(//first 0 sensor num ends read
		uchar ucSensorNum[4]	//Sensors [0]=first, [3]=last
		);

void vACTION_showActionName(
		uchar ucActionNum			//takes an action number
		);

void vACTION_showStblActionName(
		uchar ucStblIdx				//takes an Stbl index
		);

void vACTION_showBattReading(
		void
		);

void vACTION_do_Batt_Sense(
		void
		);


#endif /* ACTION_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */


/***************************  L2SRAM.H  ****************************************
*
* Header for L2SRAM routine pkg
*
* NOTE: See Main.h for the disk layout map. 
*
*
*
* V1.00 06/18/2003 wzr
*	started
*
******************************************************************************/

#ifndef L2SRAM_H_INCLUDED
 #define L2SRAM_H_INCLUDED



/***************  PARALLEL TABLES (STBL'S) DEFINITIONS  ***********************
*
* NOTE: All the tables listed below are parallel so that an index into one
* 		can be used as an index into another.  The names are likewise parallel.
*
* NOTE: The NST 1 and 2 tables are not part of the SCHED table parallelism.
*
*******************************************************************************/

 #define GENERIC_SCHED_MAX_IDX				128
 #define GENERIC_SCHED_LAST_IDX_VAL			(GENERIC_SCHED_MAX_IDX-1)
 #define GENERIC_SCHED_MAX_IDX_MASK			(GENERIC_SCHED_MAX_IDX-1)


 /*----------- TABLE 0 (PRIORITY_FUNC_TBL) ----------------------------------*/
 #define SCHED_PRIORITY_FUNC_TBL_NUM				0
 #define SCHED_PRIORITY_FUNC_TBL_BASE_ADDR			0
 #define SCHED_PRIORITY_FUNC_TBL_BYTE_WIDTH			1
 #define SCHED_PRIORITY_FUNC_TBL_END_ADDR_PLUS_1	(SCHED_PRIORITY_FUNC_TBL_BASE_ADDR + \
 				(GENERIC_SCHED_MAX_IDX * SCHED_PRIORITY_FUNC_TBL_BYTE_WIDTH))

 #define SCHED_PRIORITY_FUNC_NOT_USED_VAL	(PRIORITY_7 | SCHED_FUNC_DORMANT_SLOT)


 /*----------- TABLE 1 (SCHED SERIAL NUMBER TBL) ----------------------------*/
 #define SCHED_SN_TBL_NUM					1
 #define SCHED_SN_TBL_BASE_ADDR				SCHED_PRIORITY_FUNC_TBL_END_ADDR_PLUS_1
 #define SCHED_SN_TBL_BYTE_WIDTH			2
 #define SCHED_SN_TBL_END_ADDR_PLUS_1		(SCHED_SN_TBL_BASE_ADDR + \
 					(GENERIC_SCHED_MAX_IDX * SCHED_SN_TBL_BYTE_WIDTH))

 #define SCHED_SN_TBL_NOT_USED_VAL			0
 #define SCHED_SN_TBL_SAMPLE_BATT			1
 #define SCHED_SN_TBL_F_SERVICE				2
 #define SCHED_SN_TBL_SND_TST_MSG			3
 #define SCHED_SN_TBL_TC_12					4
 #define SCHED_SN_TBL_TC_34					5
 #define SCHED_SN_TBL_LT_12					6
 #define SCHED_SN_TBL_SL_12					7
 #define SCHED_SN_TBL_SCHEDULER				8
 


 /*----------- TABLE 2 (SCHED RANDOM NUMBER TBL 1 ) -------------------------------*/
 #define GENERIC_SCHED_RAND_TBL_NOT_USED_VAL ((USL) 0)
 #define GENERIC_SCHED_RAND_TBL_BYTE_WIDTH 3

 #define SCHED_RAND_TBL_1_NUM				2
 #define SCHED_RAND_TBL_1_BASE_ADDR			SCHED_SN_TBL_END_ADDR_PLUS_1
 #define SCHED_RAND_TBL_1_BYTE_WIDTH		GENERIC_SCHED_RAND_TBL_BYTE_WIDTH
 #define SCHED_RAND_TBL_1_END_ADDR_PLUS_1	(SCHED_RAND_TBL_1_BASE_ADDR + \
 					(GENERIC_SCHED_MAX_IDX * SCHED_RAND_TBL_1_BYTE_WIDTH))

 #define SCHED_RAND_TBL_1_NOT_USED_VAL		GENERIC_SCHED_RAND_TBL_NOT_USED_VAL



 /*----------- TABLE 3 (SCHED RANDOM NUMBER TBL 2 ) -------------------------------*/
 #define SCHED_RAND_TBL_2_NUM				3
 #define SCHED_RAND_TBL_2_BASE_ADDR			SCHED_RAND_TBL_1_END_ADDR_PLUS_1
 #define SCHED_RAND_TBL_2_BYTE_WIDTH		GENERIC_SCHED_RAND_TBL_BYTE_WIDTH
 #define SCHED_RAND_TBL_2_END_ADDR_PLUS_1	(SCHED_RAND_TBL_2_BASE_ADDR + \
 					(GENERIC_SCHED_MAX_IDX * SCHED_RAND_TBL_2_BYTE_WIDTH))

 #define SCHED_RAND_TBL_2_NOT_USED_VAL		GENERIC_SCHED_RAND_TBL_NOT_USED_VAL



 /*-----------  SCHED GENERIC DCNT TABLE  ----------------------------------------*/

/**************************  NOTE  ******************************************
*
*    15       14       13       12       11       10       9        8
*зддддддддбддддддддбддддддддбддддддддбддддддддбддддддддбддддддддбдддддддд©
*ЁCountingЁ Alarm  ЁStop @ 1Ё        Ё        Ё        Ё        Ё        Ё
*Ё дддддд Ё дддддд Ё дддддд Ё        Ё        Ё		   Ё        Ё        Ё
*Ё Fixed  Ё   0    ЁStop @ 0Ё        Ё        Ё        Ё        Ё        Ё
*юддддддддаддддддддаддддддддаддддддддаддддддддаддддддддаддддддддадддддддды
*    7        6        5        4        3         2       1        0
*зддддддддбддддддддбддддддддбддддддддбддддддддбддддддддбддддддддбдддддддд©
*Ё        Ё        Ё        Ё  Down Count     Ё        Ё        Ё        Ё
*Ё<--------------------------------------------------------------------->Ё
*Ё        Ё        Ё        Ё        Ё        Ё        Ё        Ё        Ё
*юддддддддаддддддддаддддддддаддддддддаддддддддаддддддддаддддддддадддддддды
*
* This Sched entry is handled by the routine called uiRTS_handleLoadCount().
*
* 1.If (Counting bit = fixed number) (signified by bit 15 = 0) then the
*	downcount is assumed to mean that this entry is to be loaded into the
*	NST the number of times indicated in the downcount, and the alarm
*	bit will be returned set indicating an action should be taken.
*
* 2.If (Counting bit = Counting) (signified by bit 15 = 1) then the
*	value in downcount is decremented by 1 each frame and when the
*	value of either 1 or 0 (as indicated by bit 13 = 1 or 0) is reached
*	the alarm is set as follows:
*	  a. For stop at 1 (Alarm is set every time value is 1).
*	  b. For stop at 0 (Alarm is set on first encounter with 0 and not again).
*
*
* SUMMARY:
*	1.if fixed => No Dec, Set alarm every time even if downcount=0.
*	2.if counting & stop at 1 => Dec if not 1, if(Count=1) set alarm.
*	3.if counting & stop at 0 => Dec if not 0, if(First time Count=0) set alarm.
*
*
****************************************************************************/

// #define GENERIC_DCNT_TBL_MAX_IDX			GENERIC_SCHED_MAX_IDX
 #define GENERIC_DCNT_TBL_NOT_USED_VAL		0
 #define GENERIC_DCNT_TBL_FIXED_DCNT_1		1
 #define GENERIC_DCNT_TBL_BYTE_WIDTH		2

 #define F_DCNT_COUNTING	0x8000 //1=counting, 0=fixed (not counting)
 #define F_DCNT_ALARM		0x4000 //1=alarm set, 0=alarm clr
 #define F_DCNT_STOP_AT_ONE	0x2000 //1=end at 1, 0=end at 0

 #define F_DCNT_FLAGS_ONLY_MASK	0xFF00
 #define F_DCNT_COUNT_ONLY_MASK	0x003F



 /*----------- TABLE 4 (SCHED DCNT 1 TBL) -----------------------------------------*/
 #define SCHED_DCNT_TBL_1_NUM				4
 #define SCHED_DCNT_TBL_1_BASE_ADDR			SCHED_RAND_TBL_2_END_ADDR_PLUS_1
 #define SCHED_DCNT_TBL_1_END_ADDR_PLUS_1	(SCHED_DCNT_TBL_1_BASE_ADDR + \
 					(GENERIC_SCHED_MAX_IDX * GENERIC_DCNT_TBL_BYTE_WIDTH))


 /*----------- TABLE 5 (SCHED DCNT 2 TBL) -----------------------------------------*/
 #define SCHED_DCNT_TBL_2_NUM				5
 #define SCHED_DCNT_TBL_2_BASE_ADDR			SCHED_DCNT_TBL_1_END_ADDR_PLUS_1
 #define SCHED_DCNT_TBL_2_END_ADDR_PLUS_1	(SCHED_DCNT_TBL_2_BASE_ADDR + \
 					(GENERIC_SCHED_MAX_IDX * GENERIC_DCNT_TBL_BYTE_WIDTH))



 /*----------- TABLE 6 (SCHED START TIME TBL) -------------------------------*/
 #define SCHED_ST_TBL_NUM				6
 #define SCHED_ST_TBL_BASE_ADDR			SCHED_DCNT_TBL_2_END_ADDR_PLUS_1
// #define SCHED_ST_TBL_MAX_IDX			GENERIC_SCHED_MAX_IDX
 #define SCHED_ST_TBL_BYTE_WIDTH		2
 #define SCHED_ST_TBL_END_ADDR_PLUS_1	(SCHED_ST_TBL_BASE_ADDR + \
 					(GENERIC_SCHED_MAX_IDX * SCHED_ST_TBL_BYTE_WIDTH))

 #define SCHED_ST_TBL_NOT_USED_VAL	0



 /*----------- TABLE 7 (SCHED INTERVAL TIME TBL) ----------------------------*/
 #define SCHED_INTRVL_TBL_NUM				7
 #define SCHED_INTRVL_TBL_BASE_ADDR			SCHED_ST_TBL_END_ADDR_PLUS_1
// #define SCHED_INTRVL_TBL_MAX_IDX			GENERIC_SCHED_MAX_IDX
 #define SCHED_INTRVL_TBL_BYTE_WIDTH		2
 #define SCHED_INTRVL_TBL_END_ADDR_PLUS_1	(SCHED_INTRVL_TBL_BASE_ADDR + \
 					(GENERIC_SCHED_MAX_IDX * SCHED_INTRVL_TBL_BYTE_WIDTH))

 #define SCHED_INTRVL_TBL_NOT_USED_VAL	0



/*----------- TABLE 8 (SCHED FLAG TBL) ----------------------------------------
*     7        6        5        4       3         2        1        0
*
*                    MAKES
*                     OM2's Ё<----- USE ----->Ё         RAND-NUM   SLOT
*зддддддддбддддддддбддддддддбддддддддбддддддддбддддддддбддддддддбдддддддд©
*Ё        Ё        ЁMks OM2sЁ Master Ё Slave  Ё        ЁThisRandЁ Exact  Ё
*Ё        Ё        Ё дддддд Ё  Run   Ё  Run   Ё		   Ё дддддд Ё дддддд Ё
*Ё        Ё        ЁNo OM2s Ё  Sts   Ё  Sts   Ё        Ё AnyRandЁNonExactЁ
*юддддддддаддддддддаддддддддаддддддддаддддддддаддддддддаддддддддадддддддды
*			                   00 - no run
*                              01 - Slaves only
*                              10 - Masters only  
*                              11 - Masters & Slaves
*
*----------------------------------------------------------------------------*/

 #define SCHED_FLAG_TBL_NUM				8
 #define SCHED_FLAG_TBL_BASE_ADDR		SCHED_INTRVL_TBL_END_ADDR_PLUS_1
// #define SCHED_FLAG_TBL_MAX_IDX			GENERIC_SCHED_MAX_IDX
 #define SCHED_FLAG_TBL_BYTE_WIDTH		1
 #define SCHED_FLAG_TBL_END_ADDR_PLUS_1	(SCHED_FLAG_TBL_BASE_ADDR + \
 					(GENERIC_SCHED_MAX_IDX * SCHED_FLAG_TBL_BYTE_WIDTH))

  /* FLAG TBL FLAG BITS */
  #define F_USE_EXACT_SLOT		0x01	//0000 0001
  #define F_USE_NONEXACT_SLOT	0x00	//0000 0000
								
  #define F_USE_THIS_RAND		0x02	//0000 0010
  #define F_USE_ANY_RAND		0x00	//0000 0000

//  #define F_USE_NO_RUN					0x00		//00000000
//  #define F_USE_SLAVE_ONLY_RUN			0x08		//00001000
//  #define F_USE_MASTER_ONLY_RUN			0x10		//00010000
//  #define F_USE_MASTER_AND_SLAVE_RUN		0x18		//00011000
//  #define F_USE_MASTER_SLAVE_MASK_BITS	0x18		//00011000

  #define F_USE_MAKE_OM2		0x20	//00100000
  #define F_USE_NO_MAKE_OM2		0x00	//00000000


#if 0
 #define USE_FLAGS_DELETED_ENTRY  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | \
									F_USE_NO_MAKE_OM2)

 #define USE_FLAGS_DELETED_ENTRY  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | \
									F_USE_NO_RUN | F_USE_NO_MAKE_OM2)
#endif


 /* SPECIFIC FLAG USAGES */
 #define SCHED_FLAG_TBL_NOT_USED_VAL 0x00	//00000000

 #define SCHED_FLAG_TBL_DEFAULT_VAL	(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | \
					 				F_USE_NO_SOM2_REQIRED)




/*----------- TABLE 9 & 10 (SCHED STATUS TBL) --------------------------------
*
*    15       14       13       12       11       10       9        8
*зддддддддбддддддддбддддддддбддддддддбддддддддбддддддддбддддддддбдддддддд©
*Ё        Ё        Ё        Ё        Ё        Ё    missed  msg count     Ё
*Ё        Ё        Ё        Ё        Ё        Ё<------------------------>Ё
*Ё        Ё        Ё        Ё        Ё        Ё        Ё        Ё        Ё
*юддддддддаддддддддаддддддддаддддддддаддддддддаддддддддаддддддддадддддддды
*    7        6        5        4        3         2       1        0
*зддддддддбддддддддбддддддддбддддддддбддддддддбддддддддбддддддддбдддддддд©
*ЁcountingЁcpy dcntЁ        Ё   LFactor or Linkup_dcnt                   Ё        Ё
*ЁддддддддЁддддддддЁ<--------------------------------------------------->Ё
*Ё fixed  Ё   0    Ё        Ё        Ё        Ё        Ё        Ё        Ё
*юддддддддаддддддддаддддддддаддддддддаддддддддаддддддддаддддддддадддддддды
*                            LFactor
*Ё<--------------------------------------------------------------------->Ё
*
* The status word gets stuffed with the LFactor from a number of places.
*
* 1.For the SOM2 it is stuffed with the LINKUP_DCNT received in the OM3
*	from the last contact with the downstream wizard.
*
* 2.For the ROM2 it is stuffed with the LINKUP_DCNT computed onboard and
*	sent to the upstream wizard in the last contact OM3.
*
* 3.For the sensing actions it is stuffed with LFACTOR for the particular
*   load that this sensing puts on this system in units of msgs/hr.
*
* NOTE: The LINKUP_DCNT is eventually copied into the DCNT value tbl, and
*		is used to determine how many contacts to make with the 
*		upstream/downstream wizard by the scheduler.
*		
*----------------------------------------------------------------------------*/
// #define GENERIC_STAT_TBL_MAX_IDX			GENERIC_SCHED_MAX_IDX
 #define GENERIC_STAT_TBL_NOT_USED_VAL		0
 #define GENERIC_STAT_TBL_LNKUP_CNT_1		1
 #define GENERIC_STAT_TBL_BYTE_WIDTH		2
 #define GENERIC_STAT_TBL_INIT_VAL			0

 /* STATUS TBL FLAGS */
 #define F_STS_COUNTING			0x0080
 #define F_STS_FIXED			0x0000

 #define F_STS_COPY_TO_DCNT		0x0040
 #define F_STS_NO_COPY_DCNT		0x0000

 #define F_STS_COUNT_ONLY_MASK	0x003F



/*----------- TABLE 9 (SCHED STATUS 1 TBL) ---------------------------------*/
 #define SCHED_STAT_TBL_1_NUM				9
 #define SCHED_STAT_TBL_1_BASE_ADDR			SCHED_FLAG_TBL_END_ADDR_PLUS_1
 #define SCHED_STAT_TBL_1_BYTE_WIDTH		GENERIC_STAT_TBL_BYTE_WIDTH
 #define SCHED_STAT_TBL_1_END_ADDR_PLUS_1	(SCHED_STAT_TBL_1_BASE_ADDR + \
 					(GENERIC_SCHED_MAX_IDX * GENERIC_STAT_TBL_BYTE_WIDTH))

/*----------- TABLE 10 (SCHED STATUS 2 TBL) ---------------------------------*/
 #define SCHED_STAT_TBL_2_NUM				10
 #define SCHED_STAT_TBL_2_BASE_ADDR			SCHED_STAT_TBL_1_END_ADDR_PLUS_1
 #define SCHED_STAT_TBL_2_BYTE_WIDTH		GENERIC_STAT_TBL_BYTE_WIDTH
 #define SCHED_STAT_TBL_2_END_ADDR_PLUS_1	(SCHED_STAT_TBL_2_BASE_ADDR + \
 					(GENERIC_SCHED_MAX_IDX * GENERIC_STAT_TBL_BYTE_WIDTH))



 /*----------- TABLE 11 (SCHED ACTION TBL) ----------------------------------*/
 #define SCHED_ACTN_TBL_NUM					11
 #define SCHED_ACTN_TBL_BASE_ADDR			SCHED_STAT_TBL_2_END_ADDR_PLUS_1
 #define SCHED_ACTN_TBL_BYTE_WIDTH			1
 #define SCHED_ACTN_TBL_END_ADDR_PLUS_1		(SCHED_ACTN_TBL_BASE_ADDR + \
 					(GENERIC_SCHED_MAX_IDX * SCHED_ACTN_TBL_BYTE_WIDTH))

 #define SCHED_ACTN_TBL_NOT_USED_VAL	0



 /*----------- TABLE 12 (LNKBLK COUNT TBL) ----------------------------------*/
 #define SCHED_LNKBLK_COUNT_TBL_NUM			12
 #define SCHED_LNKBLK_COUNT_TBL_BASE_ADDR		SCHED_ACTN_TBL_END_ADDR_PLUS_1
 #define SCHED_LNKBLK_COUNT_TBL_BYTE_WIDTH	1
 #define SCHED_LNKBLK_COUNT_TBL_END_ADDR_PLUS_1 (SCHED_LNKBLK_COUNT_TBL_BASE_ADDR + \
 					(GENERIC_SCHED_MAX_IDX * SCHED_LNKBLK_COUNT_TBL_BYTE_WIDTH))

 #define SCHED_LNKBLK_COUNT_TBL_NOT_USED_VAL	0



 /*----------- TABLE 13 (SENSE ACT TBL) -------------------------------------*/
 #define SCHED_SENSE_ACT_TBL_NUM			13
 #define SCHED_SENSE_ACT_TBL_BASE_ADDR		SCHED_LNKBLK_COUNT_TBL_END_ADDR_PLUS_1
 #define SCHED_SENSE_ACT_TBL_BYTE_WIDTH		4
 #define SCHED_SENSE_ACT_TBL_END_ADDR_PLUS_1 (SCHED_SENSE_ACT_TBL_BASE_ADDR + \
 					(GENERIC_SCHED_MAX_IDX * SCHED_SENSE_ACT_TBL_BYTE_WIDTH))

 #define SCHED_SENSE_ACT_TBL_NOT_USED_VAL_UL	((ulong) 0)



 /* The rest of the action numbers are in ACTION.h */


/*----------------------  LAST SCHED TBL DEFINES  ---------------------------*/

 #define LAST_SCHED_TBL_NUM				13
 #define MAX_SCHED_TBL_NUM				(LAST_SCHED_TBL_NUM + 1)
 #define SCHED_LAST_TBL_BASE_ADDR		SCHED_SENSE_ACT_TBL_BASE_ADDR
 #define SCHED_LAST_TBL_END_ADDR_PLUS_1 SCHED_SENSE_ACT_TBL_END_ADDR_PLUS_1


 /*****************************  NOTE  ***************************************
 *
 * To add a new Sched table you must:
 *
 * 1. Add an entry here in the table defines
 * 2. Add an entry in STBL.C in uslStblBaseAddr[].
 * 3. Add an entry in STBL.C in cpaStblLongName[].
 * 4. Add an entry in STBL.C in ucColTabTbl[].
 * 5. Add an entry in STBL.C in ucStblByteWidth[].
 * 6. Add an entry in STBL.C in ulStblNotUsedVal[].
 * 7. Add an entry in STBL.C in vRTS_stuffSingleStblEntry().
 * 8. Add an entry in STBL.H in vRTS_stuffSingleStblEntry().
 * 9. Add an entry in every call to vRTS_stuffSingleStblEntry().
 *10. Hook up code to setup the table in vRTS_stuffSingleStblEntry().
 *
 * 
 *11. Make sure all the calls in vSTBL_setupInitialStbls() are adjusted.
 *12. Hook up the code to use the new table in all the various places
 *
 ****************************************************************************/

/*----------------------  END SCHED TBLS  -----------------------------------*/


/*---------------  NEXT SLOT TABLES (NST) DEFINES  ---------------------------*/

 #define GENERIC_NST_MAX_IDX	 	64
 #define GENERIC_NST_LAST_IDX		(GENERIC_NST_MAX_IDX -1)
 #define GENERIC_NST_MAX_IDX_MASK	(GENERIC_NST_MAX_IDX -1)
 #define GENERIC_NST_BYTE_WIDTH		1
 #define GENERIC_NST_NOT_USED_VAL	0xCC
 #define MAX_NST_TBL_COUNT			2
 #define GENERIC_NST_TBL_SIZE		(GENERIC_NST_MAX_IDX * GENERIC_NST_BYTE_WIDTH)

 /*----------------  NST 1  -------------------------------------------------*/

 #define NST_1_TBL_BASE_ADDR		SCHED_LAST_TBL_END_ADDR_PLUS_1
 #define NST_1_TBL_END_ADDR_PLUS_1	(NST_1_TBL_BASE_ADDR + GENERIC_NST_TBL_SIZE)


 /*----------------  NST 2  -------------------------------------------------*/

 #define NST_2_TBL_BASE_ADDR		NST_1_TBL_END_ADDR_PLUS_1
 #define NST_2_TBL_END_ADDR_PLUS_1	(NST_2_TBL_BASE_ADDR + GENERIC_NST_TBL_SIZE)


/*------------------------  END NST TBLS  -----------------------------------*/


  
/*---------------  SAVED_SENSE_POINT (SSP) TABLES  --------------------------*/

 #define SSP_GENERIC_TBL_SIZE	256

/*--------------------- TABLE 0 (SSP Y READING TBL) -------------------------*/
 #define SSP_Y_TBL_NUM					0
 #define SSP_Y_TBL_BASE_ADDR			NST_2_TBL_END_ADDR_PLUS_1
 #define SSP_Y_TBL_BYTE_WIDTH			2
 #define SSP_Y_TBL_END_ADDR_PLUS_1 		(SSP_Y_TBL_BASE_ADDR + \
 					(SSP_GENERIC_TBL_SIZE * SSP_Y_TBL_BYTE_WIDTH))

 #define SSP_Y_TBL_NOT_USED_VAL	0

/*--------------------- TABLE 1 (SSP T (TIME) TBL) --------------------------*/
 #define SSP_T_TBL_NUM					1
 #define SSP_T_TBL_BASE_ADDR			SSP_Y_TBL_END_ADDR_PLUS_1
 #define SSP_T_TBL_BYTE_WIDTH			4
 #define SSP_T_TBL_END_ADDR_PLUS_1		(SSP_T_TBL_BASE_ADDR + \
 					(SSP_GENERIC_TBL_SIZE * SSP_T_TBL_BYTE_WIDTH))

 #define SSP_T_TBL_NOT_USED_VAL	0

/*--------------------- TABLE 2 (SSP DELTA_Y_TRIG TBL) -----------------------*/
 #define SSP_DELTA_Y_TRIG_TBL_NUM			2
 #define SSP_DELTA_Y_TRIG_TBL_BASE_ADDR		SSP_T_TBL_END_ADDR_PLUS_1
 #define SSP_DELTA_Y_TRIG_TBL_BYTE_WIDTH	2
 #define SSP_DELTA_Y_TRIG_TBL_END_ADDR_PLUS_1 (SSP_DELTA_Y_TRIG_TBL_BASE_ADDR + \
 					(SSP_GENERIC_TBL_SIZE * SSP_DELTA_Y_TRIG_TBL_BYTE_WIDTH))

 #define SSP_DELTA_Y_TRIG_TBL_NOT_USED_VAL	0

/*--------------------- TABLE 3 (SSP DELTA_T_TRIG TBL) -----------------------*/
 #define SSP_DELTA_T_TRIG_TBL_NUM			3
 #define SSP_DELTA_T_TRIG_TBL_BASE_ADDR		SSP_DELTA_Y_TRIG_TBL_END_ADDR_PLUS_1
 #define SSP_DELTA_T_TRIG_TBL_BYTE_WIDTH	2
 #define SSP_DELTA_T_TRIG_TBL_END_ADDR_PLUS_1 (SSP_DELTA_T_TRIG_TBL_BASE_ADDR + \
 					(SSP_GENERIC_TBL_SIZE * SSP_DELTA_T_TRIG_TBL_BYTE_WIDTH))

 #define SSP_DELTA_T_TRIG_TBL_NOT_USED_VAL	0


 #define LAST_SSP_TBL_NUM_PLUS_1	4


/*------------------------  END SSP TBLS  -----------------------------------*/



/*-----------------  RADIO LINK BLK (LNKBLK) TABLE  -------------------------*/

/*-----------------------  LNKBLK  NOTE  -------------------------------------
*
* The LnkBlk table below is a table of blocks that are 8 bytes long and each
* block is a 4 byte long used to hold the time of the next link.  There are
* 8 links in a single frame possible hence the 8 entrys.  Each Block of 8 
* corresponds to the STBL index and the base address can be computed as
* Stbl Index * 8.  Each blk entry is terminate by either the length of the 
* blk or a zero entry in the table.  The Sblk table entry above called
* LNKBLK_COUNT_TBL contains the count of the number of entry in the block as
* a quick and redundant check on the number of links planned for a frame.
* The link times only correspond to a single frame and are updated when an
* ROM2 or SOM2 link has been completed.
*
*----------------------------------------------------------------------------*/

 #define LNKBLK_TBL_BASE_ADDR        SSP_DELTA_T_TRIG_TBL_END_ADDR_PLUS_1

// #define LNKBLK_TBL_BYTE_WIDTH       4
// #define LNKBLK_TBL_BLK_SIZE         8

 #define BYTES_PER_LNKBLK_ENTRY			4
 #define ENTRYS_PER_LNKBLK_BLK			8

 #define BYTES_PER_LNKBLK_ENTRY_L	((long) BYTES_PER_LNKBLK_ENTRY)
 #define ENTRYS_PER_LNKBLK_BLK_L	((long) ENTRYS_PER_LNKBLK_BLK)
 #define BYTES_PER_LNKBLK_BLK_L		(ENTRYS_PER_LNKBLK_BLK_L * BYTES_PER_LNKBLK_ENTRY_L)

 #define LNKBLK_TBL_END_ADDR_PLUS_1	 (LNKBLK_TBL_BASE_ADDR + \
							 (GENERIC_SCHED_MAX_IDX * BYTES_PER_LNKBLK_BLK_L))

 #define LNKBLK_TBL_NOT_USED_VAL      0
 #define LNKBLK_TBL_NOT_USED_VAL_UL   ((ulong) 0)



/*----------------  END: RADIO LINK BLK (LNKBLK) TABLE  ---------------------*/



/*------------------  MSG STORE AND FORWARD MEMORY  -------------------------*/
#define L2SRAM_BASE_ADDR             LNKBLK_TBL_END_ADDR_PLUS_1
#define L2SRAM_BASE_ADDR_UL ((ulong) LNKBLK_TBL_END_ADDR_PLUS_1)


#define L2SRAM_MSG_PAGE_MASK_UL	((ulong)0x0000001F)
#define L2SRAM_MSG_Q_BEG_UL	((L2SRAM_BASE_ADDR_UL+31) & ~L2SRAM_MSG_PAGE_MASK_UL)

/* test length */
//#define SRAM_TEST_MSG_BUFF_SIZE (5UL * MAX_MSG_SIZE_UL)
//#define L2SRAM_MSG_Q_END_UL	(L2SRAM_MSG_Q_BEG_UL + SRAM_TEST_MSG_BUFF_SIZE)
/* run length */
#define L2SRAM_MSG_Q_END_UL	MAX_SRAM_ADDR_UL

#define L2SRAM_MSG_BUFF_COUNT_UL ((L2SRAM_MSG_Q_END_UL - L2SRAM_MSG_Q_BEG_UL)/MAX_MSG_SIZE_UL)

#define L2SRAM_Q_ON_ID			1
#define L2SRAM_Q_OFF_ID			2




/*--------------------  ROUTINE DEFINITIONS  --------------------------------*/

void vL2SRAM_init(
 	void
 	);

ulong ulL2SRAM_getGenericTblEntry(
		usl uslTblBaseAddr,		//Tbl base addr
		usl uslTblBaseMax,		//Tbl base max (LUL)
		uchar ucTblIdx,			//Tbl index
		uchar ucTblIdxMax,		//Tbl index Max (LUL)
		uchar ucTblByteWidth	//Tbl byte width
		);

void vL2SRAM_putGenericTblEntry(
		USL uslTblBaseAddr,		//Tbl base addr
		usl uslTblBaseMax,		//Tbl base max (LUL)
		uchar ucTblIdx,			//Tbl index
		uchar ucTblIdxMax,		//Tbl index Max (LUL)
		uchar ucTblByteWidth,	//Tbl byte width
		ulong ulEntryVal		//Tbl entry value
		);

ulong ulL2SRAM_getStblEntry(
		uchar ucTblNum,			//Tbl Number
		uchar ucTblIdx			//Tbl index
		);

void vL2SRAM_putStblEntry(
		uchar ucTblNum,			//Tbl Number
		uchar ucTblIdx,			//Tbl index
		ulong ulEntryVal		//Tbl entry value
		);

uint uiL2SRAM_getMsgCount(
 	void
 	);

uint uiL2SRAM_getVacantMsgCount(
 	void
 	);

void vL2SRAM_storeMsgToSram(
 	void
 	);

void vL2SRAM_storeMsgToSramIfAllowed(
		void
		);

uchar ucL2SRAM_getCopyOfCurMsg(
 	void
 	);

void vL2SRAM_delCurMsg(
 	void
 	);

void vL2SRAM_showTblName( //locate an SRAM addr in an SRAM table & show it
		ulong ulAddr
		);

#if 0
void vL2SRAM_dumpTblBases(
		void
		);
#endif

#endif /* L2SRAM_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */

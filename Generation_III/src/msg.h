
/***************************  MSG.H  ****************************************
*
* Header for MSG routine pkg
*
*
* V1.00 01/28/2005 wzr
*	started
*
******************************************************************************/

#ifndef MSG_H_INCLUDED
	#define MSG_H_INCLUDED

	#define YES_RECEIVER_START 1
	#define  NO_RECEIVER_START 0

	#define USE_CLK2 1
	#define USE_CLK1 0

	#define SHOW_MSG_RAW		1
	#define SHOW_MSG_COOKED		2



	#define MAX_RESERVED_MSG_SIZE 70

	#define MAX_MSG_SIZE 32							//radio msg pkt max size
//	#define MAX_MSG_SIZE 64							//radio msg pkt max size
	#define MAX_MSG_SIZE_L ((long) MAX_MSG_SIZE)
	#define MAX_MSG_SIZE_UL ((ulong) MAX_MSG_SIZE)

	#define MAX_MSG_SIZE_MASK  (MAX_MSG_SIZE -1)
	#define MAX_MSG_SIZE_MASK_L ((long)MAX_MSG_SIZE_MASK)


/* CHECK_BYTE_BIT DEFINITIONS */
	#define CHKBIT_CRC			0x80	//10000000	
	#define CHKBIT_MSG_TYPE		0x40	//01000000  
	#define CHKBIT_GRP_SEL		0x20	//00100000	
	#define CHKBIT_GID			0x10	//00010000	
	#define CHKBIT_DEST_SN		0x08	//00001000	
	#define CHKBIT_SRC_SN		0x04	//00000100	
	#define CHKBIT_UNUSED_1		0x02	//00000010	
	#define CHKBIT_UNUSED_2		0x01	//00000001	


/* SLOT AND FRAME DEFINITIONS */
	#define SLOTS_PER_FRAME_I  64
	#define SLOTS_PER_FRAME_L  ((long) SLOTS_PER_FRAME_I)

	#define SECS_PER_SLOT_I 4
	#define SECS_PER_SLOT_L ((long) SECS_PER_SLOT_I)

	#define SECS_PER_FRAME_L (SLOTS_PER_FRAME_L * SECS_PER_SLOT_L)




	/* GENERAL RADIO DEFINES */

	#define MSG_ST_DELAY_IN_nS_UL ((ulong) (300 * 1000000)) //300ms
	#define MSG_ST_DELAY_IN_TICS_UL (MSG_ST_DELAY_IN_nS_UL / CLK_nS_PER_LTIC_UL)
	#define MSG_ST_DELAY_IN_TICS_UI ((uint) MSG_ST_DELAY_IN_TICS_UL)

	#define MSG_ST_DELAY_IN_CLKS_UI (MSG_ST_DELAY_IN_TICS_UI | 0x8000)

	#define RADIO_BIT_IN_nS_UL (800UL * 1000UL)
	#define RADIO_BIT_IN_TICS_X_10000_UL ((ulong) 262144)

	#define RADIO_WARMUP_IN_nS_UL   (20UL * 1000000UL)			//20ms
	#define RADIO_WARMUP_IN_100us_UL (RADIO_WARMUP_IN_nS_UL/100000)
	#define RADIO_WARMUP_IN_100us_UI ((uint) RADIO_WARMUP_IN_100us_UL)
	#define RADIO_WARMUP_IN_TICS_UL (RADIO_WARMUP_IN_nS_UL / CLK_nS_PER_LTIC_UL)
	#define RADIO_WARMUP_IN_TICS_UI ((uint) RADIO_WARMUP_IN_TICS_UL)

	#define RECEIVER_DIGEST_IN_nS_L  ((2L * 1000000L) - 64000L) //2ms - 64us
	#define RECEIVER_DIGEST_IN_TICS_UL (RECEIVER_DIGEST_IN_nS_L / CLK_nS_PER_LTIC_UL)
	#define RECEIVER_DIGEST_IN_TICS_UI ((uint) RECEIVER_DIGEST_IN_TICS_UL)

	#define IDLE_IN_nS_UL (2UL * 1000000UL)	//2ms

	#define SUB_MINUS_PKT_IN_nS_UL (RADIO_WARMUP_IN_nS_UL + RECEIVER_DIGEST_IN_nS_UL + IDLE_IN_nS_UL)

	#define RADIO_LEADER_SIZE_IN_BYTES_UL 6UL //leader = preamble + barker
	#define RADIO_LEADER_SIZE_IN_BITS_UL (RADIO_LEADER_SIZE_IN_BYTES_UL * 8UL)
	#define RADIO_LEADER_IN_nS_UL (RADIO_LEADER_SIZE_IN_BITS_UL * RADIO_BIT_IN_nS_UL)

	#define MAX_VYING_XMITTERS_UL  8UL

	#define REPEATED_XMISSIONS_UC  8
	#define REPEATED_XMISSIONS_UI  ((uint)  REPEATED_XMISSIONS_UC)
	#define REPEATED_XMISSIONS_UL  ((ulong) REPEATED_XMISSIONS_UI)


	#define MEAN_FREE_CAPACITY_UL  (8UL/4UL)

	#define MSG_XFLDSIZE 6



	/* --------------------  MESSAGE DEFINES ------------------------------- */


	/********  MESSAGE TYPES  ***********/

	#define MSG_TYPE_STARTUP		1

	#define MSG_TYPE_DC1			2
	#define MSG_TYPE_DC2			3
	#define MSG_TYPE_DC3		    4

	#define MSG_TYPE_OM1			5
	#define MSG_TYPE_OM2			6
	#define MSG_TYPE_OM3			7

	#define MSG_TYPE_DC4			8
	#define MSG_TYPE_DC5			9
	#define MSG_TYPE_DC6		   10

	#define MSG_TYPE_TS1		   11  //TEST 1
	#define MSG_TYPE_TS2		   12  //TEST 2

	#define MSG_TYPE_MAX_COUNT	   13

	#define MAX_MSG_TYPE 16						//limit on msg type count
	#define MAX_MSG_TYPE_MASK 		0x0F			//00001111		//mask






	/* GENERIC MESSAGE HEADER */

	#define GMH_IDX_EOM_IDX			0

	#define GMH_IDX_MSG_TYPE		1
		#define GMH_MSG_TYPE_MASK	MAX_MSG_TYPE_MASK

	#define GMH_IDX_GID_HI		2
	#define GMH_IDX_GID_LO		3

	#define GMH_IDX_SRC_SN_HI	4
	#define GMH_IDX_SRC_SN_LO	5

	#define GMH_IDX_DEST_SN_HI	6
	#define GMH_IDX_DEST_SN_LO	7





	/* STARTUP MESSAGE TYPE 1 (STARTUP MSG) DEFINES */

	#define ST1_IDX_EOM_IDX		0
	#define ST1_IDX_MSG_TYPE	1

	#define ST1_IDX_GID_HI		2
	#define ST1_IDX_GID_LO		3

	#define ST1_IDX_SRC_SN_HI	4
	#define ST1_IDX_SRC_SN_LO	5

	#define ST1_IDX_DEST_SN_HI	6
	#define ST1_IDX_DEST_SN_LO	7

	#define ST1_IDX_CRC_HI		8
	#define ST1_IDX_CRC_LO		9

	#define ST1_MSG_LAST_BYTE_NUM_UC   ST1_IDX_CRC_LO
	#define ST1_MSG_LAST_BYTE_NUM_UL ((ulong) ST1_MSG_LAST_BYTE_NUM_UC)
	#define ST1_MSG_SIZE_IN_BYTES_UL (ST1_MSG_LAST_BYTE_IDX_UL + 1UL)
	#define ST1_MSG_SIZE_IN_BITS_UL (ST1_MSG_SIZE_IN_BYTES_UL * 8UL)
	#define ST1_MSG_IN_nS_UL (ST1_MSG_SIZE_IN_BITS_UL * RADIO_BIT_IN_nS_UL)




	
	/* DISCOVERY 4 MSG INDEXES */

	#define DC4_IDX_EOM_IDX	0
	#define DC4_IDX_MSG_TYPE			1

	#define DC4_IDX_GID_HI				2
	#define DC4_IDX_GID_LO				3

	#define DC4_IDX_SRC_SN_HI			4
	#define DC4_IDX_SRC_SN_LO			5

	#define DC4_IDX_DEST_SN_HI			6
	#define DC4_IDX_DEST_SN_LO			7

	#define DC4_IDX_SYNC_TIME_XI		8
	#define DC4_IDX_SYNC_TIME_HI		9
	#define DC4_IDX_SYNC_TIME_MD		10
	#define DC4_IDX_SYNC_TIME_LO		11
	#define DC4_IDX_SYNC_TIME_SUB_HI	12
	#define DC4_IDX_SYNC_TIME_SUB_LO	13

	#define DC4_IDX_SRC_LEVEL			14

	#define DC4_IDX_HR0_TO_SYSTIM0_IN_SEC_B16	15
	#define DC4_IDX_HR0_TO_SYSTIM0_IN_SEC_HI	15
	#define DC4_IDX_HR0_TO_SYSTIM0_IN_SEC_LO	16

	#define DC4_IDX_CRC_HI				17
	#define DC4_IDX_CRC_LO				18



	#define DC4_MSG_LAST_BYTE_NUM_UC   DC4_IDX_CRC_LO
	#define DC4_MSG_LAST_BYTE_NUM_UL ((ulong) DC4_MSG_LAST_BYTE_NUM_UC)

	#define DC4_MSG_SIZE_IN_BYTES_UL (DC4_MSG_LAST_BYTE_NUM_UL + 1UL)
	#define DC4_MSG_SIZE_IN_BITS_UL (DC4_MSG_SIZE_IN_BYTES_UL * 8UL)

	#define DC4_MSG_IN_nS_UL (DC4_MSG_SIZE_IN_BITS_UL * RADIO_BIT_IN_nS_UL)

	#define DC4_PKT_SIZE_IN_BITS_UL (RADIO_LEADER_SIZE_IN_BITS_UL + \
									DC4_MSG_SIZE_IN_BITS_UL)

	#define DC4_PKT_IN_TICS_UL ((DC4_PKT_SIZE_IN_BITS_UL * \
								RADIO_BIT_IN_TICS_X_10000_UL) / 10000UL)
	#define DC4_PKT_IN_TICS_UI ((uint) DC4_PKT_IN_TICS_UL)

	#define DC4_SYNC_IN_TICS_UI  (MSG_ST_DELAY_IN_TICS_UI + \
		RADIO_WARMUP_IN_TICS_UI + DC4_PKT_IN_TICS_UI + RECEIVER_DIGEST_IN_TICS_UI)
			

	#define DC4_SYNC_IN_CLKS_UI      (DC4_SYNC_IN_TICS_UI | 0x8000)








	/* DISCOVERY 5 MSG INDEXES */

	#define DC5_IDX_EOM_IDX		0
	#define DC5_IDX_MSG_TYPE				1

	#define DC5_IDX_GID_HI					2
	#define DC5_IDX_GID_LO					3

	#define DC5_IDX_SRC_SN_HI				4
	#define DC5_IDX_SRC_SN_LO				5

	#define DC5_IDX_DEST_SN_HI				6
	#define DC5_IDX_DEST_SN_LO				7

	#define DC5_IDX_SEED_HI					8
	#define DC5_IDX_SEED_MD					9
	#define DC5_IDX_SEED_LO	   			   10

	#define DC5_IDX_CRC_HI				   12
	#define DC5_IDX_CRC_LO				   13

	#define DC5_MSG_LAST_BYTE_NUM_UC   DC5_IDX_CRC_LO

	#define DC5_MSG_LAST_BYTE_IDX_UL ((ulong) DC5_MSG_LAST_BYTE_NUMBER_UC)
	#define DC5_MSG_SIZE_IN_BYTES_UL (DC5_MSG_LAST_BYTE_IDX_UL + 1UL)
	#define DC5_MSG_SIZE_IN_BITS_UL (DC5_MSG_SIZE_IN_BYTES_UL * 8UL)
	#define DC5_MSG_IN_nS_UL (DC5_MSG_SIZE_IN_BITS_UL * RADIO_BIT_IN_nS_UL)

	#define DC5_PKT_SIZE_IN_BITS_UL ((RADIO_LEADER_SIZE_IN_BITS_UL + DC5_MSG_SIZE_IN_BITS_UL)
	#define DC5_PKT_IN_TICS_UL ((DC5_PKT_SIZE_IN_BITS_UL * RADIO_BIT_IN_TICS_X_10000_UL) / 10000UL)
	#define DC5_PKT_IN_TICS_UI ((uint) DC5_PKT_IN_TICS_UL)


	/* DISCOVERY  5  CALCULATIONS */

	#define DC5_PKT_IN_nS_UL (RADIO_LEADER_IN_nS_UL + DC5_MSG_IN_nS_UL)
	#define DC5_SUB_IN_nS_UL (DC5_PKT_IN_nS_UL + SUB_MINUS_PKT_IN_nS_UL)

	#define DC5_MAX_SUBS_PER_SEC_UL (1000000000UL / DC5_SUB_IN_nS_UL)

	#define DC5_SUBS_PER_SEC_UC 7
	#define DC5_SUBS_PER_SEC_UI ((uint)  DC5_SUBS_PER_SEC_UC)
	#define DC5_SUBS_PER_SEC_UL ((ulong) DC5_SUBS_PER_SEC_UC)

	#define DC5_TOT_SUB_COUNT_UL (MAX_VYING_XMITTERS_UL * REPEATED_XMISSIONS_UL * MEAN_FREE_CAPACITY_UL)
	#define DC5_TOT_SUB_COUNT_UI ((uint) DC5_TOT_SUB_COUNT_UL)
	#define DC5_TOT_SUB_COUNT_UC ((uchar) DC5_TOT_SUB_COUNT_UI)

	#define DC5_SLOT_IN_SEC_UL ((DC5_TOT_SUB_COUNT_UL / DC5_SUBS_PER_SEC_UL) + 1UL)
	#define DC5_SLOT_IN_SEC_L  ((long) DC5_SLOT_IN_SEC_UL)
	#define DC5_SLOT_IN_SEC_UI ((uint)  DC5_SLOT_IN_SEC_UL)
	#define DC5_SLOT_IN_SEC_UC ((uchar) DC5_SLOT_IN_SEC_UI)

	#define DC5_SUB_IN_TICS_UI ((uint)(0x8000/DC5_SUBS_PER_SEC_UI))

	#define DC5_SUBSLOT_COUNT			16
	#define DC5_SUBSLOT_COUNT_MASK		(DC5_SUBSLOT_COUNT-1)

	#define DC5_SUBSLOT_WIDTH_IN_ns_UL		(200UL * 1000000UL) //200ms
	#define DC5_SUBSLOT_ST_OFFSET_IN_ns_UL	(600UL * 1000000UL) //600ms




	/* OPERATIONAL MODE DEFINES */

	/* OPERATIONAL MODE 1 INDEXES */

	#define OM1_IDX_EOM_IDX		0
		#define OM1_MSG_OUTBOUND_MSG_PENDING_BIT 0x80

	#define OM1_IDX_MSG_TYPE	1

	#define OM1_IDX_GID_HI		2
	#define OM1_IDX_GID_LO		3

	#define OM1_IDX_SRC_SN_HI	4
	#define OM1_IDX_SRC_SN_LO	5

	#define OM1_IDX_DEST_SN_HI	6
	#define OM1_IDX_DEST_SN_LO	7

	#define OM1_IDX_SYNC_TIME_XI		8
	#define OM1_IDX_SYNC_TIME_HI		9
	#define OM1_IDX_SYNC_TIME_MD	   10
	#define OM1_IDX_SYNC_TIME_LO	   11
	#define OM1_IDX_SYNC_TIME_SUB_HI   12
	#define OM1_IDX_SYNC_TIME_SUB_LO   13

	#define OM1_IDX_DELTA_SEC_TO_SLOT_END	14

	#define OM1_IDX_AVAIL_MSG_SPACE		15

	#define OM1_IDX_CRC_HI				16
	#define OM1_IDX_CRC_LO				17


	#define OM1_MSG_LAST_BYTE_NUM_UC   OM1_IDX_CRC_LO
	#define OM1_MSG_LAST_BYTE_NUM_UL ((ulong) OM1_MSG_LAST_BYTE_NUM_UC)

	#define OM1_MSG_SIZE_IN_BYTES_UL (OM1_MSG_LAST_BYTE_NUM_UL + 1UL)

	#define OM1_MSG_SIZE_IN_BITS_UL (OM1_MSG_SIZE_IN_BYTES_UL * 8UL)

	#define OM1_MSG_IN_nS_UL (OM1_MSG_SIZE_IN_BITS_UL * RADIO_BIT_IN_nS_UL)

	#define OM1_PKT_SIZE_IN_BITS_UL (RADIO_LEADER_SIZE_IN_BITS_UL + OM1_MSG_SIZE_IN_BITS_UL)

	#define OM1_PKT_IN_TICS_UL ((OM1_PKT_SIZE_IN_BITS_UL * RADIO_BIT_IN_TICS_X_10000_UL) / 10000UL)
	#define OM1_PKT_IN_TICS_UI ((uint) OM1_PKT_IN_TICS_UL)

	#define OM1_SYNC_IN_TICS_UI  (MSG_ST_DELAY_IN_TICS_UI + \
		RADIO_WARMUP_IN_TICS_UI + OM1_PKT_IN_TICS_UI + RECEIVER_DIGEST_IN_TICS_UI)

	#define OM1_SYNC_IN_CLKS_UI      (OM1_SYNC_IN_TICS_UI | 0x8000)






	/* OM2 INDEXES */

	#define OM2_IDX_EOM_IDX		0
		#define OM2_LAST_PKT_BIT	0x80
		#define OM2_NODATA_BIT	0x40

	#define OM2_IDX_MSG_TYPE	1
		#define OM2_MSG_TYPE_MASK	MAX_MSG_TYPE_MASK

	#define OM2_IDX_GID_HI		2
	#define OM2_IDX_GID_LO		3

	#define OM2_IDX_SRC_SN_HI	4
	#define OM2_IDX_SRC_SN_LO	5

	#define OM2_IDX_DEST_SN_HI	6
	#define OM2_IDX_DEST_SN_LO	7

	#define OM2_IDX_MSG_SEQ_NUM			 8

//	#define OM2_IDX_LFACTOR				 9	//Dcnt Radio Link Algorithm
//	#define OM2_IDX_LNKREQ				 9	//LnkBlk Radio Link Algorithm
	#define OM2_IDX_GENERIC_LINK_BYTE	 9  //A generic name here


	#define OM2_IDX_AGENT_NUM_HI		10
	#define OM2_IDX_AGENT_NUM_LO		11

	#define OM2_IDX_COLLECTION_TIME_XI	12
	#define OM2_IDX_COLLECTION_TIME_HI	13
	#define OM2_IDX_COLLECTION_TIME_MD	14
	#define OM2_IDX_COLLECTION_TIME_LO	15

	#define OM2_IDX_DATA_0_SENSOR_NUM	16
	#define OM2_IDX_DATA_0_HI			17
	#define OM2_IDX_DATA_0_LO			18

	#define OM2_IDX_DATA_1_SENSOR_NUM	19
	#define OM2_IDX_DATA_1_HI			20
	#define OM2_IDX_DATA_1_LO			21

	#define OM2_IDX_DATA_2_SENSOR_NUM	22
	#define OM2_IDX_DATA_2_HI			23
	#define OM2_IDX_DATA_2_LO			24

	#define OM2_IDX_DATA_3_SENSOR_NUM	25
	#define OM2_IDX_DATA_3_HI			26
	#define OM2_IDX_DATA_3_LO			27

	/*-----------------------------------------*/
	#define OM2_IDX_DATA_4_SENSOR_NUM	28	//short data area only 1 byte
	#define OM2_IDX_DATA_4_LO			29  //used to pass msg err info

	#define OM2_IDX_CRC_HI				30
	#define OM2_IDX_CRC_LO				31

	#define OM2_MAX_DATA_ENTRY_COUNT	4
	#define OM2_MAX_DBL_DATA_ENTRY_COUNT (OM2_MAX_DATA_ENTRY_COUNT / 2)


	#define OM2_MSG_LAST_BYTE_NUM_UC   OM2_IDX_CRC_LO
	#define OM2_MSG_LAST_BYTE_NUM_UL ((ulong) OM2_MSG_LAST_BYTE_NUM_UC)

	#define OM2_MSG_SIZE_IN_BYTES_UL (OM2_MSG_LAST_BYTE_IDX_UL + 1UL)
	#define OM2_MSG_SIZE_IN_BITS_UL (OM2_MSG_SIZE_IN_BYTES_UL * 8UL)

	#define OM2_MSG_IN_nS_UL (OM2_MSG_SIZE_IN_BITS_UL * RADIO_BIT_IN_nS_UL)





	/* OPERATIONAL MODE 3 (ACK) DEFINES */

	#define OM3_IDX_EOM_IDX		0
		#define OM3_MSG_LAST_PKT_BAD_BIT	0x80
		#define OM3_MSG_EMPTY_PKT_BIT		0x40

	#define OM3_IDX_MSG_TYPE	1
		#define OM3_MSG_TYPE_MASK	MAX_MSG_TYPE_MASK


	#define OM3_IDX_GID_HI		2
	#define OM3_IDX_GID_LO		3

	#define OM3_IDX_SRC_SN_HI	4
	#define OM3_IDX_SRC_SN_LO	5

	#define OM3_IDX_DEST_SN_HI	6
	#define OM3_IDX_DEST_SN_LO	7

	#define OM3_IDX_ACKED_SEQ_NUM	8

//	#define OM3_IDX_LINKUP_DCNT			9 //Dcnt Radio Link Algorithm
//	#define OM3_IDX_LNK_CONFIRM			9 //LnkBlk Radio Link Algorithm
	#define OM3_IDX_GENERIC_LINK_RETURN 9 //Generic Link Return Definition

	#define OM3_IDX_CRC_HI			10
	#define OM3_IDX_CRC_LO			11


	#define OM3_MSG_LAST_BYTE_NUM_UC   OM3_IDX_CRC_LO
	#define OM3_MSG_LAST_BYTE_NUM_UL ((ulong) OM3_MSG_LAST_BYTE_NUM_UC)

	#define OM3_MSG_SIZE_IN_BYTES_UL (OM3_MSG_LAST_BYTE_IDX_UL + 1UL)
	#define OM3_MSG_SIZE_IN_BITS_UL (OM3_MSG_SIZE_IN_BYTES_UL * 8UL)
	#define OM3_MSG_IN_nS_UL (OM3_MSG_SIZE_IN_BITS_UL * RADIO_BIT_IN_nS_UL)








	/* TS1 INDEXES */

	#define TS1_IDX_EOM_IDX		0

	#define TS1_IDX_MSG_TYPE	1
		#define TS1_MSG_TYPE_MASK	MAX_MSG_TYPE_MASK

	#define TS1_IDX_GID_HI		2
	#define TS1_IDX_GID_LO		3

	#define TS1_IDX_SRC_SN_HI	4
	#define TS1_IDX_SRC_SN_LO	5

	#define TS1_IDX_DEST_SN_HI	6
	#define TS1_IDX_DEST_SN_LO	7

	#define TS1_IDX_MSG_SEQ_NUM			 8

	/* rest of msg is test data to CRC */

	#define TS1_IDX_CRC_HI				30
	#define TS1_IDX_CRC_LO				31

	#define TS1_MSG_LAST_BYTE_NUM_UC   TS1_IDX_CRC_LO




	/* ROUTINE DEFINITIONS */
uchar ucMSG_chkMsgIntegrity(
		uchar ucCheckByteBits,
		uchar ucReportByteBits,
		uchar ucMsgType,
		uint uiExpectedSrcSN,
		uint uiExpectedDestSN
		);

void vMSG_showSNmismatch(
		uint uiExpectedVal,
		uint uiGotVal,
		uchar ucCRLF_flag			//YES_CRLF, NO_CRLF
		);

void vMSG_showStorageErr(
		const char *cpLeadinMsg,
		ulong ulFailAddr,
		ulong ulWroteVal,
		ulong ulReadVal
		);

void vMSG_buildMsgHdr_GENERIC(  //HDR=Len,Type,Group,Src,Dest
		uchar ucMsgLen,
		uchar ucMsgType,
		uint uiDestSN
		);

uchar ucMSG_waitForMsgOrTimeout(
		void
		);

void vMSG_rec_obnd_msgs(
		void
		);

void vMSG_showMsgBuffer(
		uchar ucCRLF_termFlag,	//YES_CRLF, NO_CRLF
		uchar ucShowTypeFlag  	//SHOW_MSG_RAW, SHOW_MSG_COOKED
		);

uchar ucMSG_doSubSecXmit(		//RET: 1-sent, 0-too late
		uchar ucaSendTime[6],	//full start time
		uchar ucClkChoiceFlag,	//USE_CLK2, USE_CLK1
		uchar ucStartRecFlag	//YES_RECEIVER_START, NO_RECEIVER_START
		);

void vMSG_stuffFakeMsgToSRAM(
		void
		);

void vMSG_checkFakeMsg(
		void
		);

uchar ucMSG_incMsgSeqNum( //RET: Incremented Msg Seq Num (not 0 or 255)
		void
		);

uchar ucMSG_getLastFilledEntryInOM2( //RET: 0=none, ##=idx of last entry in OM2
		void
		);


#endif /* MSG_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */

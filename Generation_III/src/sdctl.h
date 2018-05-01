
/***************************  SDCTL.H  ****************************************
*
* Header for SDCTL routine pkg
*
*
* Will likely need change! to make room for new mini SD's
*
*
* V1.00 10/27/2004 wzr
*	started
*
******************************************************************************/

#ifndef SDCTL_H_INCLUDED
	#define SDCTL_H_INCLUDED

	/* SD RESET VALUES */
	#define SD_RESET_MODE				1
	#define SD_RUN_MODE					0

	/* SD POWER VALUES */
	#define SD_POWER_ON					1
	#define SD_POWER_OFF				0

	/* SD BOOT LOADER ENABLE VALUES */
	#define SD_BOOT_LOADER_ENABLED		1
	#define SD_BOOT_LOADER_DISABLED		0

	/* BREAK SLEEP VALUES */
	#define SD_YES_BREAKOUT_OF_SLEEP	1
	#define SD_NO_BREAKOUT_OF_SLEEP 	0

	/* COMMAND MESSAGE TYPES */
	#define CMD_TYPE_NONE					0
	#define CMD_TYPE_REPLY_TO_TEST_MSG		1
	#define CMD_TYPE_SEND_FAKE_SENSOR_DATA	2
	#define CMD_TYPE_READ_SENSOR			3


	/* THE COMMAND MESSAGE INDEX LOCATIONS */
	#define CMD_MSG_SIZE_IDX			0
	#define CMD_MSG_CMD_IDX				1
	#define CMD_MSG_SENSOR_NUM_IDX		2
	#define CMD_MSG_DATA_1_IDX_HI		3
	#define CMD_MSG_DATA_1_IDX_LO		4
	#define CMD_MSG_DATA_2_IDX_HI		5
	#define CMD_MSG_DATA_2_IDX_LO		6


	/*----------------  ROUTINE DEFINITIONS  --------------------------------*/

	void vSDCTL_set_SD_power_bit(
		unsigned ucPowerFlag		//SD_POWER_ON  or SD_POWER_OFF
		);

	unsigned char ucSDCTL_test_SD_power_bit(
		void
		);

	void vSDCTL_set_SD_reset_bit(
		unsigned char ucResetFlag	//SD_RESET_MODE or SD_RUN_MODE
		);

	unsigned char ucSDCTL_test_SO_reset_bit(
		void
		);

	unsigned char ucSDCTL_test_SD_running_bits(
		void
		);

	unsigned char ucSDCTL_detect_SD_power_up_pulse(
		void
		);

	void vSDCTL_set_SD_boot_load_bit(
		unsigned ucBootLoaderFlag	//SD_BOOT_LOADER_ENABLED  or SD_BOOT_LOADER_DISABLED
		);

	unsigned char ucSDCTL_wait_for_SD_boot_done_flag(
		unsigned long ulWaitCount_ms	//timeout limit (in ms)
		);

	uchar ucSDCTL_start_SD_and_specify_boot( //1=SD up,  0=SD not up
		uchar ucBootLoaderFlag	//SD_BOOT_LOADER_ENABLED  or SD_BOOT_LOADER_DISABLED
		);

	void vSDCTL_halt_SD_and_power_down(
		void
		);

	void vSDCTL_break_SD_out_of_sleep(
		void
		);


#endif /* SDCTL_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */






/**************************  SENSEACT.C  *****************************************
*
* Routines to show SENSEACTs during events
*
*
* V1.00 10/04/2003 wzr
*		Started
*
******************************************************************************/

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e752 */		/* local declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */



#include "STD.h"			//standard defines
//#include "DIAG.h"			//diagnostic defines
//#include <msp430x54x.h>		//processor reg description */
#include "CONFIG.h"  		//system configuration description file
//#include "MAIN.H"			//
//#include "MISC.H"			//homeless functions
//#include "READING.h"		//sensor reading module
//#include "AD.h"			//AD module
//#include "CRC.h"			//CRC calculation module
#include "SERIAL.h"  		//serial IO port stuff
//#include "SRAM.H"			//sram routines
//#include "L2SRAM.h"		//level 2 sram
//#include "ACTION.h" 		//event action module
//#include "SDCTL.h"		//SD board control routines
//#include "SDCOM.h"		//SD communication package
//#include "L2FLASH.h"		//level 2 flash routines
//#include "TIME.h"			//Time routines
//#include "MSG.h"			//msg handling routines
//#include "RTS.h"			//Real Time Scheduler
//#include "EVENT.h"		//Event handler routines
//#include "DISCOVER.h"		//discovery routines
//#include "OPMODE.h"		//operational mode routines
//#include "SDSPI.h"		//SD board SPI routines
//#include "REPORT.h"		//Periodic reporting functions
#include "SENSOR.h"			//Sensor name routines



#define FIRST_NAME_BLK_SIZE		33
#define SECOND_NAME_BLK_SIZE 	32
#define THIRD_NAME_BLK_SIZE		12




/********************  DataSize Table  **************************************
*
*
*    7        6        5        4        3         2       1        0
*ÚÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄ¿
*³  User  ³ Is an  ³        ³        ³        ³        ³   Data Size     ³
*³  can   ³  SD    ³        ³        ³        ³        ³<--------------->³
*³ select ³function³        ³        ³        ³        ³        ³        ³
*ÀÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÙ
*
* User-can_select		is used by the correction function to decide if the
*						SenseAct asked for is ok to be asked for.  If not
*						it is simply deleted from the selection by the
*						correction process.
*
* Is_an_SD_function		is used to determine if the SD should be contacted
*						for the data.
*
* Data_size				This is a value of:
*						 2 - requires 2 data spaces in the OM2
*						 1 - requires 1 data space  in the OM2
*						 0 - requires 1 data space  in the OM2 but cannot be
*							 used alone. (usually the pairing value for a 
*							 Data_size of 2).
*						 
*
*****************************************************************************/

#define DS_USER_YES_SELECT  0x80		// 1000 0000
#define DS_USER_NOT_SELECT	0x00		// 0000 0000
#define DS_USER_SELECT_MASK 0x80		// 1000 0000

#define DS_YES_SD_FUNC		0x40		// 0100 0000
#define DS_NOT_SD_FUNC		0x00		// 0000 0000
#define DS_SD_FUNC_MASK		0x40		// 0100 0000

#define DS_DATA_SIZE_MASK	0x03		// 0000 0011

const uchar ucaSensorDataSize1[FIRST_NAME_BLK_SIZE] =
{
DS_USER_YES_SELECT | DS_NOT_SD_FUNC | 0, //	 0 "  NONE" blank
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 1, //	 1 "  FAK1" FAKE 1
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 1, //	 2 "  FAK2" FAKE 2
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 1, //	 3 "SDcodV" SD code version
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 1, //	 4 "SDmsgV" SD msg version
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 1, //	 5 "   TTY" TTY
DS_USER_NOT_SELECT | DS_NOT_SD_FUNC | 1, //	 6 "ErrMsg" ErrMsg
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	 7 "   TC1" TC1
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	 8 "   TC2" TC2
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	 9 "   TC3" TC3
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	10 "   TC4" TC4
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 1, //	11 "   LT1" LIGHT 1 +Reading (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 1, //	12 "   LT2" LIGHT 2 +Reading (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 1, //	13 "   LT3" LIGHT 3 +Reading (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 1, //	14 "   LT4" LIGHT 4 +Reading (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 1, //	15 "SlWet1" SOIL MOISTURE 1 +Reading (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 1, //	16 "SlWet2" SOIL MOISTURE 2 +Reading (uint)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 1, //	17 "   GP1" +Reading (uint)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 1, //	18 "   GP2" +Reading (uint)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 1, //	19 "Owire0"
DS_USER_NOT_SELECT | DS_NOT_SD_FUNC | 1, //	20 "Batt-V" +Reading (uint)
DS_USER_NOT_SELECT | DS_NOT_SD_FUNC | 1, //	21 "SD-Msg" +msg number
DS_USER_NOT_SELECT | DS_NOT_SD_FUNC | 1, //	22 "BR-Msg" +msg number
DS_USER_NOT_SELECT | DS_NOT_SD_FUNC | 1, //	23 "Sm2Lnk" +SN of link
DS_USER_NOT_SELECT | DS_NOT_SD_FUNC | 1, //	24 "Rm2Lnk" +SN of link
DS_USER_NOT_SELECT | DS_NOT_SD_FUNC | 2, //	25 "TMcgHI" +New Time HI uint
DS_USER_NOT_SELECT | DS_NOT_SD_FUNC | 0, //	26 "TMcgLO" +New Time LO uint
DS_USER_NOT_SELECT | DS_NOT_SD_FUNC | 1, //	27 "Sm2Lst" +SN of lost link
DS_USER_NOT_SELECT | DS_NOT_SD_FUNC | 1, //	28 "Rm2Lst" +SN of lost link
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 1, //	29 "KunTc1" +Reading (uint)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 1, //	30 "KunTc2" +Reading (uint)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 1, //	31 "KunTc3" +Reading (uint)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 1	 //	32 "KunTc4" +Reading (uint)
};


const uchar ucaSensorDataSize2[SECOND_NAME_BLK_SIZE] =
{
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	33 "WSminH" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0, //	34 "WSMinL" +decimal number part (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	35 "WSaveH" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0, //	36 "WSaveL" +decimal number part (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	37 "WSmaxH" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0, //	38 "WSmaxL" +decimal number part (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	39 "WDminH" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0, //	40 "WDminL" +decimal number part (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	41 "WDaveH" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0, //	42 "WDaveL" +decimal number part (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	43 "WDmaxH" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0, //	44 "WDmaxL" +decimal number part (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	45 "AirP_H" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0, //	46 "AirP_L" +decimal number part (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	47 "AirT_H" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0, //	48 "AirT_L" +decimal number part (uint)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 2, //	49 "ITmpHI" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0, //	50 "ITmpLO" +decimal number part (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	51 "RlHumH" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0, //	52 "RlHumL" +decimal number part (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	53 "RnAccH" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0, //	54 "RnAccL" +decimal number part (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	55 "RnDurH" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0, //	56 "RnDurL" +decimal number part (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, // 57 "RnIntH" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0, // 58 "RnIntL" +decimal number part (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	59 "HlAccH" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0, //	60 "HlAccL" +decimal number part (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, //	61 "HlDurH" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0, //	62 "HlDurL" +decimal number part (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 2, // 63 "HlIntH" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0	 // 64 "HlIntL" +decimal number part (uint)
};				  


const uchar ucaSensorDataSize3[THIRD_NAME_BLK_SIZE] =
{
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 2,// 65 "HtTmpH" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0,// 66 "HtTmpL" +decimal number part (uint)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 2,// 67 "HeatVH" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0,// 68 "HeatVL" +decimal number part (uint)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 2,// 69 "SrcVHI" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0,// 70 "SrcVLO" +decimal number part (uint)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 2,// 71 "RefVHI" +HI number part (int)
DS_USER_NOT_SELECT | DS_YES_SD_FUNC | 0,// 72 "RefVLO" +decimal number part (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 1,// 73 "  SAP1" +Reading (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 1,// 74 "  SAP2" +Reading (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 1,// 75 "  SAP3" +Reading (uint)
DS_USER_YES_SELECT | DS_YES_SD_FUNC | 1	// 76 "  SAP4" +Reading (uint)
};


/*------------------  SENSOR SHORT NAMES  -----------------------------------*/

const char *cpaSensorShortName1[FIRST_NAME_BLK_SIZE] =
	{
	"0",		//	 0 //blank
	"FK1",		//	 1 //Fake 1
	"FK2",		//	 2 //Fake 2
	"SDcd",		//	 3 //SD code version
	"SDMV",		//	 4 //SD msg version
	"TTY",		//	 5 //console msg
	"EMsg",		//	 6 //Error Msg
	"T1",		//	 7 //TC 1
	"T2",		//	 8 //TC 2
	"T3",		//	 9 //TC 3
	"T4",		//	10 //TC 4
	"L1",		//	11 //Light 1				+Reading (uint)
	"L2",		//	12 //Light 2				+Reading (uint)
	"L3",		//	13 //Light 3				+Reading (uint)
	"L4",		//	14 //Light 4				+Reading (uint)
	"SM1",		//	15 //Soil Moisture 1		+Reading (uint)
	"SM2",		//	16 //Soil Moisture 2		+Reading (uint)
	"GP1",		//	17 //General purpose 1		+Reading (uint)
	"GP2",		//	18 //General purpose 2		+Reading (uint)
	"OW",		//	19 //One Wire 0
	"BV",		//	20 //Battery Voltage		+Reading (uint)
	"SDM",		//	21 //SD msg					+msg number
	"BRM",		//	22 //BR msg					+msg number
	"SLk",		//	23 //+SN of Send link
	"SInf",		//	24 //+Send link info
	"TcgH",		//	25 //+New Time HI uint
	"TcgL",		//	26 //+New Time LO uint
	"RLk",		//	27 //+SN of Receive link
	"RInf",		//	28 //+Reveive link info
	"KT1",		//	29 //Kun TC1 reading		+Reading (uint)
	"KT2",		//	30 //Kun TC2 reading		+Reading (uint)
	"KT3",		//	31 //Kun TC3 reading		+Reading (uint)
	"KT4"		//	32 //Kun TC4 reading		+Reading (uint)
	};


const char *cpaSensorShortName2[SECOND_NAME_BLK_SIZE] =
	{
	"WSlH",		//	33 //wind speed lo			+HI number part (int)
	"WSlL",		//	34 //wind speed lo			+decimal number part (uint)
	"WSmH",		//	35 //wind speed md			+HI number part (int)
	"WSmL",		//	36 //wind speed md			+decimal number part (uint)
	"WShH",		//	37 //wind speed hi			+HI number part (int)
	"WShL",		//	38 //wind speed hi			+decimal number part (uint)
	"WDlH",		//	39 //wind direction lo		+HI number part (int)
	"WDlL",		//	40 //wind direction lo		+decimal number part (uint)
	"WDmH",		//	41 //wind direction md		+HI number part (int)
	"WDmL",		//	42 //wind direction md		+decimal number part (uint)
	"WDhH",		//	43 //wind direction hi		+HI number part (int)
	"WDhL",		//	44 //wind direction hi		+decimal number part (uint)
	"APH",		//	45 //air pressure			+HI number part (int)
	"APL",		//	46 //air pressure			+decimal number part (uint)
	"ATH",		//	47 //air temperature		+HI number part (int)
	"ATL",		//	48 //air tempreature		+decimal number part (uint)
	"ITH",		//	49 //Internal temperature	+HI number part (int)
	"ITL",		//	50 //Internal temparature	+decimal number part (uint)
	"RHH",		//	51 //relative humidity		+HI number part (int)
	"RHL",		//	52 //relative humidity		+decimal number part (uint)
	"RAH",		//	53 //rain accumulation		+HI number part (int)
	"RAL",		//	54 //rain accumulation		+decimal number part (uint)
	"RDH",		//	55 //rain duration			+HI number part (int)
	"RDL",		//	56 //rain duration			+decimal number part (uint)
	"RIH",		//  57 //rain intensity			+HI number part (int)
	"RIL",		//  58 //rain intensity			+decimal number part (uint)
	"HAH",		//	59 //hail accumulation		+HI number part (int)
	"HAL",		//	60 //hail accumulation		+decimal number part (uint)
	"HDH",		//	61 //hail duration			+HI number part (int)
	"HDL",		//	62 //hail duration			+decimal number part (uint)
	"HIH",		//  63 //hail intensity			+HI number part (int)
	"HIL"		//  64 //hail intensity			+decimal number part (uint)
	};


const char *cpaSensorShortName3[THIRD_NAME_BLK_SIZE] =
	{
	"HtTH",		//  65 //heat temperature		+HI number part (int)
	"HtTL",		//  66 //heat temperature		+decimal number part (uint)
	"HtVH",		//  67 //heat voltage			+HI number part (int)
	"HtVL",		//  68 //heat voltage			+decimal number part (uint)
	"VH",		//  69 //source voltage			+HI number part (int)
	"VL",		//  70 //source voltage			+decimal number part (uint)
	"RVH",		//	71 //reference voltage		+HI number part (int)
	"RVL",		//	72 //reference voltage		+decimal number part (uint)
	"SP1",		//  73 //sap flow 1				+Reading (uint)
	"SP2",		//  74 //sap flow 2				+Reading (uint)
	"SP3",		//  75 //sap flow 3				+Reading (uint)
	"SP4"		//  76 //sap flow 4				+Reading (uint)
	};





/*****************************  CODE STARTS HERE  ****************************/



/****************** ucSENSEACT_getRawSensorDataSizeTblEntry()  ****************************************
*
* RET: Sensor Data Size Table entry (flags + size)
*
******************************************************************************/
uchar ucSENSEACT_getRawSensorDataSizeTblEntry(
		uchar ucSensorNum
		)
	{
	if(ucSensorNum < FIRST_NAME_BLK_SIZE)
		{
		return(ucaSensorDataSize1[ucSensorNum]);
		}

	ucSensorNum -= FIRST_NAME_BLK_SIZE;
	if(ucSensorNum < SECOND_NAME_BLK_SIZE)
		{
		return(ucaSensorDataSize2[ucSensorNum]);
		}

	ucSensorNum -= SECOND_NAME_BLK_SIZE;
	if(ucSensorNum < THIRD_NAME_BLK_SIZE)
		{
		return(ucaSensorDataSize3[ucSensorNum]);
		}

	return(0);

	}/* END: ucSENSEACT_getRawSensorDataSizeTblEntry() */






/****************** ucSENSEACT_getSensorUserFlag()  **************************
*
* RET: Sensor Data Size Table entry (flags + size)
*
******************************************************************************/
uchar ucSENSEACT_getSensorUserFlag(
		uchar ucSensorNum
		)
	{
	uchar ucVal;

	ucVal = ucSENSEACT_getRawSensorDataSizeTblEntry(ucSensorNum);
	ucVal &= DS_USER_SELECT_MASK;

	return(ucVal);

	}/* END: ucSENSEACT_getSensorUserFlag() */




/****************** ucSENSEACT_getSensorSDfuncFlag()  **************************
*
* RET: Sensor Data Size Table entry (flags + size)
*
******************************************************************************/
uchar ucSENSEACT_getSensorSDfuncFlag(
		uchar ucSensorNum
		)
	{
	uchar ucVal;

	ucVal = ucSENSEACT_getRawSensorDataSizeTblEntry(ucSensorNum);
	ucVal &= DS_SD_FUNC_MASK;

	return(ucVal);

	}/* END: ucSENSEACT_getSensorSDfuncFlag() */





/****************** ucSENSEACT_getSensorDataSize()  **************************
*
* RET: Sensor Data Size Table entry (flags + size)
*
******************************************************************************/
uchar ucSENSEACT_getSensorDataSize(
		uchar ucSensorNum
		)
	{
	uchar ucVal;

	ucVal = ucSENSEACT_getRawSensorDataSizeTblEntry(ucSensorNum);
	ucVal &= DS_DATA_SIZE_MASK;

	return(ucVal);

	}/* END: ucSENSEACT_getSensorDataSize() */




/****************** vSENSEACT_showSensorShortName()  *************************
*
*
*
******************************************************************************/
void vSENSEACT_showSensorShortName(
		uchar ucSensorNum
		)
	{
	if(ucSensorNum < FIRST_NAME_BLK_SIZE)
		{
		vSERIAL_rom_sout(cpaSensorShortName1[ucSensorNum]);
		return;
		}

	ucSensorNum -= FIRST_NAME_BLK_SIZE;
	if(ucSensorNum < SECOND_NAME_BLK_SIZE)
		{
		vSERIAL_rom_sout(cpaSensorShortName2[ucSensorNum]);
		return;
		}

	ucSensorNum -= SECOND_NAME_BLK_SIZE;
	if(ucSensorNum < THIRD_NAME_BLK_SIZE)
		{
		vSERIAL_rom_sout(cpaSensorShortName3[ucSensorNum]);
		return;
		}

	return;

	}/* END: vSENSEACT_showSensorShortName() */





/********************  vSENSEACT_showSenseActWordInText()  *********************
*
*
*
******************************************************************************/

void vSENSEACT_showSenseActWordInText(
		ulong ulSenseActWord
		)
	{
	uchar ucc;
	uchar ucSenseActNum[4];

	if(ulSenseActWord == 0)
		{
		vSERIAL_bout('0');
		return;
		}
	for(ucc=3; ucc<4; ucc--)
		{
		ucSenseActNum[ucc] = (uchar)(ulSenseActWord);
		ulSenseActWord >>=8;
		}

	for(ucc=0; ucc<4;  ucc++)
		{
		if(ucSenseActNum[ucc] == 0) break;
		if(ucc != 0) vSERIAL_bout(',');
		vSENSEACT_showSensorShortName(ucSenseActNum[ucc]);

		}/* END: for() */

	return;

	}/* END: vSENSEACT_showSenseActWordInText() */






/********************  ulSENSEACT_correctSenseActWord()  ***********************
*
* Verify that a SenseActWord is a valid combination of Sense Acts, or
* return the corrected value.
*
* RET:	new SenseActWord
*
******************************************************************************/

ulong ulSENSEACT_correctSenseActWord(
		ulong ulSenseActWord
		)
	{
	uchar ucii;
	uchar ucjj;
	uchar ucSenseActNum[4];
	uchar ucFirstActNum;
	uchar ucSecondActNum;
	uchar ucActSize;
	ulong ulNewSenseActWord;

	/* IF SENSE ACT WORD IS NONE ITS VALID */
	if(ulSenseActWord == 0) return(0UL);

	/* UNPACK THE SENSE ACT WORD */
	for(ucii=3; ucii<4; ucii--)
		{
		ucSenseActNum[ucii] = (uchar)(ulSenseActWord);
		ulSenseActWord >>=8;
		}

	/* NOW CHECK IT -- NO SIZE OVER 4 */
	ulNewSenseActWord = 0UL;
	for(ucii=0,ucjj=0; ucii<4;  ucii++) //ucii=src cnt, ucjj=dest count
		{
		ucFirstActNum = ucSenseActNum[ucii];
		if(ucFirstActNum == 0) break;		//hit src termination
		if(ucjj >3) break;			 		//hit dest termination

		if(ucSENSEACT_getSensorUserFlag(ucFirstActNum) == 0) continue;	//user can't have this
		ucActSize = ucSENSEACT_getSensorDataSize(ucFirstActNum);
		if((ucjj == 3) && (ucActSize == 2))	break; //no 2 wides in space 3

		#if 0
		vSERIAL_rom_sout("Byte= ");
		vSERIAL_HB8out(ucFirstActNum);
		vSERIAL_rom_sout("  Size= ");
		vSERIAL_HB8out(ucSENSEACT_getSensorDataSize);
		vSERIAL_crlf();
		#endif

		ulNewSenseActWord <<= 8;
		ulNewSenseActWord |= ucFirstActNum; //stuff first byte as is
		ucjj++;

		if(ucActSize == 2)
			{
			/* CHOOSE THE SECOND ENTRY */
			switch(ucFirstActNum)
				{
				case SENSOR_TC_1:
				case SENSOR_TC_2:
				case SENSOR_TC_3:
				case SENSOR_TC_4:
					ucSecondActNum = SENSOR_ONEWIRE_0;
					break;

				default:
					ucSecondActNum = ucFirstActNum+1; //load next val as 2nd part
					break;

				}/* END: switch() */

			ulNewSenseActWord <<= 8;				 //move over for 2nd part
			ulNewSenseActWord |= ucSecondActNum;	 //stuff it it
			ucjj++;									 //bump counter

			}/* END: if() */

		}/* END: for() */

	/* LEFT JUSTIFY THE NEW SENSE ACT WORD */
	for(ucii=0;  ucii<3;  ucii++)
		{
		if(ulNewSenseActWord & 0xFF000000) break;
		ulNewSenseActWord <<= 8;
		}

	return(ulNewSenseActWord);

	}/* END: ulSENSEACT_correctSenseActWord() */





/*-------------------------------  MODULE END  ------------------------------*/

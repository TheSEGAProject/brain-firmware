
/**************************  SERIAL.C  *******************************************
*
* Serial port handler
*
* V1.02 04/27/2004 wzr
*		Moved this code to work on the PIC-8720
*
* V1.01 09/11/2002 wzr
*		Repaired the receiver (bout) routine to correct for overrrun errors.
*
* V1.00 04/29/2002 wzr
*		Started
*
******************************************************************************/


/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* function not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */
/*lint -e768 */		/* global struct member not referenced */



/********************  OPTIMIZATION DEFINITIONS  ******************************/




/****************************  INCLUDES  *************************************/

#include "diag.h"			//diagnostic defines
#include <msp430x54x.h>		//
#include "std.h"			//standard defines
#include "config.h"			//system configuration definitions
#include "serial.h"			//serial port defines
#include "delay.h"			//delay routines
#ifdef ESPORT_ENABLED		//defined in diag.h
 #include "ESPORT.h"		//external serial port
#endif	/* END: INC_ESPORT */


/****************************  DEFINES  **************************************/



#define NUMSTR_ARRAY_SIZE 11

#define DEC_3_UNSIGNED_MASK			0UL
#define DEC_3_SIGN_MASK				0x00000004UL
#define DEC_3_NUMERIC_MASK			0x00000007UL

#define DEC_3_CHAR_COUNT			1
#define UNSIGNED_DEC_3_FLD_SIZE		DEC_3_CHAR_COUNT
#define SIGNED_DEC_3_FLD_SIZE		(DEC_3_CHAR_COUNT+1)



#define DEC_8_UNSIGNED_MASK			0UL
#define DEC_8_SIGN_MASK				0x00000080UL
#define DEC_8_NUMERIC_MASK			0x000000FFUL

#define DEC_8_CHAR_COUNT			3
#define UNSIGNED_DEC_8_FLD_SIZE		DEC_8_CHAR_COUNT
#define SIGNED_DEC_8_FLD_SIZE		(DEC_8_CHAR_COUNT+1)



#define DEC_16_UNSIGNED_MASK		0UL
#define DEC_16_SIGN_MASK			0x00008000UL
#define DEC_16_NUMERIC_MASK			0x0000FFFFUL

#define DEC_16_CHAR_COUNT 			5
#define UNSIGNED_DEC_16_FLD_SIZE	DEC_16_CHAR_COUNT
#define SIGNED_DEC_16_FLD_SIZE		(DEC_16_CHAR_COUNT+1)



#define DEC_24_UNSIGNED_MASK		0UL
#define DEC_24_SIGN_MASK			0x00800000UL
#define DEC_24_NUMERIC_MASK			0x00FFFFFFUL

#define DEC_24_CHAR_COUNT 			8
#define UNSIGNED_DEC_24_FLD_SIZE	DEC_24_CHAR_COUNT
#define SIGNED_DEC_24_FLD_SIZE		(DEC_24_CHAR_COUNT+1)



#define DEC_32_UNSIGNED_MASK		0UL
#define DEC_32_SIGN_MASK			0x80000000UL
#define DEC_32_NUMERIC_MASK			0xFFFFFFFFUL

#define DEC_32_CHAR_COUNT 			10
#define UNSIGNED_DEC_32_FLD_SIZE	DEC_32_CHAR_COUNT
#define SIGNED_DEC_32_FLD_SIZE		(DEC_32_CHAR_COUNT+1)

#define VARIABLE_FLD_SIZE			0

/********************************  GLOBALS  **********************************/


 extern volatile union				//ucFLAG0_BYTE
  {
  uchar byte;
  struct
    {
	unsigned FLG0_BIGSUB_CARRY_BIT:1;	 		//bit 0 ;1=CARRY, 0=NO-CARRY
	unsigned FLG0_BIGSUB_6_BYTE_Z_BIT:1;		//bit 1 ;1=all diff 0, 0=otherwise
	unsigned FLG0_BIGSUB_TOP_4_BYTE_Z_BIT:1;	//bit 2 ;1=top 4 bytes 0, 0=otherwise
	unsigned FLG0_REDIRECT_COMM_TO_ESPORT_BIT:1;//bit 3 ;1=REDIRECT, 0=COMM1
	unsigned FLG0_RESET_ALL_TIME_BIT:1;			//bit 4 ;1=do time  reset, 0=dont
												//SET:	when RDC4 gets finds first
												//		SOM2.
												//		or
												//		In a Hub when it is reset.
												//
												//CLR: 	when vMAIN_computeDispatchTiming()
												//		runs next.
	unsigned FLG0_SERIAL_BINARY_MODE_BIT:1;		//bit 5 1=binary mode, 0=text mode
	unsigned FLG0_HAVE_WIZ_GROUP_TIME_BIT:1;	//bit 6 1=Wizard group time has
												//        been aquired from a DC4
												//      0=We are using startup time
	unsigned FLG0_NOTUSED7_BIT:1;				//bit 7
	}FLAG0_STRUCT;
  }ucFLAG0_BYTE;


#ifdef SERIAL_Q_OUTPUT_ENABLED
extern volatile uchar ucaCommQ[ COMM_Q_SIZE ];
#endif

extern volatile uchar ucQonIdx_LUL;
extern volatile uchar ucQoffIdx_LUL;
extern volatile uchar ucQcount;


extern uchar ucGLOB_lineCharPosition;	//line position for computing tabs






/*************************  LOCAL DECLARATIONS  ******************************/

static void vSERIAL_generic_IVout(
		ulong ulSignMaskBit,			//sign mask
		ulong ulNumericMask,			//numeric mask
		char cDesiredFldSize,			//desired output field size
		uchar ucLeadFillChar,			//leading zero fill char
		long lVal						//numeric value to show
		);

static void vSERIAL_B2D_Str(
		long lVal,					   //IN: value to convert
		ulong ulSignBitMask,   //IN: bit on in Sign position (0 if none)
		ulong ulNumericMask,   //IN: Mask of number including sign
		uchar ucaValArray[NUMSTR_ARRAY_SIZE]  //OUT: ten digits and a leading sign
		);

static void vSERIAL_FormatStrout(
		char cDesiredFldSize,	//fld size (digits+sign), 0 = don't care
		uchar ucLeaderChar,		// ' ' or '0'
		uchar ucaValArray[NUMSTR_ARRAY_SIZE] //ten digits and a leading sign
		);

#ifdef SERIAL_Q_OUTPUT_ENABLED
static void vSERIAL_pushQ(
		uchar ucChar
		);
#endif


/********************************  CODE  *************************************/



/***************************  vSERIAL_init()  *********************************
*
*
******************************************************************************/

void vSERIAL_init(
		void
		)
	{

	/* SETUP Q EVEN IF WE DON'T USE IT */
	ucQcount = 0;
	ucQonIdx_LUL = 0;
	ucQoffIdx_LUL = 0;
	
	/* Put state machine in reset */
	UCA0CTL1 |= UCSWRST;									

	/* SETUP THE BAUD RATE */
	UCA0BR0 = UCA0BR0_VALUE;									// Setup UCBR0
	UCA0BR1 = UCA0BR1_VALUE;
	UCA0MCTL |= (UCBRS0_VALUE*UCBRS0) + (UCBRF0_VALUE*UCBRF0);	// Setup UCBRS0 and UCBRF0

	/* ENABLE THE LOW-SPEED/OVERSAMPLING BIT */
	#if (UCOS16_BIT_VALUE == 1)
	UCA0MCTL |= UCOS16;		//SET UCOS16
	#else
	UCA0MCTL &= ~UCOS16;	//CLR UCOS16
	#endif
	
	/* Initialize USCI state machine */
	UCA0CTL1 &= ~UCSWRST;	

	/* TURN ON THE RECEIVER AND TRANSMITTER FOR THE COM PORT */
	UCA0IE |= UCRXIE;				// Enable USCI_A0 RX interrupt
	UCA0IE |= UCTXIE;				// Enable USCI_A0 TX interrupt
	P3SEL = 0x30;					// P3.4,5 = USCI_A0 TXD/RXD


	vDELAY_wait100usTic(5);

	return;

	}/* END: vSERIAL_init() */





/***************************  vSERIAL_quit()  *********************************
*
*
******************************************************************************/

void vSERIAL_quit(
		void
		)
	{
	/* WAIT FOR COUNT TO ZERO */
	while(ucQcount);					

	/* WAIT FOR TRANSMIT TO FINISH */
	while(!(UCA0IFG&UCTXIFG));					

	vDELAY_wait100usTic(5);

	/* DISABLE THE SERIAL PORT */
	UCA0IE &= ~UCRXIE;				// Disable USCI_A0 RX interrupt
	UCA0IE &= ~UCTXIE;				// Disable USCI_A0 TX interrupt

	return;

	}/* END: vSERIAL_quit() */





/************************  vSERIAL_setBinaryMode()  **************************
*
*
*
******************************************************************************/

void vSERIAL_setBinaryMode(
		uchar ucBinModeFlg	//BINARY_MODE_ON  or  BINARY_MODE_OFF
		)
	{

	if(ucBinModeFlg) ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_SERIAL_BINARY_MODE_BIT = 1;
	else			 ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_SERIAL_BINARY_MODE_BIT = 0;

	return;

	}/* END: vSERIAL_setBinaryMode() */






/************************  vSERIAL_bout()  *********************************
*
*
*
******************************************************************************/

void vSERIAL_bout(
		uchar ucChar
		)
	{
	uchar ucc;
	uchar uccMax;

	if(!ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_SERIAL_BINARY_MODE_BIT)
		{

		if(ucChar == '\r') ucGLOB_lineCharPosition = 0;

		if(ucChar == '\t')
			{
			uccMax = 4 - (ucGLOB_lineCharPosition % 4);
			for(ucc=0;  ucc<uccMax;  ucc++){vSERIAL_bout('x');}
			return;
			}
		}

	if(0)
		{
		#ifdef ESPORT_ENABLED				//defined in diag.h
//		vESPORT_bout(ucChar);
		#endif
		}
	else
		{
		#ifdef SERIAL_Q_OUTPUT_ENABLED
		vSERIAL_pushQ(ucChar);			//go stuff the char
		#else
		while(!(UCA0IFG&UCTXIFG));			
		UCB0TXBUF = ucChar;				//stuff the xmit reg
		#endif
		}

	if(!ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_SERIAL_BINARY_MODE_BIT)
		{
		switch(ucChar)
			{
			case '\r':
			case '\n':
			case '\v':
				break;

			default:
				ucGLOB_lineCharPosition++;
				break;

			}/* END: switch() */

		}/* END: if() */

	return;

	}/* END: vSERIAL_bout() */







/************************  ucSERIAL_bin()  *********************************
*
*
*
******************************************************************************/

uchar ucSERIAL_bin(
		void
		)
	{
	uchar ucTmp;


	if(0)
		{
		ucTmp = '*';
		#ifdef ESPORT_ENABLED				//defined in diag.h
		ucTmp = ucESPORT_bin();
		#endif
		}
	else
		{
		/* WAIT FOR A COMPLETED TRANSMISSION */
		while(!(UCA0IFG&UCRXIFG));

		/* READ THE CHAR */
		ucTmp = UCB0RXBUF;
		}
	return(ucTmp);

	}/* END: ucSERIAL_bin() */



/*****************  uiSERIAL_timedBin()  **************************************
*
* RET: >  255 = timeout
*	   <= 255 = char
*
******************************************************************************/

uint uiSERIAL_timedBin(		// uiRET <= 255 is char, uiRET > 255 is Timeout
		uint uiReqTime_in_ms	//Time limit in mS
		)
	{
	uint uiChar;
	ulong ulCntLimit;
	ulong uli;

	/* CONVERT MSEC TO LOOP COUNT */
	ulCntLimit = 40UL * uiReqTime_in_ms;

	/* WAIT FOR A CHAR */
	for(uli=0;  uli<ulCntLimit;  uli++)
		{
		if(ucSERIAL_kbhit())
			{
			uiChar = (uint)ucSERIAL_bin();
			return(uiChar);
			}

		}/* END: for() */

	return(0xFFFF);

	}/* END: uiSERIAL_timedBin() */









/********************  vSERIAL_repeatChar()  *********************************
*
* Does a multiple repeat of any specified char
*
* NOTE: If count is < or == 0 then nothing is printed
*
******************************************************************************/

void vSERIAL_repeatChar( //repeated output of a single char
		uchar ucChar,	//char to output
		char cCount		//number of times to output the char
		)
	{
	char cii;
	
	cii = 0;
	while(cii < cCount)
		{
		vSERIAL_bout(ucChar);
		cii++;
		}/* END: while() */

	return;

	}/* END: vSERIAL_repeatChar() */








/************************  vSERIAL_dash()  *********************************
*
* Print a row of dashes
*
* NOTE: If count is < or == 0 then nothing is printed
*
******************************************************************************/

void vSERIAL_dash(		//repeated output of dashes
		char cCount		//count if <= 0 then nothing output
		)
	{
	vSERIAL_repeatChar('-', cCount);
	return;

	}/* END: vSERIAL_dash() */




/************************  vSERIAL_spaces()  *********************************
*
* Print a row of spacees
*
* NOTE: If count is < or == 0 then nothing is printed
*
******************************************************************************/

void vSERIAL_spaces(	//repeated output of spaces
		char cCount		//count: if <= 0 then nothing output
		)
	{

	vSERIAL_repeatChar(' ', cCount);
	return;

	}/* END: vSERIAL_spaces() */






/*************************  ucSERIAL_kbhit()  ********************************
*
* check for a key hit on the serial port
*
*
*
******************************************************************************/

uchar ucSERIAL_kbhit(
		void
		)
	{

	return((uchar) (UCA0IFG&UCRXIFG));

	}/* END: ucSERIAL_kbhit() */







/************************  vSERIAL_crlf()  *********************************
*
* Carriage return line feed
*
******************************************************************************/

void vSERIAL_crlf(
		void
		)
	{
	vSERIAL_bout('\r');
	vSERIAL_bout('\n');
	return;

	}/* END: vSERIAL_crlf() */




/************************  vSERIAL_Hformat()  *******************************
*
* Put out the Hex format leader "0x"
*
******************************************************************************/

void vSERIAL_Hformat(
		void
		)
	{
	vSERIAL_bout('0');
	vSERIAL_bout('x');
	return;

	}/* END: vSERIAL_Hformat() */






/********************  ucSERIAL_toupper()  ***************************************
*
* convert the char passed to an uppercase char
*
******************************************************************************/

uchar ucSERIAL_toupper(
		uchar ucChar
		)
	{
	if((ucChar >= 0x61) && (ucChar <= 0x7A)) ucChar &= ~0x20;
	return(ucChar);

	}/* END: ucSERIAL_toupper() */





/********************  ucSERIAL_isnum()  ***************************************
*
* Ret:	1= num
*		0= not num
*
******************************************************************************/

uchar ucSERIAL_isnum(			//RET: 1=is a number, 0=not a number
		uchar ucChar			//char to check
		)
	{
	if((ucChar >= '0') && (ucChar <= '9')) return(1);
	return(0);

	}/* END: ucSERIAL_isnum() */






/************************  vSERIAL_rom_sout()  *******************************
*
* String Output from ROM string
*
******************************************************************************/

void vSERIAL_rom_sout(
		const char *cStrPtr		
		) 
	{											
   	while(*cStrPtr)
		{
		vSERIAL_bout((uchar)*cStrPtr++);
		}
	return;

	}/* END: vSERIAL_rom_sout() */




/************************  vSERIAL_padded_rom_sout()  *******************************
*
* Padded String Output from ROM string, Space padding is either on left if
* pad count is (-) or is on the right if pad count is (+).
*
* NOTE: If pad count is less than string length then no padding is used.
*
* NOTE: If pad count is 0 then no padding is used.
*
******************************************************************************/

void vSERIAL_padded_rom_sout(
		const char *cStrPtr,	
		char cPadCount				//number of pad spaces (-=left, +=Right)
		) 
	{											
	char cStrLen;
	char cPosPadCount;
	char cLenDiff;

	cStrLen = cSERIAL_rom_strlen(cStrPtr);

	cPosPadCount = cPadCount;
	if(cPosPadCount < 0) cPosPadCount = -cPosPadCount;
	cLenDiff = cPosPadCount - cStrLen;

	if(cPadCount < 0)
		{
		vSERIAL_spaces(cLenDiff);
		}

	vSERIAL_rom_sout(cStrPtr);

	if(cPadCount > 0)
		{
		vSERIAL_spaces(cLenDiff);

		}/* END: if() */

	return;

	}/* END: vSERIAL_padded_rom_sout() */





/************************  cSERIAL_rom_strlen()  ****************************
*
* Find string length of a rom string.
*
* RET: the length of the string (not including the 0 terminator).
*
******************************************************************************/

char cSERIAL_rom_strlen(	//return the length of a rom string
		const char *cStrPtr	
		) 
	{											
	char cStrLen;

	cStrLen = 0;
   	while(*cStrPtr++)
		{
		cStrLen++;
		}
	return(cStrLen);

	}/* END: cSERIAL_rom_strlen() */






/*******************  vSERIAL_any_char_to_cont()  ****************************
*
*
*
******************************************************************************/

void vSERIAL_any_char_to_cont(
		void
		)
	{											

	vSERIAL_rom_sout("AnyChrToCont..");
	ucSERIAL_bin();	
	vSERIAL_crlf();
	return;

	}/* END: vSERIAL_any_char_to_cont() */





/*******************  vSERIAL_showXXXmsg()  ********************************
*
*
*
******************************************************************************/

void vSERIAL_showXXXmsg( //SHOWS: " XXX\r\n" 
		void
		)
	{											

	vSERIAL_rom_sout(" XXX\r\n");
	return;

	}/* END: vSERIAL_showXXXmsg() */





/*******************  ucSERIAL_confirm()  ************************************
*
* RET:	1 = confirmed
*		0 = not confirmed
*
******************************************************************************/

uchar ucSERIAL_confirm( //RET: 1=YES CONFIRMED, 0=NO NOT CONFIRMED
		uchar ucAbortTextingFlag //YES_SHOW_ABORT_MSG or NO_SHOW_ABORT_MSG
		)
	{											
	uchar ucChar;

	vSERIAL_rom_sout("[HitEntrToCnfrm] ");
	ucChar = ucSERIAL_bin();
	if((ucChar != '\r') && (ucChar != '\n'))
		{
		if(ucAbortTextingFlag)
			{
			vSERIAL_showXXXmsg();
			}
		return(0);
		}
	
	vSERIAL_crlf();
	return(1);

	}/* END: ucSERIAL_confirm() */




#if 0

/* If'd out because it is not used */

/*******************  ucSERIAL_YorN()  ****************************************
*
* RET:	1 = YES
*		0 = not YES
*
******************************************************************************/

uchar ucSERIAL_YorN( //RET: 1=YES, 0=NOT YES
		void
		)
	{											
	uchar ucChar;

	vSERIAL_rom_sout("[YorN] ");
	ucChar = ucSERIAL_bin();
	vSERIAL_crlf();
	if((ucChar != 'Y') && (ucChar != 'y')) return(1);
	return(0);

	}/* END: ucSERIAL_YorN() */


#endif







/****************** ucSERIAL_readColTab() *********************************
*
* RET: cur col number (first col = 0)
*
******************************************************************************/

uchar ucSERIAL_readColTab(	//RET: cur col number (first col = 0)
		void
		)
	{
	return(ucGLOB_lineCharPosition);

	}/* END: ucSERIAL_readColTab() */




/*******************  vSERIAL_colTab()  *************************************
*
*
*
******************************************************************************/

void vSERIAL_colTab( //Tab to a particular column (first col = 0)
		uchar ucColNum
		)
	{											
	uchar ucc;
	uchar uccMax;

	if(ucGLOB_lineCharPosition >= ucColNum) return;
	uccMax = ucColNum - ucGLOB_lineCharPosition;

	for(ucc=0; ucc<uccMax;  ucc++)
		{
		vSERIAL_bout(' ');
		}

	return;

	}/* END: vSERIAL_colTab() */







/*************************  vSERIAL_HB4out()  **********************************
*
* SERIAL HEX NIBBLE OUT
*
*
*
******************************************************************************/

void vSERIAL_HB4out(
		uchar ucByte
		)
	{
	uchar uc;

	uc = (ucByte & 0x0F) + 0x30;
	if(uc >= 0x3A) uc+= 7;

	vSERIAL_bout(uc);

	return;

	}/* END: vSERIAL_HB4out() */





/*************************  vSERIAL_HB8out()  ********************************
*
*
******************************************************************************/

void vSERIAL_HB8out(
		uchar ucByte
		)
	{
	vSERIAL_HB4out(ucByte>>4);
	vSERIAL_HB4out(ucByte);

	return;

	}/* END: vSERIAL_HB8out() */





/************************* vSERIAL_HB8Fout()  *******************************
*
* HEX formatted output
*
****************************************************************************/

void vSERIAL_HB8Fout(	//HEX formatted output (ie 0x55)
		uchar ucByte
		)
	{

	vSERIAL_Hformat();
	vSERIAL_HB8out(ucByte);

	return;

	}/* END: vSERIAL_HB8Fout() */






/*************************  vSERIAL_I8out()  ********************************
*
* Signed Integer 8-Bit output
*
******************************************************************************/

void vSERIAL_I8out(
		char cInt
		)
	{
	long lVal;

	lVal = (long)cInt;
	vSERIAL_generic_IVout(	DEC_8_SIGN_MASK,		//sign mask
							DEC_8_NUMERIC_MASK,		//numeric mask
							SIGNED_DEC_8_FLD_SIZE,	//desired output fld size
							' ',					//Lead fill char
							lVal					//numeric value
							);

#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)cInt,DEC_8_SIGN_MASK,DEC_8_NUMERIC_MASK,ucaValArray);

	vSERIAL_FormatStrout(SIGNED_DEC_8_FLD_SIZE, ' ', ucaValArray);
#endif
	return;

	}/* END: vSERIAL_I8out() */







/*************************  vSERIAL_IV8out()  ********************************
*
* Signed Integer output
*
******************************************************************************/

void vSERIAL_IV8out(
		char cVal
		)
	{
	long lVal;

	lVal = (long)cVal;
	vSERIAL_generic_IVout(
					DEC_8_SIGN_MASK,		//sign mask
					DEC_8_NUMERIC_MASK,		//numeric mask
					VARIABLE_FLD_SIZE,		//desired output fld size
					' ',					//Lead fill char
					lVal					//numeric value
					);

#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)ucVal, DEC_8_SIGN_MASK, DEC_8_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(VARIABLE_FLD_SIZE, ' ', ucaValArray);
#endif

	return;

	}/* END: vSERIAL_IV8out() */







/*************************  vSERIAL_UI8out()  ********************************
*
* Unsigned Integer 8-Bit output
*
******************************************************************************/

void vSERIAL_UI8out(
		uchar ucVal
		)
	{
	long lVal;

	lVal = (long)ucVal;
	vSERIAL_generic_IVout(
				DEC_8_UNSIGNED_MASK,		//sign mask
				DEC_8_NUMERIC_MASK,			//numeric mask
				UNSIGNED_DEC_8_FLD_SIZE,	//desired output fld size
				' ',						//Lead fill char
				lVal						//numeric value
				);

	return;

	}/* END: vSERIAL_UI8out() */






/*************************  vSERIAL_UI8_2char_out()  ********************************
*
* Unsigned Integer 8-Bit output
*
******************************************************************************/

void vSERIAL_UI8_2char_out(	//2 char fixed width
		uchar ucVal,			//value
		uchar ucLeadFillChar	//leading fill char
		)
	{
	long lVal;

	lVal = (long)ucVal;
	vSERIAL_generic_IVout(
				DEC_8_UNSIGNED_MASK,		//sign mask
				DEC_8_NUMERIC_MASK,			//numeric mask
				2,							//desired output fld size
				ucLeadFillChar,				//Lead fill char
				lVal						//numeric value
				);

	return;

	}/* END: vSERIAL_UI8_2char_out() */








/*************************  vSERIAL_UIV8out()  ********************************
*
* Unsigned Integer variable width 8-Bit output
*
*
*
******************************************************************************/

void vSERIAL_UIV8out(
		uchar ucVal
		)
	{
	long lVal;

	lVal = (long)ucVal;
	vSERIAL_generic_IVout(
				DEC_8_UNSIGNED_MASK,		//sign mask
				DEC_8_NUMERIC_MASK,			//numeric mask
				VARIABLE_FLD_SIZE,			//desired output fld size
				' ',						//Lead fill char
				lVal						//numeric value
				);
#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)ucVal, DEC_8_UNSIGNED_MASK, DEC_8_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(VARIABLE_FLD_SIZE, ' ', ucaValArray);
#endif
	return;

	}/* END: vSERIAL_UIV8out() */






/*************************  vSERIAL_HB16out()  **********************************
*
* Hex Integer output
*
******************************************************************************/

void vSERIAL_HB16out(
		unsigned int uiVal
		)
	{
	vSERIAL_HB8out((uchar)(uiVal>>8));
	vSERIAL_HB8out((uchar) uiVal);

	return;

	}/* END: vSERIAL_HB16out() */




/*************************  vSERIAL_HBV16out()  ******************************
*
* Hex Variable output
*
******************************************************************************/

void vSERIAL_HBV16out(
		uint uiVal
		)
	{
	ulong ulVal;

	ulVal = uiVal;

	vSERIAL_HBV32out(ulVal);

	return;

	}/* END: vSERIAL_HBV16out() */






/************************* vSERIAL_HB16Fout()  *******************************
*
* HEX formatted output
*
****************************************************************************/

void vSERIAL_HB16Fout(
		unsigned int uiInt
		)
	{

	vSERIAL_Hformat();
	vSERIAL_HB16out(uiInt);

	return;

	}/* END: vSERIAL_HB16Fout() */






/*************************  vSERIAL_UI16out()  **********************************
*
* Unsigned Integer output
*
******************************************************************************/

void vSERIAL_UI16out(
		unsigned int uiVal
		)
	{
	long lVal;

	lVal = (long)uiVal;
	vSERIAL_generic_IVout(
				DEC_16_UNSIGNED_MASK,		//sign mask
				DEC_16_NUMERIC_MASK,		//numeric mask
				UNSIGNED_DEC_16_FLD_SIZE,	//desired output fld size
				' ',						//Lead fill char
				lVal						//numeric value
				);

#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)uiVal, DEC_16_UNSIGNED_MASK, DEC_16_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(UNSIGNED_DEC_16_FLD_SIZE, ' ', ucaValArray);
#endif
	return;

	}/* END: vSERIAL_UI16out() */







/*************************  vSERIAL_UIV16out()  **********************************
*
* Unsigned Integer variable field 16 bit output
*
*
*
******************************************************************************/

void vSERIAL_UIV16out(
		unsigned int uiVal
		)
	{
	long lVal;

	lVal = (long)uiVal;
	vSERIAL_generic_IVout(
				DEC_16_UNSIGNED_MASK,		//sign mask
				DEC_16_NUMERIC_MASK,		//numeric mask
				VARIABLE_FLD_SIZE,			//desired output fld size
				' ',						//Lead fill char
				lVal						//numeric value
				);
#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)uiVal, DEC_16_UNSIGNED_MASK, DEC_16_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(VARIABLE_FLD_SIZE, ' ', ucaValArray);
#endif
	return;

	}/* END: vSERIAL_UIV16out() */





/*************************  vSERIAL_I16outt()  *******************************
*
* Signed Integer output
*
*
*
******************************************************************************/

void vSERIAL_I16out(
		int iVal
		)
	{
	long lVal;

	lVal = (long)iVal;
	vSERIAL_generic_IVout(
				DEC_16_SIGN_MASK,			//sign mask
				DEC_16_NUMERIC_MASK,		//numeric mask
				SIGNED_DEC_16_FLD_SIZE,		//desired output fld size
				' ',						//Lead fill char
				lVal						//numeric value
				);
#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)iVal, DEC_16_SIGN_MASK, DEC_16_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(SIGNED_DEC_16_FLD_SIZE, ' ', ucaValArray);
#endif

	return;

	}/* END: vSERIAL_I16outFormat() */






/*************************  vSERIAL_IV16out()  ********************************
*
* Signed Integer output
*
*
*
******************************************************************************/

void vSERIAL_IV16out(
		int iVal
		)
	{
	long lVal;

	lVal = (long)iVal;
	vSERIAL_generic_IVout(
				DEC_16_SIGN_MASK,		//sign mask
				DEC_16_NUMERIC_MASK,		//numeric mask
				VARIABLE_FLD_SIZE,			//desired output fld size
				' ',						//Lead fill char
				lVal						//numeric value
				);
#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)iVal, DEC_16_SIGN_MASK, DEC_16_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(VARIABLE_FLD_SIZE, ' ', ucaValArray);
#endif
	return;

	}/* END: vSERIAL_IV16out() */







/*************************  vSERIAL_I16outFormat()  ***************************
*
* Signed Integer output
*
******************************************************************************/

void vSERIAL_I16outFormat(
		int iVal,
		char cFormatWidth
		)
	{
	long lVal;

	lVal = (long)iVal;
	vSERIAL_generic_IVout(
				DEC_16_SIGN_MASK,			//sign mask
				DEC_16_NUMERIC_MASK,		//numeric mask
				cFormatWidth,				//desired output fld size
				' ',						//Lead fill char
				lVal						//numeric value
				);
#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)iVal, DEC_16_SIGN_MASK, DEC_16_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(cFormatWidth, ' ', ucaValArray);
#endif
	return;

	}/* END: vSERIAL_I16outFormat() */










/*************************  vSERIAL_HB24out()  ******************************
*
* Hex 24bit word output
*
******************************************************************************/

void vSERIAL_HB24out(
		USL uslB24	
		)
	{

	vSERIAL_HB8out((uchar)(uslB24>>16));
	vSERIAL_HB8out((uchar)(uslB24>>8));
	vSERIAL_HB8out((uchar) uslB24);

	return;

	}/* END: vSERIAL_HB24out() */




/*************************  vSERIAL_HBV24out()  ******************************
*
* Hex Variable output
*
******************************************************************************/

void vSERIAL_HBV24out(
		usl uslVal
		)
	{
	ulong ulVal;

	ulVal = uslVal;

	vSERIAL_HBV32out(ulVal);

	return;

	}/* END: vSERIAL_HBV24out() */






/************************* vSERIAL_HB24Fout()  *******************************
*
* HEX formatted output
*
****************************************************************************/

void vSERIAL_HB24Fout(
		USL uslB24	
		)
	{

	vSERIAL_Hformat();
	vSERIAL_HB24out(uslB24);

	return;

	}/* END: vSERIAL_HB24Fout() */






/*************************  vSERIAL_UI24out()  ********************************
*
* UNSigned Integer 24-Bit output
*
******************************************************************************/

void vSERIAL_UI24out(
		USL uslVal
		)
	{
	long lVal;

	lVal = (long)uslVal;
	vSERIAL_generic_IVout(
				DEC_24_UNSIGNED_MASK,		//sign mask
				DEC_24_NUMERIC_MASK,		//numeric mask
				UNSIGNED_DEC_24_FLD_SIZE,	//desired output fld size
				' ',						//Lead fill char
				lVal						//numeric value
				);
	return;

	}/* END: vSERIAL_UI24out() */







/*************************  vSERIAL_UIV24out()  ********************************
*
* UNSigned Integer 24-Bit output
*
******************************************************************************/

void vSERIAL_UIV24out(
		USL uslVal
		)
	{
	long lVal;

	lVal = (long)uslVal;
	vSERIAL_generic_IVout(
				DEC_24_UNSIGNED_MASK,		//sign mask
				DEC_24_NUMERIC_MASK,		//numeric mask
				VARIABLE_FLD_SIZE,			//desired output fld size
				' ',						//Lead fill char
				lVal						//numeric value
				);
	return;

	}/* END: vSERIAL_UIV24out() */







/*************************  vSERIAL_HB32out()  **********************************
*
* Hex Long Integer output
*
******************************************************************************/

void vSERIAL_HB32out(
		ulong ulLong
		)
	{

	vSERIAL_HB16out((unsigned int)(ulLong>>16));
	vSERIAL_HB16out((unsigned int) ulLong);

	return;

	}/* END: vSERIAL_HB32out() */






/*************************  vSERIAL_HBV32out()  ******************************
*
* Hex Long Integer output
*
******************************************************************************/

void vSERIAL_HBV32out(
		ulong ulLong
		)
	{
	signed char cc;
	uchar ucFoundNonZro;
	uchar ucNumVal;

	ucFoundNonZro = 0;
	for(cc=7; cc>=0;  cc--)
		{
		ucNumVal = (uchar)((ulLong >> (4*cc)) & 0xFL);
		if((ucNumVal == 0) && (ucFoundNonZro == 0) && (cc != 0)) continue;
		ucFoundNonZro = 1;
		vSERIAL_HB4out(ucNumVal);
		}
	return;

	}/* END: vSERIAL_HBV32out() */





/************************* vSERIAL_HB32Fout()  *******************************
*
* HEX formatted output
*
****************************************************************************/

void vSERIAL_HB32Fout(
		ulong ulLong
		)
	{

	vSERIAL_Hformat();
	vSERIAL_HB32out(ulLong);

	return;

	}/* END: vSERIAL_HB32Fout() */






/*************************  vSERIAL_UI32out()  ********************************
*
* UNSigned Integer 32-Bit output
*
******************************************************************************/

void vSERIAL_UI32out(
		ulong ulVal
		)
	{
	long lVal;

	lVal = (long)ulVal;
	vSERIAL_generic_IVout(
				DEC_32_UNSIGNED_MASK,		//sign mask
				DEC_32_NUMERIC_MASK,		//numeric mask
				UNSIGNED_DEC_32_FLD_SIZE,	//desired output fld size
				' ',						//Lead fill char
				lVal						//numeric value
				);
#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)ulVal, DEC_32_UNSIGNED_MASK, DEC_32_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(UNSIGNED_DEC_32_FLD_SIZE, ' ', ucaValArray);
#endif
	return;

	}/* END: vSERIAL_UI32out() */






/*************************  vSERIAL_UIV32out()  ********************************
*
* Signed Integer 32-Bit output
*
******************************************************************************/

void vSERIAL_UIV32out(
		ulong ulVal
		)
	{
	long lVal;

	lVal = (long)ulVal;
	vSERIAL_generic_IVout(
				DEC_32_UNSIGNED_MASK,		//sign mask
				DEC_32_NUMERIC_MASK,		//numeric mask
				VARIABLE_FLD_SIZE,			//desired output fld size
				' ',						//Lead fill char
				lVal						//numeric value
				);
	return;

	}/* END: vSERIAL_UIV32out() */








/*************************  vSERIAL_I32out()  ********************************
*
* Signed Integer 32-Bit output
*
******************************************************************************/

void vSERIAL_I32out(
		long lVal
		)
	{
	vSERIAL_generic_IVout(
				DEC_32_SIGN_MASK,			//sign mask
				DEC_32_NUMERIC_MASK,		//numeric mask
				SIGNED_DEC_32_FLD_SIZE,		//desired output fld size
				' ',						//Lead fill char
				lVal						//numeric value
				);
#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str(lVal, DEC_32_SIGN_MASK, DEC_32_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(SIGNED_DEC_32_FLD_SIZE, ' ', ucaValArray);
#endif
	return;

	}/* END: vSERIAL_I32out() */






/*************************  vSERIAL_IV32out()  ********************************
*
* Signed Integer 32-Bit output variable width
*
******************************************************************************/

void vSERIAL_IV32out(
		long lVal
		)
	{
	vSERIAL_generic_IVout(
				DEC_32_SIGN_MASK,			//sign mask
				DEC_32_NUMERIC_MASK,		//numeric mask
				VARIABLE_FLD_SIZE,			//desired output fld size
				' ',						//Lead fill char
				lVal						//numeric value
				);
#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str(lVal, DEC_32_SIGN_MASK, DEC_32_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(VARIABLE_FLD_SIZE, ' ', ucaValArray);
#endif
	return;

	}/* END: vSERIAL_IV32out() */








/*************************  vSERIAL_UI32MicroDecOut()  ***********************
*
* Unsigned Integer 32-Bit Micro Decimal output
*
******************************************************************************/

void vSERIAL_UI32MicroDecOut(
		long lVal
		)
	{
//	#define TEST_MICRO_DEC_OUT

	#ifdef TEST_MICRO_DEC_OUT
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];
	uchar uci;
	#endif


	vSERIAL_generic_IVout(
				DEC_32_UNSIGNED_MASK,		//sign mask
				0xFFFFFL,					//numeric mask
				6,							//desired output fld size
				'0',						//Lead fill char
				lVal						//numeric value
				);



	#ifdef TEST_MICRO_DEC_OUT

	#if 1
	vSERIAL_crlf();
	vSERIAL_rom_sout("ValPassedIn= ");
	vSERIAL_UIV32out((ulong)lVal);
	vSERIAL_crlf();
	#endif

	#if 1
	/* ZRO THE VALUE */
	for(uci=0;  uci<NUMSTR_ARRAY_SIZE;  uci++)
		{
		ucaValArray[uci] = 0;
		}

	vSERIAL_rom_sout("ucaValArrayBeforeFill= ");
	for(uci=0;  uci<NUMSTR_ARRAY_SIZE;  uci++)
		{
		vSERIAL_HB8out(ucaValArray[uci]);
		vSERIAL_bout(',');
		}
	vSERIAL_crlf();
	#endif


	vSERIAL_B2D_Str(lVal, DEC_32_UNSIGNED_MASK, 0xFFFFFL, ucaValArray);


	#if 1
	vSERIAL_crlf();
	vSERIAL_rom_sout("ucaValArrayAfterFill= ");
	for(uci=0;  uci<NUMSTR_ARRAY_SIZE;  uci++)
		{
		vSERIAL_HB8out(ucaValArray[uci]);
		vSERIAL_bout(' ');
		}
	vSERIAL_crlf();
	#endif


	vSERIAL_FormatStrout(6,'0', ucaValArray);
	#endif

	return;

	}/* END: vSERIAL_UI32MicroDecOut() */




#if 0

/************************** uiSERIAL_UI16in()  *******************************
*
* Unsigned integer input
*
******************************************************************************/

unsigned int uiSERIAL_UI16in(
		void
		)
	{
	unsigned int uiRetVal;

	uiRetVal = 0;



	return(uiRetVal);

	}/* END: uiSERIAL_UI16in() */

#endif





/*************************  vSERIAL_B2D_Str()  ***************************
*
* BINARY TO DECIMAL CONVERSION
*
* This routine calculates a signed or unsigned decimal number to a string
*
* Returns array str:
*
*   0    1    2    3    4    5    6    7    8    9   10
* ÚÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄ¿
* ³ S ³ D9 ³ D8 ³ D7 ³ D6 ³ D5 ³ D4 ³ D3 ³ D2 ³ D1 ³ D0 ³
* ÀÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÙ
*  ' '
*  or
*  '-'
*
******************************************************************************/

static void vSERIAL_B2D_Str(
		long lVal,					   //IN: value to convert
		ulong ulSignBitMask,   //IN: bit on in Sign position (0 if none)
		ulong ulNumericMask,   //IN: Mask of number including sign
		uchar ucaValArray[NUMSTR_ARRAY_SIZE]  //OUT: ten digits and a leading sign
		)
	{
	char cc;
	ulong ulUnsignedVal;

	/* ASSUME THAT WE HAVE A POSITIVE NUMBER */
	ucaValArray[0] = ' ';
	ulUnsignedVal = (ulong) lVal;

	/* HANDLE NEGATIVE NUMBER IF REQUIRED */
	if(ulSignBitMask != 0)
		{
		/* CHECK IF NUMBER WAS REALLY NEGATIVE */
		if(ulUnsignedVal & ulSignBitMask)
			{
			ucaValArray[0] = '-';	//its a negative -- save sign and convert

			/* NEGATE THE VALUE */
			ulUnsignedVal = ~ulUnsignedVal;
			ulUnsignedVal++;
			ulUnsignedVal &= ulNumericMask;

			}/* END: if() */

		}/* END: if() */

	/* BREAK THE NUMBER INTO ITS DIGITS */
	for(cc=10;  cc >= 1;  cc--)
		{
		ucaValArray[cc] = (uchar)(ulUnsignedVal % 10);
		ulUnsignedVal /= 10;
		}

	#if 0
	/* SHOW THE PASSED BACK ARRAY */
	vSERIAL_rom_sout("ARRAY= ");
	for(cc=0;  cc<11;  cc++)
		{
		vSERIAL_HB8out(ucaValArray[cc]);
		vSERIAL_bout(',');
		}
	vSERIAL_crlf();
	#endif

	return;

	}/* END: vSERIAL_B2D_Str() */






/*************************  vSERIAL_FormatStrout()  ********************************
*
* formatted output
*
* Accepts:
*   0    1    2    3    4    5    6    7    8    9   10
* ÚÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄ¿
* ³ S ³ D9 ³ D8 ³ D7 ³ D6 ³ D5 ³ D4 ³ D3 ³ D2 ³ D1 ³ D0 ³
* ÀÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÙ
*
*
* EXAMPLE:
*
* If input was -2135
*
* Accepts:
*   0    1    2    3    4    5    6    7    8    9   10
* ÚÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄ¿
* ³ - ³  0 ³  0 ³  0 ³  0 ³  0 ³  0 ³  2 ³  1 ³  3 ³  5 ³
* ÀÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÙ
*
******************************************************************************/

static void vSERIAL_FormatStrout(
		char cDesiredFldSize,	//fld size (digits+sign), 0 = don't care
		uchar ucLeaderChar,		// ' ' or '0'
		uchar ucaValArray[NUMSTR_ARRAY_SIZE] //ten digits and a leading sign
		)
	{
	char cc;
	char cFirstNonZro;
	char cActualFldSize;
	char cPrintStartIdx;


	/* WALK THE STRING CONVERTING IT TO ASCII */
	for(cc=1;  cc<11;  cc++)
		{
		ucaValArray[cc] |= 0x30;		//convert to ascii
		}

	/* WALK THE STRING AND CONVERT ALL LEADING ZEROS TO LEADER CHAR */
	cFirstNonZro = 10;			//start with a good value in position 10
	for(cc=1;  cc<=9;  cc++)
		{
		if(ucaValArray[cc] == '0')
			{
			ucaValArray[cc] = ucLeaderChar;
			}
		else
			{
			cFirstNonZro = cc;	//shorten up the good value position
			break;
			}
		}/* END: for() */
	
	/* IF THE STRING IS NEGATIVE AND FIRSTNONZRO DIGIT IS NOT POSITION 1 */
	/* MOVE MINUS SIGN TO FIRSTNONZRO DIGIT -1 */
	if((cFirstNonZro != 1) && (ucaValArray[0] == '-')) 
		{
		cFirstNonZro--;
		ucaValArray[cFirstNonZro] = '-';
		ucaValArray[0] = ' ';
		}

	/* COMPUTE THE FIELD SIZE */
	cActualFldSize = 11 - cFirstNonZro;
	if(cDesiredFldSize != 0)
		{
		if(cDesiredFldSize > cActualFldSize)  cActualFldSize = cDesiredFldSize;
		}

	cPrintStartIdx = 11 - cActualFldSize;
	if((cDesiredFldSize == 0) && 
	   (cActualFldSize == 10) &&
	   (ucaValArray[0] == '-')
	  ) cPrintStartIdx--;


	/* NOW PRINT OUT THE STRING */
	for(cc = cPrintStartIdx;  cc < 11;  cc++)
		{
		vSERIAL_bout(ucaValArray[cc]);
		}/* END: for(cc) */


	return;

	}/* END: vSERIAL_FormatStrout() */






/***********************  ucSERIAL_getEditedInput()  ************************
*
* Get a short edited input str from console
*
* RET 0 if msg is good
*	  1 if msg has been aborted (ESC was hit)
*
*****************************************************************************/

uchar ucSERIAL_getEditedInput(	//RET: 0 if msg is good, 1 if msg aborted
		uchar ucStr[],			//ASCIZ str passed back-- CRLF = 0;
		uchar ucMaxCount		//max chars = max msg size -1
		)
	{
	uchar ucii;
	uchar ucRetVal;
	uchar ucBreakoutFlg;

	ucRetVal = 0;			//assume msg is good
	ucBreakoutFlg = FALSE;
	for(ucii=0;  ucii<ucMaxCount;  )
		{
		/* GET A CHAR */
		ucStr[ucii] = ucSERIAL_bin();
		
		switch(ucStr[ucii])
			{
			/* HANDLE BACKSPACE */
			case 0x08:
				if(ucii != 0)
					{
					vSERIAL_bout(0x08);			//echo backspace
					ucii--;						//backup the ptr
					}
				break;

			case 0x0D:
			case 0x0A:
				if(ucSERIAL_readColTab() != 0) vSERIAL_crlf();	//echo CR-LF
				ucStr[ucii] = 0;					//terminate the line
				ucBreakoutFlg = TRUE;
				break;

			case 0x1B:								//escape char
				ucRetVal = 1;
				ucBreakoutFlg = TRUE;
				break;

			default:
				vSERIAL_bout(ucStr[ucii]);			//echo backspace
				ucii++;								//bump the ptr

			}/* END: switch() */

		if(ucBreakoutFlg) break;

		}/* END: for(ucii) */

	if(ucii >= ucMaxCount)
		{
		if(ucSERIAL_readColTab() != 0) vSERIAL_crlf();	//echo CR-LF
		}

	return(ucRetVal);

	}/* END: vSERIAL_getEditedInput() */







/************************  lSERIAL_AsciiToNum()  *****************************
*
* convert an ascii string to a longnumeric value
*
*
******************************************************************************/

long lSERIAL_AsciiToNum(
		uchar ucStr[],
		uchar ucSignFlag,	//UNSIGNED = 0, SIGNED = 1
		uchar ucRadix
		)
	{
	long lRetVal;
	uchar ucDigitVal;
	uchar uc;
	uchar ucNegateValue;
	uchar ucIsPreNumeric;

	lRetVal = 0;
	ucNegateValue = FALSE;
	ucIsPreNumeric = TRUE;

	for(uc=0;  uc<12;  uc++)
		{

		/* EAT ANY LEADING SPACES */
		if((ucStr[uc] == ' ') && (ucIsPreNumeric == TRUE))
			{
			continue;
			}

		/* CHECK FOR A LEADING SIGN */
		if((ucStr[uc] == '-') && (ucSignFlag == SIGNED) && (ucIsPreNumeric == TRUE))
			{
			ucNegateValue = TRUE;
			continue;
			}

		ucIsPreNumeric = FALSE;

		/* CONVERT THE CHAR TO A DIGIT VALUE */
		ucDigitVal = ucRadix;
		if((ucStr[uc] >= '0') && (ucStr[uc] <= '9')) ucDigitVal = ucStr[uc] - '0';
		if((ucStr[uc] >= 'A') && (ucStr[uc] <= 'Z')) ucDigitVal = ucStr[uc] - 'A' + 10;
		if((ucStr[uc] >= 'a') && (ucStr[uc] <= 'z')) ucDigitVal = ucStr[uc] - 'a'+ 10;
		#if 0
		vSERIAL_HB8out(ucDigitVal);
		vSERIAL_bout(',');
		#endif
		if(ucDigitVal >= ucRadix) break;		//throw out illegal chars

		lRetVal *= ucRadix;		//shift up for next value
		lRetVal += ucDigitVal;	//add in next value

		}/* END: for() */

	#if 0
	vSERIAL_crlf();
	#endif

	if(ucNegateValue) lRetVal = -lRetVal;

	return(lRetVal);

	}/* END: lSERIAL_AsciiToNum() */








/********************  vSERIAL_printDashIntro()  ***************************
*
*
*
******************************************************************************/
void vSERIAL_printDashIntro(
		const char *cStrPtr	
		)
	{
	vSERIAL_rom_sout("\r\n\r\n---  ");
	vSERIAL_rom_sout(cStrPtr);
	vSERIAL_rom_sout("  ---\r\n\r\n");

	}/* END: vSERIAL_printDashIntro() */






/*************************  ulSERIAL_H32in()  *******************************
*
* do HEX unsigned long number input.
*
*
*****************************************************************************/

ulong ulSERIAL_H32in(
		void
		)
	{
	ulong ulVal;
	uchar ucStr[13];

	/* GET THE NUMERIC STRING */
	while(ucSERIAL_getEditedInput(ucStr, 13));

	#if 0
	{
	uchar ucc;
	vSERIAL_rom_sout("ucStr=");
	for(ucc=0;  ucc<13;  ucc++)
		{
		if(ucStr[ucc] == 0) break;
		vSERIAL_bout(ucStr[ucc]);
		vSERIAL_bout(',');
		}
	vSERIAL_crlf();
	}
	#endif

	/* CONVERT IT TO A LONG */
	ulVal = (ulong)lSERIAL_AsciiToNum(ucStr, UNSIGNED, HEX);

	#if 0
	vSERIAL_rom_sout("Numeric= ");
	vSERIAL_HB32out(ulVal);
	vSERIAL_crlf();
	#endif

	return(ulVal);

	}/* END: ulSERIAL_H32in() */





/*********************  lSERIAL_I32in()  ***********************************
*
* do DECIMAL long number input.
*
*
*****************************************************************************/

long lSERIAL_I32in(
		void
		)
	{
	long lValue;
	uchar ucStr[13];

	/* GET THE NUMERIC STRING */
	while(ucSERIAL_getEditedInput(ucStr, 13));

	/* CONVERT IT TO A LONG */
	lValue = lSERIAL_AsciiToNum(ucStr, SIGNED, DECIMAL);

	return(lValue);

	}/* END: lSERIAL_I32in() */




/*********************  ulSERIAL_UI32in()  ***********************************
*
* do DECIMAL ulong number edited input.
*
*
*****************************************************************************/

ulong ulSERIAL_UI32in(
		void
		)
	{
	ulong ulValue;
	uchar ucStr[13];

	/* GET THE NUMERIC STRING */
	while(ucSERIAL_getEditedInput(ucStr, 13));

	/* CONVERT IT TO A LONG */
	ulValue = (ulong)lSERIAL_AsciiToNum(ucStr, UNSIGNED, DECIMAL);

	return(ulValue);

	}/* END: ulSERIAL_UI32in() */




/********************  vSERIAL_generic_IVout()  ******************************
*
*
*
*****************************************************************************/

static void vSERIAL_generic_IVout(
		ulong ulSignMaskBit,			//sign mask
		ulong ulNumericMask,			//numeric mask
		char cDesiredFldSize,			//desired output field size
		uchar ucLeadFillChar,			//leading zero fill char
		long lVal						//numeric value to show
		)
	{
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str(lVal, ulSignMaskBit, ulNumericMask, ucaValArray);

	vSERIAL_FormatStrout(cDesiredFldSize, ucLeadFillChar, ucaValArray);

	return;

	}/* END: vSERIAL_IV32out() */


/***********************  vSERIAL_HB6ByteOut()  ******************************
*
*
*
*****************************************************************************/

void vSERIAL_HB6ByteOut(
		uchar *ucpArrayPtr
		)
	{
	uchar ucc;

	for(ucc=0;  ucc<6;  ucc++)
		{
		vSERIAL_HB8out(*ucpArrayPtr);
		ucpArrayPtr++;
		}/* END: for() */

	return;
	
	}/* END: vSERIAL_HB6ByteOut() */






#ifdef SERIAL_Q_OUTPUT_ENABLED

/*********************  vSERIAL_pushQ()  **************************************
*
*
*
******************************************************************************/

static void vSERIAL_pushQ(
		uchar ucChar
		)
	{
	ulong ulWaitCount;


	/* CHK IF THE Q IS FULL */
	ulWaitCount = 10000000;
	while(ucQcount >= COMM_Q_SIZE)			//loop until there is space
		{
		ulWaitCount--;
		if(!ulWaitCount) return;			//timeout
		}


	/* Q IS NOT FULL -- STUFF THE CHAR ONTO THE Q */
	ucQonIdx_LUL++;							//inc idx
	if(ucQonIdx_LUL >= COMM_Q_SIZE) ucQonIdx_LUL = 0;
	ucaCommQ[ucQonIdx_LUL] = ucChar;		//stuff the char

	/* TURN INTS OFF TO WRITE THE COUNT */
	UCA0IE &= ~UCTXIE;						//go no-int
	ucQcount++;		  						//boost the count
	UCA0IE |= UCTXIE;						//go yes-int
	UCA0CTL1 &= ~UCSWRST;					//transmit enabled

	return;

	}/* END: vSERIAL_pushQ() */
#endif



/* -----------------------  END OF MODULE  --------------------------------- */

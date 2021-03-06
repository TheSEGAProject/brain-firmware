

/***************************  SERIAL.H  ****************************************
*
* Serial port header file
*
*
* V1.00 04/29/2002 wzr
*	started
*
******************************************************************************/

#ifndef SERIAL_H_INCLUDED
	#define SERIAL_H_INCLUDED


	#define CASE_INSENSITIVE	1
	#define CASE_SENSITIVE		0

	#define YES_CRLF			1
	#define  NO_CRLF			0

	#define YES_SHOW_ERR		1
	#define  NO_SHOW_ERR		0

	#define YES_TITLE	1
	#define NO_TITLE	0

	#define YES_HDR		1
	#define NO_HDR		0

	#define YES_SHOW_ALL_ENTRYS		1
	#define  NO_SHOW_ALL_ENTRYS		0

	#define YES_SHOW_ZRO_ENTRYS		1
	#define NO_SHOW_ZRO_ENTRYS		0

	#define BINARY_MODE_ON		1
	#define BINARY_MODE_OFF		0

	#define YES_SHOW_ABORT_MSG		1
	#define  NO_SHOW_ABORT_MSG		0


	#define SERIAL_Q_OUTPUT_ENABLED 1	//comment out this line to disable Q
										//ifdef is the way it is used

//	#define COMM_Q_SIZE	32	//must stay consistant with define in IRUPTLO.ASM
//	#define COMM_Q_SIZE	64  //must stay consistant with define in IRUPTLO.ASM
	#define COMM_Q_SIZE 48	//must stay consistant with define in IRUPTLO.ASM	

	#define COMM_Q_IDX_MASK	 (COMM_Q_SIZE -1)

	/* RADIX'S */
	#define RADIX10 10
	#define DECIMAL	10

	#define RADIX16 16
	#define HEX 16

	#define SIGNED 1
	#define UNSIGNED 0

	#define WIDTH2		2
	#define WIDTH3		3
	#define WIDTH4		4
	#define WIDTH5		5
	#define WIDTH6		6
	#define WIDTH7		7
	#define WIDTH8		8
	#define WIDTH9		9
	#define WIDTH10		10
	#define WIDTH11		11


//	Uses settings from users guide. Defines in config.h will allow for either 4MHz or 8MHz at baud-rate
//	of 19200. If you change this you will have to look up the new settings and make the change!

	#if (DESIRED_BAUD_RATE == 19200)
		//	Low-Freq Baud-Rate Generation used. 
		#if (SMCLK_RATE == 4194304)
		#define UCOS16_BIT_VALUE	0
		#define UCA0BR0_VALUE		0xDE
		#define UCA0BR1_VALUE		0x00
		#define UCBRF0_VALUE		0x00
		#define UCBRS0_VALUE		0x04
		#endif
		
		#if (SMCLK_RATE == 8388608)
		#define UCOS16_BIT_VALUE	0
		#define UCA0BR0_VALUE		0xB4
		#define UCA0BR1_VALUE		0x01
		#define UCBRF0_VALUE		0x00
		#define UCBRS0_VALUE		0x07
		#endif
	#endif

	/************************  DECLARATIONS  ********************************/


 void vSERIAL_init(				//init the Serial port
		void
		);

 void vSERIAL_quit(
		void
		);

 void vSERIAL_setBinaryMode(
 		uchar ucBinModeFlg	//BINARY_MODE_ON,  BINARY_MODE_OFF
		);

 void vSERIAL_bout(				//byte output
		uchar ucChar
		);

 void vSERIAL_repeatChar( //repeated output of a single char
		uchar ucChar,	//char to output
		char cCount		//number of times to output the char
		);

 void vSERIAL_dash(		//repeated output of dashes
		char cCount		//count if <= 0 then nothing output
		);

 void vSERIAL_spaces(	//repeated output of spaces
		char cCount		//count: if <= 0 then nothing output
		);

 char cSERIAL_rom_strlen(	//return the length of a rom string
		const char *cStrPtr		//lint !e125
		);

 void vSERIAL_crlf(				//carriage return line feed
		void
		);

 void vSERIAL_Hformat(	//hex formatting chars in leader "0x"
		void
		);

 uchar ucSERIAL_toupper(
		uchar ucChar
		);

 uchar ucSERIAL_isnum(			//RET: 1=is a number, 0=not a number
 		uchar ucChar			//char to check
		);

 void vSERIAL_rom_sout(			//string output
		const char *cStrPtr		//lint !e125 !e10
		);									//lint !e49

 void vSERIAL_padded_rom_sout(	//Output a space padded fixed length string
		const char *cStrPtr,	//lint !e125
		char cPadCount				//number of pad spaces (-=left, +=Right)
		);

 void vSERIAL_any_char_to_cont(
		void
		);

 void vSERIAL_showXXXmsg( //SHOWS: " XXX\r\n" 
		void
		);

 uchar ucSERIAL_confirm( //RET: 1=YES CONFIRMED, 0=NO NOT CONFIRMED
		uchar ucAbortTextingFlag //YES_SHOW_ABORT_MSG or NO_SHOW_ABORT_MSG
		);

 void vSERIAL_colTab( //Tab to a particular column (first col = 0)
		uchar ucColNum
		);

 uchar ucSERIAL_readColTab(	//RET: cur col number (first col = 0)
		void
		);

 uchar ucSERIAL_bin(
		void
		);

 uint uiSERIAL_timedBin(	// uiRET <= 255 is char, uiRET > 255 is Timeout
		uint uiMilliSec		//Time limit in mS
		);

 uchar ucSERIAL_kbhit(		//1=key was hit, 0=no key hit
		void
		);

 uchar ucSERIAL_getEditedInput(	//RET: 0=good, 1=aborted
		uchar ucStr[],			//ASCIZ str passed back-- CRLF = 0;
		uchar ucMaxCount		//max msg size -1 (extra byte 0 for CRLF)
		);

 long lSERIAL_AsciiToNum(
		uchar ucStr[],
		uchar ucSignFlag,	//UNSIGNED = 0, SIGNED = 1
		uchar ucRadix
		);

 void vSERIAL_printDashIntro(
		const char *cStrPtr		//lint !e125 !e10
		);

/**********************  4bit  ***********************************************/

 void vSERIAL_HB4out(			//hex nibble out
		uchar ucByte
		);


/**********************  8bit  ***********************************************/

 void vSERIAL_HB8out(	//HEX byte out
		uchar ucByte
		);

 void vSERIAL_HB8Fout(	//HEX formatted output (ie 0x55)
		uchar ucByte
		);

 void vSERIAL_UI8out(
		uchar ucVal
		);

 void vSERIAL_UI8_2char_out(	//2 char fixed width
		uchar ucVal,			//value
		uchar ucLeadFillChar	//leading fill char (ie. '0', or ' ')
		);

 void vSERIAL_UIV8out(
		uchar ucVal
		);

 void vSERIAL_I8out(
		char cVal
		);

 void vSERIAL_IV8out(
		char cVal
		);

/**********************  16bit  **********************************************/

 void vSERIAL_HB16out(
		uint uiInt
		);

 void vSERIAL_HBV16out(
		uint uiVal
		);

 void vSERIAL_HB16Fout(
		uint uiInt
		);

 void vSERIAL_UI16out(
		uint uiInt
		);

 void vSERIAL_UIV16out(
		uint uiVal
		);

 void vSERIAL_I16out(
		int iVal
		);

 void vSERIAL_IV16out(
		int iInt
		);

 void vSERIAL_I16outFormat(
		int iVal,
		char cFormatWidth
		);

#if 0
 uint uiSERIAL_UI16in(
		void
		);
#endif

/**********************  24bit  **********************************************/

 void vSERIAL_HB24out(
		USL uslB24	
		);

 void vSERIAL_HBV24out(
		usl uslVal
		);

 void vSERIAL_HB24Fout(
		USL uslB24	
		);

 void vSERIAL_UIV24out(
		USL uslVal
		);

/**********************  32bit  **********************************************/

 void vSERIAL_HB32out(
		unsigned long ulLong
		);

 void vSERIAL_HBV32out(
		unsigned long ulLong
		);

 void vSERIAL_HB32Fout(
		unsigned long ulLong
		);

 void vSERIAL_UI32out(
		unsigned long ulVal
		);

 void vSERIAL_UIV32out(
		unsigned long ulVal
		);

 void vSERIAL_I32out(
		long lVal
		);

 void vSERIAL_IV32out(
		long lVal
		);

 void vSERIAL_UI32MicroDecOut(
		long lVal
		);

 ulong ulSERIAL_H32in(
		void
		);

 long lSERIAL_I32in(
		void
		);

 unsigned long ulSERIAL_UI32in(
		void
		);


/***************************  6Byte  **************************************/

 void vSERIAL_HB6ByteOut(
		uchar *ucpArrayPtr
		);



#endif /* SERIAL_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */

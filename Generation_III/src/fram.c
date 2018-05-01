
/**************************  FRAM.C  *****************************************
*
* Routines to handle low level FRAM control.
*
*
* V1.00 10/14/2003 wzr
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
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */



#include <msp430x54x.h>		//processor reg description */
#include "STD.h"			//standard definitions
#include "CONFIG.h" 		//system configuration description file
#include "SERIAL.h" 		//serial IO port stuff
#include "SPI.h"			//SPI low level control routines
#include "FRAM.h"			//FRAM control routines
//#include "FULLDIAG.h"		//full blown diag package
#include "MSG.h"			//msg helper routines




/*************************  DECLARATIONS  ***********************************/

static void vFRAM_send_WE_cmd(
		void
		);

static uchar ucFRAM_bin(
		uint uiAddr
		);

static void vFRAM_bout(
		uint uiAddr,
		uchar ucData
		);







/******************  vFRAM_init()  *******************************************
*
* Routine to initialize the fram SPI for use
*
*
******************************************************************************/

void vFRAM_init(
		void
		)
	{

	/* SETUP THE SPI REGS */
	vSPI_master_init(SPI_MODE_0);

	return;

	}/* END: vFRAM_init() */




/******************  vFRAM_quit()  *******************************************
*
* Routine to shut off the Fram
*
*
******************************************************************************/

void vFRAM_quit(
		void
		)
	{

	/* TURN OFF THE SPI REGS */
	vSPI_master_quit();

	/* SELECT THE FRAM CHIP */
	FRAM_SEL_OUT_PORT |= FRAM_SEL_BIT;					//de-select FRAM

	return;

	}/* END: vFRAM_quit() */




/****************** ucFRAM_read_sts_reg()  ***********************************
*
* Return the status byte from the FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
******************************************************************************/

uchar ucFRAM_read_sts_reg(
		void
		)
	{
	uchar ucData;

	FRAM_SEL_OUT_PORT &= ~FRAM_SEL_BIT;	//select the chip

	vSPI_bout(FRAM_READ_STS_REG_CMD); 	//send the read ststus cmd
	ucData = ucSPI_bin();				//get the data
	
	/* DROP FRAM CHIP SELECT FOR NEXT CMD */
	FRAM_SEL_OUT_PORT |= FRAM_SEL_BIT;						//deselect chip

	return(ucData);

	}/* END: ucFRAM_read_sts_reg() */





/****************** vFRAM_send_WE_cmd()  ***********************************
*
* Return the status byte from the FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
******************************************************************************/

static void vFRAM_send_WE_cmd(
		void
		)
	{

	FRAM_SEL_OUT_PORT &= ~FRAM_SEL_BIT;					//select the chip

	vSPI_bout(FRAM_WRITE_ENA_CMD);		//send write enable cmd
	
	FRAM_SEL_OUT_PORT |= FRAM_SEL_BIT;						//deselect chip

	return;

	}/* END: vFRAM_send_WE_cmd() */





/****************** ucFRAM_bin()  **************************************
*
* Byte input from fram
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
******************************************************************************/

static uchar ucFRAM_bin(
		uint uiAddr
		)
	{
	uchar ucData;
	uchar ucAddrHI;
	uchar ucAddrLO;

	ucAddrHI = (uchar)((uiAddr & 0x1FFF)>>8);
	ucAddrLO = (uchar)(uiAddr & 0xFF);

	FRAM_SEL_OUT_PORT &= ~FRAM_SEL_BIT;							//select the chip

	if(ucAddrHI != 0)
		vSPI_bout(FRAM_READ_DATA_CMD | 0x08);	//CMD set HI addr bit
	else
		vSPI_bout(FRAM_READ_DATA_CMD);			//CMD

//	vSPI_bout(ucAddrHI);						//HI addr
	vSPI_bout(ucAddrLO);						//LO addr
	ucData = ucSPI_bin();						//Read the data
	
	/* DROP FRAM CHIP SELECT FOR NEXT CMD */
	FRAM_SEL_OUT_PORT |= FRAM_SEL_BIT;								//deselect chip

	return(ucData);

	}/* END: ucFRAM_bin() */






/****************** vFRAM_bout()  ********************************************
*
* byte output to FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
******************************************************************************/

static void vFRAM_bout(
		uint uiAddr,
		uchar ucData
		)
	{
	uchar ucAddrHI;
	uchar ucAddrLO;

	ucAddrHI = (uchar)((uiAddr & 0x1FFF)>>8);
	ucAddrLO = (uchar)(uiAddr & 0xFF);

	vFRAM_send_WE_cmd();				//turn on write enable

	FRAM_SEL_OUT_PORT &= ~FRAM_SEL_BIT;					//select the chip

	if(ucAddrHI != 0)
		vSPI_bout(FRAM_WRITE_DATA_CMD | 0x08);		//CMD or'ed with HI addr bit
	else
		vSPI_bout(FRAM_WRITE_DATA_CMD);		//CMD with zro HI addr bit
	vSPI_bout(ucAddrLO);					//LO addr
	vSPI_bout(ucData);						//data value
	
	/* DROP FRAM CHIP SELECT FOR NEXT CMD */
	FRAM_SEL_OUT_PORT |= FRAM_SEL_BIT;								//deselect chip

	#if 0
	vSERIAL_rom_sout("FramWrit: Addr=");
	vSERIAL_HB8out(ucAddrHI);
	vSERIAL_HB8out(ucAddrLO);
	vSERIAL_rom_sout("  Val=");
	vSERIAL_HB8out(ucData);
	vSERIAL_crlf();
	#endif

	return;

	}/* END: vFRAM_bout() */







/****************** ucFRAM_read_B8()  **************************************
*
* Return the data at a specified address
*
* NOTE: This routine turn on the SPI bus on entry and off on exit
*
******************************************************************************/

uchar ucFRAM_read_B8(
		uint uiAddr
		)
	{
	uchar ucData;

	vFRAM_init();

	ucData = ucFRAM_bin(uiAddr);

	vFRAM_quit();

	return(ucData);

	}/* END: ucFRAM_read_B8() */








/****************** vFRAM_write_B8()  **************************************
*
* Write a byte of data at a specific addr to FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
******************************************************************************/

void vFRAM_write_B8(
		uint uiAddr,
		uchar ucData
		)
	{

	vFRAM_init();

	vFRAM_bout(uiAddr,ucData);

	vFRAM_quit();

	return;

	}/* END: vFRAM_write_B8() */







/***********************  uiFRAM_read_B16  ***********************************
*
* Read a Word from the FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
*****************************************************************************/

uint uiFRAM_read_B16(
		uint uiAddr
		)
	{
	uchar ucHI_val;
	uchar ucLO_val;
	uint uiTmp;

	vFRAM_init();

	/* GET THE HI BYTE VALUE */
	ucHI_val = ucFRAM_bin(uiAddr);
	
	/* GET THE LOW BYTE VALUE */
	uiAddr++;
	ucLO_val = ucFRAM_bin(uiAddr);

	uiTmp = (uint)ucHI_val;
	uiTmp =  uiTmp << 8;
	uiTmp |= (uint) ucLO_val;

	vFRAM_quit();

	return(uiTmp);

	}/* END: uiFRAM_read_B16() */






/***********************  vFRAM_write_B16  ***********************************
*
* Write a 16 bit value to the FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
*****************************************************************************/

void vFRAM_write_B16(
		uint uiAddr,
		uint uiData
		)
	{
	uchar ucVal;


	vFRAM_init();

	/* WRITE THE HI BYTE VALUE */
	ucVal = ((uchar) (uiData >> 8));
	vFRAM_bout(uiAddr, ucVal);
	
	/* INC THE ADDR PTR */
	uiAddr++;

	/* WRITE THE LOW BYTE VALUE */
	ucVal = ((uchar) uiData);
	vFRAM_bout(uiAddr, ucVal);

	vFRAM_quit();

	return;

	}/* END: vFRAM_write_B16() */








/***********************  uiFRAM_read_B24  **********************************
*
* This will change! or be deleted - only seems to be called in diag
* 
* Read a 24bit word from the FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
*****************************************************************************/
/*
USL uslFRAM_read_B24(		
		uint uiAddr
		)		
	{
	uchar ucHI_val;
	uchar ucMD_val;
	uchar ucLO_val;
	USL uslTmp;	

	vFRAM_init();

	// GET THE HI BYTE VALUE //
	ucHI_val = ucFRAM_bin(uiAddr);
	
	// GET THE MID BYTE VALUE //
	uiAddr++;
	ucMD_val = ucFRAM_bin(uiAddr);

	// GET THE LOW BYTE VALUE //
	uiAddr++;
	ucLO_val = ucFRAM_bin(uiAddr);

	uslTmp = ucHI_val;
	uslTmp <<= 8;
	uslTmp |= (USL) ucMD_val; 
	uslTmp <<= 8;
	uslTmp |= (USL) ucLO_val; 

	vFRAM_quit();

	return(uslTmp);

	}// END: uslFRAM_read_B24() //


*/



/***********************  vFRAM_write_B24  ***********************************
*
* This will change! or be deleted - only used in diag
* 
* Write a 24 bit value to the FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
*****************************************************************************/
/*
void vFRAM_write_B24(
		uint uiAddr,
		USL uslData		
		)
	{
	uchar ucVal;

	vFRAM_init();

	// WRITE THE HI BYTE VALUE //
	ucVal = ((uchar) (uslData >> 16));
	vFRAM_bout(uiAddr, ucVal);
	
	// WRITE THE MD BYTE VALUE //
	uiAddr++;
	ucVal = ((uchar) (uslData >> 8));
	vFRAM_bout(uiAddr, ucVal);

	// WRITE THE LOW BYTE VALUE //
	uiAddr++;
	ucVal = ((uchar) uslData);
	vFRAM_bout(uiAddr, ucVal);

	vFRAM_quit();

	return;

	}// END: vFRAM_write_B24() //

*/






/***********************  uiFRAM_read_B32  **********************************
*
* Read a 32bit word from the FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
*****************************************************************************/

ulong ulFRAM_read_B32(
		uint uiAddr
		)
	{
	uchar ucXI_val;
	uchar ucHI_val;
	uchar ucMD_val;
	uchar ucLO_val;
	ulong ulTmp;

	vFRAM_init();

	/* GET THE XI BYTE VALUE */
	ucXI_val = ucFRAM_bin(uiAddr);

	/* GET THE HI BYTE VALUE */
	uiAddr++;
	ucHI_val = ucFRAM_bin(uiAddr);
	
	/* GET THE MID BYTE VALUE */
	uiAddr++;
	ucMD_val = ucFRAM_bin(uiAddr);

	/* GET THE LOW BYTE VALUE */
	uiAddr++;
	ucLO_val = ucFRAM_bin(uiAddr);

	ulTmp = ucXI_val;
	ulTmp <<= 8;
	ulTmp |= (ulong) ucHI_val;
	ulTmp <<= 8;
	ulTmp |= (ulong) ucMD_val;
	ulTmp <<= 8;
	ulTmp |= (ulong) ucLO_val;

	vFRAM_quit();

	return(ulTmp);

	}/* END: uslFRAM_read_B32() */






/***********************  vFRAM_write_B32  ***********************************
*
* Write a 32 bit value to the FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
*****************************************************************************/

void vFRAM_write_B32(
		uint uiAddr,
		ulong ulData
		)
	{
	uchar ucVal;

	vFRAM_init();

	/* WRITE THE XI BYTE VALUE */
	ucVal = ((uchar) (ulData >> 24));
	vFRAM_bout(uiAddr, ucVal);

	/* WRITE THE HI BYTE VALUE */
	uiAddr++;								//inc addr
	ucVal = ((uchar) (ulData >> 16));
	vFRAM_bout(uiAddr, ucVal);
	
	/* WRITE THE MD BYTE VALUE */
	uiAddr++;								//inc addr
	ucVal = ((uchar) (ulData >> 8));
	vFRAM_bout(uiAddr, ucVal);

	/* WRITE THE LOW BYTE VALUE */
	uiAddr++;								//inc addr
	ucVal = ((uchar) ulData);
	vFRAM_bout(uiAddr, ucVal);

	vFRAM_quit();

	return;

	}/* END: vFRAM_write_B32() */





/*******************  vFRAM_fillFramBlk()  *******************************
*
* This is a block memory setter.  It is used to fill blocks of memory with
* a specified value.
*
*****************************************************************************/

void vFRAM_fillFramBlk(
		uint uiStartAddr,
		uint uiCount,
		uchar ucSetVal
		)
	{
	uint uiCurAddr;

	for(uiCurAddr = uiStartAddr;
		uiCurAddr<(uiStartAddr + uiCount);
		uiCurAddr++
	   )
		{
		vFRAM_write_B8(uiCurAddr, ucSetVal);

		}/* END: for(uiCurAddr) */

	return;

	}/* END: vFRAM_fillFramBlk() */
	






/*******************  ucFRAM_chk_fram_blk()  *******************************
*
* This is a block memory set checker.  It is used to check blks of mem for
* a specified value.
*
* RET:	1 = ok
*		0 = err
*
*****************************************************************************/

uchar ucFRAM_chk_fram_blk(
		uint uiStartAddr,
		uint uiCount,
		uchar ucSetVal
		)
	{
	uint uiCurAddr;
	uchar ucReadVal;
	uchar ucRetVal;

	ucRetVal = 1;						//assume FRAM works good return

	for(uiCurAddr = uiStartAddr;
		uiCurAddr<(uiStartAddr + uiCount);
		uiCurAddr++
	   )
		{
		ucReadVal = ucFRAM_read_B8(uiCurAddr);
		if(ucReadVal != ucSetVal)
			{
			vMSG_showStorageErr(
				"FRAM byte fail ",
				(ulong)uiCurAddr,
				(ulong)ucSetVal,
				(ulong)ucReadVal
				);
			ucRetVal = 0;
			}

		}/* END: for(uiCurAddr) */

	return(ucRetVal);

	}/* END: ucFRAM_chk_fram_blk() */






/**********************  vFRAM_show_fram()  **********************************
*
*
******************************************************************************/

void vFRAM_show_fram(
		uint uiStartAddr,
		uint uiCount
		)
	{
	uint uiCurAddr;
	uchar ucVal;

	vSERIAL_rom_sout("Fram[");
	vSERIAL_HB16out(uiStartAddr);
	vSERIAL_rom_sout("]=");
	
	for(uiCurAddr = uiStartAddr;
		uiCurAddr<(uiStartAddr + uiCount);
		uiCurAddr++
	   )
		{
		ucVal = ucFRAM_read_B8(uiCurAddr);

		vSERIAL_HB8out(ucVal);
		vSERIAL_bout(',');

		}/* END: for(uiCurAddr) */

	vSERIAL_crlf();

	return;

	}/* END: vFRAM_show_fram() */




/*-------------------------------  MODULE END  ------------------------------*/

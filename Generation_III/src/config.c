
/**************************  CONFIG.C  ***************************************
*
* Config register module 
* 
* This has not been ported and would need to change! if later implemented.
*
* NOTE: This module is currently not used in the runtime code.
* 		It has been saved here is because it took a number of hours to 
*		discover how to read & write the config registers without compiler
*		and	programmer errors, so it is here to help the next soul who passes
*		this way.
*
*		The read routine has been debugged and works well for both flash
*		and config memory.
*
*		The write routine has been debugged and work only for config memory.
*
*
* V1.00 10/15/2006 wzr
*		Started
*
*****************************************************************************/

/*lint -e526 */		/* function not defined */
/*lint -e563 */		/* label not referencecd */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e716 */		/* while(1) ... */
/*lint -e750 */		/* local macro not referenced */
/*lint -e752 */		/* local declarator not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


//#include "std.h"			//std defines
//#include "config.h"  		//system configuration definitions
//#include "P18f8720.h"		//register and ram definition file
//#include "delay.h"  		//approx delay routine
//#include "serial.h"  		//serial port



// *******************************  CODE  **************************************
/*

#if 0
		ucConfigVal = ucCONFIG_read_config_reg(0x300001);
		vSERIAL_rom_sout("Osc=");
		vSERIAL_HB8out(ucConfigVal);
		vSERIAL_crlf();

		ucConfigVal = ucCONFIG_read_config_reg(0x300002);
		vSERIAL_rom_sout("BorConfg=");
		vSERIAL_HB8out(ucConfigVal);
		vSERIAL_crlf();

		ucConfigVal = ucCONFIG_read_config_reg(0x300008);
		vSERIAL_rom_sout("CodeProtLO=");
		vSERIAL_HB8out(ucConfigVal);
		vSERIAL_crlf();

		ucConfigVal = ucCONFIG_read_config_reg(0x300009);
		vSERIAL_rom_sout("CodeProtHI=");
		vSERIAL_HB8out(ucConfigVal);
		vSERIAL_crlf();

		ucConfigVal = ucCONFIG_read_config_reg(0x3FFFFE);
		vSERIAL_rom_sout("ID_HI=");
		vSERIAL_HB8out(ucConfigVal);
		vSERIAL_crlf();

		ucConfigVal = ucCONFIG_read_config_reg(0x3FFFFF);
		vSERIAL_rom_sout("ID_LO=");
		vSERIAL_HB8out(ucConfigVal);
		vSERIAL_crlf();
#endif


*/




/********************  ucCONFIG_read_config_reg()  ***************************
*
* RET: byte of config reg
*
******************************************************************************

uchar ucCONFIG_read_config_reg(
		usl uslAddr
		)
	{
	if(uslAddr < 0x020000)			//flash mem access
		{
		EECON1bits.EEPGD = 1;
		EECON1bits.CFGS = 0;
		}

	if(uslAddr > 0x300000)			//config mem access
		{
		EECON1bits.EEPGD = 1;
		EECON1bits.CFGS = 1;
		}

	TBLPTR = uslAddr;				//setup the addr

	_asm
	  tblrd							//lint !e40 !e522
	_endasm;

	return(TABLAT);

	} // END: vCONFIG_read_config_reg() 

*/





/********************  vCONFIG_write_config_reg()  ***************************
*
*
*
******************************************************************************


void vCONFIG_write_config_reg(
		usl uslAddr,
		uchar ucDataVal
		)
	{
	uchar ucIntsOnFlg;

	TBLPTR = uslAddr;				//setup addr
	TABLAT = ucDataVal;				//setup data val

	_asm
	tblwt							//lint !e40 !e522 !e10
	_endasm;

	EECON1bits.EEPGD = 1;			//flash memory
	EECON1bits.CFGS = 1;			//config regs
	EECON1bits.WREN = 1;			//write enable

	ucIntsOnFlg = 0;				//assume no ints
	if(INTCONbits.GIEH == 1)
		{
		INTCONbits.GIEH = 0;		//go no-int
		INTCONbits.GIEH = 0;		//go no-int
		INTCONbits.GIEL = 0;		//go no-int
		INTCONbits.GIEL = 0;		//go no-int
		ucIntsOnFlg = 1;			//flag ints on
		}

	EECON2 = 0x55;
	EECON2 = 0xAA;
	EECON1bits.WR = 1;
	Nop();

	EECON1bits.WREN = 0;			//write disable

	if(ucIntsOnFlg)					//re-enable ints
		{
		INTCONbits.GIEH = 1;		//go yes-int
		INTCONbits.GIEL = 1;		//go yes-int
		}

	return;

	} // END: vCONFIG_write_config_reg()

*/




// -----------------------  END OF MODULE  --------------------------------- 



/***************************  ROM.H  ****************************************
*
* ROM configuration parameters:
*	1. ROM Serial Number 
*	2. ROM Config Flags
*
* V1.00 12/11/2003 wzr
*	started
*
******************************************************************************/

#ifndef ROM_H_INCLUDED
 #define ROM_H_INCLUDED

 /* BUILD VALUES */

// #define MASTER 1
// #define SLAVE  0

// #define CONFIG_MASTER_MASK_UC ((uchar) 0x01)



// #define BLANK_SN_VAL		0x0000



 unsigned char ucROM_getRomConfigByte(
		void
		);

 uint uiROM_getRomConfigSnumAsUint(
		void
		);


#endif /* ROM_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */


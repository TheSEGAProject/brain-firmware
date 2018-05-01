
/***************************  STD.H  ****************************************
*
* STD port header file
*
*
* V1.00 04/29/2002 wzr
*	started
*
******************************************************************************/

#ifndef STD_H_INCLUDED
	#define STD_H_INCLUDED

	#define TRUE 1
	#define FALSE 0

	#ifndef YES
	  #define YES 1
	#endif
	#ifndef NO
	  #define NO 0
	#endif

	#define ON 1
	#define OFF 0

	#define NONE 0

	#define YES_INITQUIT 1
	#define NO_INITQUIT 0

	typedef unsigned long USL; // Formerly was unsigned short long, now unsigned long
	typedef unsigned long usl; // Formerly was unsigned short long, now unsigned long

	typedef unsigned char UCHAR;
	typedef unsigned char uchar;

	typedef unsigned short USHORT;
	typedef unsigned short ushort;

	typedef unsigned long ULONG;
	typedef unsigned long ulong;

	typedef unsigned int  UINT;
	typedef unsigned int  uint;

#endif /* STD_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */

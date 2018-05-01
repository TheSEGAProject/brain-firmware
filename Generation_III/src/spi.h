
/***************************  SPI.H  ****************************************
*
* Header for SPI routine pkg
*
*
*
* V1.00 5/11/2003 wzr
*	started
*
******************************************************************************/

#ifndef SPI_H_INCLUDED
	#define SPI_H_INCLUDED


	#define SPI_MODE_0		0
	#define SPI_MODE_1		1
	#define SPI_MODE_2		2
	#define SPI_MODE_3		3


	/* ROUTINE DEFINITIONS */

	void vSPI_master_init(
		unsigned char ucMode
		);

	void vSPI_master_quit(
		void
		);

	unsigned char ucSPI_IO_Transaction(
		unsigned char ucOutByte
		);

	unsigned char ucSPI_bin(
		void
		);

	void vSPI_bout(
		unsigned char ucOutByte
		);


#endif /* SPI_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */

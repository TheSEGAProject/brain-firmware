
/***************************  SDSPI.H  ****************************************
*
* Header for SDSPI routine pkg
*
*
* Will likely need change! to make room for new mini SD's
*
*
* V1.00 6/11/2004 wzr
*	started
*
******************************************************************************/

#ifndef SDSPI_H_INCLUDED
	#define SDSPI_H_INCLUDED

	#define MAX_SDSPI_SIZE 16
	#define MAX_SDSPI_SIZE_L ((long) MAX_SDSPI_SIZE)
	#define MAX_SDSPI_SIZE_MASK  (MAX_SDSPI_SIZE -1)

	/* ROUTINE DEFINITIONS */

	void vSDSPI_init(
		void
		);

	void vSDSPI_quit(
		void
		);

	unsigned char ucSDSPI_wait_for_slave_ready(
		unsigned long ulWaitTime
		);

	unsigned char ucSDSPI_bin(
		void
		);

	void vSDSPI_bout(
		unsigned char ucOutByte
		);

	void vSDSPI_rom_sout(
		const char *cStrPtr		//lint !e125
		);

#endif /* SDSPI_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */


/***************************  ESPORT.H  ****************************************
*
* This code is no longer used and has not been ported to the MSP430
* 
* Header for ESPORT routine pkg
* 
* V1.00 7/20/2005 wzr
*	started
*
******************************************************************************

#ifndef ESPORT_H_INCLUDED
  #define ESPORT_H_INCLUDED


 //Values for the USE compilation switch
 #define USING_BRAIN_COM2		0
 #define USING_BRAIN_RF567		1
 #define USING_SD_PWM			2


 #define REDIRECT_TO_ESPORT		1
 #define REDIRECT_TO_SERIAL		2


  // ROUTINE DEFINITIONS 
  void vESPORT_init(
		void
		);

 void vESPORT_quit(
		void
		);

 uchar ucESPORT_bin(
		void
		);

 void vESPORT_bout(
		uchar ucByte
		);
  
 void vESPORT_redirectOutput(
		uchar ucRedirectionFlg //REDIRECT_TO_ESPORT, REDIRECT_TO_SERIAL
		);


#endif // ESPORT_H_INCLUDED 

/* --------------------------  END of MODULE  ------------------------------- */

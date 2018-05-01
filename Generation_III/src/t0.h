
/***************************  T0.H  ****************************************
*
* Header for T0 routine pkg
*
*
* V1.00 11/02/2005 wzr
*	started
*
******************************************************************************/


#ifndef T0_H_INCLUDED
	#define T0_H_INCLUDED

	// the following values have been calculated to use 32KHz/4 as the T0 clock
 #define T0_TICS_PER_SEC_UI ((uint)	8192)

 #define T0_nS_PER_TIC_UL		122070UL
 #define T0_uS_X_100_PER_TIC_UL	12207UL
 #define T0_uS_PER_TIC_UL		122UL

 #define RATIO_LTICS_TO_T0TICS	4UL





 void vT0_init(
		void
		);

 void vT0_quit(
		void
		);

 void vT0_start_T0_timer(
 		void
		);

 void vT0_continue_T0_timer(
		void
		);

 void vT0_stop_T0_timer(
		void
		);

 uint uiT0_get_val_in_tics(
		void
		);

 ulong ulT0_get_val_in_Ltics(
 		void
		);

 ulong ulT0_get_val_in_uS(
		void
		);

 void vT0_set_val(
		uint uiVal
		);

 void vT0_clr_val(
		void
		);

 void vT0_show_T0_in_uS(
		void
		);

#endif /* T0_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */



/***************************  CONFIG.H  ****************************************
*
* CONFIG port header file
*
*
* V1.00 04/29/2002 wzr
*	started
*
******************************************************************************/

#ifndef CONFIG_H_INCLUDED
	#define CONFIG_H_INCLUDED



/********************  SRAM DELCARATIONS  ************************************/

#define MAX_SRAM_ADDR_UL 0x3FFFF		//256K


/********************  FLASH DECLARATIONS  ***********************************/

#define FLASH_MAX_SIZE_IN_PAGES_I	4096
#define FLASH_MAX_SIZE_IN_PAGES_L	((long)FLASH_MAX_SIZE_IN_PAGES_I)

#define FLASH_BYTES_PER_PAGE_L		512



/********************  SYS CLK DECLARATIONS  *********************************/

#define MCLK_RATE 16777216L						// MCLK - Master Clock
#define SMCLK_RATE  4194304L         			// SMCLK - 4MHz
//#define SMCLK_RATE 8388608L					// SMCLK - 8MHZ

#ifndef SMCLK_RATE
	#error "SMCLK_RATE not defined"
#endif

#define DCORSEL_16MHZ           DCORSEL_5
#define DCO_MULT_16MHZ          512
#define VCORE_16MHZ             PMMCOREV_2


/********************* T1 CLK TIMER DECLARATIONS  *****************************/
/* NOTE: uses LTICS (for clarity) */

#define CLK_nS_PER_LTIC_L        30518L
#define CLK_nS_PER_LTIC_UL       30518UL
#define CLK_uS_PER_LTIC_UL          31UL
#define CLK_uS_x_100_PER_LTIC_UL  3152UL



/*********************  BAUD RATE DECLARATIONS  ******************************/

//#define DESIRED_BAUD_RATE 9600
#define DESIRED_BAUD_RATE 19200

#ifndef DESIRED_BAUD_RATE
	#error "DESIRED_BAUD_RATE not defined"
#endif

/*
#if 0
uchar ucCONFIG_read_config_reg(
		usl uslConfigAddr
		);

void vCONFIG_write_config_reg(
		usl uslConfigAddr,
		uchar ucByteVal
		);
#endif
*/

/*************************  PORT VALUE DECLARATIONS  *************************/

	

// Current with Port Map Rev 1.4

	#define PAOUT_INIT_VAL	0x0000
	#define PBOUT_INIT_VAL	0xAA08
	#define PCOUT_INIT_VAL	0x000A
	#define PDOUT_INIT_VAL	0x0028
	#define PEOUT_INIT_VAL	0x0000
	#define PFOUT_INIT_VAL	0x0000
	
	#define PADIR_INIT_VAL	0xC030
	#define PBDIR_INIT_VAL	0xFFDF
	#define PCDIR_INIT_VAL	0x3FBF
	#define PDDIR_INIT_VAL	0x00FC
	#define PEDIR_INIT_VAL	0xFFFF
	#define PFDIR_INIT_VAL	0x0007
	
	#define PAREN_INIT_VAL	0x1000
	#define PBREN_INIT_VAL	0x0000
	#define PCREN_INIT_VAL	0x0000
	#define PDREN_INIT_VAL	0x0000
	#define PEREN_INIT_VAL	0x0000
	#define PFREN_INIT_VAL	0x0000
	
	#define PASEL_INIT_VAL	0x0030
	#define PBSEL_INIT_VAL	0x00F0
	#define PCSEL_INIT_VAL	0x0030
	#define PDSEL_INIT_VAL	0x0003
	#define PESEL_INIT_VAL	0x0000
	#define PFSEL_INIT_VAL	0x0000
	
	#define PAIE_INIT_VAL	0x6FEF
	
	#define PAIES_INIT_VAL	0x0000
	
	// LED defines
	#define LED_BIT			(0x80)				// bit 7
	#define LED_PORT		(P7OUT)				// port 7
	
	// Define for the external power bit
	#define POWER_SRC_BIT	(P2IN & 0x20)		// bit 5 port 2
	
	// Button defines
	#define BUTTON_BIT			(0x10)			// BIT 4
	#define BUTTON_DIR_PORT		(P2DIR)			// PORT 2 DIR
	#define BUTTON_OUT_PORT		(P2OUT)			// PORT 2 OUT
	#define BUTTON_IFG_PORT		(P2IFG)			// PORT 2 INT FLAG
	#define BUTTON_IE_PORT		(P2IE)			// PORT 2 INT EN
	#define BUTTON_IN_PORT		(P2IN)			// PORT 2 IN (read only)
	
	// Subsecond timer defines
	#define SUB_SEC_TIM_CTL		(TA1CTL)
	#define SUB_SEC_TIM			(TA1R)
	#define SUB_SEC_TIM_H		(TA1R_H)
	#define SUB_SEC_TIM_L		(TA1R_L)
	#define SUB_SEC_CTL_INIT 	(0x0112)
	
	#define ENDSLOT_TIM_CTL		(TB0CTL)
	#define ENDSLOT_TIM			(TB0R)
	#define ENDSLOT_INTFLG_BIT	(TBIFG)
	#define ENDSLOT_INTEN_BIT	(TBIE)
	
	// SRAM defines
	#define SRAM_ADDR_LO_PORT	(PEOUT)			// PORT E (PORTS 9 & 10)
	#define	SRAM_ADDR_HI_PORT	(P3OUT)			// PORT 3
	#define	SRAM_ADDR_HI_2BIT	(0x03)			// BITS 0 & 1
	#define SRAM_SELRW_PORT		(P3OUT)			// PORT 3
	#define SRAM_CHIP_SELECT	(0x04)			// BIT 2
	#define SRAM_READWRITE		(0x08)			// BIT 3
	#define SRAM_DATA_DIR_PORT	(P8DIR)			// PORT 8 DIR
	#define SRAM_DATA_OUT_PORT	(P8OUT)			// PORT 8 OUT
	#define SRAM_DATA_IN_PORT	(P8IN)			// PORT 8 IN (read only)
	
	// Buzzer defines
	#define BUZZER_OUT_PORT		(P1OUT)			// PORT 1 OUT
	#define BUZZER_DIR_PORT		(P1DIR)			// PORT 1 DIR
	#define BUZZER_BIT			(0x10)			// BIT 4
	
	// Flash and FRAM defines
	#define FRAM_SEL_BIT		(0x01)			// BIT 0
	#define FRAM_SEL_OUT_PORT	(P11OUT)		// PORT 11 OUT
	#define FLASH_SEL_BIT		(0x02)			// BIT 1
	#define FLASH_SEL_OUT_PORT	(P11OUT)		// PORT 11 OUT
	
	// SPI uses USCI B1 - these should not be changed unless pin-out/USCI changes
	#define	SPI_OUT_BIT			(0x80)			// BIT 7
	#define SPI_IN_BIT			(0x10)			// BIT 4
	#define SPI_CLK_BIT			(0x20)			// BIT 5
	#define SPI_OUT_DIR_PORT	(P3DIR)			// PORT 3 DIR
	#define SPI_OUT_SEL_PORT	(P3SEL)			// PORT 3 SEL
	#define SPI_INCLK_DIR_PORT	(P5DIR)			// PORT 5 DIR
	#define SPI_INCLK_SEL_PORT	(P5SEL)			// PORT 5 SEL
	#define SPI_CTL0			(UCB1CTL0)
	#define SPI_CTL1			(UCB1CTL1)
	#define SPI_CLK_DIVIDER		(UCB1BRW)
	#define SPI_INTFLAG_REG		(UCB1IFG)
	#define SPI_TX_BUF			(UCB1TXBUF)
	#define SPI_RX_BUF			(UCB1RXBUF)
	
	// T0 debug timer defines
	#define T0_CTL				(TB0CTL)		// Timer B0
	#define T0_COUNTER			(TB0R)
	#define T0_DIV_EX_CTL		(TB0EX0)
	#define T0_INT_FLG_BIT		(0x0001)
	
	// ADC channel defines
	#define ADC_INPUTS_PORT6	(0x01)			// using only BIT 0 
	#define ADC_INPUTS_PORT7	(0x00)			// None used
	#define BAT_SEN_EN_BIT		(0x40)
	#define BAT_VIN_BIT			(0x01)
	#define BAT_SEN_EN_PORT_DIR	(P7OUT)
	#define BAT_SEN_EN_PORT_OUT	(P7OUT)
	#define BAT_VIN_PORT_DIR	(P6DIR)
	

#endif /* CONFIG_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */

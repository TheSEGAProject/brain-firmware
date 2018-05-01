//!
//! \file radio.c
//! \brief Main code file for ADF7020 Radio functions
//!

//  by: Kenji Yamamoto & Matthew Wyant
//      Wireless Networks Research Lab
//      Northern Arizona University, 2009

//	This code has not been ported. It has only been gutted and pretends to function.
//	change! needed

//! \addtogroup Radio
//! \{

////////////////////////////////////////////////////////////////////////////////
////////////////////  DEFINES  /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//! \def RADIO_DEBUG
//! \brief Turns on (1) or off (0) debug messages for the radio module
#define RADIO_DEBUG 1

//! \name Radio Setup Defines
//! Defines to setup radio.
//! \{
  //! \def FRAC_N_START_TX
  //! \brief Unknown TODO
  #define FRAC_N_START_TX         0x47D1
  //! \def FRAC_N_START_RX
  //! \brief Unknown TODO
  #define FRAC_N_START_RX         0x4580
  //! \def N_START
  //! \brief Unknown TODO
  #define N_START                 0x0051
  //! \def FRAC_N_CHANNEL_STEP
  //! \brief Unknown TODO
  #define FRAC_N_CHANNEL_STEP     0x0128
  
  //! \def FRAC_N_MASK
  //! \brief Unknown TODO
  #define FRAC_N_MASK             0x0007FFF0
  //! \def FRAC_N_SHIFT
  //! \brief Unknown TODO
  #define FRAC_N_SHIFT            4

  //! \def N_MASK
  //! \brief Unknown TODO
  #define N_MASK                  0x07F80000
  //! \def N_SHIFT
  //! \brief Unknown TODO
  #define N_SHIFT                 19

  //! \def REGISTER0_CHAN0_RX
  //! \brief N-Register settings for RX
  #define REGISTER0_CHAN0_RX      0x7A8C5800
  //! \def REGISTER0_CHAN0_TX
  //! \brief N-Register settings for TX
  #define REGISTER0_CHAN0_TX      0x728C7D10

  //! \def REGISTER2_RX
  //! \brief Transmit Modulation Settings for RX
  #define REGISTER2_RX            0x8007E012
  //! \def REGISTER2_TX
  //! \brief Transmit Modulation Settings for TX
  #define REGISTER2_TX            0xC0077E12

  //! \def REGISTER1_VALUE
  //! \brief Oscillator/Filter Settings
  #define REGISTER1_VALUE         0x000A9011
  //! \def REGISTER3_VALUE
  //! \brief Reciever Clock Settings
  #define REGISTER3_VALUE         0x006F2453
  //! \def REGISTER4_VALUE
  //! \brief Demodulator Settings
  #define REGISTER4_VALUE         0x01000154
  //! \def REGISTER5_VALUE
  //! \brief Sync Byte Settings
  #define REGISTER5_VALUE         0x12345675
  //! \def REGISTER6_VALUE
  //! \brief Correlator/Demodulator Settings
  #define REGISTER6_VALUE         0x0DD81146
  //! \def REGISTER7_VALUE
  //! \brief Readback Settings
  #define REGISTER7_VALUE     
  //! \def REGISTER8_VALUE
  //! \brief Power-Down Test Settings
  #define REGISTER8_VALUE     
  //! \def REGISTER9_VALUE
  //! \brief AGC1 Settings
  #define REGISTER9_VALUE         0x00B231E9
  //! \def REGISTERA_VALUE
  //! \brief AGC2 Settings
  #define REGISTERA_VALUE     
  //! \def REGISTERB_VALUE
  //! \brief AFC Settings
  #define REGISTERB_VALUE         0x00102F7B
  //! \def REGISTERC_VALUE
  //! \brief Test Settings
  #define REGISTERC_VALUE     
  //! \def REGISTERD_VALUE
  //! \brief Offset Removal and Signal Gain Settings
  #define REGISTERD_VALUE     

  //! \def PA_MASK
  //! \brief Unknown TODO
  #define PA_MASK                 0x00007E00
  //! \def PA_SHIFT
  //! \brief Unknown TODO
  #define PA_SHIFT                9
//! \}

//! \name Temporary Variable Convienience Defines
//! Rename the temp variables to make the code more readable.
//! \{
  /*
  //! \def ucRadioTXBit
  //! \brief Counter to keep track of which bit the radio is on
  #define ucRadioTXBit            ucINT_TEMP1
  //! \def ucRadioTXByte
  //! \brief Counter to keep track of which byte the radio is on
  #define ucRadioTXByte           ucINT_TEMP2
  //! \def ucRadioTXState
  //! \brief Current portion of the message being transmitted
  //! \sa RADIO_TX_STATE_OFF
  //! \sa RADIO_TX_STATE_SYNC
  //! \sa RADIO_TX_STATE_PREAMBLE
  //! \sa RADIO_TX_STATE_MESSAGE
  #define ucRadioTXState          ucINT_TEMP3
  */
  //! \def ucRadioMsgLen
  //! \brief Length of the current message
  #define ucRadioMsgLen           ucaMSG_BUFF[0]
  /*
  //! \def ucaRadioPreamble
  //! \brief Preamble storage
  #define ucaRadioPreamble        ucaX0FLD
  */
//! \}

//! \name Flag Defines
//! Flags to allow communication with the rest of the code.
//! These defines are grandfathered in from Gen. 2,
//! and it is possible they are no longer needed
//! \{
  //! \def bRadioTXDone
  //! \brief Bit cleared during transmission
  #define bRadioTXDone            ucFLAG1_BYTE.FLG1_X_DONE_BIT
  //! \def bRadioTX
  //! \brief Bit set during transmit, clear during receive
  #define bRadioTX                ucFLAG1_BYTE.FLG1_X_FLAG_BIT
  //! \def bRadioAbort
  //! \brief If bit is set externally, current operation will be aborted
  #define bRadioAbort             ucFLAG1_BYTE.FLG1_R_ABORT_BIT
  //! \def bRadioNextBit
  //! \brief The next bit to transmit from, or store in, the buffer
  #define bRadioNextBit           ucFLAG1_BYTE.FLG1_X_NXT_LEVEL_BIT
//! \}

//! \name State Defines
//! Defines for the TX state machine.
//! \sa G_XMIT
//! \{
  //! \def RADIO_TX_STATE_OFF
  //! \brief The radio is not transmitting.
  #define RADIO_TX_STATE_OFF      0x00
  //! \def RADIO_TX_STATE_SYNC
  //! \brief The radio is transmitting sync bits
  #define RADIO_TX_STATE_SYNC     0x01
  //! \def RADIO_TX_STATE_PREAMBLE
  //! \brief The radio is transmitting preamble bits
  #define RADIO_TX_STATE_PREAMBLE 0x02
  //! \def RADIO_TX_STATE_MESSAGE
  //! \brief The radio is transmitting data bits
  #define RADIO_TX_STATE_MESSAGE  0x03
//! \}

//! \name TX Length Defines
//! Lengths of various message parts.
//! \{
  //! \def RADIO_SYNC_LEN
  //! \brief The length of the sync pattern, in bytes
  #define RADIO_SYNC_LEN          0x02
  //! \def RADIO_PREAMBLE_LEN
  //! \brief The length of the preamble, in bytes
  #define RADIO_PREAMBLE_LEN      0x03
//! \}

//! \name Channel Organization Methods
//! Allows switching between various channel organizations.
//! These defines are grandfathered in from Gen. 2,
//! and it is possible they are no longer needed
//! \{
  //! \def FIXED_CHANNEL_0
  //! \brief Channel organization #0: data=0, discover=0, test=0
  #define FIXED_CHANNEL_0         1
  //! \def FIXED_CHANNEL_1
  //! \brief Channel organization #1: data=0, discover=2, test=6
  #define FIXED_CHANNEL_1         2
  //! \def RANDOM_CHANNEL_1
  //! \brief Channel organization #2: data=0-31, discover=35, test=39
  #define RANDOM_CHANNEL_1        3
  
  //! \def CUR_CHAN_ALGORITHM
  //! \brief The current organization in use
  #define CUR_CHAN_ALGORITHM      FIXED_CHANNEL_1
  
  #ifndef CUR_CHAN_ALGORITHM
    #error "CHANNEL algorithm not specified"
  #endif
  
  //! \def DATA_CHAN_VAL
  //! \brief The data channel number
  
  //! \def DISCOVERY_CHAN_VAL
  //! \brief The discovery channel number
  
  //! \def TEST_CHAN_VAL
  //! \brief The test channel number
  
  //! \def ILLEGAL_CHAN_VAL
  //! \brief The illegal channel number
  
  #if (CUR_CHAN_ALGORITHM == FIXED_CHANNEL_0)
    #define DATA_CHAN_VAL         0
    #define DISCOVERY_CHAN_VAL    0
    #define TEST_CHAN_VAL         0
    #define ILLEGAL_CHAN_VAL      128
  #endif
  
  #if (CUR_CHAN_ALGORITHM == FIXED_CHANNEL_1)
    #define DATA_CHAN_VAL         0
    #define DISCOVERY_CHAN_VAL    2
    #define TEST_CHAN_VAL         6
    #define ILLEGAL_CHAN_VAL      128
  #endif
  
  #if (CUR_CHAN_ALGORITHM == RANDOM_CHANNEL_1)
    #define DATA_CHAN_VAL         0
    #define DISCOVERY_CHAN_VAL    35
    #define TEST_CHAN_VAL         39
    #define ILLEGAL_CHAN_VAL      128
  #endif
//! \}

////////////////////////////////////////////////////////////////////////////////
////////////////////  INCLUDES  ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include <msp430x54x.h>
#include "std.h"
#include "radio.h"
#include "msg.h"
#include "delay.h"
#include "misc.h"
#include "buz.h"

#if RADIO_DEBUG == 1
  #include "config.h"
  #include "serial.h"
#endif

// Not sure if we need the rest of these
//#include "DIAG.h"
//#include "main.h"
//#include "L2SRAM.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////  EXTERNS  /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

extern const uchar ucaRandTblNum[2];
extern uchar ucGLOB_lastAwakeNSTtblNum;
extern uchar ucGLOB_lastAwakeStblIdx;

extern volatile union
{
  uchar byte;
  struct
  {
    unsigned FLG1_X_DONE_BIT:		1;	//bit 0
    unsigned FLG1_X_LAST_BIT_BIT:	1;	//bit 1
    unsigned FLG1_X_FLAG_BIT:		1;	//bit 2 ;1=XMIT, 0=RECEIVE
    unsigned FLG1_R_HAVE_MSG_BIT:	1;	//bit 3	;1=REC has a msg, 0=no msg
    unsigned FLG1_R_CODE_PHASE_BIT: 1;	//bit 4 ;1=MSG PHASE, 0=BARKER PHASE
    unsigned FLG1_R_ABORT_BIT:		1;	//bit 5
    unsigned FLG1_X_NXT_LEVEL_BIT:	1;	//bit 6
    unsigned FLG1_R_SAMPLE_BIT: 	1;	//bit 7
  }FLAG1_STRUCT;
}ucFLAG1_BYTE;

extern volatile union
{
  uchar byte;
  struct
  {
    unsigned FLG2_T3_ALARM_MCH_BIT:1;		//bit 0 ;1=T3 Alarm, 0=no alarm
    unsigned FLG2_T1_ALARM_MCH_BIT:1;		//bit 1 ;1=T1 Alarm, 0=no alarm
    unsigned FLG2_BUTTON_INT_BIT:1;		//bit 2 ;1=XMIT, 0=RECEIVE
    unsigned FLG2_CLK_INT_BIT:1;			//bit 3	;1=clk ticked, 0=not
    unsigned FLG2_X_FROM_MSG_BUFF_BIT:1;	//bit 4
    unsigned FLG2_R_BUSY_BIT:1;			//bit 5 ;int: 1=REC BUSY, 0=IDLE
    unsigned FLG2_R_BARKER_ODD_EVEN_BIT:1;	//bit 6 ;int: 1=odd, 0=even
    unsigned FLG2_R_BITVAL_BIT:1;			//bit 7 ;int: 
  }FLAG2_STRUCT;
}ucFLAG2_BYTE;

extern volatile union
{
  uchar byte;
  struct
  {
    unsigned FLG3_RADIO_ON_BIT:1;
    unsigned FLG3_RADIO_MODE_BIT:1;
    unsigned FLG3_RADIO_PROGRAMMED:1;
    unsigned FLG3_UNUSED_BIT3:1;
    unsigned FLG3_UNUSED_BIT4:1;
    unsigned FLG3_UNUSED_BIT5:1;
    unsigned FLG3_UNUSED_BIT6:1;
    unsigned FLG3_UNUSED_BIT7:1;
  }FLAG3_STRUCT;
}ucFLAG3_BYTE;

extern uchar ucGLOB_radioChannel;

extern volatile uchar ucaMSG_BUFF[MAX_RESERVED_MSG_SIZE];
extern volatile uchar ucINT_TEMP1;
extern volatile uchar ucINT_TEMP2;
extern volatile uchar ucINT_TEMP3;
extern volatile uchar ucaX0FLD[MSG_XFLDSIZE];

////////////////////////////////////////////////////////////////////////////////
////////////////////  GLOBALS  /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//! \var uslADF7020_Register0_TX
//! \brief Channel-dependent TX settings for Register 0
ulong ulADF7020_Register0_TX;

//! \var uslADF7020_Register0_RX
//! \brief Channel-dependent RX settings for Register 0
ulong ulADF7020_Register0_RX;

//! \var uslADF7020_Register2_TX
//! \brief Channel-dependent TX settings for Register 2
ulong ulADF7020_Register2_TX;

//! \var uslADF7020_Register2_RX
//! \brief Channel-dependent RX settings for Register 2
ulong ulADF7020_Register2_RX;


//! \var ucaADF7020_Sync
//! \brief The radio messsage sync pattern
uchar ucaADF7020_Sync[3] = { 0x12, 0x34, 0x56};

//! \var ucRadioTXBit
//! \brief Counter to keep track of which bit the radio is on
uchar ucRadioTXBit = 0x00;

//! \var ucRadioTXByte
//! \brief Counter to keep track of which byte the radio is on
uchar ucRadioTXByte = 0x00;

//! \var ucRadioTXState
//! \brief Current portion of the message being transmitted
//! \sa RADIO_TX_STATE_OFF
//! \sa RADIO_TX_STATE_SYNC
//! \sa RADIO_TX_STATE_PREAMBLE
//! \sa RADIO_TX_STATE_MESSAGE
uchar ucRadioTXState = 0x00;

////////////////////////////////////////////////////////////////////////////////
////////////////////  LOCALS  //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//! Changes the RX and TX channels to the channel specified regardless of
//! current state
//! \param ucChanNum The channel number to switch to (1-127)
//! \return None
/*
static void vRADIO_forceBothFrequencys (
  uchar ucChanNum
);
*/
//! I have no idea what this function does, but I'm guessing that it sets either
//! the RX or TX frequency to the given channel number. -MW TODO
//! \param ucChannel The channel number to switch to (1-127)
//! \param uiXMITorRECfactor TX or RX channel
//! \return None
//! \sa RADIO_XMIT_FACTOR
//! \sa RADIO_REC_FACTOR
/*
static void vRADIO_forceSingleFrequency (
  uchar ucChannel,
  uint  uiXMITorRECfactor
);
*/
////////////////////////////////////////////////////////////////////////////////
////////////////////  DEFINITIONS  /////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////  ucRADIO_chk_for_radio_board  /////////////////////////////

unsigned char ucRADIO_chk_for_radio_board (void)
{
#if RADIO_DEBUG == 1
  vSERIAL_rom_sout("ucRADIO_chk_for_radio_board\r\n");
#endif

/*  
  // Setup the necessary pins
  TRISJbits.RADIO_CHIP_ENABLE_BIT = 0;  // Output
  TRISAbits.RADIO_MUXOUT_BIT = 1;       // Input

  // Turn off the AD converter on RA2
  if ( (ADCON1 & 0x0F) < 0x0D)
    ADCON1 |= 0x0D;

  // Raise the CE bit to start radio and wait for MUXOUT to go high
  PORTJbits.RADIO_CHIP_ENABLE_BIT = 1;

  // Clocking at 10MHz, so give the radio some time to power up (100us)
  vDELAY_wait100usTic(1);

  // Check for MUXOUT high, fail if not
  if(!PORTAbits.RADIO_MUXOUT_BIT)
  {
    PORTJbits.RADIO_CHIP_ENABLE_BIT = 0;
    return(FALSE);
  }

  // If we get here then the radio has been detected, lets shut off and 
  // return
  PORTJbits.RADIO_CHIP_ENABLE_BIT = 0;
*/
  return (TRUE);
}

////////////////////  vRADIO_init  /////////////////////////////////////////////

void vRADIO_init (void)
{
/*
#if RADIO_DEBUG == 1
  vSERIAL_init();
  vSERIAL_rom_sout("vRADIO_init\r\n");
#endif

  // configure TMR0 (~9.6kBd)
  T0CONbits.T08BIT      = ON;
  T0CONbits.T0CS        = 0;    // Use internal clock (Fosc)
  T0CONbits.PSA         = 1;    // Bypass prescaler
  RCONbits.IPEN         = ON;   // Enable priority-based interrupts
  INTCON2bits.TMR0IP    = 1;    // High-priority interrupt
  INTCONbits.TMR0IE     = ON;   // Enable overflow interrupt
  INTCONbits.GIEH       = ON;   // Enable high-priority interrupts

  // Port A setup
  PORTAbits.RADIO_SERIAL_LATCH_ENABLE_BIT = 0;
  TRISAbits.RADIO_SERIAL_LATCH_ENABLE_BIT = 0;

  TRISAbits.RADIO_MUXOUT_BIT = 1;

  TRISAbits.RADIO_SERIAL_READ_BIT = 1;

  // Port G setup
  PORTGbits.RADIO_SERIAL_DATA_BIT = 0;
  TRISGbits.RADIO_SERIAL_DATA_BIT = 0;

  TRISGbits.RADIO_INT_LOCK_BIT = 1;

  // Port J setup
  PORTJbits.RADIO_DATA_CLK_BIT = 0;
  TRISJbits.RADIO_DATA_CLK_BIT = 1;

  PORTJbits.RADIO_DATA_IO_BIT = 0;
  TRISJbits.RADIO_DATA_IO_BIT = 1;

  PORTJbits.RADIO_SDATA_CLK_BIT = 0;
  TRISJbits.RADIO_SDATA_CLK_BIT = 0;

  PORTJbits.RADIO_CHIP_ENABLE_BIT = 0;
  TRISJbits.RADIO_CHIP_ENABLE_BIT = 0;

  // Turn off AD converter if necessary
  if ( (ADCON1 & 0x0F) < 0x0D)
    ADCON1 |= 0x0D;

  // Pin init done, lets boot the radio, wait for MUXOUT
  PORTJbits.RADIO_CHIP_ENABLE_BIT = 1;
  while(!PORTAbits.RADIO_MUXOUT_BIT);

  // Set the flag
  ucFLAG3_BYTE.FLG3_RADIO_ON_BIT = 1;

  // Setup the channel calculation registers
  ulADF7020_Register0_TX = REGISTER0_CHAN0_TX;
  ulADF7020_Register0_RX = REGISTER0_CHAN0_RX;
  ulADF7020_Register2_TX = REGISTER2_TX;
  ulADF7020_Register2_RX = REGISTER2_RX;

  ucGLOB_radioChannel = 0x00;

  // Init radio in RX mode, default channel (0)
  vRADIO_ForceRXMode();
  ucFLAG3_BYTE.FLG3_RADIO_PROGRAMMED = 1;
*/
  return;
}

////////////////////  vRADIO_quit  /////////////////////////////////////////////

void vRADIO_quit (void)
{
/*
#if RADIO_DEBUG == 1
  vSERIAL_rom_sout("vRADIO_quit\r\n");
#endif

  vRADIO_abort_receiver();

  PORTJbits.RADIO_CHIP_ENABLE_BIT = 0;
  T0CONbits.TMR0ON = 0;
  ucFLAG3_BYTE.FLG3_RADIO_ON_BIT = 0;
*/ 
  return;
}

////////////////////  ucRADIO_write_register  //////////////////////////////////

uchar ucRADIO_write_register (
  ulong ulRegisterValue
)
{

//  uchar ucBitCount;

#if RADIO_DEBUG == 1
  vSERIAL_rom_sout("ucRADIO_write_register\r\n");
#endif
/*
  // If the radio isn't on, fail out
  if (!ucFLAG3_BYTE.FLG3_RADIO_ON_BIT)
    return RADIO_NOT_ON;
  
  // Setup ports
  TRISGbits.RADIO_SERIAL_DATA_BIT = 0;
  TRISJbits.RADIO_SDATA_CLK_BIT = 0;
  TRISAbits.RADIO_SERIAL_LATCH_ENABLE_BIT = 0;

  // Make sure serial lines are low
  PORTGbits.RADIO_SERIAL_DATA_BIT = 0;
  PORTJbits.RADIO_SDATA_CLK_BIT = 0;
  PORTAbits.RADIO_SERIAL_LATCH_ENABLE_BIT = 0;

  // Shift in the 32 bits
  for(ucBitCount=0x00; ucBitCount<0x20; ucBitCount++)
  {
    // Lower the clock
    PORTJbits.RADIO_SDATA_CLK_BIT = 0;
    
    // Set the data
    PORTGbits.RADIO_SERIAL_DATA_BIT = 0;
    if(ulRegisterValue & 0x80000000)
      PORTGbits.RADIO_SERIAL_DATA_BIT = 1;
    else
      PORTGbits.RADIO_SERIAL_DATA_BIT = 0;

    // Clock it in
    PORTJbits.RADIO_SDATA_CLK_BIT = 1;

    ulRegisterValue <<= 1;
  }

  // Make sure that the lines are low
  PORTJbits.RADIO_SDATA_CLK_BIT = 0;
  PORTGbits.RADIO_SERIAL_DATA_BIT = 0;

  // Strobe the latch
  PORTAbits.RADIO_SERIAL_LATCH_ENABLE_BIT = 1;
  PORTAbits.RADIO_SERIAL_LATCH_ENABLE_BIT = 0;
  
  // check to see if the oscillator register was written, and if so, wait
  // for the oscillator to stabilize
  if ( (ulRegisterValue &  0x01    ) &&
      !(ulRegisterValue & (0x01<<1)) &&
      !(ulRegisterValue & (0x01<<2)) &&
      !(ulRegisterValue & (0x01<<3)) )
  {
    vDELAY_wait100usTic(1);
  }
*/
  return(0);
}

////////////////////  ucRADIO_getChanAssignment  ///////////////////////////////

uchar ucRADIO_getChanAssignment (
  uchar ucChanIdx
)
{

  uchar ucChanNum;

#if RADIO_DEBUG == 1
  vSERIAL_init();
  vSERIAL_rom_sout("ucRADIO_getChanAssignment\r\n");
#endif
/*
  switch(ucChanIdx)
  {
    case DATA_CHANNEL_INDEX:
      #if (CUR_CHAN_ALGORITHM == FIXED_CHANNEL_0)
        ucChanNum = DATA_CHAN_VAL;
      #endif

      #if (CUR_CHAN_ALGORITHM == FIXED_CHANNEL_1)
        ucChanNum = DATA_CHAN_VAL;
      #endif

      #if (CUR_CHAN_ALGORITHM == RANDOM_CHANNEL_1)
        ucChanNum = (uchar) ulL2SRAM_getStblEntry (
          ucaRandTblNum[ucGLOB_lastAwakeNSTtblNum],
          ucGLOB_lastAwakeStblIdx
        ) & 0x1F;
      #endif
      break;

    case DISCOVERY_CHANNEL_INDEX:
      ucChanNum = DISCOVERY_CHAN_VAL;
      break;

    case TEST_CHANNEL_INDEX:
      ucChanNum = TEST_CHAN_VAL;
      break;

    case ILLEGAL_CHANNEL_INDEX:
      ucChanNum = ILLEGAL_CHAN_VAL;
      break;

    default:
      ucChanNum = 0;
      break;
  }
*/ 
  return(ucChanNum);


}

////////////////////  vRADIO_setBothFrequencys  ////////////////////////////////

void vRADIO_setBothFrequencys (
  uchar ucChanNum,
  uchar ucForceFlag
)
{
#if RADIO_DEBUG == 1
  vSERIAL_rom_sout("vRADIO_setBothFrequencys\r\n");
#endif
/*
  if((ucForceFlag == YES_FORCE_FREQ) || (ucChanNum != ucGLOB_radioChannel))
  {
    vRADIO_forceBothFrequencys(ucChanNum);
  }
*/
  return;
}

////////////////////  vRADIO_forceBothFrequencys  //////////////////////////////
/*
static void vRADIO_forceBothFrequencys (
  uchar ucChanNum
)
{
#if RADIO_DEBUG == 1
  vSERIAL_rom_sout("vRADIO_forceBothFrequencys\r\n");
#endif

  // Set the global channel variable
  ucGLOB_radioChannel = ucChanNum;

  // Set the transmit channel
  vRADIO_forceSingleFrequency(ucChanNum, RADIO_XMIT_FACTOR);

  // Set the receive channel
  vRADIO_forceSingleFrequency(ucChanNum, RADIO_REC_FACTOR);

  return;
}
*/

////////////////////  vRADIO_forceSingleFrequency  /////////////////////////////
/*
static void vRADIO_forceSingleFrequency (
  uchar ucChannel,
  uint uiXMITorRECfactor
)
{
  ulong ulTempFracN;
  ulong ulTempN;

  ulTempN = N_START;

#if RADIO_DEBUG == 1
  vSERIAL_rom_sout("vRADIO_forceSingleFrequency\r\n");
#endif

  switch (uiXMITorRECfactor)
  {
    case RADIO_XMIT_FACTOR:
      ulTempFracN = FRAC_N_START_TX + (ucChannel * FRAC_N_CHANNEL_STEP);
      if (ulTempFracN > 0x00007FFF)
      {
        ulTempN++;
        ulTempFracN = ulTempFracN - 0x00008000;
      }

      // Take the N value, shift over
      ulTempN <<= N_SHIFT;
      // Clear the n value, the insert new one
      ulADF7020_Register0_TX &= ~N_MASK;
      ulADF7020_Register0_TX |= ulTempN;

      // Take the FracN, shift over
      ulTempFracN <<= FRAC_N_SHIFT;
      // Clear the FracN, insert
      ulADF7020_Register0_TX &= ~FRAC_N_MASK;
      ulADF7020_Register0_TX |= ulTempFracN;
      break;

    case RADIO_REC_FACTOR:
      ulTempFracN = FRAC_N_START_RX + (ucChannel * FRAC_N_CHANNEL_STEP);
      if (ulTempFracN > 0x00007FFF)
      {
        ulTempN++;
        ulTempFracN = ulTempFracN - 0x00008000;
      }

      // Take the N value, shift over
      ulTempN <<= N_MASK;
      // Clear the n value, then insert
      ulADF7020_Register0_RX &= ~N_MASK;
      ulADF7020_Register0_RX |= ulTempN;

      // Take the FracN, shift over
      ulTempFracN <<= FRAC_N_SHIFT;
      // Clear the FracN, insert
      ulADF7020_Register0_RX &= ~FRAC_N_MASK;
      ulADF7020_Register0_RX |= ulTempFracN;
      break;

    default:
      break;
  }

  return;
}
*/

////////////////////  vRADIO_setTransmitPower  /////////////////////////////////

void vRADIO_setTransmitPower (
  uchar ucXmitPwr
)
{
  usl uslTempPower = ucXmitPwr;
  uslTempPower <<= PA_SHIFT;

#if RADIO_DEBUG == 1
  vSERIAL_rom_sout("vRADIO_setTransmitPower\r\n");
#endif
/*
  // Clear PA settings
  ulADF7020_Register2_TX &= ~PA_MASK;
  ulADF7020_Register2_TX |= uslTempPower;
*/
  return;
}

////////////////////  uslRADIO_getRandomNoise  /////////////////////////////////

usl uslRADIO_getRandomNoise (void)
{
  uchar ucByteCount, ucZeroByteCount, ucBitCount;
  uchar ucaRandByte[3];
  usl uslRetVal;

#if RADIO_DEBUG == 1
  vSERIAL_rom_sout("uslRADIO_getRandomNoise\r\n");
#endif
/*
  // Start the radio, the init function contains the necessary delays
  if (!ucFLAG3_BYTE.FLG3_RADIO_ON_BIT)
    vRADIO_init();

  // Force Radio into receive mode
  if (ucFLAG3_BYTE.FLG3_RADIO_MODE_BIT != RADIO_RX_MODE)
    vRADIO_ForceRXMode();

  for (
    ucByteCount = 0x00, ucZeroByteCount = 0x00; 
    ucByteCount < 0x03;
    ucZeroByteCount++
  )
  {
    // Build a byte
    for (ucBitCount = 0x00; ucBitCount < 0x08; ucBitCount++)
    {
      ucaRandByte[ucByteCount] <<= 1;
      if (PORTJbits.RADIO_DATA_IO_BIT)
        ucaRandByte[ucByteCount] |= 0x01;

      vDELAY_wait100usTic(4);
    }
    #if RADIO_DEBUG == 1
      vSERIAL_HB8Fout(ucaRandByte[ucByteCount]);
      vSERIAL_rom_sout("\r\n");
    #endif

    // Make sure that the byte is not zero
    if (ucaRandByte[ucByteCount] != 0x00 && ucaRandByte[ucByteCount] != 0xFF)
    {
      ucByteCount++;
      continue;
    }

    // Complain if we get too many zero bytes 
    if (ucZeroByteCount == 0xFF)
    {
      vBUZ_raspberry();
      vSERIAL_rom_sout("SlntRdio\r\n");
    }
  }

  uslRetVal = ulMISC_buildUlongFromBytes(&ucaRandByte[0], NO_NOINT);

  vRADIO_quit();
*/
  return uslRetVal;
}

////////////////////  ucRADIO_chk_rec_busy  ////////////////////////////////////

unsigned char ucRADIO_chk_rec_busy (void)
{
#if RADIO_DEBUG == 1
  vSERIAL_rom_sout("ucRADIO_chk_rec_busy\r\n");
#endif

  return( (unsigned char) ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_R_BUSY_BIT);
}

////////////////////  vRADIO_ForceTXMode  //////////////////////////////////////

void vRADIO_ForceTXMode (void)
{
#if RADIO_DEBUG == 1
  vSERIAL_rom_sout("vRADIO_ForceTXMode\r\n");
#endif
/*
  while(!PORTAbits.RADIO_MUXOUT_BIT);
  
  // Write registers
  ucRADIO_write_register(REGISTER1_VALUE);
  ucRADIO_write_register(ulADF7020_Register2_TX);
  ucRADIO_write_register(REGISTER3_VALUE);
  ucRADIO_write_register(REGISTER4_VALUE);
  ucRADIO_write_register(REGISTER5_VALUE);
  ucRADIO_write_register(REGISTER6_VALUE);
  ucRADIO_write_register(REGISTER9_VALUE);
  ucRADIO_write_register(REGISTERB_VALUE);
  ucRADIO_write_register(ulADF7020_Register0_TX);
  
  // Setup pins
  TRISJbits.RADIO_DATA_IO_BIT = 0;
  TRISJbits.RADIO_DATA_CLK_BIT = 0;
  
  // Set flag
  ucFLAG3_BYTE.FLG3_RADIO_MODE_BIT = RADIO_TX_MODE;
*/
  return;
}

////////////////////  vRADIO_ForceRXMode  //////////////////////////////////////

void vRADIO_ForceRXMode (void)
{
#if RADIO_DEBUG == 1
  vSERIAL_rom_sout("vRADIO_ForceRXMode\r\n");
#endif
/*
  while(!PORTAbits.RADIO_MUXOUT_BIT);
  
  ucRADIO_write_register(REGISTER1_VALUE);
  ucRADIO_write_register(ulADF7020_Register2_RX);
  ucRADIO_write_register(REGISTER3_VALUE);
  ucRADIO_write_register(REGISTER4_VALUE);
  ucRADIO_write_register(REGISTER5_VALUE);
  ucRADIO_write_register(REGISTER6_VALUE);
  ucRADIO_write_register(REGISTER9_VALUE);
  ucRADIO_write_register(REGISTERB_VALUE);
  ucRADIO_write_register(ulADF7020_Register0_RX);
  
  // Set flag
  ucFLAG3_BYTE.FLG3_RADIO_MODE_BIT = RADIO_RX_MODE;
  
  // Setup pins
  TRISJbits.RADIO_DATA_IO_BIT = 1;
  TRISJbits.RADIO_DATA_CLK_BIT = 1;
*/
  return;
}

////////////////////  vRADIO_xmit_msg  /////////////////////////////////////////

void vRADIO_xmit_msg (void)
{
#if RADIO_DEBUG == 1
  vSERIAL_rom_sout("vRADIO_xmit_msg\r\n");
#endif
/*
  // Make sure radio is on, and in TX mode
  if (!ucFLAG3_BYTE.FLG3_RADIO_ON_BIT)
    vRADIO_init();
  
  if (ucFLAG3_BYTE.FLG3_RADIO_MODE_BIT != RADIO_TX_MODE)
    vRADIO_ForceTXMode();
  
  vRADIO_ForceTXMode();


  ucRadioTXState  = RADIO_TX_STATE_SYNC;
  ucRadioTXByte   = 0x00;
  ucRadioTXBit    = 0x00;
  bRadioNextBit   = 1;
  bRadioTX        = 1;
  
  // Turn on the timer, ISR takes it from here
  T0CONbits.TMR0ON = 1;
  
  // Block further calls
  while (ucRadioTXState != RADIO_TX_STATE_OFF);
*/
  return;
}

////////////////////  vRADIO_start_receiver  ///////////////////////////////////

void vRADIO_start_receiver (void)
{
#if RADIO_DEBUG == 1
  vSERIAL_rom_sout("vRADIO_start_receiver\r\n");
#endif
/*
  // Make sure radio is on, and in RX mode
  if (!ucFLAG3_BYTE.FLG3_RADIO_ON_BIT)
  //  vRADIO_init();
  
  if (ucFLAG3_BYTE.FLG3_RADIO_MODE_BIT != RADIO_RX_MODE)
  //  vRADIO_ForceRXMode();
  
  // Call assembler routine
  //G_REC();
  while(ucFLAG1_BYTE.FLG1_R_ABORT_BIT == 0);
*/ 
  return;
}

////////////////////  vRADIO_abort_receiver  ///////////////////////////////////

void vRADIO_abort_receiver (void)
{
#if RADIO_DEBUG == 1
  vSERIAL_rom_sout("vRADIO_abort_receiver\r\n");
#endif
/*
  //force an abort
  ucFLAG1_BYTE.FLG1_R_ABORT_BIT = 1;

  // Wait for abort
  while(ucFLAG2_BYTE.FLG2_R_BUSY_BIT);

  //force a no-msg condition
  ucFLAG1_BYTE.FLG1_R_HAVE_MSG_BIT = 0;
*/ 
  return;
}


/*
////////////////////////////////////////////////////////////////////////////////
////////////////////  INTERRUPTS  //////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//! High-priority interrupt handler for radio TX timing. Triggered by Timer0
//! overflow.
//! \return None
//! \sa vRADIO_xmit_msg
void INT_VECTOR_ENTRY_HI (void)
{
  uchar i = 0;
  // Are we transmitting?
  if (bRadioTX)
  {
    // clock in next bit
    PORTJbits.RADIO_DATA_IO_BIT   = bRadioNextBit;
    PORTJbits.RADIO_DATA_CLK_BIT  = 1;
    
    // move to the next bit
    ucRadioTXBit++;
    if (ucRadioTXBit > 7)
    {
      ucRadioTXByte++;
      ucRadioTXBit = 0x00;
    }
    
    if (ucRadioTXState == RADIO_TX_STATE_SYNC
      && ucRadioTXByte >= RADIO_SYNC_LEN)
    {
      ucRadioTXState  = RADIO_TX_STATE_PREAMBLE;
      ucRadioTXByte   = 0x00;
      ucRadioTXBit    = 0x00;
    }
    if (ucRadioTXState == RADIO_TX_STATE_PREAMBLE
      && ucRadioTXByte >= RADIO_PREAMBLE_LEN)
    {
      ucRadioTXState  = RADIO_TX_STATE_MESSAGE;
      ucRadioTXByte   = 0x00;
      ucRadioTXBit    = 0x00;
    }
    if (ucRadioTXState == RADIO_TX_STATE_MESSAGE
      && ucRadioTXByte >= ucRadioMsgLen)
    {
      ucRadioTXState  = RADIO_TX_STATE_OFF;
      ucRadioTXByte   = 0x00;
      ucRadioTXBit    = 0x00;
      bRadioTX        = FALSE;
      vRADIO_ForceRXMode();
    }
    
    // retrieve the next bit value
    if (bRadioTX)
    switch (ucRadioTXState)
    {
      case RADIO_TX_STATE_SYNC:
      bRadioNextBit = ~bRadioNextBit;
      break;
      
      case RADIO_TX_STATE_PREAMBLE:
      bRadioNextBit = (ucaADF7020_Sync[ucRadioTXByte] >> ucRadioTXBit) & 0x01;
      break;
      
      case RADIO_TX_STATE_MESSAGE:
      bRadioNextBit = (ucaMSG_BUFF[ucRadioTXByte] >> ucRadioTXBit) & 0x01;
      break;
    }
    
    PORTJbits.RADIO_DATA_CLK_BIT = 0;
  }
  
  // If we get here, something is very wrong in the code,
  // so turn off the timer just to be safe
  else
  {
    T0CONbits.TMR0ON = 0;
  }
  
  // clear the interrupt flag
  INTCONbits.TMR0IF = 0;
  return;
}

*/

//! \}

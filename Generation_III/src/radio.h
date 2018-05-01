//!
//! \file radio.h
//! \brief The header file for ADF7020 Radio
//!

//  by: Kenji Yamamoto & Matthew Wyant
//      Wireless Networks Research Lab
//      Northern Arizona University, 2009

//	This code has not been ported. It has only been gutted and pretends to function.
//	change! needed

//! \addtogroup Radio
//! \{
#ifndef RADIO_H_INCLUDED
#define RADIO_H_INCLUDED

//! \name Old Defines
//! These defines are grandfathered in from Gen. 2,
//! and it is possible they are no longer needed
//! \{
  //! \def RADIO_XMIT_FACTOR
  //! \brief TODO
  #define RADIO_XMIT_FACTOR 4510
  //! \def RADIO_REC_FACTOR
  //! \brief TODO
  #define RADIO_REC_FACTOR  4456

  //! \def MAKE_RADIO_ACTIVE
  //! \brief TODO
  #define MAKE_RADIO_ACTIVE 1
  //! \def MAKE_RADIO_SLEEP
  //! \brief TODO
  #define MAKE_RADIO_SLEEP  0

  //! \def YES_FORCE_FREQ
  //! \brief TODO
  #define YES_FORCE_FREQ		1
  //! \def NO_FORCE_FREQ
  //! \brief TODO
  #define  NO_FORCE_FREQ		0
//! \}

//! \name Transmit Powers
//! The powers at which the radio can transmit
//! \{
  //! \def XMIT_PWR_OFF
  //! \brief Transmitter PA is off
  #define XMIT_PWR_OFF     0x00
  
  //! \def XMIT_PWR_LOW
  //! \brief Transmitter PA is at low power
  #define XMIT_PWR_LOW     0x10
  
  //! \def XMIT_PWR_MEDIUM
  //! \brief Transmitter PA is at medium power
  #define XMIT_PWR_MEDIUM  0x20
  
  //! \def XMIT_PWR_HI
  //! \brief Transmitter PA is at maximum power
  #define XMIT_PWR_HI      0x3F
//! \}

//! \name Radio Statuses
//! The states the radio can be in at any time
//! \{
  //! \def RADIO_NOT_ON
  //! \brief Radio is off
  #define RADIO_NOT_ON		1

  //! \def RADIO_RX_MODE
  //! \brief Radio is in receive mode (but may not be actually recieving)
  #define RADIO_RX_MODE		0

  //! \def RADIO_TX_MODE
  //! \brief Radio is in transmit mode (but may not be actually transmitting)
  #define RADIO_TX_MODE		1
//! \}

//! \name Data Channel Indicies
//! The abstract channel numbers the radio is set at.
//! NOTE: The indexes below are not the actual channel numbers. The indexes
//! are here because the actual channel number depends on the transmission
//! algorithm that is being used. To get an actual channel number you must
//! call:	ucRADIO_getChanAssignment() with the channel index that you wish
//! and the routine will return the actual channel number.
//! \{
  //! \def DATA_CHANNEL_INDEX
  //! \brief Channel for data transmission
  #define DATA_CHANNEL_INDEX      1
  
  //! \def DISCOVERY_CHANNEL_INDEX
  //! \brief Channel for network discovery
  #define DISCOVERY_CHANNEL_INDEX 2
  
  //! \def TEST_CHANNEL_INDEX
  //! \brief Channel for testing
  #define TEST_CHANNEL_INDEX			3
  
  //! \def ILLEGAL_CHANNEL_INDEX
  //! \brief Illegal channel assignment
  #define ILLEGAL_CHANNEL_INDEX		4
//! \}



//! Checks to see if the radio board is present & working. Takes at least
//! 100us to complete.
//! \return uchar
uchar ucRADIO_chk_for_radio_board (void);

//! Turns on and initializes the radio in receive mode. DOES NOT FAIL if the
//! radio is not present. Check the return value of ucRADIO_chk_for_radio_board.
//! first.
//! \return None
//! \sa ucRADIO_chk_for_radio_board
void vRADIO_init (void);

//! Turns off the radio and any associated peripherials.
//! \return None
void vRADIO_quit (void);

//! Writes a long value to a radio register specified by the for lowest
//! significant bits.
//! \param ulRegisterValue The value to write
//! \return Returns RADIO_NOT_ON if the radio is off
//! \sa RADIO_NOT_ON
uchar ucRADIO_write_register (
  unsigned long ulRegisterValue
);

//! Returns the actual channel number for the given index.
//! \param ucChanIdx The channel index for which to find the channel number
//! \return The channel number for the given channel index
//! \sa DATA_CHANNEL_INDEX
//! \sa DISCOVERY_CHANNEL_INDEX
//! \sa TEST_CHANNEL_INDEX
//! \sa ILLEGAL_CHANNEL_INDEX
uchar ucRADIO_getChanAssignment (
  uchar ucChanIdx
 );

//! Switches channels for both TX and RX. If ucForceFlag is NO_FORCE_FREQ, then
//! the channel is changed only if it's different than the current channel.
//! \param ucChanNum The channel number to switch to (1-127)
//! \param ucForceFlag Force the change
//! \return None
//! \sa YES_FORCE_FREQ
//! \sa NO_FORCE_FREQ
void vRADIO_setBothFrequencys (
  uchar ucChanNum,
  uchar ucForceFlag
);

//! Set the PA power for the radio transmitter.
//! \param ucXmitPwr The power setting
//! \return None
//! \sa XMIT_PWR_OFF
//! \sa XMIT_PWR_LOW
//! \sa XMIT_PWR_MEDIUM
//! \sa XMIT_PWR_HI
void vRADIO_setTransmitPower (
  uchar ucXmitPwr
);

//! Returns a long with data from atmospheric noise.
//! \return 32-bits of "random" data
usl uslRADIO_getRandomNoise (void);

//! Checks to see if the radio is currently receiving a packet.
//! This function is grandfathered in from Gen. 2,
//! and it is possible it is no longer needed
//! \return TRUE/FALSE if the receiver is busy
uchar ucRADIO_chk_rec_busy (void);

//! Switches the radio to TX mode.
//! DOES NOT CHECK to see if this would be dangerous.
//! \return None
void vRADIO_ForceTXMode (void);

//! Switches the radio to RX mode.
//! DOES NOT CHECK to see if this would be dangerous.
//! \return None
void vRADIO_ForceRXMode (void);

//! Switches the radio to RX mode and calls the assembly routine G_REC.
//! \return None
//! \sa G_REC
//! \sa ucaMSG_BUFF
void vRADIO_start_receiver (void); 

//! Waits for a packet, and stores it in the buffer.
//! \return None
//! \sa vRADIO_start_receiver
//! \sa ucaMSG_BUFF
void G_REC (void);

//! Switches the radio to TX mode, and transmits from the buffer
//! in the buffer.
//! \return None
//! \sa ucaMSG_BUFF
void vRADIO_xmit_msg (void);

//! Aborts any packet reception
//! \return None
void vRADIO_abort_receiver (void);

#endif /* RADIO_H_INCLUDED */
//! \}

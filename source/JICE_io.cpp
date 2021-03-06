/*
 * stk_io.cpp
 *
 * Created: 18-11-2017 15:20:29
 *  Author: JMR_2
 */

// Includes
#include <avr/io.h>
#include "JICE_io.h"
#include "sys.h"
#include "dbg.h"

namespace {
  // *** Baud rate lookup table for UBRR0 register ***
  // Indexed by valid values for PARAM_BAUD_RATE_VAL (defined in JTAG2.h)
  FLASH<uint16_t> baud_tbl[8] = {baud_reg_val(2400), baud_reg_val(4800), baud_reg_val(9600), baud_reg_val(19200), baud_reg_val(38400), baud_reg_val(57600), baud_reg_val(115200), baud_reg_val(14400)};
}

// Functions
uint8_t JICE_io::put(char c) {
#if defined XAVR
  loop_until_bit_is_set(HOST_USART.STATUS, USART_DREIF_bp);
  HOST_USART.STATUS=1<<USART_TXCIF_bp;
  return HOST_USART.TXDATAL = c;
#else
  loop_until_bit_is_set(HOST_UCSRA, UDRE);
  HOST_UCSRA|=1<<TXC;
  return HOST_UDR = c;
#endif
}


bool JICE_io::dataavail() {
#if defined XAVR
  return (HOST_USART.STATUS & (1<<USART_RXCIF_bp))!=0; /* Wait until data exists. */
#else
  return (HOST_UCSRA & (1<<RXC))!=0;
#endif
	
}


uint8_t JICE_io::get(void) {
#if defined XAVR
  loop_until_bit_set_or_host_timeout(HOST_USART.STATUS, USART_RXCIF_bp); /* Wait until data exists. */
  //while (HOST_USART.STATUS USART_RXCIF_bp);
  return HOST_USART.RXDATAL;
#else
  //loop_(until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
  loop_until_bit_set_or_host_timeout(HOST_UCSRA, RXC);
  return HOST_UDR;
#endif
}

void JICE_io::init(void) {
#if defined XAVR
  // Init TxD pin (PA6 on tiny412)
  PORT(HOST_TX_PORT) |= 1 << HOST_TX_PIN;
  DDR(HOST_TX_PORT) |= 1 << HOST_TX_PIN;
  /* Set initial baud rate */
  HOST_USART.BAUD = baud_reg_val(19200);
  /* Enable receiver and transmitter */
  HOST_USART.CTRLB = USART_TXEN_bm | USART_RXEN_bm | USART_RXMODE_NORMAL_gc;
#else
  /* Set double speed */
  HOST_UCSRA = (1<<U2X);
  /* Set initial baud rate */
  HOST_UBRR = baud_reg_val(19200);
  /* Enable receiver and transmitter */
  HOST_UCSRB = (1<<RXEN)|(1<<TXEN);
  /* Set frame format: 8data, 1stop bit */
  /* this is default configuration, so leave it */
  //#ifdef URSEL //the one case where we can't handle the weird UART on atmega16 with #defines to rename registers...
  //UCSRC = (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1);
  //#else
  //UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);
  //#endif
#endif
}


void JICE_io::flush(void) {
#if defined(XAVR)
  loop_until_bit_set_or_host_timeout(HOST_USART.STATUS, USART_TXCIF_bp);
#else
  loop_until_bit_set_or_host_timeout(HOST_UCSRA, TXC);
#endif
}

void JICE_io::set_baud(JTAG2::baud_rate rate) {
#if defined XAVR
  HOST_USART.BAUD = baud_tbl[rate - 1];
#else
  HOST_UBRR = baud_tbl[rate - 1];
#endif
}

/*
 * sys.cpp
 *
 * Created: 02-10-2018 13:07:52
 *  Author: JMR_2
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/delay.h>
#include "sys.h"
#include "UPDI_lo_lvl.h"
#include "dbg.h"
#include <stdio.h>
#include <string.h>


#include <stdio.h>
#include <string.h>



void SYS::init(void) {
  #ifdef DEBUG_ON
    DBG::initDebug();
  #endif

  #ifdef XAVR
    #ifdef __AVR_DA__
      #if (F_CPU == 24000000)
        /* No division on clock */
        _PROTECTED_WRITE(CLKCTRL_OSCHFCTRLA, (CLKCTRL_OSCHFCTRLA & ~CLKCTRL_FREQSEL_gm ) | (0x09<< CLKCTRL_FREQSEL_gp ));

      #elif (F_CPU == 20000000)
        /* No division on clock */
        _PROTECTED_WRITE(CLKCTRL_OSCHFCTRLA, (CLKCTRL_OSCHFCTRLA & ~CLKCTRL_FREQSEL_gm ) | (0x08<< CLKCTRL_FREQSEL_gp ));

      #elif (F_CPU == 16000000)
        /* No division on clock */
        _PROTECTED_WRITE(CLKCTRL_OSCHFCTRLA, (CLKCTRL_OSCHFCTRLA & ~CLKCTRL_FREQSEL_gm ) | (0x07<< CLKCTRL_FREQSEL_gp ));
      #else
        #error "F_CPU defined as an unsupported value"
      #endif
    #else //0-series or 1-series
      _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);
    #endif
  #else
    #if defined(ARDUINO_AVR_LARDU_328E)
    #include <avr/power.h>
    clock_prescale_set ( (clock_div_t) __builtin_log2(32000000UL / F_CPU));
    #endif
    PORT(UPDI_PORT) |= 1<<UPDI_PIN;
  #endif

  #if defined(__AVR_ATmega_Mini__)
  #if defined (USE_HV_PROGRAMMING)
    PORT(HV_PORT) &= ~(1<<HV_PIN);
    DDR(HV_PORT) |= 1<<HV_PIN;
  #endif
  #endif

  DDR(LED_PORT) |= (1 << LED_PIN);
  #ifdef LED2_PORT
  DDR(LED2_PORT) |= (1 << LED2_PIN);
  #endif
  #ifndef DISABLE_HOST_TIMEOUT
  TIMER_HOST_MAX=HOST_TIMEOUT;
  #endif
  #ifndef DISABLE_TARGET_TIMEOUT
  TIMER_TARGET_MAX=TARGET_TIMEOUT;
  #endif
  #if defined(DEBUG_ON)
  DBG::debug(0x18,0xC0,0xFF, 0xEE);
  #endif
}

void SYS::setLED(void){
  PORT(LED_PORT) |= 1 << LED_PIN;
}

void SYS::clearLED(void){
  PORT(LED_PORT) &= ~(1 << LED_PIN);
}

void SYS::setVerLED(void){
  #ifdef LED2_PORT
  PORT(LED2_PORT) |= 1 << LED2_PIN;
  #endif
}

void SYS::clearVerLED(void){
  #ifdef LED2_PORT
  PORT(LED2_PORT) &= ~(1 << LED2_PIN);
  #endif
}

void SYS::setHVLED(void){
  #if defined(USE_HV_PROGRAMMING)
  PORT(HVLED_PORT) |= 1 << HVLED_PIN;
  #endif
}

void SYS::clearHVLED(void){
  #if defined(USE_HV_PROGRAMMING)
  PORT(HVLED_PORT) &= ~(1 << HVLED_PIN);
  #endif
}

void SYS::pulseHV(void) {
/*  while (1) {
    SYS::setLED();
  PORT(UPDI_PORT) |= (1<<UPDI_PIN);
  DDR(UPDI_PORT) |= (1<<UPDI_PIN);
  PORT(HV_PORT) |= 1<<HV_PIN;
    for (unsigned long int laux=0; laux<5000000; laux++) asm("nop");
    SYS::clearLED();
  PORT(HV_PORT) &= ~(1<<HV_PIN);
  DDR(UPDI_PORT) &= ~(1<<UPDI_PIN);
  PORT(UPDI_PORT) &= ~(1<<UPDI_PIN);
    for (unsigned long int laux=0; laux<5000000; laux++) asm("nop");
  }*/

  #if defined(USE_HV_PROGRAMMING)
  _delay_ms(1); // initial delay after startup
  PORT(UPDI_PORT) |= (1<<UPDI_PIN);
  DDR(UPDI_PORT) |= (1<<UPDI_PIN);
  PORT(HV_PORT) |= 1<<HV_PIN;
  _delay_ms(50);
  PORT(HV_PORT) &= ~(1<<HV_PIN);
  DDR(UPDI_PORT) &= ~(1<<UPDI_PIN);
  PORT(UPDI_PORT) &= ~(1<<UPDI_PIN);
  _delay_us(5);          // tri-state duration after HV pulse
  #endif
}

void SYS::updiTriState(void) {
  #if defined(USE_HV_PROGRAMMING)
  #if defined(__AVR_ATmega_Mini__)
  DDR(UPDI_PORT) &= ~(1<<UPDI_PIN);
  PORT(UPDI_PORT) &= ~(1<<UPDI_PIN); // Disable pull-up
  #endif
  #endif
}

void SYS::updiHigh(void) {
  #if defined(USE_HV_PROGRAMMING)
  #if defined(__AVR_ATmega_Mini__)
  PORT(UPDI_PORT) |= (1<<UPDI_PIN);
  DDR(UPDI_PORT) |= (1<<UPDI_PIN);
  #endif
  _delay_us(20);
  #endif
}

void SYS::updiIdle(void) {
  #if defined(USE_HV_PROGRAMMING)
  SYS::updiHigh();
  SYS::updiTriState();
  _delay_us(521);
  SYS::updiHigh();
  #endif
}

void SYS::updiInitiate(void) {
  #if defined(USE_HV_PROGRAMMING)
  // Release UPDI Reset and initiate UPDI Enable by driving low (0.7µs) then tri-state
  #if defined(__AVR_ATmega_Mini__)
  //SYS::updiHigh();
  PORT(UPDI_PORT) &= ~(1<<UPDI_PIN); // Low
  DDR(UPDI_PORT) |= (1<<UPDI_PIN);
  _delay_us(10); 
  SYS::updiTriState();
  //_delay_us(200); 
  //if (PIN(UPDI_PORT) & (1<<UPDI_PIN)) SYS::setLED(); else SYS::clearLED();
  #endif
  _delay_us(521);         // tri-state duration after UPDI Enable trigger pulse
  #endif

}

void SYS::updiEnable(void) {

  #if defined(USE_HV_PROGRAMMING)
  SYS::updiInitiate();
  SYS::updiHigh();
  UPDI::write_key(UPDI::NVM_Prog);
  SYS::updiIdle();
  UPDI_io::put(UPDI::SYNCH);
  SYS::updiHigh();
  SYS::updiTriState();
  _delay_us(521);
  #endif
}


uint8_t SYS::checkHVMODE() {                    // Check HV Programming Mode Switch
  return 120;
}

#ifndef AVR_SLEEP_H
#define AVR_SLEEP_H

/*============================================================
 * The AVR_sleep class allows the Arduino and/or PlatformIO
 * (and other systems) the ability to simply put the device to 
 * sleep in any of the applicable modes. The Arduino is not
 * able to use the two modes where Timer 2 is running in
 * asynchronous mode, as the crystal is taking up the required
 * pins.
 * 
 * Norman Dunbar
 * 17th February 2021.
 *===========================================================*/

#include "avr/io.h"
#include "avr/wdt.h"
#include "avr/sleep.h"
#include "avr/interrupt.h"
#include <stdint.h>



/*-------------------------------------------------------------
 * To put the device to sleep:
 *
 * All pins should be Hi-Z (Input?), be digitally disabled and
 * not driving any resistive loads, before going to sleep.
 *
 * Set bits  SM2:0  in  SMCR  (Sleep Mode Control Register) to 
 * select the sleep mode required.
 *
 * Disable interrupts, if running.
 * Set bit  SE in  SMCR  to enable the sleep mode.
 * Power off any peripherals of the ATmega328P as necessary.
 * Enable interrupts, always.
 * If disabling BOD, do it right now immediately before the
 * 'sleep' instructions is executed. You have three cycles!
 * Execute the  sleep instruction.
 *------------------------------------------------------------
 * On waking  up:
 *  
 * Clear SE bit in SMCR.
 * Power on any required peripherals. If TWI or SPI were off
 * during sleep, they will need to be reconfigured on wake.
 * Reset interrupts as they were before setting SE bit.
 *-----------------------------------------------------------*/

namespace sleep {

	//---------------------------------------------------------
	// Call here before sleeping.
	//---------------------------------------------------------
	typedef void (*preSleepFN)();
	
	//---------------------------------------------------------
	// Call here after wake up.
	//---------------------------------------------------------
	typedef void (*afterWakeFN)();
	
	//---------------------------------------------------------
	// Typedef for the various sleep modes. These are
	// using the defines from the avr/sleep header but
	// I'm using an enum to ensure we can validate them.
	//---------------------------------------------------------
	typedef enum sleepMode : uint8_t {
	    SM_IDLE = SLEEP_MODE_IDLE,
	    SM_ADC = SLEEP_MODE_ADC,
	    SM_POWER_DOWN = SLEEP_MODE_PWR_DOWN,
	    SM_POWER_SAVE = SLEEP_MODE_PWR_SAVE,
	    SM_STANDBY = SLEEP_MODE_STANDBY,
	    SM_EXT_STANDBY = SLEEP_MODE_EXT_STANDBY
	} sleepMode_t;

	//---------------------------------------------------------
	// Typedef for the various power off peripherals. These
	// will save even more power during sleep, and can be used
	// even when not sleeping, if the various peripherals are
	// not used. 
	// Most of these are bits in the PRR register, but the rest
	// are using additional bits as flags. We need to be able 
	// to restart/reconfigure these peripherals on wake up.
	//---------------------------------------------------------
	typedef enum powerMode : uint16_t {
	    PM_NONE = 0,                        // Nothing at all
	    PM_TWI_OFF = (1 << PRTWI),          // PRR Bit 7
	    PM_TIMER2_OFF = (1 << PRTIM2),      // PRR Bit 6
	    PM_TIMER0_OFF = (1 << PRTIM0),      // PRR Bit 5
	    PM_TIMER1_OFF = (1 << PRTIM1),      // PRR Bit 3
	    PM_SPI_OFF = (1 << PRSPI),          // PRR Bit 2
	    PM_USART_OFF = (1 << PRUSART0),     // PRR Bit 1
	    PM_ADC_OFF = (1 << PRADC),          // PRR Bit 0
	    PM_PRR_OFF = 0b11101111,            // All PRR off
	    // These have no bits in the PRR
	    // so use the 8 bits in the high byte of
	    // the uint16_t.
	    PM_AC_OFF = 8,              // Analog Comparator
	    PM_BOD_OFF = 9,             // Brown Out Detector
	    PM_WDT_OFF = 10,            // Watchdog Timer
	    PM_EVERYTHING_OFF = 0x07ef  // Everything off
	} powerMode_t;


	class AVR_sleep {

	public:
		//---------------------------------------------------------
		// Constructor.
		//---------------------------------------------------------
		AVR_sleep();


		//---------------------------------------------------------
		// Which sleep mode?
		// Which peripherals need disabling?
		//---------------------------------------------------------
		void setSleepMode(
		        const sleepMode_t sleepMode,
		        const powerMode_t powerOffBits);
		
		//---------------------------------------------------------
		// Do it.
		//---------------------------------------------------------
		void goToSleep();
		
		//---------------------------------------------------------
		// Attach sketch functions to pre/post sleep.
		//---------------------------------------------------------
		void attachPreSleep(const preSleepFN psfn);
		void attachWakeUp(const afterWakeFN awfn);

	private:
		//---------------------------------------------------------
		// Function to call before going to sleep.
		//---------------------------------------------------------
		preSleepFN ps;

		//---------------------------------------------------------
		// Function to call after waking up.
		//---------------------------------------------------------
		afterWakeFN aw;

		//---------------------------------------------------------
		// Saved copy of the PRR register. Restored after wake up.
		//---------------------------------------------------------
		uint8_t copyPRR;

		//---------------------------------------------------------
		// Flags for everything we are turning off.
		//---------------------------------------------------------
		powerMode_t powerBits;
	};

} // End of namespace.

//-------------------------------------------------------------
// We need one of these which is declared in the cpp file.
//-------------------------------------------------------------
extern sleep::AVR_sleep AVRsleep;

#endif // AVR_SLEEP_H


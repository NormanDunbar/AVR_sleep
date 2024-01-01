#include "AVR_sleep.h"

namespace sleep {

	//-------------------------------------------------------------
	// Constructor. Just nulls out the two function pointers.
	//-------------------------------------------------------------
	AVR_sleep::AVR_sleep() :
		ps(nullptr),
		aw(nullptr),
		copyPRR(0),
		powerBits(sleep::PM_NONE)
		{}

	//-------------------------------------------------------------
	// The Arduino is not able to use all of the sleep modes as it
	// cannot set timer 2 into asynchronous mode. So, those are 
	// trapped and a suitable replacement mode used instead.
	//-------------------------------------------------------------
	void AVR_sleep::setSleepMode(
		    const sleepMode_t sleepMode,
		    const powerMode_t powerOffBits) {
		
		sleepMode_t mode = sleepMode;

		//---------------------------------------------------------
		// If the AC, BOD and WDT are to be powered off, record it!
		//---------------------------------------------------------
		powerBits = powerOffBits;

	#ifdef ARDUINO
		//---------------------------------------------------------
		// Can't use SM_PWR_SAVE on Arduino.
		//---------------------------------------------------------
		if (mode == sleep::SM_POWER_SAVE) {
		    mode = sleep::SM_POWER_DOWN;
		} else {
		    //-----------------------------------------------------
		    // Can't use SM_EXT_STANDBY on Arduino.
		    //-----------------------------------------------------
		    if (mode == sleep::SM_EXT_STANDBY) {
		        mode = sleep::SM_STANDBY;
		    }
		}
	#endif
		    
		//---------------------------------------------------------
		// Set thenrequested sleep mode for later.
		//---------------------------------------------------------
		set_sleep_mode(mode);
	}


	//-------------------------------------------------------------
	// Puts the board to sleep. If the flag is set to power off the
	// BOD (Brown Out Detector) then that needs doing within 3
	// clock cycles -- need to be quick!
	//-------------------------------------------------------------
	void AVR_sleep::goToSleep() {

		//---------------------------------------------------------
		// Check the powerBits and if anything needs powering off,
		// do it. Save a copy of the PRR to enable after wakeup.
		//
		// NOTE: While the PRR disables TWI, SPI, USART, ADC and
		// Time 0, Timer 1 and Timer 2, TWI and SPI need to be
		// reconfigured after wake up.
		//---------------------------------------------------------
		copyPRR = PRR;

		// Those flags that match the PRR register are easy.
		PRR = (powerBits & 0x00ff);

		//---------------------------------------------------------
		// Now do the other peripherals not covered by the PRR. The
		// Brown Out Detector will be disabled, if requested, right
		// before going to sleep as it is on a time limit.
		//
		// Analog Comparator first.
		//
		// NOTE: AC will not be automagically re-enabled on wake.
		//---------------------------------------------------------
		if (powerBits & (1 << sleep::PM_AC_OFF)) {
		    ACSR |= (1 << ACD);
		}

		//---------------------------------------------------------
		// Then Watchdog Timer.
		// Reset the WDT.
		// Disable the WDT.
		//
		// NOTE: WDT will not be automagically re-enabled on wake.
		//---------------------------------------------------------
		if (powerBits & (1 << sleep::PM_WDT_OFF)) {
		    wdt_reset();
		    MCUSR &= (1 << WDRF);
		    wdt_disable();
		}

		//---------------------------------------------------------
		// Call preSleep function, if defined.
		//---------------------------------------------------------
		if (ps) {
		    (ps)();
		}
		
		//---------------------------------------------------------
		// Save interrupt state and disable interrupts.
		//---------------------------------------------------------
		uint8_t oldSREG = SREG;
		cli();
		
		//---------------------------------------------------------
		// Enable the sleep mode.
		//---------------------------------------------------------
		sleep_enable();
		
		//---------------------------------------------------------
		// if disabling the BOD, we need to do it immediately prior
		// to calling sleep_cpu(). We have only 3 clock cycles
		// between disabling the BOD and calling sleep_cpu or it
		// will not disable.
		//---------------------------------------------------------
		if (powerBits & (1 << sleep::PM_BOD_OFF)) {
		    sleep_bod_disable();
		}

		//---------------------------------------------------------
		// Interrupts on, or we won't wake up!
		//---------------------------------------------------------
		sei();

		//---------------------------------------------------------
		// Sleepy time, bye byes!
		//---------------------------------------------------------
		sleep_cpu();

		//---------------------------------------------------------
		// The microcontroller is now asleep. It will wake on an
		// interrupt or a reset. If interrupted, it will continue
		// with the remainder of this function. It will attempt to:
		//
		// 1. Disable sleep mode as required by the data sheet.
		// 2. Restore the Power Reduction Register, but this may
		//    re-enable TWI and SPI. Those will need to be set up
		//    again in the afterWake() function.
		// 3. Restore global interrupts, if they were enabled.
		// 4. The Watch Dog Timer will NOT be restarted.
		// 5. The previous sleep mode will be preserved for another
		//    sleep session, but can be changed if necessary.
		//---------------------------------------------------------

		// Zzzzzzzzzzzzzzzzzz! ;-)

		//---------------------------------------------------------
		// When we get here, we were woken by an interrupt, not a
		// reset.
		//---------------------------------------------------------

		//---------------------------------------------------------
		// Must disable sleep enable bit on wake.
		//---------------------------------------------------------
		sleep_disable();

		//---------------------------------------------------------
		// Restore original PRR and global interrupt settings.
		//
		// NOTE: TWI and SPI will need to be reconfigured after we
		// wake up. Everything else carries on regardless. This can
		// be done in the afterWake() function if necessary.
		//---------------------------------------------------------
		PRR = copyPRR;
		SREG = oldSREG;

		//---------------------------------------------------------
		// Call afterWake function, if defined.
		//---------------------------------------------------------
		if (aw) {
		    (aw)();
		}
	}

	//-------------------------------------------------------------
	// Attach a function to call before sleeping.
	//-------------------------------------------------------------
	void AVR_sleep::attachPreSleep(const preSleepFN psfn) {
		ps = psfn;
	}

	//-------------------------------------------------------------
	// Attach a function to call after waking.
	//-------------------------------------------------------------
	void AVR_sleep::attachWakeUp(const afterWakeFN awfn) {
		aw = awfn;
	}

} // End of namespace.

//-------------------------------------------------------------
// And here we declare our one AVR_sleep object.
//-------------------------------------------------------------
sleep::AVR_sleep AVRsleep;



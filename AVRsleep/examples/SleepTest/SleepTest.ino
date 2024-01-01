#include "AVR_sleep.h"
#include "AVR_wdt.h"

//============================================================
// AVRSleep Demonstration.
//============================================================
// This demonstartions shows how to put the ATmega328P to
// sleep. 
//
// On setup, it will blink an LED twice.
// In the loop, it will blink the LED four times, then sleep.
// On wakeup, it will blink four times and go to sleep again.
// 
// What wakes it up? The Watch Dog Timer interrupt which will
// be configured to fire every 8 seconds. The WDT will not be
// configured to reset the system though, just to interrupt.
//
// Norman Dunbar
// 18th February 2021.
//============================================================


//-------------------------------------------------------------
// Flash the built in LED a few times. The LED is on D13/PB5.
//-------------------------------------------------------------
void flashLED(uint8_t times, uint16_t delayMS) {
    // To flash we need to toggle twice!
    for (uint8_t x = 0; x < times * 2; x++) {
        PINB |= (1 << PINB5);
        _delay_ms(delayMS);
    }
}

//-------------------------------------------------------------
// Watchdog interrupt handler. We don't need any real work here
// just a handler.
//-------------------------------------------------------------
void wdtTimeoutFunction() {
    return;
}

//-------------------------------------------------------------
// Function to call just before sleeping. This does a bit of
// housekeeping before we go to sleep.
// In this demo, we only have one pin configured, D13 so we
// make it INPUT during sleep. 
//-------------------------------------------------------------
void preSleepFunction() {
    pinMode(LED_BUILTIN, INPUT);
}

//-------------------------------------------------------------
// When we wake up, we need to put D13 back as OUTPUT.
//-------------------------------------------------------------
void postWakeFunction() {
    pinMode(LED_BUILTIN, OUTPUT);

    // If the work being done in the loop takes longer than the
    // WDT timeout, then disable the WDT here:
    //
    // AVRwdt.end();
    //
    // It can be enabled again in the loop().
}


void setup() {
    //---------------------------------------------------------
    // SETUP:
    //---------------------------------------------------------
    // D13/PB5 is an output. All other pins are inputs.
    //---------------------------------------------------------
    pinMode(LED_BUILTIN, OUTPUT);


    //---------------------------------------------------------
    // Flash twice at startup.
    //---------------------------------------------------------
    flashLED(2, 500);
    delay(1000);

    //---------------------------------------------------------
    // Configure sleep modes. We are turning everything off but
    // we need the Watchdog, so that's left running.
    //---------------------------------------------------------
    AVRsleep.setSleepMode(
        sleep::SM_POWER_DOWN,
        sleep::PM_PRR_OFF |
        sleep::PM_AC_OFF |
        sleep::PM_BOD_OFF
    );

    //---------------------------------------------------------
    // prior to sleeping, we need to High-Z the pins. This is 
    // just D13/PB5 in this example. High-Z means, INPUT!
    //---------------------------------------------------------
    AVRsleep.attachPreSleep(preSleepFunction);

    //---------------------------------------------------------
    // After waking up, we need to make D13/PB5 OUTPUT again.
    //---------------------------------------------------------
    AVRsleep.attachWakeUp(postWakeFunction);

    //---------------------------------------------------------
    // Configure the Watchdog. Wakes up every 4 seconds.
    //---------------------------------------------------------
    AVRwdt.begin(
            wdt::TIMEOUT_4S,
            wdtTimeoutFunction,
            false);
}

void loop() {
    //---------------------------------------------------------
    // Flash LED 4 times, quickly, then sleep. After Timer 1 
    // overflows, we will wake up and go around again. We must
    // reset the WDT or the next flash time will be in 4
    // seconds minus the time taken to flash 4 times = in 2 
    // seconds.
    //
    // Beware of the WDT waking up faster than you can process
    // whatever you are trying to do while awake. Call reset()
    // if you think it will take longer than the WDT wake up
    // time.
    //
    // You could disable the WDT in the postWakeUp() function
    // and enable it in the loop, before going back to sleep.
    //---------------------------------------------------------
    flashLED(4, 250);

    // If the work being done here takes less time than the WDT
    // timeout, just reset the WDT. Only required if the WDT
    // configured to reset the board.
    //AVRwdt.reset();

    // If it takes longer, disable the WDT in the postWakeUp()
    // function, and enable it here when ready.
    //
    // AVRwdt.begin(
    //    AVR_wdt::WDT_TIMEOUT_4S,
    //    wdtTimeoutFunction,
    //    false);

    AVRsleep.goToSleep();
}



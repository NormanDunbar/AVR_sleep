# AVR_sleep Library

This library can be usd with the Arduino IDE and also with other systems, such as PlatformIO. The library makes handling the various sleep modes on the Arduino very simple indeed.

## Library Documentation

### Namespaces

This library uses the *sleep* namespace.

### Types

#### preSleepFN

This type defines the function that will be called just before the Arduino is put to sleep. The function must be as follows:

```
void preSleepFunction() {
    // Do something on before sleeping.
}
```

#### afterWakeFN

This type defines the function that will be called just after the Arduino wakes from sleep. The function must be as follows:

```
void afterSleepFunction() {
    // Do something on after waking.
}
```
It should be noted that as an interrupt of some kind is required to wakr the board from a sleep mode, the interrupt handler will have finished executing its code *before* control passes to this function.



#### sleepMode_t

The `sleepMode_t` type defines the 6 different sleep modes that can be passed to the `setSleepMode()` function. The different values are:

* **sleep::SM_IDLE** The lightest sleep mode and the only one that the Analog Comparator interrupt can be used to wake the board. Pretty useless on an Arduino as there is a Timer 0 overflow interrupt every 1,024 microseconds to keep the `millis()` and `micros()` functions working.
* **sleep::SM_ADC** This mde is useful when using the ADC. It reduces the amount of digital "noise" that the board creates and helps increase the accuracy of the ADC readings.
* **sleep::SM_POWER_DOWN** A heavy sleep mode, almost everything is powered off until an interrupt arrives.
* **sleep::SM_POWER_SAVE** Not usable on an Arduino. Requires the use of Timer 2 in Asynchronous mode.
* **sleep::SM_STANDBY** Similar to `SM_POWER_DOWN` but an extra oscillator is kept running.
* **sleep::SM_EXT_STANDBY** Not usable on an Arduino. Requires the use of Timer 2 in Asynchronous mode.

#### powerMode_t

The `powerMode_t` type defines which peripherals are to be powered off during sleep. The different values are:

The following power mode settings set or clear a bit in the Power Reduction Register, PRR.

* **sleep::PM_NONE** Nothing at all powered off.
* **sleep::PM_TWI_OFF** TWI/I2C powered off.
* **sleep::PM_TIMER2_OFF** Timer 2 powered off.
* **sleep::PM_TIMER0_OFF** Timer 0 powered off.
* **sleep::PM_TIMER1_OFF** Timer 1 powered off.
* **sleep::PM_SPI_OFF** SPI powered off.
* **sleep::PM_USART_OFF** USART (Serial) powered off.
* **sleep::PM_ADC_OFF** ADC (AnalogRead) powered off.
* **sleep::PM_PRR_OFF** All of the above, except `sleep::PM_NONE`, powered off.

The following power modes do not have a bit in the PRR, but have bits in other registers.

* **sleep::PM_AC_OFF** Analog Comparator powered off.
* **sleep::PM_BOD_OFF** Brown Out Detector disabled. SOme boards ign ore this option.
* **sleep::PM_WDT_OFF** Watchdog Timer powered off.

The following option turns off all the PRR modes and those external to the PRR.

* **sleep::PM_EVERYTHING_OFF** Everything above is powered off.

### Functions

#### **`void AVR_sleep.setSleepMode()`**

This function sets the desired sleep mode. If the mode chosen is not suitable for use on an Arduino board---`SM_POWER_SAVE` or `SM_EXT_STANDBY`---then a suitable Arduino permitted alternative will be used instead without causing any errors. The alternatives are `SM_POWER_DOWN` and `SM_STANDBY`

```
void setSleepMode(const sleepMode_t sleepMode,
                  const powerMode_t powerOffBits);
```

Example:

```
	AVRsleep.setSleepMode(
		AVR_sleep::SM_POWER_DOWN,
		AVR_sleep::PM_PRR_OFF | AVR_sleep::PM_AC_OFF | AVR_sleep::PM_BOD_OFF
	);
```

#### **`void AVR_sleep.attachPreSleep()`**

This function attaches a user defined function which will be called immediately before the board is put to sleep. This can be useful when various pins, which need to be, can be reconfigured in High-Z mode (reset to `INPUT`), for example. 

```
void attachPreSleep(const preSleepFN psfn);
```

Example:

```
void preSleep() {
	// Do something before sleeping.
	...
}

void setup() {
	...
	AVRsleep.attachPreSleep(preSleep);
	...
}
```

#### **`void AVR_sleep.attachWakeUp()`**

This function attaches a user defined function which will be called immediately after the board is wakened from sleep. This can be useful when various pins, which need to be, can were reconfigured in High-Z mode prior to sleeping, can be properly configured again after waking up. 

```
void attachWakeUp(const afterWakeFN awfn);
```

Example:

```
void postSleep() {
	// Do something before sleeping.
	...
}

void setup() {
	...
	AVRsleep.attachWakeUp(postSleep);
	...
}
```



#### **`void AVR_sleep.goToSleep()`**

This function sends the board to sleep.

```
void goToSleep();
```


## Example Sketches

The following code shows an example of using this interrupt to toggle an LED.
### Example Arduino Sketch

```
//============================================================
// AVR_sleep Demonstration.
//============================================================
// Attach an LED to Arduino pin D13, AVR pin PB5
// and toggle it in the loop.
//
// Another LED on Arduino pin D5, AVR pin PD5, will be
// toggled when a switch is pressed. The switch
// MUST be attached to Arduino pin D3, AVR pin PD3.
//
// For the sake of this test, debouncing is not used so 
// some flickering is to be expected. (Use long presses!)
//
// Norman Dunbar
// 7th August 2020.
//============================================================

#include "AVR_sleep.h"

// My toggling function for the ISR to call. It should 
// be kept short and sweet.
void toggleLED() {
    digitalWrite(5, !digitalRead(5));
}

// What to do after awakening.
void postSleep() {
	Serial.println("Awake again.");
}

// What to do before sleeping.
void preSleep() {
	Serial.println("Sleeping...");
}

void setup() {
	Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(2, INPUT_PULLUP);
    
    // Attach my function to the INT1 interrupt.
    AVR_sleep.onInterruptTriggered(toggleLED);

    // Enable the interrupt on FALLING stimulus this
    // requires the switch to be INPUT PULLUP.
    AVR_sleep.enable(sleep::TRGR_FALLING);
}

void loop() {
        // Toggle built in LED on D13/PB5.
        digitalWrite(13, !digitalRead(13));
        delay(1000);
}
```

### Example for PlatformIO

```
#include "AVR_sleep.h"
#include <util/delay.h>

//============================================================
// AVR_sleep Demonstration.
//============================================================
// Attach an LED to Arduino pin D13, AVR pin PB5
// and toggle it in the loop.
//
// Another LED on Arduino pin D5, AVR pin PD5, will be
// toggled when a switch is pressed. The switch
// MUST be attached to Arduino pin D3, AVR pin PD3.
//
// For the sake of this test, debouncing is not used so 
// some flickering is to be expected. (Use long presses!)
//
// Norman Dunbar
// 7th August 2020.
//============================================================


// My toggling function for the ISR to call. It should 
// be kept short and sweet.
void toggleLED() {
    PIND |= (1 << PIND5);
}


int main() {

    // SETUP:
    DDRB |= (1 << DDB5);        // D13/PB5 is OUTPUT.
    PORTB &= ~(1 << PORTB5);    // D13/PB5 is OFF/LOW.

    DDRD &= ~(1 << DDD3);       // D3/PD3 is INPUT ...
    PORTD |= (1 << PORTD3);     // D3/PD3 is INPUT PULLUP.

    DDRD |= (1 << DDD5);        // D5/PD5 is OUTPUT.
    PORTD &= ~(1 << PORTD5);    // D5/PD5 is OFF/LOW.


    // Attach my function to the INT1 interrupt.
    AVR_sleep.onInterruptTriggered(toggleLED);

    // Enable the interrupt on FALLING stimulus this
    // requires the switch to be INPUT PULLUP.
    AVR_sleep.enable(sleep::TRGR_FALLING);

    // Enable Global interrupts.
    sei();


    // LOOP:
    while (1) {
        // Toggle built in LED on D13/PB5.
        PINB |= (1 << PINB5);
        _delay_ms(1000);
    }
}
```

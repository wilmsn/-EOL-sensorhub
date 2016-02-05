#ifndef Sleeplib_h
#define Sleeplib_h

#if ARDUINO >= 100
#include <Arduino.h> // Arduino 1.0
#else
#include <WProgram.h> // Arduino 0022
#endif
#include <stdint.h>
#include <avr/pgmspace.h>
void watchdogInterrupts (char mode);
void powerDown ();
void dosleep (char wdp);
void watchdogEvent();
/// Do not forget to include this lines in your sourcecode:
///
/// #include <sleeplib.h>
/// ISR(WDT_vect) { watchdogEvent(); }
///
/// After that you can call
///
/// sleep4ms(1000); // Sleeptime in milliseconds maximum sleeptime is <size of unsigned long>!!!!!!!!!!!!! 
///
void sleep4ms (unsigned long ms);

#endif
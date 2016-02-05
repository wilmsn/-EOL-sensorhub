#include <sleeplib.h>
#include <avr/sleep.h>
#include <util/atomic.h>

volatile byte watchdogCounter;
	
void watchdogEvent() {
    ++watchdogCounter;
}

void watchdogInterrupts (char mode) {
    if (mode & bit(3))
        mode ^= bit(3) | bit(WDP3);
    byte wdtcsr = mode >= 0 ? bit(WDIE) | mode : 0;
    MCUSR &= ~(1<<WDRF);
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        WDTCSR |= (1<<WDCE) | (1<<WDE); // timed sequence
        WDTCSR = wdtcsr;
    }
}

void powerDown () {
    byte adcsraSave = ADCSRA;
    ADCSRA &= ~ bit(ADEN); // disable the ADC
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        sleep_enable();
    }
    sleep_cpu();
    sleep_disable();
    ADCSRA = adcsraSave;
}

void dosleep (char wdp) {
    watchdogCounter = 0;
    watchdogInterrupts(wdp);
    powerDown();
    watchdogInterrupts(-1); // off
}

void sleep4ms (unsigned long ms) {
   // not very accurate but good enough for what we do
    unsigned long ms_left = ms;
    while (ms_left > 8200) { dosleep(9); ms_left=ms_left-8200; }
    if (ms_left > 4100) { dosleep(8); ms_left=ms_left-4100; }
    if (ms_left > 2000) { dosleep(7); ms_left=ms_left-2000; }
    if (ms_left > 1000) { dosleep(6); ms_left=ms_left-1000; }
    if (ms_left > 500) { dosleep(5); ms_left=ms_left-500; }
    if (ms_left > 250) { dosleep(4); ms_left=ms_left-250; }
    if (ms_left > 125) { dosleep(3); ms_left=ms_left-125; }
    if (ms_left > 60) { dosleep(2); ms_left=ms_left-60; }
    if (ms_left > 30) { dosleep(1); ms_left=ms_left-30; }
    if (ms_left > 15) { dosleep(1); }
}





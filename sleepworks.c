#define F_CPU 1000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/wdt.h> 
#include <avr/sfr_defs.h>
#include <avr/pgmspace.h>

#define BUTTON_PIN 1
#define BUTTON_PC 1
#define LED_PIN 3

void setup(void);
void loop(void);
void blink(uint8_t times, uint8_t speed);

volatile uint8_t pcint_count;

int main(void)
{
    setup();
	while (1) loop();
}

void blink(uint8_t times, uint8_t speed)
{
    uint8_t i, j;

    for (i = 0; i < times; i++) {
        PORTB |= _BV(LED_PIN);
        for (j = 0; j < speed; j++) {
            _delay_ms(10);
        }
        PORTB &= ~(_BV(LED_PIN));
        for (j = 0; j < speed; j++) {
            _delay_ms(10);
        }
    }

    _delay_ms(1200);
}

void setup()
{
//    blink(3, 30);
	cli();
    // Set LED pin to output, bring high (P-channel MOSFET)
    // Enable pullup on button
    DDRB = _BV(LED_PIN);
    PORTB = _BV(BUTTON_PIN);

    // Pin change interrupt for the button
    PCMSK = _BV(BUTTON_PC);
    GIMSK = _BV(PCIE);
	GIFR = _BV(PCIF);
	sei();
    _delay_ms(200);
	
	set_sleep_mode(SLEEP_MODE_IDLE);
    blink(5, 30);
}

void loop()
{
	sleep_enable();
	sleep_cpu();
	sleep_disable();
	blink(2, 15);
	return;
    volatile uint8_t blinks = pcint_count;
    if (pcint_count == 0) return;
    blink(blinks, 10);
    pcint_count = 0;
//	PORTB = (PORTB & _BV(BUTTON_PIN)) << (BUTTON_PIN - LED_PIN);
// 	if (!(PINB & _BV(BUTTON_PIN))) {
// 		_delay_ms(100);
// 		if (!(PINB & _BV(BUTTON_PIN))) {
// 			blink(1,30);
// 		}
// 	}
}

ISR(PCINT0_vect) {
    pcint_count++;
}

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

#define MODE_OFF 0
#define MODE_LOW 1
#define MODE_HIGH 2
#define MODE_SLOWBLINK 3
#define MODE_FASTBLINK 4
#define MODE_MAX 5

volatile uint8_t mode;
volatile uint16_t pcint_count;
volatile uint16_t timer0_count = 0;
volatile uint8_t blink_on = 0;

uint8_t bright = 0;
uint8_t dim = 180;

int main(void)
{
    setup();
	while (1) loop();
}

void setup()
{
	cli();
    // Set LED pin to output, bring high (P-channel MOSFET)
    // Enable pullup on button
    DDRB = _BV(LED_PIN);
    PORTB = _BV(BUTTON_PIN);
	
	mode = MODE_OFF;

    // Pin change interrupt for the button
    PCMSK = _BV(BUTTON_PC);
    GIMSK = _BV(PCIE);
	GIFR = _BV(PCIF);

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    // Timer0 for blinking
    TCCR0B = _BV(CS02) | _BV(CS00);     // Timer0 prescaler /1024
    TIMSK = _BV(TOIE0);
    TCNT0 = 0;

    // Timer1 for PWM

	TCCR1 = 0;
	OCR1C = 255;
	OCR1B = 255;
	
    GTCCR = _BV(PWM1B) | _BV(COM1B0);   // Timer1 PWM mode, OCR1B only
//    TCCR1 = _BV(CS13) | _BV(CS10);
    //PORTB &= ~(_BV(LED_PIN));
    TCNT1 = 0;
	TCCR1 = _BV(CS11);

//    PRR |= _BV(PRTIM1);
	sei();
}

void loop()
{
	sleep_enable();
	sleep_cpu();
	sleep_disable();

    cli();

    if (!(PINB & _BV(BUTTON_PIN))) {
        _delay_ms(100);
        if (!(PINB & _BV(BUTTON_PIN))) {
            if (++mode == MODE_MAX) {
                mode = 0;
            }

            switch (mode) {
                case MODE_OFF:
                    OCR1B = 255;
//                    PRR |= _BV(PRTIM1);
//                    PORTB &= ~(_BV(LED_PIN));
                    break;
                case MODE_LOW:
                    OCR1B = dim;
                    set_sleep_mode(SLEEP_MODE_IDLE);
//                    PRR &= ~(_BV(PRTIM1));
//                    TCNT1 = 0;
                    break;
                case MODE_HIGH:
                    OCR1B = bright;
                    set_sleep_mode(SLEEP_MODE_IDLE);
//                    PRR &= ~(_BV(PRTIM1));
//                    TCNT1 = 0;
                    break;
                case MODE_SLOWBLINK:
                case MODE_FASTBLINK:
                    OCR1B = 255;
                    blink_on = 0;
                    TCNT0 = 0;
                    timer0_count = 0;
                    set_sleep_mode(SLEEP_MODE_IDLE);
                    break;
            }

            // Wait until button is released
            while (!(PINB & _BV(BUTTON_PIN)));
        }
    }

    sei();
}

ISR(PCINT0_vect) {
    pcint_count++;
}

ISR(TIMER0_OVF_vect) {
    timer0_count++;

    if ((mode == MODE_SLOWBLINK && timer0_count == 3) || (mode == MODE_FASTBLINK && timer0_count == 1)) {
        blink_on = 1 - blink_on;

        if (blink_on) {
            OCR1B = bright;
        } else {
            OCR1B = 255;
        }

        timer0_count = 0;
    }
}

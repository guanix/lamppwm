/*
 * lamppwm.c
 *
 * Created: 1/21/2013 3:47:13 PM
 * Author: rmd, guan
 */ 

#define DEBOUNCE_MS         50
#define STARTUP_LED_MS      700
#define SLOWBLINK_ON_MS     1000
#define SLOWBLINK_OFF_MS    700
#define FASTBLINK_ON_MS     300
#define FASTBLINK_OFF_MS    200

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/wdt.h> 
#include <avr/sfr_defs.h>
#include <avr/pgmspace.h>

#define MODE_OFF 0
#define MODE_ON 1
#define MODE_SLOWBLINK 2
#define MODE_FASTBLINK 3
#define MODE_FADE 4
#define MODE_MAX 5

#define BUTTON_PIN 1
#define BUTTON_PC 1
#define LED_PIN 3

void setup(void);
void loop(void);
void sleep(uint8_t sleep_mode);
void blink(uint8_t times);

volatile uint8_t mode;
volatile uint8_t full_pwm;
volatile uint16_t count;
volatile uint16_t adc_count;
volatile uint8_t sleep_mode;

volatile uint8_t slowblink_on;
volatile uint8_t fastblink_on;
volatile uint8_t fade_phase;

int main(void)
{
    setup();
	while (1) loop();
}

void sleep(uint8_t sm)
{
    sleep_mode = sm;
    // enable the pin change interrupt
    GIMSK |= _BV(PCIE);

    if (sm == SLEEP_MODE_PWR_DOWN) {
        // shut down timer0 and timer1
        PRR |= _BV(PRTIM1) | _BV(PRTIM0);
        blink(1);
    }

    set_sleep_mode(sm);
    sleep_enable();
    sleep_cpu();
    sei();

    // We've just woken up from sleep, disable
    cli();
    sleep_disable();
    blink(2);

    if (GIFR & _BV(PCIF)) {
        blink(1);
    }

    // disable the pin change interrupt
    GIMSK &= ~(_BV(PCIE));

    if (sm == SLEEP_MODE_PWR_DOWN) {
        // turn timer1 back on
        PRR &= ~(_BV(PRTIM1));

        // turn timer0 back on if we're in a mode that requires it
        if (mode == MODE_SLOWBLINK || mode == MODE_FASTBLINK || mode == MODE_FADE) {
            PRR = 0;
        }
    }

    sei();
}

void blink(uint8_t times)
{
    uint8_t i;

    DDRB |= _BV(LED_PIN);
    PORTB |= _BV(BUTTON_PIN);

    for (i = 0; i < times; i++) {
        PORTB |= _BV(LED_PIN);
        _delay_ms(150);
        PORTB &= ~(_BV(LED_PIN));
        _delay_ms(150);
    }

    _delay_ms(2000);
}

void setup()
{
    wdt_disable();

    blink(3);

    // Initialize full power
    full_pwm = 100;

    // Pin change interrupt for the button
    PCMSK = _BV(BUTTON_PC);

    // Set LED pin to output, bring high (P-channel MOSFET)
    // Enable pullup on button
    DDRB = _BV(LED_PIN);
    PORTB = _BV(BUTTON_PIN);

    _delay_ms(1000);

    // Turn on for 500 ms to show that we are working

#if 0
    // Set up the timer with a sensible value
    // see Table 12-1 on page 89
    // Timer 1 used for LED PWM, timer 0 for other operations
    GTCCR = _BV(PWM1B) | _BV(COM1B1);   // Timer1 PWM mode, OCR1B only
    TCCR0B = _BV(CS02) | _BV(CS00);     // Timer0 prescaler /1024
    TCNT0 = 0;                          // reset the timers
    TCCR1 = _BV(CS13) | _BV(CS10);

    TCNT1 = 0;
    OCR1C = 255;
    OCR1B = 0;
    TIMSK = _BV(TOIE0);                 // Timer0 overflow interrupt
  // ... but turn the timer off
    pwm_off();

  // Start in off mode
    mode = MODE_OFF;

    // Reset the Timer0 count
    count = 0;
    adc_count = 0;
#endif

#if 0
    // PWM for 500 ms too
    _delay_ms(STARTUP_LED_MS);
    pwm_on();
    OCR1B = 50;
    _delay_ms(STARTUP_LED_MS);
    OCR1B = 0;
    pwm_off();
#endif

    sleep(SLEEP_MODE_PWR_DOWN);

    blink(4);
}

void loop()
{
    // Read the ADC when we hit 2^16 ms
    if (adc_count == 0) {
        // adc code here
    }

    // Is there a button push?
    if (!(PORTB & _BV(BUTTON_PIN))) {
        _delay_ms(DEBOUNCE_MS);
        if (!(PORTB & _BV(BUTTON_PIN))) {
            blink(5);

            // button was pressed
            mode++;
            if (mode == MODE_MAX) {
                mode = 0;
            }
        } else {
            sleep(sleep_mode);
            return;
        }
    } else {
        sleep(sleep_mode);
        return;
    }

    // Sleep until we see a button push
    switch (mode) {
        case MODE_OFF:
            PRR = _BV(PRTIM1) | _BV(PRTIM0); // turn off timer0 and timer1
            sleep(SLEEP_MODE_PWR_DOWN);
            break;
        case MODE_ON:
            PRR = _BV(PRTIM0);
            count = 0;
            adc_count = 0;
            OCR1B = full_pwm;
            sleep(SLEEP_MODE_IDLE);
            break;
        case MODE_SLOWBLINK:
            PRR = 0;
            OCR1B = full_pwm;
            slowblink_on = 1;
            sleep(SLEEP_MODE_IDLE);
            break;
        case MODE_FASTBLINK:
            PRR = 0;
            OCR1B = full_pwm;
            slowblink_on = 1;
            sleep(SLEEP_MODE_IDLE);
            break;
        case MODE_FADE:
            PRR = 0;
            fade_phase = 0;
            OCR1B = full_pwm;
            sleep(SLEEP_MODE_IDLE);
            break;
    }
}

EMPTY_INTERRUPT(TIMER0_OVF_vect);
EMPTY_INTERRUPT(PCINT0_vect);

// Timer0 overflow
#if 0
ISR(TIMER0_OVF_vect) {
    return;
    count++;
    adc_count++;

    // count is 1 MHz /1024, or close enough to 1 KHz
    switch (mode) {
        case MODE_SLOWBLINK:
            if (slowblink_on && count == SLOWBLINK_ON_MS) {
                OCR1B = 0;
                count = 0;
                slowblink_on = 0;
            } else if (!slowblink_on && count == SLOWBLINK_OFF_MS) {
                OCR1B = full_pwm;
                count = 0;
                slowblink_on = 1;
            }
            break;
        case MODE_FASTBLINK:
            if (fastblink_on && count == FASTBLINK_ON_MS) {
                OCR1B = 0;
                count = 0;
                fastblink_on = 0;
            } else if (!fastblink_on && count == FASTBLINK_OFF_MS) {
                OCR1B = full_pwm;
                count = 0;
                fastblink_on = 1;
            }
            break;
    }
}
#endif

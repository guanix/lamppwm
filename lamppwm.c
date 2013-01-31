/*
 * lamppwm.c
 *
 * Created: 1/21/2013 3:47:13 PM
 * Author: rmd, guan
 */ 

#define DEBOUNCE_MS 100
#define STARTUP_LED_MS 500

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/wdt.h> 
#include <avr/sfr_defs.h>

#define MODE_OFF 0
#define MODE_ON 1
#define MODE_SLOWBLINK 2
#define MODE_FASTBLINK 3
#define MODE_FADE 4
#define MODE_MAX 5

#define BUTTON_PIN PB1
#define BUTTON_PC PCINT1
#define LED_PIN PB3
#define LED_PWM OCR1B

void setup(void);
void loop(void);
void sleep(uint8_t sleep_mode);
void pwm_on(void);
void pwm_off(void);

volatile uint8_t mode;

int main(void)
{
    setup();
	while (1) loop();
}

void sleep(uint8_t sleep_mode)
{
    // sleep and wake up on the button pin change
	PCMSK = _BV(BUTTON_PIN);
    set_sleep_mode(sleep_mode);
    sleep_enable();
    sleep_bod_disable();
    sei();
    sleep_cpu();

    // We've just woken up from sleep, disable
    cli();
    sleep_disable();
    PCMSK = 0;
}

// Turn PWM on
inline void pwm_on(void)
{
    // OC1B cleared on compare match, set on TCNT1 == 0
    // ^OC1B not connected
    TCCR1 = _BV(CS13) | _BV(CS10);
    TCNT1 = 0;
}

inline void pwm_off(void)
{
    TCCR1 = 0;
    TCNT1 = 0;
}

inline void setup()
{
    // Disable watchdog timer
    wdt_disable();

    // Set LED pin to output, bring high (N-channel MOSFET)
    // Set button to output, enable pullup
    DDRB = _BV(LED_PIN);
    PORTB = _BV(LED_PIN) | _BV(BUTTON_PIN);

    // Turn on for 500 ms to show that we are working
    PORTB |= _BV(LED_PIN);
    _delay_ms(STARTUP_LED_MS);
    PORTB &= ~(_BV(LED_PIN));

    // Set up the timer with a sensible value
    // see Table 12-1 on page 89
    GTCCR = _BV(PWM1B) | _BV(COM1B1);
    OCR1C = 255;
    OCR1B = 0;
    PLLCSR = 0;
    // ... but turn the timer off
    pwm_off();

    // Start in off mode
    mode = MODE_OFF;
}

inline void loop()
{
    // Is there a button push?
    if (!bit_is_set(PORTB, BUTTON_PIN)) {
        _delay_ms(DEBOUNCE_MS);
        if (!bit_is_set(PORTB, BUTTON_PIN)) {
            // button was pressed
            mode++;
            if (mode == MODE_MAX) {
                mode = 0;
            }
        }
    }

    // Sleep until we see a button push
    switch (mode) {
        case MODE_OFF:
            pwm_off();
            sleep(SLEEP_MODE_PWR_DOWN);
            break;
        case MODE_ON:
            pwm_on();
            OCR1B = 250;
            sleep(SLEEP_MODE_IDLE);
            break;
        case MODE_SLOWBLINK:
            pwm_on();
            OCR1B = 30;
            sleep(SLEEP_MODE_IDLE);
            break;
        case MODE_FASTBLINK:
            pwm_on();
            OCR1B = 120;
            sleep(SLEEP_MODE_IDLE);
            break;
        case MODE_FADE:
            pwm_on();
            OCR1B = 180;
            sleep(SLEEP_MODE_IDLE);
            break;
    }
}

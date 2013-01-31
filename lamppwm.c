/*
 * lamp2.c
 *
 * Created: 1/21/2013 3:47:13 PM
 *  Author: rmd
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

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

#define DEBOUNCE_MS 100

void setup(void);
void loop(void);
void sleep(uint8_t sleep_mode);
void pwm_on(void);
void pwm_off(void);

uint16_t count;
volatile uint8_t mode = MODE_OFF;

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
void pwm_on(void)
{
    GTCCR = _BV(PWM1B) | _BV(COM1B0);
    TCCR1 = _BV(CS13) | _BV(CS10);
    TCNT1 = 0;
    OCR1C = 255;
    OCR1B = 60;
}

void pwm_off(void)
{
    GTCCR = 0;
    TCCR1 = 0;
    TCNT1 = 0;
}

int i, dir;

void setup()
{
    // Set LED pin to output, bring high (N-channel MOSFET)
    // Set button to output, enable pullup
    DDRB = _BV(LED_PIN);
    PORTB = _BV(LED_PIN) | _BV(BUTTON_PIN);
}

void loop() 
{
    // Sleep until we see a button push

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

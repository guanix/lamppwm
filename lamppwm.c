/*
 * lamp2.c
 *
 * Created: 1/21/2013 3:47:13 PM
 *  Author: rmd
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define MODE_OFF 0
#define MODE_SLOWBLINK 1
#define MODE_FASTBLINK 2
#define MODE_FADE 3

#define BUTTON_PIN PB1
#define BUTTON_PC PCINT1

void setup(void);
void loop(void);
void sleep(void);
void pwm_on(void);

uint16_t count;
volatile uint8_t mode = MODE_OFF;

int main(void)
{
    setup();
	while (1) loop();
}

void sleep(void)
{
    // sleep and wake up on the button pin change
	PCMSK = _BV(BUTTON_PIN);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_bod_disable();
    sei();
    sleep_cpu();

    // We've just woken up from sleep, disable
    cli();
    sleep_disable();
    PCMSK = 0;
}

// Turn PWM on with given duty cicle
void pwm_on(void)
{
    
}

int i, dir;

void setup()
{
}

void loop() 
{
	
}

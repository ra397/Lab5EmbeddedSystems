/*
 * Lab5.c
 *
 * Created: 3/26/2024 12:24:30 PM
 * Author : Rabi Alaya and Alex Banning
 */ 

#ifndef F_CPU
#define F_CPU 8000000UL // 8 MHz clock speed
#endif

#include <avr/io.h>
#include <avr/power.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LCD_lib/defines.h"
#include "LCD_lib/lcd.h"

FILE lcd_str = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE);

#define PWM_PERIOD 1000

volatile uint8_t on_time = 10;

void timer0_init();

ISR(TIMER0_COMPA_vect)
{
	static uint8_t is_high = 0;

	if (is_high) {
		// If the pin was high, set it low and adjust OCR0A for the remainder of the period
		PORTD &= ~(1 << 5);
		OCR0A = PWM_PERIOD - on_time;
		is_high = 0;
		} else {
		// If the pin was low, set it high and adjust OCR0A for the on_time
		PORTD |= (1 << 5);
		OCR0A = on_time;
		is_high = 1;
	}
}

int main(void)
{
	stdout = &lcd_str; // redefines std output to output to LCD file output, that file is what writes to LCD
	clock_prescale_set(2);

	lcd_init();
	printf("I GOT IT TO WORK");
	
	DDRD = DDRD | (1 << 5); // set PD5 as output
	
	timer0_init();
	
	
    while (1) {

    }
	
}

void timer0_init() {
	// Set the timer to CTC mode (Clear Timer on Compare Match) (WGM01 = 1, WGM00 = 0)
	TCCR0A |= (1 << WGM01);
	
	// Set the pre-scaler to 1024 and start the timer
	TCCR0B |= (1 << CS02) | (1 << CS00);

	// Enable Timer0 Output Compare Match A Interrupt
	TIMSK0 |= (1 << OCIE0A);
	
	// Set OCR0A for the compare match value
	OCR0A = PWM_PERIOD;

	// Enable global interrupts
	sei();
}


/*
 * Lab5.c
 *
 * Created: 3/26/2024 12:24:30 PM
 * Author : Rabi Alaya and Alex Banning
 */ 

#ifndef F_CPU
#define F_CPU 16000000UL // 16 MHz clock speed
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

#define TOP 200
#define RPG_A_PIN PINB1
#define RPG_B_PIN PINB0
#define RPG_PORT PINB

volatile uint8_t COMPARE = 100;

void timer0_init();
void read_rpg();
void display_to_LCD();
void doubleToString(double double_value, char* str_freq, size_t max_len);


ISR(TIMER0_OVF_vect)
{
	// Update the COMPARE value
	OCR0B = COMPARE;
} 

int main(void)
{
	stdout = &lcd_str; // redefines std output to output to LCD file output, that file is that writes to LCD
	clock_prescale_set(1);

	lcd_init();
	
	DDRD = DDRD | (1 << 5); // set PD5 as output
	DDRB = DDRB & ~((1 << RPG_A_PIN) | (1 << RPG_B_PIN)); // Set RPG pins as inputs
	PORTB |= (1 << RPG_A_PIN) | (1 << RPG_B_PIN);
	
	
	timer0_init();
	
	printf("Hello TA:");
	
    while (1) {
		display_to_LCD();
		read_rpg();
		_delay_ms(10);
    }
}

/*
* Function that initializes and starts 8-bit Timer0.
* https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf
* WGM00 = 1, WGM01 = 0, WGM02 = 1 for Mode 5 (PWM, phase correct) from table 14-8
* COM0B0 = 0, COM0B1 = 1 to clear OC0B on compare match when up-counting. Set OC0B on compare match when
down-counting. Table 14-7. This is what is toggling OC0B, thus toggling PD5.
* CS00 = 1, CS01 = 0, CS02 = 0 for no prescaling from table 14-9
*/
void timer0_init() {
	// Set mode
	TCCR0A |= (1 << WGM00) | (1 << COM0B1); // Set timer to PWM, phase correct mode (mode 5), WGM00 = 1, WGM02 = 1
	
	// Set the prescaler
	TCCR0B = (1 << CS00) | (1 << WGM02); // No prescaling
	
	// Set OCR0A to TOP value and OCR0B to COMPARE value
	OCR0A = TOP;
	
	OCR0B = COMPARE;

	// Enable overflow interrupt
	TIMSK0 |= (1 << TOIE0);
	
	// Enable global interrupts
	sei();
}

/*
* A function converts a double value to a string
*/
void doubleToString(double double_value, char *str_freq, size_t max_len) {
	// Extract integer part
	int intPart = (int) double_value;
	
	// Extract fractional part
	int fracPart = (int)((double_value - intPart) * 100); // Considering two decimal places

	// Convert to string
	snprintf(str_freq, max_len, "%d.%02d", intPart, fracPart);
}

/*
* A function that reads the state of the RPG and updates COMPARE accordingly6
*/
void read_rpg() {
	static uint8_t old_state = 0;
	uint8_t new_state = RPG_PORT & ((1 << RPG_A_PIN) | (1 << RPG_B_PIN));

	if (new_state == old_state) return; // No change

	// Determine direction based on state changes
	if (((old_state == 0x00) && (new_state == 0x01)) ||
	((old_state == 0x01) && (new_state == 0x03)) ||
	((old_state == 0x03) && (new_state == 0x02)) ||
	((old_state == 0x02) && (new_state == 0x00))) {
		// Clockwise rotation
		COMPARE--;
		} else {
		// Counterclockwise rotation
		COMPARE++;
	}

	old_state = new_state;
}

void display_to_LCD() {
	row2();
	double dutyCycle = (double) COMPARE / TOP;
	char str_freq[20];
	doubleToString(dutyCycle, str_freq, sizeof(str_freq));
	printf("Duty Cycle: %s", str_freq);
}

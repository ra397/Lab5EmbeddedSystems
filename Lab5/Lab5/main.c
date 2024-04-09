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

#define RPG_A_PIN PINB1
#define RPG_B_PIN PINB0
#define RPG_PORT PINB

static uint16_t TOP = 200;
volatile uint16_t COMPARE = 100;

volatile uint8_t encoder_old_state = 0;

volatile uint16_t pulse_count = 0;
volatile uint16_t rpm = 0;

void timer0_init();
void timer1_init();
void read_rpg();
void display_to_LCD();

// timer0 overflow ISR
ISR(TIMER0_OVF_vect)
{
	cli();
	OCR0B = COMPARE; // Update the COMPARE value
	sei();
}

// PD3 Toggle ISR
ISR(INT1_vect)
{
	cli();
	pulse_count++;
	sei();
}

// timer1 overflow ISR
ISR(TIMER1_OVF_vect) 
{
	cli();
	uint16_t pulses = pulse_count;
	pulse_count = 0; // Reset pulse count for the next period
	rpm = (pulses/4) * 60;
	sei();
}

int main(void)
{
	stdout = &lcd_str; // redefines std output to output to LCD file output, that file is that writes to LCD
	clock_prescale_set(1);

	lcd_init();
	
	DDRD = DDRD | (1 << 5); // set PD5 as output
	
	DDRB = DDRB & ~((1 << RPG_A_PIN) | (1 << RPG_B_PIN)); // Set RPG pins as inputs
	PORTB |= (1 << RPG_A_PIN) | (1 << RPG_B_PIN); // Enable pull-up resistors on RPG
	
	DDRD &= ~(1 << 3); // Set PD3 as input
	// Setup PD3 ISR
	EICRA |= (1 << ISC11) | (1 << ISC10);
	EIMSK |= (1 << INT1);
	
	timer0_init();
	timer1_init();
		
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

void timer1_init() {
	// Configure Timer1
	TCCR1A = 0; // Set entire TCCR1A register to 0
	TCCR1B = 0; // Same for TCCR1B

	// Set timer count for 1s overflow assuming a 16MHz clock with 1024 prescaler
	// For different clock speeds or prescalers, adjust this value
	TCNT1 = 63973;

	TCCR1B |= (1 << CS10) | (1 << CS12); // Set 1024 prescaler
	TIMSK1 |= (1 << TOIE1); // Enable timer overflow interrupt
	sei(); // Enable global interrupts
}

/*
* A function that reads the state of the RPG and updates COMPARE accordingly
*/
void read_rpg() {
	uint8_t encoder_new_state = RPG_PORT & ((1 << RPG_A_PIN) | (1 << RPG_B_PIN));

	if (encoder_new_state == encoder_old_state) return; // No change

	// Determine direction based on state changes
	if (((encoder_old_state == 0x00) && (encoder_new_state == 0x01)) ||
	((encoder_old_state == 0x01) && (encoder_new_state == 0x03)) ||
	((encoder_old_state == 0x03) && (encoder_new_state == 0x02)) ||
	((encoder_old_state == 0x02) && (encoder_new_state == 0x00))) {
		COMPARE--;
		} else {
		COMPARE++;
	}
	
	if (COMPARE > 200) COMPARE = 200;
	if (COMPARE < 0) COMPARE = 0;

	encoder_old_state = encoder_new_state;
}

void display_to_LCD() {
	home();
	char str_rpm[20];
	sprintf(str_rpm, "RPM = %d", rpm);
	printf("%s", str_rpm);
	
	row2();
	float dutyCycle = (100.0 * ((float) ((2 * (float) COMPARE + 1) / (2 * TOP)))) - 0.25;
	char str_duty[20];
	sprintf(str_duty, "D = %4.2f", dutyCycle);
	printf("%s", str_duty);
}
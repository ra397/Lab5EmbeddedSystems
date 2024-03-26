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
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LCD_lib/defines.h"
#include "LCD_lib/lcd.h"

FILE lcd_str = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE);

int main(void)
{
	stdout = &lcd_str; // redefines std output to output to LCD file output, that file is what writes to LCD
	clock_prescale_set(2);

	lcd_init();

	printf("HELLO WORLD");
	

    while (1) {

    }
}


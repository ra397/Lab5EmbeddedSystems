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
#include "LCD_lib/lcd.h"

int main(void)
{
	lcd_init(2);
	lcd_clrscr();
	lcd_putc('A');

    while (1) 
    {
		
    }
}


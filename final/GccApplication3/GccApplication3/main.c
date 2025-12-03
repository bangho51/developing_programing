/*
 * GccApplication3.c
 *
 * Created: 2025-12-01 오후 1:03:38
 * Author : jbhma
 */ 

#include <avr/io.h>


int main(void)
{
    /* Replace with your application code */
	DDRB=0xff;
    while (1) 
    {
		PORTB = 0x80;
    }
}


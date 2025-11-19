/*
 * ADC_ex3.c
 *
 * Created: 2025-10-22 오후 5:00:18
 * Author : jbhma
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>

extern void init_fnd(void);
extern void fnd_number(int number);

int c_adc;
unsigned char adc_ch=0;

// ADC interrupt service routine
ISR(ADC_vect)
{
	c_adc = ADC;
	ADCSRA|=(1<<ADSC);
}

void init_ADC_interrupt(void)
{
	sei();
	ADMUX = 0; // Single ended input channel 0
	ADCSRA = 0b11001111; // ADC enable, ADC Start Conversion, ADC Interrupt Enable, 125kHz
}

int main(void)
{
	init_ADC_interrupt();
	init_fnd();
	while(1)
	{
		fnd_number(c_adc);
	}
}
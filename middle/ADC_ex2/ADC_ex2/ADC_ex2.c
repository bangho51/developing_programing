/*
 * ADC_ex2.c
 *
 * Created: 2025-10-22 오후 4:58:46
 * Author : jbhma
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

extern void init_fnd(void);
extern void fnd_number(int number);

// Read the AD conversion result
unsigned int read_adc(unsigned char adc_channel)
{
	ADMUX = (ADMUX & 0b11100000) | adc_channel;
	_delay_us(10);
	while ((ADCSRA & (1<<ADIF))==0); // Wait for the AD conversion to complete
	ADCSRA|=(1<<ADIF);
	return ADC;
}

void init_ADC_free_running(void)
{
	ADMUX = 0; // Channel 0
	// ADEN | ADSC | ADFR | ADIF | ADIE | ADPS2 | ADPS1 | ADPS0;
	ADCSRA=0b11100111; // ADC enable, ADC Start Conversion, ADC free running, 125kHz
}

int main(void)
{
	int adc_data;
	
	init_ADC_free_running();
	init_fnd();
	while(1)
	{
		adc_data=read_adc(0); // 0~1023
		fnd_number(adc_data); // 0~255
	}
}
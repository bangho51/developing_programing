/*
 * ADC_ex1.c
 *
 * Created: 2025-10-22 오후 4:54:37
 * Author : jbhma
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

extern void init_fnd(void);
extern void fnd_number(int number);

/* --- ADC 채널에서 ADC 결과 읽기 --- */
unsigned int read_adc(unsigned char adc_channel)
{
	ADMUX = (ADMUX & 0b11100000) | adc_channel; // 채널 선택
	_delay_us(10); // ADC 입력 전압이 안정될 때까지 기다림
	ADCSRA |= (1<<ADSC); // AD conversion 시작
	while ((ADCSRA & (1<<ADIF))==0); // AD conversion 끝날 때까지 기다림
	return ADC;
}

/* --- ADC 초기화 --- */
void init_adc(void)
{
	// ADC Clock frequency: 125.000 kHz
	// ADC Voltage Reference: AREF pin
	// ADMUX : REFS1 | REFS0 | ADLAR | MUX4 | MUX3 | MUX2 | MUX1 | MUX0
	// ADCSRA : ADEN | ADSC | ADFR | ADIF | ADIE | ADPS2 | ADPS1 | ADPS0
	ADMUX = 0b00000000;
	ADCSRA = 0b10000111; // ADEN, prescaler 128, 125kHz
}

void init_LED(void)
{
	DDRA=0XFF; // PORTA를 출력으로 설정
}

void LED_on(unsigned char on)
{
	PORTA = on;
}

int main(void)
{
	unsigned int adc_data;
	unsigned char adc_ch=0;
	init_LED();
	init_fnd();
	init_adc();
	while(1)
	{
		adc_data=read_adc(adc_ch); // 0~1023
		LED_on((unsigned char)(adc_data/4)); // 0~255
		fnd_number(adc_data);
	}
}
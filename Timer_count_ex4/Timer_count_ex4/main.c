/*
 * Timer_count_ex4.c
 *
 * Created: 2025-10-22 오후 4:52:36
 * Author : jbhma
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#include <avr/interrupt.h>
#include <util/delay.h>

void init_OC0(void)
{
	DDRB = 0x10; // PB4(OC0) 출력으로 설정
}

void init_timer0_PCPWM(int prescaler, unsigned char ocr)
{
	// TCCR0 = FOC0 | WGM00 | COM01 | COM00 | WGM01 | CS02 | CS01 | CS00
	// PCPWM mode, Clear OC0 on compare match when up-counting,
	// Set OC0 on compare match when down-counting.
	TCCR0 = 0b01100000;
	if(prescaler == 1) TCCR0 |= 1;
	if(prescaler == 8) TCCR0 |= 2;
	if(prescaler == 32) TCCR0 |= 3;
	if(prescaler == 64) TCCR0 |= 4;
	if(prescaler == 128) TCCR0 |= 5;
	if(prescaler == 256) TCCR0 |= 6;
	if(prescaler == 1024) TCCR0 |= 7;
	OCR0 = ocr;
}

void OC0_0(void) // 소리 안나게 하기
{
	TCCR0 = 0;
	PORTB = 0;
}

void phone_bell(int count)
{
	int i,j;
	for(j=0; j<count; j++)
	{
		for(i=0; i<10; i++)
		{
			init_timer0_PCPWM(64,128); // 490Hz sound
			_delay_ms(25);
			init_timer0_PCPWM(128,128); // 245Hz sounds
			_delay_ms(25);
		}
		OC0_0();
		_delay_ms(2000);
	}
}

int main(void)
{
	init_OC0(); // PB4(OC0) 출력으로 설정
	phone_bell(3); // 벨 소리 3번 내기
}
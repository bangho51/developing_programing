/*
 * ATmega128_ex1.c
 *
 * Created: 2025-09-24 오후 10:37:11
 * Author : jbhma
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

void LED_PORTA(unsigned char on) 
{
	DDRA = 0xff; // 포트 A를 출력으로 설정
	PORTA = on;  // on의 bit가 1이면, PA0에 연결된 LED가 켜짐, bit1->PA1, ..., bit7->PA7
}

void LED_0_255(void) 
{
	// PA 0부터 255까지의 값을 출력
	unsigned char number = 0;
	while(1) 
	{
		LED_PORTA(number);
		_delay_ms(50); // 50ms 지연
		if(number == 0xff) break;
		number++;
	}
}

void LED_bit_left_shift(void) 
{
	// left shift 비트 연산자로 PA0에서 PA7까지 순차적으로 LED 켜기
	unsigned char bit = 1;
	while(bit != 0) {
		LED_PORTA(bit);
		_delay_ms(500);
		bit <<= 1;
	}
}

int main(void) 
{
	while(1) {
		LED_0_255();
		LED_bit_left_shift();
	}
}

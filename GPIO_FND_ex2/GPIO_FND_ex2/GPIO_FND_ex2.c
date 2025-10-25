/*
 * GPIO_FND_ex2.c
 *
 * Created: 2025-09-26 오후 10:44:12
 * Author : jbhma
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

#define absolute(x) (x < 0) ? -x : x
#define FND_DELAY 2 // 현재의 FND 디스플레이 최소 유지 시간
unsigned char fnd_digit[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71}; // common cathode FND 0~F
unsigned char fnd_minus = 0x40;
unsigned char fnd_dp = 0x80;

void init_fnd(void) 
{
	DDRC = 0xff; // PC 출력 fnd display, 1: ON, 0: OFF
	DDRG = 0x0f; // 4개의 fnd 중, 1: 선택, 0: 선택 안함
	PORTC = 0; // fnd 모두 꺼짐
	PORTG = 0; // fnd 선택 없음
}

void fnd(int ith, int digit) 
{
	PORTG = 1 << ith; // fnd 선택
	if (digit >= 0 && digit < 16) PORTC = fnd_digit[digit]; // 16진수 표시
	else PORTC = fnd_minus;
	_delay_ms(FND_DELAY);
}

void fnd_overflow(void) // 표현 불가 표시하기
{ 
	int i;
	for (i = 0; i < 4; i++) 
		fnd(i, -1);
}

void test_fnd(void) 
{
	int i, ith;
	for (ith = 0; ith < 4; ith++) 
	for (i = 0; i < 16; i++) {
			fnd(ith, i);
			_delay_ms(500);
	}
	
}

int main(void) {
	init_fnd();
	test_fnd();
}


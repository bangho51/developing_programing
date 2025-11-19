/*
 * init_fnd.c
 *
 * Created: 2025-11-03 오후 1:37:03
 * Author : jbhma
 */ 
#include <avr/io.h>
#define F_CPU 16000000UL // ATMEGA128의 클럭속도 16MHz
#include <util/delay.h>

#define absolute(x) (x<0) ? -x : x
#define FND_DELAY 2 // 현재의 FND 디스플레이 최소 유지 시간
unsigned char fnd_digit[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71}; // common cathode FND 0-F
unsigned char fnd_minus = 0x40;
unsigned char fnd_dp = 0x80;
void init_fnd(void)
{
	DDRC = 0xff; // PC 출력 fnd display, 1:ON, 0:OFF
	DDRG = 0x0f; // 4개의 fnd 중, 1:선택, 0:선택 안함
	PORTC = 0; // fnd 모두 꺼짐
	PORTG = 0; // fnd 선택 없음
}
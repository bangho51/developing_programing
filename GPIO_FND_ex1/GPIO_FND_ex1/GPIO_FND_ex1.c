/*
 * GPIO_FND_ex1.c
 *
 * Created: 2025-09-24 오후 10:43:56
 * Author : jbhma
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#define PULL_UP_DELAY 10000

void LED_PA(unsigned char on) 
{
	// PORT A로 on 값을 출력함
	DDRA = 0xff; // 포트 A를 출력으로 설정
	PORTA = on; // on의 bit가 1이면, PA0에 연결된 LED가 켜짐, bit1->PA1, ..., bit7->PA7
}

void PD_pull_up(unsigned char on, unsigned char *button) 
{
	// 포트 D를 입력으로 설정
	DDRD = 0x00;
	PORTD = on; // 포트 D의 내부 풀업 저항을 활성화(1), 비활성화(0)
	*button = PIND; // 포트 D의 입력값을 읽어들임
}

void PD_pull_up_on_off(void) 
{
	unsigned char button;
	while(1) 
	{
		PD_pull_up(0b10101010, &button); // PA7~PA0 값을 활성/비활성/.../활성/비활성
		LED_PA(button);
		_delay_ms(PULL_UP_DELAY); // PULL_UP_DELAY 시간을 조절하면서 LED 상태 확인해 보기
		PD_pull_up(0b01010101, &button);
		LED_PA(button);
		_delay_ms(PULL_UP_DELAY);
	}
}

int main(void) 
{
	PD_pull_up_on_off();
}

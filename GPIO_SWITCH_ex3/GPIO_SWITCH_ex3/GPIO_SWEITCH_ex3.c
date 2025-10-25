/*
 * GPIO_SWITCH_ex3.c
 *
 * Created: 2025-09-26 오후 11:23:23
 * Author : jbhma
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

#define SW_DELAY 10

void LED_PA(unsigned char on) // PORT A로 on 값을 출력함
{
	DDRA = 0xff; // 포트 A를 출력으로 설정
	PORTA = on; // on의 bit가 1이면, PA0에 연결된 LED가 켜짐, bit1->PA1, ..., bit7->PA7
}

unsigned char sw_pe4(void)
{
	if((PINE & (1 << PE4)) == 0) // PE4 눌림
	{
		_delay_ms(SW_DELAY); // 디바운싱 처리
		if((PINE & (1 << PE4)) == 0) return 1 << PE4; // PE4 눌림
	}
	return 0; // PE4 안눌림
}

void LED_PE4(void)
{
	while(1)
	{
		LED_PA(sw_pe4());
	}
}

int main(void)
{
	LED_PE4();
}
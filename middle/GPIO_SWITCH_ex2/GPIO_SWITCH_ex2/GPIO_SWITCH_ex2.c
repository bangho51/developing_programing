/*
 * GPIO_SWITCH_ex2.c
 *
 * Created: 2025-09-26 오후 11:20:35
 * Author : jbhma
 */ 

#include <avr/io.h>

void button_PE(unsigned char *button)
{
	DDRE = 0x00; // 포트 E를 입력으로 설정
	*button = PINE; // 포트 E의 입력 레지스터 PINE의 값을 button에 저장
}

void LED_PA(unsigned char on) // PORT A로 on 값을 출력함
{
	DDRA = 0xff; // 포트 A를 출력으로 설정
	PORTA = on; // on의 bit가 1이면, PA0에 연결된 LED가 켜짐, bit1->PA1, ..., bit7->PA7
}

void button2LED(void)
{
	unsigned char button;
	button_PE(&button);
	LED_PA(button);
}

int main(void)
{
	while (1)
	{
		button2LED();
	}
}
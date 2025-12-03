#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

int main(void)
{
	DDRB = 0xFF; // 포트 B 출력 설정

	while (1)
	{
		// 1. 단순 무식하게 정방향 회전 (PWM 아님)
		// LB1630 진리표: IN1=High, IN2=Low -> Forward
		PORTB |= (1 << PB6);  // PB6 켜기
		PORTB &= ~(1 << PB7); // PB7 끄기
		
		// 이 상태로 유지 (만약 여기서 안 돌면 배선/드라이버 문제 100%)
	}
}
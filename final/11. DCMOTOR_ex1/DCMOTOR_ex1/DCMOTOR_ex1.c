/*
 * DCMOTOR_ex1.c
 *
 * Created: 2025-11-17 오후 2:42:09
 * Author : jbhma
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>

#define IN1 PB6
#define IN2 PB7

// --- 기본 모터 제어 함수 ---
void DC_Forward(void)
{
	PORTB |= 1<<IN1;    // PB6 = 1
	PORTB &= ~(1<<IN2); // PB7 = 0
}

void DC_Reverse(void)
{
	PORTB &= ~(1<<IN1); // PB6 = 0
	PORTB |= 1<<IN2;    // PB7 = 1
}

void DC_standby(void)
{
	// 보통 H-Bridge에서 둘 다 High면 Brake 모드이지만,
	// 여기서는 변수명이 standby로 되어 있습니다.
	PORTB |= (1<<IN1 | 1<<IN2);
}

// --- PWM 제어 함수 (50Hz, 20ms 주기) ---

// Forward는 _delay_ms를 사용하여 구현됨
void PWM_Forward(double duty_ratio)
{
	double On;
	PORTB |= 1<<IN1;    // PB6 = 1
	PORTB &= ~(1<<IN2); // PB7 = 0
	
	On = duty_ratio * .2; // 100%일 때 20ms
	
	_delay_ms(On);
	DC_standby();
	_delay_ms(20.0 - On);
}

// Reverse는 _delay_us를 사용하여 정밀하게 구현됨
void PWM_Reverse(double duty_ratio)
{
	double On;
	PORTB &= ~(1<<IN1); // PB6 = 0
	PORTB |= 1<<IN2;    // PB7 = 1
	
	On = duty_ratio * 200; // 100%일 때 20000us (=20ms)
	
	_delay_us(On);
	DC_standby();
	_delay_us(20000.0 - On);
}

// --- 메인 함수 ---
int main(void)
{
	double duty_ratio;
	int i;

	DDRB = 0xff; // PORTB 출력 설정

	while (1)
	{
		// 1. 단순 On/Off 제어 테스트
		DC_Forward();
		_delay_ms(3000);
		DC_standby();
		_delay_ms(1000);
		DC_Reverse();
		_delay_ms(3000);
		DC_standby();
		_delay_ms(1000);

		// 2. PWM을 이용한 가속/감속 테스트
		
		// 정방향 가속 (0 -> 100)
		for(duty_ratio = 0.0; duty_ratio <= 100.0; duty_ratio += 10.)
		{
			for(i=0; i<100; i++) // 각 속도를 일정 시간 유지
			PWM_Forward(duty_ratio);
		}

		// 정방향 감속 (100 -> 0)
		for(duty_ratio = 100.0; duty_ratio >= 0.0; duty_ratio-= 10.)
		{
			for(i=0; i<100; i++)
			PWM_Forward(duty_ratio);
		}

		// 역방향 가속 (0 -> 100)
		for(duty_ratio = 0.0; duty_ratio <= 100.0; duty_ratio += 10.)
		{
			for(i=0; i<100; i++)
			PWM_Reverse(duty_ratio);
		}

		// 역방향 감속 (100 -> 0)
		for(duty_ratio = 100.0; duty_ratio >= 0.0; duty_ratio -= 10.)
		{
			for(i=0; i<100; i++)
			PWM_Reverse(duty_ratio);
		}
	}
}
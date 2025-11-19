/*
 * DCMOTOR_ex1.c
 *
 * Created: 2025-11-17 오후 2:42:09
 * Author : jbhma
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#define _DELAY_BACKWARD_COMPATIBLE_
#include <util/delay.h>

#define IN1 PB6
#define IN2 PB7

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
	PORTB |= (1<<IN1 | 1<<IN2);
}
void PWM_Forward(double duty_ratio) // %값 입력, 50Hz (20ms) 주기의 pwm
{
	double On;
	PORTB |= 1<<IN1;    // PB6 = 1
	PORTB &= ~(1<<IN2); // PB7 = 0
	On = duty_ratio*0.2;
	_delay_ms(On);
	DC_standby();
	_delay_ms(20.0-On);
}

void PWM_Reverse(double duty_ratio) // %값 입력, 50Hz (20ms) 주기의 pwm
{
	double On;
	PORTB &= ~(1<<IN1); // PB6 = 0
	PORTB |= 1<<IN2;    // PB7 = 1
	On = duty_ratio*200;
	_delay_us(On);
	DC_standby();
	_delay_us(20000.0-On);
}
int main(void)
{
	double duty_ratio;
	int i;

	// PB6(IN1)와 PB7(IN2)를 출력으로 설정
	DDRB |= (1<<IN1) | (1<<IN2);
	
	while (1)
	{
		// --- 1. 방향 테스트 (최대 속도) ---
		DC_Forward();
		_delay_ms(3000); // 3초 정회전
		DC_standby();
		_delay_ms(1000); // 1초 정지
		DC_Reverse();
		_delay_ms(3000); // 3초 역회전
		DC_standby();
		_delay_ms(1000); // 1초 정지

		// --- 2. PWM 정회전 속도 증가 (0% -> 100%) ---
		// 듀티비를 0%에서 100%까지 10%씩 증가
		for(duty_ratio = 0.0; duty_ratio<=100.0; duty_ratio+=10.)
		{
			// 각 듀티비를 100 * 20ms = 2초간 유지
			for(i=0;i<100;i++)
			{
				PWM_Forward(duty_ratio);
			}
		}
		
		// --- 3. PWM 정회전 속도 감소 (100% -> 0%) ---
		for(duty_ratio = 100.0; duty_ratio>=0.0; duty_ratio-=10.)
		{
			for(i=0;i<100;i++)
			{
				PWM_Forward(duty_ratio);
			}
		}
		
		// --- 4. PWM 역회전 속도 증가 (0% -> 100%) ---
		for(duty_ratio = 0.0; duty_ratio<=100.0; duty_ratio+=10.)
		{
			for(i=0;i<100;i++)
			{
				PWM_Reverse(duty_ratio);
			}
		}
		
		// --- 5. PWM 역회전 속도 감소 (100% -> 0%) ---
		for(duty_ratio = 100.0; duty_ratio>=0.0; duty_ratio-=10.)
		{
			for(i=0;i<100;i++)
			{
				PWM_Reverse(duty_ratio);
			}
		}
	}
}
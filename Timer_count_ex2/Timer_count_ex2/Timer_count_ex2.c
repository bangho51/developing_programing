/*
 * Timer_count_ex2.c
 *
 * Created: 2025-10-10 오후 9:40:37
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

void init_timer0_fastPWM(int prescaler, unsigned char ocr)
{
	sei();
	// TCCR0: FOC0=0 | COM01=1, COM00=0 | WGM01=1, WGM00=1
	// Fast PWM 모드, Compare Match 시 OC0 Clear (비반전 모드)
	TCCR0 = 0b01101000;

	// 분주비(prescaler) 설정: TCCR0 레지스터의 CS02, CS01, CS00 비트 설정
	if(prescaler == 1) TCCR0 |= 1; // OC0B PWM 주파수 = F_CPU/(prescaler*256)
	if(prescaler == 8) TCCR0 |= 2;
	if(prescaler == 32) TCCR0 |= 3;
	if(prescaler == 64) TCCR0 |= 4;
	if(prescaler == 128) TCCR0 |= 5;
	if(prescaler == 256) TCCR0 |= 6;
	if(prescaler == 1024) TCCR0 |= 7;
	
	OCR0 = ocr; // 듀티비 50% (실제 코드에서는 OCR0 값이 ocr로 설정됨)
}

void OC0_0(void) // 소리 안나게 하기
{
	TCCR0 = 0; // TCCR0 레지스터를 0으로 초기화하여 타이머/PWM 동작 중지
	PORTB = 0;
}

void phone_bell(int count)
{
	int i,j;
	for(j=0; j<count; j++) // count 횟수만큼 벨소리 반복
	{
		for(i=0; i<10; i++) // 높은 음과 낮은 음을 10회 반복 (따르릉 소리 패턴)
		{
			// 488Hz (높은 음) 생성: 16000000 / (128 * 256) = 488.28 Hz
			// 듀티비 50% (OCR0=128)
			init_timer0_fastPWM(128, 128);
			_delay_ms(25);
			
			// 244Hz (낮은 음) 생성: 16000000 / (256 * 256) = 244.14 Hz
			// 듀티비 50% (OCR0=128)
			init_timer0_fastPWM(256, 128);
			_delay_ms(25);
		}
		
		OC0_0(); // 소리 끄기 (휴지기)
		_delay_ms(2000); // 2초 대기
	}
}

int main(void)
{
	init_OC0(); // PB4(OC0) 출력으로 설정
	phone_bell(3); // 벨 소리 3번 내기
}
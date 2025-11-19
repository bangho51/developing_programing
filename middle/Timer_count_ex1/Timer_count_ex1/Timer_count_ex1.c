#include <avr/io.h>
#define F_CPU 16000000UL
#include <avr/interrupt.h>

extern void init_fnd(void);
extern void fnd_number(int number);

unsigned int msec = 0; // msec를 세기 위한 변수
unsigned int sec = 0; // sec를 세기 위한 변수

ISR(TIMER0_OVF_vect) // Timer 0 오버플로우 인터럽트
{
	TCNT0=0x06; // 1ms 오버플로우를 만들기 위해 타이머 레지스터 초기값 설정
	if(msec++ == 1000) // 1000ms 일때 동작
	{
		sec++;
		msec=0;
		if(sec == 10000) sec = 0;
	}
}

void init_timer0_normal_ovf_int(void)
{
	sei();

	TCCR0=0x04; // FOC0 | WGM00 | COM01 | COM00 | WGM01 | CS02 | CS01 | CS00; F_CPU/64 = 16000000/64=250,000Hz, T=4us, 4*250 = 1msec
	TCNT0=0x06; // 1ms 오버플로우를 만들기 위해 타이머 레지스터 초기값 설정
	// Timer(s)/Counter(s) Interrupt(s) Initialization
	TIMSK=0x01; // TOIE2 | OCIE2 | TICIE1 | OCIE1A | OCIE1B | TOIE1 | OCIE0 | TOIE0; // Enable overflow interrupt
}

int main(void)
{
	init_fnd();
	init_timer0_normal_ovf_int();
	while(1)
	{
		fnd_number(sec);
	}
}
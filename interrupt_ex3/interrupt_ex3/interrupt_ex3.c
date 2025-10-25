#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>

#define DEBOUNCING_TIME 10
extern void init_fnd(void);
extern void fnd_number(int number);

unsigned int count=0;

ISR(INT4_vect)
{
	_delay_ms(DEBOUNCING_TIME); // 디바운싱 처리
	if((PINE & (1<<PE4)) == 0) // PE4가 Low(0)이면 버튼이 눌린 상태
	{
		count++;
		if(count > 9999) count = 0;
	}
}

/* PE4 : 스위치 입력 */
void init_PE4(void)
{
	DDRE &= ~(1 << PE4); // PE4만 0으로 만듦 [PE4 : input], 내부 pull_up 저항에 연결되어 항상 1 입력
}

void set_external_interrupt(void)
{
	sei(); // Global Interrupt Enable
	EIMSK |= 1 << PE4; // Enable INT4
	EICRB = 0b00000010; // 11(rising edge), 10(falling edge), 01(reserved), 00(low level)
}

int main(void)
{
	init_PE4(); // PE4 스위치 입력 설정
	init_fnd();
	set_external_interrupt(); // INT4 설정
	while (1)
	{
		fnd_number(count);
	}
}
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>

extern void fnd_running(unsigned char ith);
extern void init_fnd(void);

unsigned char ith = 0;
/* INT4 호출시 count를 1 증가 시키는 함수 */
ISR(INT4_vect)
{
	cli(); // Global Interrupt Disable
	_delay_ms(10); // 디바운싱 처리
	ith = ith + 1;
	if(ith == 4) ith = 0;
	sei(); // Global Interrupt Enable
}

/* Global interrupt와 PE4(INT4)를 활성화 시키고, PE4 입력이 하강에지(버튼이 눌림)일 때 INT4 발생시킴*/
void init_INT4(void)
{
	DDRE &= ~(1 << PE4); // PE4만 0으로 만듦 [PE4 : input], 내부 pull_up 저항에 연결되어 항상 1 입력

	sei(); // Global Interrupt Enable
	EIMSK |= 1 << PE4; // Enable INT4
	EICRB = 0b00000010; // 11(rising edge), 10(falling edge), 01(reserved), 00(low level)
}

int main(void)
{
	init_fnd();
	init_INT4();
	while(1)
	{
		fnd_running(ith);
	}
}

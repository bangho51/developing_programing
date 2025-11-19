/*
 * interrupt_ex1.c
 *
 * Created: 2025-10-22 오후 12:35:12
 * Author : jbhma
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

extern void init_fnd(void);
extern void fnd_number(int number);

unsigned int count=0;
#define YES 1
#define NO 0
unsigned char counted = NO;

/* PE4 : 스위치 입력 */
void init_PE4(void)
{
	DDRE &= ~(1 << PE4); // PE4만 0으로 만듦 [PE4 : input]
}
void check_PE4(void)
{
	if((PINE & (1<<PE4)) == 0) // PE4가 Low(0)이면 버튼이 눌린 상태
	{
		_delay_ms(10); // 디바운싱
		if(((PINE & (1<<PE4)) == 0) && counted == NO) // PE4가 Low(0)이면 버튼이 눌린 상태
		{
			count++;
			if(count > 9999) count = 0;
			counted = YES;
		}
	}
	else
			counted = NO;
}
int main(void)
{
	init_PE4();
	init_fnd();
	while (1)
	{
		check_PE4();
		fnd_number(count);
	}
}
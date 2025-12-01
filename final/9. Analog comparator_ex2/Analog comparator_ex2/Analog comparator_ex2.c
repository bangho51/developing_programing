/*
 * Analog comparator_ex2.c
 *
 * Created: 2025-11-24 오후 2:09:20
 * Author : jbhma
 */ 

#include <avr/io.h>
#define F_CPU 16000000
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h> // 이 줄이 빠져서 에러가 발생했습니다.

int uart0_putchar(char c, FILE *stream);
FILE uart0_stream = FDEV_SETUP_STREAM(uart0_putchar, NULL, _FDEV_SETUP_RW);

void usart0_9600_8_1_np(void)
{
	UCSR0B = 0x18;   // USART0 Rx Tx Enable
	UCSR0C = 0x06;   // No parity, 1 stop bit, 8 data bits
	UBRR0H = 0; UBRR0L = 103;   // 9600
	stdout = &uart0_stream;
}

int uart0_putchar(char c, FILE *stream)
{
	while((UCSR0A & 0x20) == 0); // 송신 가능할 때까지 기다림
	UDR0 = c;
	while((UCSR0A & 0x40) == 0); // 송신 완료까지 기다림
	return 0;
}

void Tx0(unsigned char c)
{
	while((UCSR0A & 0x20) == 0); // 송신 가능할 때까지 기다림
	UDR0 = c;
	while((UCSR0A & 0x40) == 0); // 송신 완료까지 기다림
}
// AIN0(PE2):positive, AIN1(PE3):negative
// ACO=1, if AIN0 is is equal to or higher than AIN1
ISR(ANALOG_COMP_vect)
{
	if(ACSR & 0x20) printf("AIN0(PE2) is higher than AIN1(PE3)\n");
	else printf("AIN0(PE2) is equal to or less than AIN1(PE3)\n");
}

void init_AC_PE23_INT(void)
{
	DDRE = 0x00;    // AIN0(PE2), AIN1(PE3) 입력 설정
	SFIOR = 0x00;   // ACME=0: AIN1(PE3) as negative
	// ACSR : [ ACD ACBG ACO ACI ACIE ACIC ACIS1 ACIS0 ]
	// ACIE:Interrupt Enable, ACIS1/ACIS0:Comparator Interrupt on Output Toggle
	ACSR = 0x08;
	sei();  // SREG=0x80;
}

int main(void)
{
	usart0_9600_8_1_np();
	init_AC_PE23_INT();
	while (1);
}
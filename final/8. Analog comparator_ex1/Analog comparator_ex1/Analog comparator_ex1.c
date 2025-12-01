/*
 * Analog comparator_ex1.c
 *
 * Created: 2025-11-24 오후 2:04:24
 * Author : jbhma
 */ 

#include <avr/io.h>
#define F_CPU 16000000
#include <util/delay.h>
#include <stdio.h>
int uart0_putchar(char c, FILE *stream);
FILE uart0_stream = FDEV_SETUP_STREAM(uart0_putchar, NULL, _FDEV_SETUP_RW);

// AIN0(PE2):positive, AIN1(PE3):negative
// AC0=1 if AIN0 is equal to or higher than AIN1
void init_AC_PE23(void)
{
	DDRE = 0x00;    // AIN0(PE2), AIN1(PE3) 입력 설정
	SFIOR = 0x00;   // ACME=0: AIN1(PE3) as negative
	ACSR = 0x00;    // ACD ACBG ACO ACI ACIE ACIC ACIS1 ACIS0
}

void check_ACO(void)
{
	if(ACSR & (1<<ACO)) printf("AIN0(PE2) is higher than AIN1(PE3)\n");
	else printf("AIN0(PE2) is equal to or less than AIN1(PE3)\n");
}

/* --- UART0 functions --- */
void usart0_9600_8_1_np(void)
{
	UCSR0B = 0x18; // USART0 Rx Tx Enable
	UCSR0C = 0x06; // No parity, 1 stop bit, 8 data bits
	UBRR0H = 0; UBRR0L = 103; // 9600
	stdout = &uart0_stream;
}
int uart0_putchar(char c, FILE *stream)
{
	while((UCSR0A & 0x20) == 0); // 송신 가능할 때까지 기다림
	UDR0 = c;
	while((UCSR0A & 0x40) == 0); // 송신 완료까지 기다림.
	return 0;
}
void Tx0(unsigned char c)
{
	while((UCSR0A & 0x20) == 0); // 송신 가능할 때까지 기다림
	UDR0 = c;
	while((UCSR0A & 0x40) == 0); // 송신 완료까지 기다림.
}

int main(void)
{
	usart0_9600_8_1_np();
	init_AC_PE23();
	while (1)
	{
		_delay_ms(500);
		check_ACO();
	}
}

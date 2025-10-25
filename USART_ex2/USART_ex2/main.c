/*
 * USART_ex2.c
 *
 * Created: 2025-10-22 오후 4:22:34
 * Author : jbhma
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#include <stdio.h> // printf, scanf를 사용하기 위함
#include <util/delay.h>

int uart0_putchar(char c, FILE *stream) // UART0를 printf 포트로 사용하기 위함
{
	while((UCSR0A & (1 << UDRE)) == 0); // 데이터 레지스터가 빌 때까지 기다림
	UDR0 = c; // 데이터 레지스터에 값을 씀
	while((UCSR0A & (1 << TXC)) == 0); // 송신 완료까지 기다림
	return 0;
}

int uart0_getchar(FILE *stream) // UART0를 scanf 포트로 사용하기 위함
{
	while((UCSR0A & (1 << RXC)) == 0); // 데이터 수신될 때까지 기다림
	return(UDR0);
}

FILE uart0_stream = FDEV_SETUP_STREAM(uart0_putchar, uart0_getchar, _FDEV_SETUP_RW);

void init_uart0(unsigned int baud) // baud - 8 data bits - No parity - 1 stop bit
{
	unsigned int ubrr;
	ubrr = (unsigned int)(F_CPU/16/baud - 1); // Ref. Manual p. 173
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1<<RXEN) | (1<<TXEN); // 0b00011000; // usart0 Tx, Rx 활성화
	
	stdout = &uart0_stream; // printf
	stdin = &uart0_stream; // scanf
	fdevopen(uart0_putchar, uart0_getchar);
}

void echo_uart0_scanf_printf(void)
{
	char buffer[100];
	init_uart0(9600);
	while(1)
	{
		scanf("%s", buffer);
		printf("%s\n", buffer);
	}
}

int main(void)
{
	echo_uart0_scanf_printf();
}
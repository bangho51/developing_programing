/*
 * SPI_ex1_M_extern.c
 *
 * Created: 2025-11-17 오후 12:28:41
 * Author: jbhma
 * Description: Contains UART functions and stdio stream setup for AVR.
 */ 
#include <avr/io.h>
#define F_CPU 16000000UL
#include <stdio.h> // printf, scanf를 사용하기 위함

int uart0_putchar(char c, FILE *stream) // UART0를 printf 포트로 사용하기 위함
{
	while((UCSR0A & (1 << UDRE)) == 0); // 데이터 레지스터가 빌 때까지 기다림
	UDR0 = c; // 데이터 레지스터에 값을 씀
	while((UCSR0A & (1 << TXC)) == 0); // 송신 완료까지 기다림
	return 0;
}

int uart0_getchar(FILE *stream) // UART0를 scanf 포트로 사용하기 위함.
{
	while((UCSR0A & (1 << RXC)) == 0); // 데이터 수신될 때까지 기다림
	return(UDR0);
}
FILE uart0_stream = FDEV_SETUP_STREAM(uart0_putchar, uart0_getchar,_FDEV_SETUP_RW);


// =========================================================
// UART0 기본 통신 함수
// =========================================================

void Tx0(unsigned char c)
{
	while((UCSR0A & 0x20) == 0); // 송신 가능할 때까지 기다림 (UDRE0 플래그)
	UDR0 = c;
	while((UCSR0A & 0x40) == 0); // 송신 완료까지 기다림. (TXC0 플래그)
}

unsigned char Rx0(void)
{
	while((UCSR0A & 0x80) == 0); // 수신될 때까지 기다림 (RXC0 플래그)
	return(UDR0);
}



// =========================================================
// UART0 초기화 함수
// =========================================================

void init_uart0(unsigned int baud) // baud - 8 data bits - No parity - 1 stop bit
{
	unsigned int ubrr;
	ubrr = (unsigned int)(F_CPU/16/baud - 1); // 보율 계산
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1<<RXEN0) | (1<<TXEN0); 

	// stdio 스트림 연결
	stdout = &uart0_stream; // printf
	stdin = &uart0_stream; // scanf
	
    // 래퍼 함수를 사용하여 fdevopen 호출
	fdevopen(uart0_putchar, uart0_getchar);
}
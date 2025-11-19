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

// --- 전역 변수 선언 (에러 해결) ---
// 🌟 'uart0_stream' undeclared 에러 해결: FILE 구조체 변수 선언
FILE uart0_stream; 


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
// stdio.h 래퍼 함수 (fdevopen 요구사항 충족)
// =========================================================

// 🌟 'uart0_putchar' undeclared 에러 해결: fdevopen에 필요한 putchar 정의
// 형식: int (*putchar)(char, FILE*)
int uart0_putchar_wrapper(char c, FILE *stream) {
    // Tx0를 사용하여 문자 출력
    Tx0(c); 
    return 0; // 성공 시 0 반환
}

// 🌟 'uart0_getchar' undeclared 에러 해결: fdevopen에 필요한 getchar 정의
// 형식: int (*getchar)(FILE*)
int uart0_getchar_wrapper(FILE *stream) {
    // Rx0를 사용하여 문자 수신
    return Rx0(); 
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
	// UCSR0B = (1<<RXEN) | (1<<TXEN); // Rx, Tx 활성화
	// 안전을 위해 UCSR0B 비트 마스크 대신 비트 시프트를 사용합니다.
	UCSR0B = (1<<RXEN0) | (1<<TXEN0); 

	// stdio 스트림 연결
	stdout = &uart0_stream; // printf
	stdin = &uart0_stream; // scanf
	
    // 래퍼 함수를 사용하여 fdevopen 호출
	fdevopen(uart0_putchar_wrapper, uart0_getchar_wrapper);
}
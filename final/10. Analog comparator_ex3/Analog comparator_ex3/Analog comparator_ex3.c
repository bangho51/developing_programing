/*
 * Analog comparator_ex3.c
 *
 * Created: 2025-11-24 오후 2:26:52
 * Author : jbhma
 */ 

#include <avr/io.h>
#define F_CPU 16000000
#include <util/delay.h>
#include <stdio.h>

// --- UART 관련 설정 시작 ---
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
// --- UART 관련 설정 끝 ---


// AIN0(PE2):positive, AIN1(PE3):negative (이 주석은 일반 모드일 때이고, 여기선 MUX 사용)
// ACO=1 if AIN0 is equal to or higher than AIN1

void init_AC_MUX(void)
{
	DDRE = 0x00;    // AIN0(PE2) 입력 설정
	DDRF = 0x00;    // AIN1(PF0:7) 입력 설정 (ADC 핀들)
	
	// SFIOR 레지스터 설정이 핵심입니다.
	SFIOR = 0x08;   // ACME=1: Analog Comparator Multiplexer Enable (ADMUX 사용)
	
	// ADC 기능이 켜져 있으면 MUX가 비교기에 연결되지 않으므로 꺼야 합니다.
	ADCSRA = 0;     // ADEN=0 in ADCSRA, ADMUX 사용
	
	ACSR = 0x00;    // ACD ACBG ACO ACI ACIE ACIC ACIS1 ACIS0 (인터럽트 사용 안 함)
}

void check_AC_MUX(void)
{
	unsigned char i;
	for(i=0; i<8; i++)
	{
		ADMUX = i;  // PFi as AIN1 (비교기의 음(-) 입력으로 PF(i) 선택)
		_delay_ms(500); // 0.5초 대기
		
		// ACSR의 5번 비트(ACO)를 확인하여 비교 결과 출력
		if(ACSR & 0x20)
		printf("AIN0(PE2) is higher than ADC%d(PF%d)\n", i, i);
		else
		printf("AIN0(PE2) is equal to or less than ADC%d(PF%d)\n", i, i);
	}
}

int main(void)
{
	usart0_9600_8_1_np();
	init_AC_MUX();
	while (1)
	{
		check_AC_MUX();
	}
}
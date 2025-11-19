#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

void init_uart0(unsigned int baud) // baud rate - 8 data bits - No Parity - 1 stop bit
{
	unsigned int ubrr;
	ubrr = (unsigned int)(F_CPU/16/baud - 1); // Ref. Manual p. 173
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1<<RXEN) | (1<<TXEN); // 0b00011000; // usart0 Tx, Rx 활성화
}

void Tx0(unsigned char c)
{
	while((UCSR0A & 0x20) == 0); // 송신 가능할 때까지 기다림
	UDR0 = c;
	while((UCSR0A & 0x40) == 0); // 송신 완료까지 기다림.
}

unsigned char Rx0(void)
{
	while((UCSR0A & 0x80) == 0); // 수신될 때까지 기다림
	return(UDR0);
}

void uart0_echo(void)
{
	init_uart0(9600); // uart0, 9600-8-N-1
	while(1)
	{
		Tx0(Rx0());
	}
}

int main(void)
{
	uart0_echo();
}
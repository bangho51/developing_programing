/*
 * SPF02_ex1.c
 *
 * Created: 2025-11-03 오후 2:13:12
 * Author : jbhma
 */ 

#include <avr/io.h>
#include <stdio.h>
#define F_CPU 16000000L
#include <util/delay.h>
#define SRF02_address 0x00

/* --- uart0 polling --- */
int uart0_putchar(char c, FILE *stream);
FILE uart0_stream = FDEV_SETUP_STREAM(uart0_putchar, NULL, _FDEV_SETUP_RW);

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

unsigned char Rx0(void)
{
	while((UCSR0A & 0x80) == 0); // 수신될 때까지 기다림
	return(UDR0);
}

/* --- USART1 --- */
void usart1_9600_8_2_np(void)
{
	UCSR1B = 0x18; // USART0 Rx Tx Enable
	UCSR1C = 0b00001110; // No parity, 2 stop bit, 8 data bits
	UBRR1H = 0; UBRR1L = 103; // 9600
}

void Tx1(unsigned char c)
{
	while((UCSR1A & 0x20) == 0); // 송신 가능할 때까지 기다림
	UDR1 = c;
	while((UCSR1A & 0x40) == 0); // 송신 완료까지 기다림.
}

unsigned char Rx1(void)
{
	while((UCSR1A & 0x80) == 0); // 수신될 때까지 기다림
	return(UDR1);
}
/* ... */
/* --- SRF02 Serial --- */
void SRF02_serial_command(unsigned char address, unsigned char command)
{
	Tx1(address); Tx1(command);
}

void SRF02_serial_dist(unsigned char address, unsigned int *dist)
{
	unsigned char hb, lb;

	SRF02_serial_command(address, 0x51); // real ranging mode - result in cm
	_delay_ms(70); // 70ms 기다려야 함

	SRF02_serial_command(address, 0x5e); // get range, returns two bytes(high byte first) from the most recent ranging
	hb = Rx1();
	lb = Rx1();
	*dist = ((unsigned int)hb << 8) + lb;
}

void SRF02_serial_change_address(unsigned char current_address, unsigned char new_address)
{
	SRF02_serial_command(current_address, 0xA0);
	SRF02_serial_command(current_address, 0xAA);
	SRF02_serial_command(current_address, 0xA5);
	SRF02_serial_command(current_address, new_address);
}

void SRF02_serial_init(unsigned char address)
{
	usart0_9600_8_1_np(); // To PC
	usart1_9600_8_2_np(); // To SRF02
	SRF02_serial_command(address, 0x51); // real ranging mode - result in cm
	_delay_ms(70); // 70ms 기다려야 함
}

void SRF02_serial_test(void)
{
	unsigned int dist;

	SRF02_serial_init(SRF02_address);
	while(1)
	{
		SRF02_serial_dist(SRF02_address, &dist);
		printf("distance: %u cm\n",dist);
		_delay_ms(500);
	}
}

int main(void)
{
	SRF02_serial_test();
}
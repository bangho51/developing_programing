/*
 * SPI_ex1_S.c
 *
 * Created: 2025-11-17 오후 12:39:08
 * Author : jbhma
 */ 

#include <avr/io.h>
#include <stdio.h>
#define F_CPU 16000000L
#include <util/delay.h>

extern void init_uart0(unsigned int baud);
extern void Tx0(unsigned char c);
extern unsigned char Rx0(void);

#define SS PORTB |= 0x01;
#define CS PORTB &= 0xfe;

void SPI_SInit(void) // 1MHz mode 0
{
	DDRB = DDRB | 0b00001000; // PB3(MISO), PB2(MOSI), PB1(SCK), PB0(SS)
	// SPCR : SPIE SPE DORD MSTR CPOL CPHA SPR1 SPR0
	SPCR = (1<<SPE); // SPI 활성화, 모드 0
}

void SPI_STx(unsigned data)
{
	SPDR = data;
	while((SPSR & 0x80) == 0); // SPIF = 1 이 될 때까지 기다림; 데이터 송신 완료 기다림
}

unsigned char SPI_SRx(void)
{
	while((SPSR & 0x80) == 0); // SPIF = 1 이 될 때까지 기다림;
	return(SPDR);
}

int main(void)
{
	unsigned char data;
	init_uart0(9600);
	SPI_SInit();
	while (1)
	{
		data = SPI_SRx();
		SPI_STx(data);
	}
}


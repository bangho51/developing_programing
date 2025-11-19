/*
 * EEPROM_ex1.c
 *
 * Created: 2025-11-17 오후 12:42:17
 * Author : jbhmac
 */ 

#include <avr/io.h>
#define F_CPU 16000000
#include <util/delay.h>
#include <stdio.h>
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
void EEPROM_write(unsigned int address, unsigned char data)
{
	while(EECR & (1<<EEWE)); // Wait for completion of previous write. EEWE = 0이 될 때
	EEAR = address; // Set up address
	EEDR = data; // Set up data
	EECR |= (1<<EEMWE); // EEPROM Master Write Enable
	EECR |= (1<<EEWE); // Start EEPROM write by setting EEWE=1
}
unsigned char EEPROM_read(unsigned int address)
{
	while(EECR & (1<<EEWE)); // Wait for completion of previous write. EEWE = 0이 될 때
	EEAR = address; // Set up address
	EECR |= (1<<EERE); // Start EEPROM read
	return EEDR; // return data
}
void EEPROM_WString(unsigned int address,unsigned char *str)
{
	unsigned char *ptr;
	ptr = str;
	while(*ptr != 0)
	{
		EEPROM_write(address++,*ptr++);
	}
}
void EEPROM_rstring(unsigned int address,unsigned char nbytes)
{
	unsigned char data;
	unsigned int i;
	for(i=0;i<nbytes;i++)
	{
		data = EEPROM_read(address+i);
		printf("%c",data);
	}
}
void EEPROM_usart0(void)
{
	unsigned int address=0;
	unsigned char data;
	while(1)
	{
		data = Rx0();
		if(data == 0x1b) // Escape key
		EEPROM_rstring(0,address);
		else
		{
			Tx0(data);
			EEPROM_write(address++,data);
		}
	}
}
int main(void)
{
	usart0_9600_8_1_np();
	EEPROM_usart0();
}
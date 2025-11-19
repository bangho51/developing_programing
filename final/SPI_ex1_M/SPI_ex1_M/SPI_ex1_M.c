/*
 * SPI_ex1_M.c
 *
 * Created: 2025-11-17 오후 12:26:09
 * Author : jbhma
 */ 

#include <avr/io.h>
#include <stdio.h>
#define F_CPU 16000000L
#include <util/delay.h>

extern void init_uart0(unsigned int baud);
extern void Tx0(unsigned char c);
extern unsigned char Rx0(void);

#define SS PORTB |= 0x01; // SS high
#define CS PORTB &= 0xfe; // SS low (Chip Select)

void SPI_MInit(void) // SPI 마스터 4MHz SCK로 시작
{
    DDRB = DDRB | 0b00001111; // PB3(MISO), PB2(MOSI), PB1(SCK), PB0(SS)
    //SPCR = SPIE SPE DORD MSTR CPOL CPHA SPR1 SPR0
    SPCR = (1<<SPE) | (1<<MSTR); // 마스터 모드, SPI 활성화, SCK = F_CPU/4, 모드 0
}

void SPI_MTx(unsigned char data) // 슬레이브에게 데이터 전송
{
    CS;
    SPDR = data;
    while((SPSR & 0x80) == 0); // SPIF = 1 이 될 때까지 기다림; 데이터 송신 완료 기다림
    SS;
}

unsigned char SPI_MRx(void) // 슬레이브에서 데이터 수신
{
    unsigned char data;
    CS;
    while((SPSR & 0x80) == 0); // SPIF = 1 이 될 때까지 기다림; 
    data = SPDR;
    SS;
    return(data);
}

int main(void)
{
    unsigned char udata,sdata;

    init_uart0(9600);
    SPI_MInit();

    while (1)
    {
        udata = Rx0();
        SPI_MTx(udata);
        sdata = SPI_MRx();
        Tx0(sdata);
    }
}
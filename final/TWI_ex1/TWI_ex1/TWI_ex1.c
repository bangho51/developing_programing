/*
 * TWI_ex1.c
 *
 * Created: 2025-11-01 오후 8:50:38
 * Author : jbhma
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#define F_CPU 16000000L

extern void init_fnd(void);
extern void ffnd_number(float number); // 소수점 1자리 실수 표시
// TWCR: TWINT TWEA TWSTA TWSTO TWWC TWEN - TWIE
#define S 0xa4 // Start TWI
#define P 0x94 // stop TWI
// Temperature sensor SLA
#define SLA_W_TS 0b10011000 // 8 비트 with SLA+W
#define SLA_R_TS 0b10011001 // 8 비트 with SLA+R
/* --- TWI --- */
void set_SCL_40kHz(void)
{
	TWBR = 192;
	TWSR = TWSR & 0xFC;
}
unsigned char wait_TWINT(void) // 대기
{
	while(!(TWCR & (1<<TWINT)));
	return(TWSR & 0xF8);
}
unsigned char write_Data(unsigned char data) // TWI로 1 바이트 송신
{
	TWDR = data;
	TWCR = 0x84;
	return (wait_TWINT());
}
unsigned char read_Data(void) // TWI에서 1 바이트 수신
{
	TWCR = 0xc4; // 0xc4;
	wait_TWINT();
	TWCR = 0x84; // Read byte
	wait_TWINT();
	return (TWDR);
}
/* 마스터에서 슬레이브로부터 여러 바이트 수신 */
void MR_bytes(unsigned char SLA_R, unsigned char *command, unsigned char *data, unsigned char nofdata)
{
	unsigned char i=0;
	MT_bytes(SLA_R & 0xFE,command,1); // 슬레이브에게 명령어 보냄
	TWCR = S; // Repeated Start TWI
	wait_TWINT();
	TWDR = SLA_R; // 슬레이브에게 데이터 전송 요구
	for(i=0; i < nofdata;i++) // 데이터 읽기
		data[i] = read_Data();
	TWCR = P; // stop TWI
}

/* 온도 센서 atS75 */
void init_TWI_TS(void)
{
	DDRD = 0x00;
	PORTD = 0x03; // 내부 풀업 활성화 PD0 : SCL, PD1 :SDA
	SFIOR &= ~(1<< PUD); // PUD = 0 : 풀업 활성화
	set_SCL_40kHz(); // SCL 100kHz로 설정
}

void init_TS(void)
{
	unsigned char data[]={0x01, 0x00}; // configuration register에 0을 써서 ADC 9 bits로 설정
	MT_bytes(SLA_W_TS,data,2);
}

/* fnd dot convert */
float TS_conv9(unsigned char *data)
{
	int sign = 1;
	int temp,TS_ADC;
	TS_ADC = (data[0] << 8) | data[1];
	if(TS_ADC < 0) sign = -1;
	temp = (TS_ADC < 0) ? -TS_ADC : TS_ADC;
	temp = temp >> 7;
	if(sign < 0) temp = -temp;
	return ((float)temp/2.0);
}
/* --- 마스터에서 슬레이브에게 여러 바이트 송신 --- */
void MT_bytes(unsigned char SLA_W,unsigned char *data,unsigned char nofdata)
{
	unsigned char i=0;
	TWCR = S; // Start TWI
	wait_TWINT();
	TWDR = SLA_W; // 슬레이브에게 데이터 보낸다고 전송
	TWCR = 0x84;
	for(i=0;i < nofdata;i++)
	write_Data(data[i]);
	TWCR = P; // stop TWI
}
int main(void)
{
	int i;
	unsigned char data[2],command[]={0};
	init_fnd();
	init_TWI_TS();
	init_TS();
	while (1)
	{
		MR_bytes(SLA_R_TS,command,data,2); // 2 바이트 온도 데이터 읽기
		for(i=0;i<30;i++);
			ffnd_number(TS_conv9(data));
	}
}
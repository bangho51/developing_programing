/*
 * SSD1306.c
 *
 * Created: 2025-11-10 오후 2:07:58
 * Author : jbhma
 */ 

#include <avr/io.h>
#define F_CPU 16000000
#include <util/delay.h>
#include <stdio.h>
#include <assert.h>
// #include "font.h" // 폰트 배열을 위한 헤더 파일이 있다면 추가합니다.

// TWI Status Codes for Master Mode
#define MT_S 0x08        // A Start condition has been transmitted
#define MT_SLA_W_A 0x18  // SLA+W has been transmitted; ACK has been received
#define MT_SLA_W_Na 0x20 // SLA+W has been transmitted; NOT ACK has been received
#define MT_Al 0x38       // Arbitration lost in SLA+W or data bytes
#define MR_S 0x08        // A Start condition has been transmitted
#define MR_Al 0x38       // Arbitration lost in SLA+R or NOT ACK bit
#define MR_SLA_R_A 0x40  // SLA+R has been transmitted; ACK has been received
#define MR_SLA_R_Na 0x48 // SLA+R has been transmitted; NOT ACK has been received

// TWI Control
#define S 0xa4           // Start TWI
#define P 0x94           // stop TWI

// SSD1306 TWI
#define SSD1306_SLA_W 0x78
#define COMMAND 0
#define GDDRAM 1

// --- 외부 변수 선언 ---
extern const unsigned char ascii8x8v[][8]; // 폰트 배열 선언

// --- 함수 프로토타입 선언 ---
unsigned char wait_TWINT(void);
unsigned char write_Data(unsigned char data);
void set_SCL_400kHz(void); 
unsigned char MT_OLED(unsigned char SLA_W, unsigned char *data, unsigned int nofdata, unsigned char data_type);
void OLED_command(unsigned char command); 
void OLED_init(void);
void OLED_page_addressing_mode(unsigned char pagex, unsigned char segx);
void GD_put_char(unsigned char x,unsigned char y,unsigned char c);
void init_I2C(void);

// =========================================================
// I2C/TWI 통신 함수 정의
// =========================================================

// TWI 클럭 설정 (수정 완료: 세미콜론 제거)
void set_SCL_400kHz(void)
{
	TWBR = 0b00001100; // 12
	TWSR = 0;
}

// TWI 인터럽트 플래그 대기
unsigned char wait_TWINT(void)
{
	while(!(TWCR & (1<<TWINT)));
	return(TWSR & 0xF8);
}

// TWI 데이터 송신
unsigned char write_Data(unsigned char data)
{
	TWDR = data;
	TWCR = 0x84;
	return (wait_TWINT());
}

// TWI 마스터 송신 코어 함수 (OLED 전송용)
unsigned char MT_OLED(unsigned char SLA_W, unsigned char *data, unsigned int nofdata, unsigned char data_type)
{
	unsigned char status;
	unsigned int i=0;
	TWCR = S; // Start TWI
	while(1)
	{
		status = wait_TWINT();
		switch(status)
		{
			case MT_S : // A start condition has been transmitted.
			TWDR = SLA_W;
			TWCR = 0x84;
			break;
			case MT_SLA_W_A : 
			case MT_SLA_W_Na : 
			if(data_type == COMMAND) write_Data(0); // Control byte: Command (0x00)
			if(data_type == GDDRAM) write_Data(0x40); // Control byte: Data (0x40)
			for(i=0; i < nofdata;i++)
			{
				write_Data(data[i]);
			}
			TWCR = P; // stop TWI
			return 0;
			break;
			default:
			return status;
		}
	}
}

// =========================================================
// OLED 제어 함수 정의
// =========================================================

// OLED 명령어 전송 (누락 함수 추가)
void OLED_command(unsigned char command)
{
	// 명령어는 주소(&command)와 데이터 타입 COMMAND로 MT_OLED를 호출
	MT_OLED(SSD1306_SLA_W, &command, 1, COMMAND);
}

// OLED 초기화
void OLED_init(void) // Figure 2 : Software Initialization Flow Chart
{
	OLED_command(0xa8); // Set MUX Ratio
	OLED_command(0x3f);
	OLED_command(0xd3); // Set Display Offset
	OLED_command(0x00); // 0x00 : No offset
	OLED_command(0x40); // Set Display Start Line
	OLED_command(0xa1); // Set segment re-map: column address 127 is mapped to SEGO
	OLED_command(0xc8); // set COM Output scan direction: remapped mode.
	OLED_command(0xda); // set COM Pins Hardware configuration:
	OLED_command(0x12); // 0x02
	OLED_command(0x81); // set Contrast control for BANK0 00~FF
	OLED_command(0xff); // 0x7f
	OLED_command(0xa4); // Disable Entire Display On
	OLED_command(0xa6); // Set Normal Display
	OLED_command(0xd5); // set display clock divide ratio/oscillator frequency
	OLED_command(0x80); // RESET value
	OLED_command(0x8d); // Charge pump command
	OLED_command(0x14); // Enable charge pump during display on
	OLED_command(0x2e); // Deactivate scroll
	OLED_command(0xaf); // set display on
}

// OLED 페이지 주소 지정 모드 설정
void OLED_page_addressing_mode(unsigned char pagex, unsigned char segx)
{
	OLED_command(0x20);
	OLED_command(2); // page addressing mode
	OLED_command(0xb0 | pagex); // starting page
	OLED_command(segx & 0x0f); // lower start column
	OLED_command(((segx>>4) & 0xf) | 0x10); // upper start column
}

// OLED 수평 주소 지정 모드 설정
void OLED_horizontal_addressing_mode(unsigned char spage, unsigned char epage, unsigned char scol, unsigned char ecol)
{
	OLED_command(0x20); // set memory addressing mode
	OLED_command(0); // horizontal addressing mode
	OLED_command(0x21); // set column address
	OLED_command(scol); // start column address
	OLED_command(ecol); // end column address
	OLED_command(0x22); // set page address
	OLED_command(spage); // start page address
	OLED_command(epage); // end page address
}

// OLED 수직 주소 지정 모드 설정
void OLED_vertical_addressing_mode(unsigned char spage, unsigned char epage, unsigned char scol, unsigned char ecol)
{
	OLED_command(0x20); // set memory addressing mode
	OLED_command(1); // vertical addressing mode
	OLED_command(0x21); // set column address
	OLED_command(scol); // start column address
	OLED_command(ecol); // end column address
	OLED_command(0x22); // set page address
	OLED_command(spage); // start page address
	OLED_command(epage); // end page address
}

// =========================================================
// 그래픽/텍스트 출력 함수 정의
// =========================================================

// 문자 하나 출력
void GD_put_char(unsigned char x,unsigned char y,unsigned char c)
{
	OLED_page_addressing_mode(y,x*8);
	MT_OLED(SSD1306_SLA_W,ascii8x8v[c],8,GDDRAM);
}

// 문자열 출력
void GD_put_string(unsigned char x, unsigned char y, char *str)
{
	unsigned char i=0;
	while(1)
	{
		GD_put_char(x+i,y,str[i]);
		i++;
		if(str[i] == 0 || (x+i) > 15) break;
	}
}

// 모든 아스키 문자 출력 (테스트용)
void GD_ascii_all(void)
{
	unsigned char i;
	for(i=0;i<128;i++)
	GD_put_char(i%16,i/16,i);
}

// =========================================================
// 메인 및 초기화
// =========================================================

// I2C 통신 초기화
void init_I2C(void)
{
	DDRD = 0x00;
	PORTD = 0x03; // 내부 풀업 활성화 PD0:SCL, PD1:SDA
	SFIOR &= ~(1<<PUD); // PUD=0: 풀업 활성화 (ATmega128용)
	set_SCL_400kHz();
}

// OLED 텍스트 테스트
void OLED_text_test(void)
{
	init_I2C();
	OLED_init();

	GD_ascii_all();
	// GD_ascii_all이 화면에 출력한 후 덮어씁니다.
	GD_put_string(0,0,"Hello, World!");
}

int main(void)
{
	OLED_text_test();
	
	// 테스트 코드가 한 번 실행된 후 main 함수가 종료됩니다.
	// 일반적으로 AVR 프로그램은 무한 루프(while(1))를 사용합니다.
	// 필요하다면 여기에 while(1)을 추가하여 OLED 화면을 유지하거나 업데이트할 수 있습니다.
	return 0;
}
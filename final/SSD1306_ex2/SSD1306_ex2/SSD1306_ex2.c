/*
 * SSD1306_Full_Test.c
 *
 * "SSD1306 문자.pdf"와 "SSD1306 그래픽.pdf"의 모든 기능을 통합한
 * AVR ATmega128용 SSD1306 OLED 제어 통합 코드입니다.
 *
 * 기능:
 * - TWI (I2C) 통신 (400kHz)
 * - OLED 초기화
 * - 텍스트 출력 (페이지 모드, 직접 전송)
 * - 그래픽 버퍼(MC[1024])를 이용한 픽셀 제어
 * - 그래픽 함수: 직선, 사각형, 원(외곽선/채우기)
 * - 베지에 곡선 함수: 2차, 3차 곡선
 * - 화면 스크롤
 */ 

#include <avr/io.h>
#include <math.h>       // abs() 함수를 위해 필요 (Bresenham 알고리즘)
#include <stdlib.h>     // abs() 함수를 위해 필요
#define F_CPU 16000000L // 16MHz
#include <util/delay.h>
#include <assert.h>

// =========================================================
// PDF에서 추출한 TWI 및 SSD1306 상수 정의
// =========================================================

// TWI Status Codes (Master Transmitter Mode)
#define MT_S 0x08        
#define MT_SLA_W_A 0x18  
#define MT_SLA_W_Na 0x20 
#define MT_Al 0x38       
// (Master Receiver Mode - 이 코드에서는 사용되지 않음)

// TWI Control
#define S 0xa4           // Start TWI
#define P 0x94           // stop TWI

// SSD1306 TWI
#define SSD1306_SLA_W 0x78 // 슬레이브 주소 (SA0=0)
#define COMMAND 0
#define GDDRAM 1

// =========================================================
// 전역 변수 (그래픽 버퍼 및 폰트 배열)
// =========================================================

// "그래픽.pdf" Page 2: 그래픽 버퍼 (Memory Canvas)
unsigned char MC[1024]; // 128x64 비트 SRAM 내장 메모리 버퍼

// "문자.pdf" Page 5: 8x8 폰트 배열 (ASCII 0-127)
// PDF의 데이터가 불완전하므로, 완전한 표준 8x8 폰트 배열로 대체합니다.
const unsigned char ascii8x8v[128][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 00 NUL
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 01 SOH
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 02 STX
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 03 ETX
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 04 EOT
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 05 ENQ
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 06 ACK
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 07 BEL
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 08 BS
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 09 TAB
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0A LF
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0B VT
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0C FF
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0D CR
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0E SO
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0F SI
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 10 DLE
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 11 DC1
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 12 DC2
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 13 DC3
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 14 DC4
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 15 NAK
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 16 SYN
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 17 ETB
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 18 CAN
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 19 EM
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 1A SUB
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 1B ESC
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 1C FS
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 1D GS
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 1E RS
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 1F US
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 20 ' '
	{0x00, 0x00, 0x5F, 0x00, 0x00, 0x00, 0x00, 0x00}, // 21 '!'
	{0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00}, // 22 '"'
	{0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00, 0x00, 0x00}, // 23 '#'
	{0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00, 0x00, 0x00}, // 24 '$'
	{0x23, 0x13, 0x08, 0x64, 0x62, 0x00, 0x00, 0x00}, // 25 '%'
	{0x36, 0x49, 0x55, 0x22, 0x50, 0x00, 0x00, 0x00}, // 26 '&'
	{0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}, // 27 '''
	{0x00, 0x1C, 0x22, 0x41, 0x00, 0x00, 0x00, 0x00}, // 28 '('
	{0x00, 0x41, 0x22, 0x1C, 0x00, 0x00, 0x00, 0x00}, // 29 ')'
	{0x14, 0x08, 0x3E, 0x08, 0x14, 0x00, 0x00, 0x00}, // 2A '*'
	{0x08, 0x08, 0x3E, 0x08, 0x08, 0x00, 0x00, 0x00}, // 2B '+'
	{0x00, 0x50, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00}, // 2C ','
	{0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00}, // 2D '-'
	{0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00}, // 2E '.'
	{0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 0x00, 0x00}, // 2F '/'
	{0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, 0x00, 0x00}, // 30 '0'
	{0x00, 0x42, 0x7F, 0x40, 0x00, 0x00, 0x00, 0x00}, // 31 '1'
	{0x42, 0x61, 0x51, 0x49, 0x46, 0x00, 0x00, 0x00}, // 32 '2'
	{0x21, 0x41, 0x45, 0x4B, 0x31, 0x00, 0x00, 0x00}, // 33 '3'
	{0x18, 0x14, 0x12, 0x7F, 0x10, 0x00, 0x00, 0x00}, // 34 '4'
	{0x27, 0x45, 0x45, 0x45, 0x39, 0x00, 0x00, 0x00}, // 35 '5'
	{0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00, 0x00, 0x00}, // 36 '6'
	{0x01, 0x71, 0x09, 0x05, 0x03, 0x00, 0x00, 0x00}, // 37 '7'
	{0x36, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00, 0x00}, // 38 '8'
	{0x06, 0x49, 0x49, 0x29, 0x1E, 0x00, 0x00, 0x00}, // 39 '9'
	{0x00, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00}, // 3A ':'
	{0x00, 0x56, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00}, // 3B ';'
	{0x08, 0x14, 0x22, 0x41, 0x00, 0x00, 0x00, 0x00}, // 3C '<'
	{0x14, 0x14, 0x14, 0x14, 0x14, 0x00, 0x00, 0x00}, // 3D '='
	{0x41, 0x22, 0x14, 0x08, 0x00, 0x00, 0x00, 0x00}, // 3E '>'
	{0x02, 0x01, 0x51, 0x09, 0x06, 0x00, 0x00, 0x00}, // 3F '?'
	{0x32, 0x49, 0x79, 0x41, 0x3E, 0x00, 0x00, 0x00}, // 40 '@'
	{0x7E, 0x11, 0x11, 0x11, 0x7E, 0x00, 0x00, 0x00}, // 41 'A'
	{0x7F, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00, 0x00}, // 42 'B'
	{0x3E, 0x41, 0x41, 0x41, 0x22, 0x00, 0x00, 0x00}, // 43 'C'
	{0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00, 0x00, 0x00}, // 44 'D'
	{0x7F, 0x49, 0x49, 0x49, 0x41, 0x00, 0x00, 0x00}, // 45 'E'
	{0x7F, 0x09, 0x09, 0x09, 0x01, 0x00, 0x00, 0x00}, // 46 'F'
	{0x3E, 0x41, 0x49, 0x49, 0x7A, 0x00, 0x00, 0x00}, // 47 'G'
	{0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, 0x00, 0x00}, // 48 'H'
	{0x00, 0x41, 0x7F, 0x41, 0x00, 0x00, 0x00, 0x00}, // 49 'I'
	{0x20, 0x40, 0x41, 0x3F, 0x01, 0x00, 0x00, 0x00}, // 4A 'J'
	{0x7F, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00, 0x00}, // 4B 'K'
	{0x7F, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00}, // 4C 'L'
	{0x7F, 0x02, 0x0C, 0x02, 0x7F, 0x00, 0x00, 0x00}, // 4D 'M'
	{0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00, 0x00, 0x00}, // 4E 'N'
	{0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00, 0x00, 0x00}, // 4F 'O'
	{0x7F, 0x09, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00}, // 50 'P'
	{0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00, 0x00, 0x00}, // 51 'Q'
	{0x7F, 0x09, 0x19, 0x29, 0x46, 0x00, 0x00, 0x00}, // 52 'R'
	{0x46, 0x49, 0x49, 0x49, 0x31, 0x00, 0x00, 0x00}, // 53 'S'
	{0x01, 0x01, 0x7F, 0x01, 0x01, 0x00, 0x00, 0x00}, // 54 'T'
	{0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00, 0x00, 0x00}, // 55 'U'
	{0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00, 0x00, 0x00}, // 56 'V'
	{0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00, 0x00, 0x00}, // 57 'W'
	{0x63, 0x14, 0x08, 0x14, 0x63, 0x00, 0x00, 0x00}, // 58 'X'
	{0x07, 0x08, 0x70, 0x08, 0x07, 0x00, 0x00, 0x00}, // 59 'Y'
	{0x61, 0x51, 0x49, 0x45, 0x43, 0x00, 0x00, 0x00}, // 5A 'Z'
	{0x00, 0x7F, 0x41, 0x41, 0x00, 0x00, 0x00, 0x00}, // 5B '['
	{0x02, 0x04, 0x08, 0x10, 0x20, 0x00, 0x00, 0x00}, // 5C '\'
	{0x00, 0x41, 0x41, 0x7F, 0x00, 0x00, 0x00, 0x00}, // 5D ']'
	{0x04, 0x02, 0x01, 0x02, 0x04, 0x00, 0x00, 0x00}, // 5E '^'
	{0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00}, // 5F '_'
	{0x01, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00}, // 60 '`'
	{0x20, 0x54, 0x54, 0x54, 0x78, 0x00, 0x00, 0x00}, // 61 'a'
	{0x7F, 0x48, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00}, // 62 'b'
	{0x38, 0x44, 0x44, 0x44, 0x20, 0x00, 0x00, 0x00}, // 63 'c'
	{0x38, 0x44, 0x44, 0x48, 0x7F, 0x00, 0x00, 0x00}, // 64 'd'
	{0x38, 0x54, 0x54, 0x54, 0x18, 0x00, 0x00, 0x00}, // 65 'e'
	{0x08, 0x7E, 0x09, 0x01, 0x02, 0x00, 0x00, 0x00}, // 66 'f'
	{0x0C, 0x52, 0x52, 0x52, 0x3E, 0x00, 0x00, 0x00}, // 67 'g'
	{0x7F, 0x08, 0x04, 0x04, 0x78, 0x00, 0x00, 0x00}, // 68 'h'
	{0x00, 0x44, 0x7D, 0x40, 0x00, 0x00, 0x00, 0x00}, // 69 'i'
	{0x20, 0x40, 0x44, 0x3D, 0x00, 0x00, 0x00, 0x00}, // 6A 'j'
	{0x7F, 0x10, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00}, // 6B 'k'
	{0x00, 0x41, 0x7F, 0x40, 0x00, 0x00, 0x00, 0x00}, // 6C 'l'
	{0x7C, 0x04, 0x18, 0x04, 0x78, 0x00, 0x00, 0x00}, // 6D 'm'
	{0x7C, 0x08, 0x04, 0x04, 0x78, 0x00, 0x00, 0x00}, // 6E 'n'
	{0x38, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00}, // 6F 'o'
	{0x7C, 0x14, 0x14, 0x14, 0x08, 0x00, 0x00, 0x00}, // 70 'p'
	{0x08, 0x14, 0x14, 0x18, 0x7C, 0x00, 0x00, 0x00}, // 71 'q'
	{0x7C, 0x08, 0x04, 0x04, 0x08, 0x00, 0x00, 0x00}, // 72 'r'
	{0x48, 0x54, 0x54, 0x54, 0x20, 0x00, 0x00, 0x00}, // 73 's'
	{0x04, 0x3F, 0x44, 0x40, 0x20, 0x00, 0x00, 0x00}, // 74 't'
	{0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00, 0x00, 0x00}, // 75 'u'
	{0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00, 0x00, 0x00}, // 76 'v'
	{0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00, 0x00, 0x00}, // 77 'w'
	{0x44, 0x28, 0x10, 0x28, 0x44, 0x00, 0x00, 0x00}, // 78 'x'
	{0x0C, 0x50, 0x50, 0x50, 0x3C, 0x00, 0x00, 0x00}, // 79 'y'
	{0x44, 0x64, 0x54, 0x4C, 0x44, 0x00, 0x00, 0x00}, // 7A 'z'
	{0x08, 0x36, 0x41, 0x41, 0x00, 0x00, 0x00, 0x00}, // 7B '{'
	{0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00}, // 7C '|'
	{0x00, 0x41, 0x41, 0x36, 0x08, 0x00, 0x00, 0x00}, // 7D '}'
	{0x08, 0x08, 0x2A, 0x1C, 0x08, 0x00, 0x00, 0x00}, // 7E '~'
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // 7F DEL
};

// =========================================================
// 함수 프로토타입 선언 (코드 가독성 및 순서 보장)
// =========================================================

// TWI(I2C) 함수
void set_SCL_400kHz(void); 
unsigned char wait_TWINT(void);
unsigned char write_Data(unsigned char data);
unsigned char MT_OLED(unsigned char SLA_W, unsigned char *data, unsigned int nofdata, unsigned char data_type);
void init_I2C(void); 

// OLED 제어 함수
void OLED_command(unsigned char command); 
void OLED_init(void);
void OLED_page_addressing_mode(unsigned char pagex, unsigned char segx);
void OLED_horizontal_addressing_mode(unsigned char spage, unsigned char epage, unsigned char scol, unsigned char ecol); 
void OLED_vertical_addressing_mode(unsigned char spage, unsigned char epage, unsigned char scol, unsigned char ecol); 
void OLED_on(void);
void OLED_off (void);
void OLED_normal_display(void);
void OLED_inverse_display(void);
void OLED_activate_scroll(void);
void OLED_deactivate_scroll(void);
void OLED_HS(unsigned char left, unsigned char s_page, unsigned char e_page, unsigned char step);

// 텍스트 출력 함수
void GD_put_char(unsigned char x,unsigned char y,unsigned char c);
void GD_put_string(unsigned char x, unsigned char y, char *str); 
void GD_ascii_all(void); 

// 그래픽 버퍼 함수
void MC_clear(void);
void MC_display(void);
void OLED_clear(void);
void MC_xy2ib(unsigned char x, unsigned char y, unsigned int *ith, unsigned char *bit);
void MC_set_pixel(unsigned char x, unsigned char y);
void MC_clear_pixel(unsigned char x, unsigned char y);

// 그래픽 알고리즘 함수
void MC_line(int x0, int y0, int x1, int y1);
void MC_circle(int x, int y, int radius, int fill);
void MC_rectangle(int lx, int ly, int ux, int uy);
void linear(float p0[], float p1 [], float u, float *p);
void Bezier2(float p0[], float p1[], float p2[], float u, float *p);
void Bezier3(float p0[], float p1 [], float p2[], float p3[], float u, float *p);
void MC_Bezier2 (float p0[], float p1 [], float p2[]);
void MC_Bezier3(float p0[], float p1 [ ], float p2[], float p3[]);

// 테스트 루틴
void OLED_text_test(void);
void OLED_graphic_test(void);
void OLED_scroll_test(void);

// =========================================================
// TWI(I2C) 통신 함수 정의 ("문자.pdf" Page 6)
// =========================================================

// TWI 클럭 400kHz로 설정
void set_SCL_400kHz(void) 
{
	TWBR = 12; // 0b00001100 (F_CPU 16MHz, Prescaler 1 기준)
	TWSR = 0;
}

// TWI 동작 완료 대기
unsigned char wait_TWINT(void)
{
	while(!(TWCR & (1<<TWINT)));
	return(TWSR & 0xF8); // TWI 상태 코드 반환
}

// TWI 1바이트 전송
unsigned char write_Data(unsigned char data)
{
	TWDR = data;
	TWCR = 0x84; // TWINT, TWEN
	return (wait_TWINT());
}

// TWI 마스터 송신 코어 함수 (OLED 전송용)
unsigned char MT_OLED(unsigned char SLA_W, unsigned char *data, unsigned int nofdata, unsigned char data_type)
{
	unsigned char status;
	unsigned int i=0;
	TWCR = S; // Start TWI (TWSTA | TWINT | TWEN)
	
	while(1)
	{
		status = wait_TWINT();
		switch(status)
		{
			case MT_S : // 0x08: Start 전송 완료
			TWDR = SLA_W;
			TWCR = 0x84; // TWINT, TWEN
			break;
			
			case MT_SLA_W_A : // 0x18: SLA+W 전송, ACK 수신
			case MT_SLA_W_Na : // 0x20: SLA+W 전송, NACK 수신
			// Control Byte 전송
			if(data_type == COMMAND) write_Data(0x00); // 0x00 (명령어)
			if(data_type == GDDRAM) write_Data(0x40); // 0x40 (데이터)
			
			// Data Byte들 전송
			for(i=0; i < nofdata;i++)
			{
				write_Data(data[i]);
			}
			
			TWCR = P; // stop TWI (TWSTO | TWINT | TWEN)
			return 0; // 성공
			break;
			
			default:
			return status; // 에러 상태 반환
		}
	}
}

// =========================================================
// OLED 제어 함수 정의 ("문자.pdf" Page 9, 10, 11-13, "그래픽.pdf" Page 7)
// =========================================================

// 명령어 1바이트 전송 ("문자.pdf" Page 9)
void OLED_command(unsigned char command)
{
	MT_OLED(SSD1306_SLA_W, &command, 1, COMMAND);
}

// OLED 초기화 ("문자.pdf" Page 10)
void OLED_init(void) 
{
	OLED_command(0xa8); // Set MUX Ratio
	OLED_command(0x3f);
	OLED_command(0xd3); // Set Display Offset
	OLED_command(0x00); // 0x00 : No offset
	OLED_command(0x40); // Set Display Start Line
	OLED_command(0xa1); // Set segment re-map: column address 127 is mapped to SEGO
	OLED_command(0xc8); // set COM Output scan direction: remapped mode.
	OLED_command(0xda); // set COM Pins Hardware configuration:
	OLED_command(0x12); 
	OLED_command(0x81); // set Contrast control
	OLED_command(0xff); // 0x7f or 0xff
	OLED_command(0xa4); // Disable Entire Display On
	OLED_command(0xa6); // Set Normal Display
	OLED_command(0xd5); // set display clock divide ratio/oscillator frequency
	OLED_command(0x80); // RESET value
	OLED_command(0x8d); // Charge pump command
	OLED_command(0x14); // Enable charge pump during display on
	OLED_command(0x2e); // Deactivate scroll
	OLED_command(0xaf); // set display on
}

// 페이지 주소 지정 모드 설정 ("문자.pdf" Page 11)
void OLED_page_addressing_mode(unsigned char pagex, unsigned char segx)
{
	OLED_command(0x20); // Set Memory Addressing Mode
	OLED_command(2);    // 2 = Page addressing mode
	OLED_command(0xb0 | pagex); // starting page
	OLED_command(segx & 0x0f); // lower start column
	OLED_command(((segx>>4) & 0xf) | 0x10); // upper start column
}

// 수평 주소 지정 모드 설정 ("문자.pdf" Page 12)
void OLED_horizontal_addressing_mode(unsigned char spage, unsigned char epage, unsigned char scol, unsigned char ecol)
{
	OLED_command(0x20); // set memory addressing mode
	OLED_command(0);    // 0 = horizontal addressing mode
	OLED_command(0x21); // set column address
	OLED_command(scol); // start column address
	OLED_command(ecol); // end column address
	OLED_command(0x22); // set page address
	OLED_command(spage); // start page address
	OLED_command(epage); // end page address
}

// 수직 주소 지정 모드 설정 ("문자.pdf" Page 13)
void OLED_vertical_addressing_mode(unsigned char spage, unsigned char epage, unsigned char scol, unsigned char ecol)
{
	OLED_command(0x20); // set memory addressing mode
	OLED_command(1);    // 1 = vertical addressing mode
	OLED_command(0x21); // set column address
	OLED_command(scol); // start column address
	OLED_command(ecol); // end column address
	OLED_command(0x22); // set page address
	OLED_command(spage); // start page address
	OLED_command(epage); // end page address
}

// --- OLED 화면 제어 ("그래픽.pdf" Page 7) ---
void OLED_on(void) { OLED_command(0xaf); }          // 화면 켜기
void OLED_off (void) { OLED_command(0xae);}         // 화면 끄기
void OLED_normal_display(void){OLED_command(0xa6);} // 일반 표시 모드
void OLED_inverse_display(void) {OLED_command(0xa7);} // 반전 표시 모드
void OLED_activate_scroll(void) { OLED_command(0x2f);}
void OLED_deactivate_scroll(void) { OLED_command(0x2e); }

// OLED 수평 스크롤 설정
void OLED_HS(unsigned char left, unsigned char s_page, unsigned char e_page, unsigned char step)
{
	OLED_command(0x2e); // deactivate scroll
	if(left)
	OLED_command(0x27); // Right horizontal scroll
	else
	OLED_command(0x26); // Left horizontal scroll
	OLED_command(0); // dummy
	OLED_command(s_page); // start page
	OLED_command(step); // scroll step 0-5 frames
	OLED_command(e_page); // end page
	OLED_command(0);
	OLED_command(0xff);
	OLED_command(0x2f); // activate scroll
}

// =========================================================
// 그래픽 버퍼 및 픽셀 제어 함수 정의 ("그래픽.pdf" Page 2)
// =========================================================

// MC 메모리를 0 만들기
void MC_clear(void) 
{
    int i;
    for(i=0;i<1024;i++) MC[i] = 0;
}

// MC 메모리를 SSD1306의 GDDRAM으로 보내기
void MC_display(void) 
{
    OLED_horizontal_addressing_mode(0,7,0,127);
    MT_OLED(SSD1306_SLA_W, MC, 1024, GDDRAM);
}

// 화면 지우기 (버퍼 지우고 전송)
void OLED_clear(void) 
{
    MC_clear();
    MC_display();
}

// (x,y) 좌표 -> 버퍼 인덱스(ith)와 비트(bit)로 변환
void MC_xy2ib(unsigned char x, unsigned char y, unsigned int *ith, unsigned char *bit)
{
    unsigned char pagex;
    pagex = y/8;
    *bit = y % 8; // y-(pagex*8)와 동일
    *ith = (unsigned int) pagex * 128 + x;
}

// (x,y)의 화소 켜기
void MC_set_pixel(unsigned char x, unsigned char y)
{
    unsigned char bit;
    unsigned int ith;
    
    // 화면 범위 체크
    if (x >= 128 || y >= 64) return;
    
    MC_xy2ib(x, y, &ith, &bit);
    MC[ith] = MC[ith] | (1<<bit); 
}

// (x,y)의 화소 끄기
void MC_clear_pixel(unsigned char x, unsigned char y)
{
    unsigned char bit;
    unsigned int ith;

    // 화면 범위 체크
    if (x >= 128 || y >= 64) return;
    
    MC_xy2ib(x, y, &ith, &bit);
    MC[ith] = MC[ith] & (~(1<<bit)); 
}

// =========================================================
// 그래픽 알고리즘 함수 정의 ("그래픽.pdf" Page 3, 5)
// =========================================================

// MC에 직선 그리기 (Bresenham's line algorithm)
void MC_line(int x0, int y0, int x1, int y1) 
{
    int dx, dy, incx, incy, err, e2;
    dx = abs(x1-x0);
    dy = abs(y1-y0);
    incx = (x0 < x1) ? 1 : -1;
    incy = (y0 < y1) ? 1 : -1;
    err = (dx > dy) ? dx/2 : -dy/2;

    while(1)
    {
        MC_set_pixel((unsigned char) x0, (unsigned char)y0);
        if(x0 == x1 && y0 == y1) break;
        e2 = err;
        if(e2 > -dx) { err -= dy; x0 += incx;}
        if(e2 < dy) { err += dx; y0 += incy; }
    }
}

// MC에 원 그리기 (Midpoint circle algorithm)
void MC_circle(int x, int y, int radius, int fill) 
{
    int a, b;
    int r1; 

    a = 0;
    b = radius;
    r1 = 1 - radius; 

    do
    {
        if (fill)
        {
             MC_line(x-a, y+b, x+a, y+b);
             MC_line(x-a, y-b, x+a, y-b);
             MC_line(x-b, y+a, x+b, y+a);
             MC_line(x-b, y-a, x+b, y-a);
        }
        else // 외곽선
        {
            MC_set_pixel( (a+x), (b+y) ); MC_set_pixel( (b+x), (a+y) );
            MC_set_pixel( (x-a), (b+y) ); MC_set_pixel( (x-b), (a+y) );
            MC_set_pixel( (b+x), (y-a) ); MC_set_pixel( (a+x), (y-b) );
            MC_set_pixel( (x-a), (y-b) ); MC_set_pixel( (x-b), (y-a) );
        }

        if (r1 < 0)
        {
            r1 += 3 + 2 * a;
            a++;
        }
        else
        {
            r1 += 5 + 2 * (a++ - b--);
        }
    } while (a <= b);
}

// MC에 사각형 그리기
void MC_rectangle(int lx, int ly, int ux, int uy) 
{
    MC_line(lx, ly, ux, ly); // 윗변
    MC_line(ux, ly, ux, uy); // 우변
    MC_line(ux, uy, lx, uy); // 아랫변
    MC_line(lx, uy, lx, ly); // 좌변
}

// --- 베지에 곡선 함수 ("그래픽.pdf" Page 5) ---

// 직선 보간
void linear(float p0[], float p1 [], float u, float *p)
{
    p[0]=(1-u)*p0[0]+u*p1[0];
    p[1]=(1-u)*p0[1]+u*p1[1];
}

// 2차 베지에 곡선
void Bezier2(float p0[], float p1[], float p2[], float u, float *p)
{
    float p01[2],p11[2];
    linear (p0, p1, u, p01);
    linear (p1, p2, u, p11);
    linear (p01, p11,u,p);
}

// 3차 베지에 곡선
void Bezier3(float p0[], float p1 [], float p2[], float p3[], float u, float *p)
{
    float p01[2],p11[2];
    Bezier2(p0, p1, p2, u, p01);
    Bezier2(p1, p2, p3, u, p11);
    linear (p01, p11,u,p);
}

// MC에 2차 베지에 곡선 그리기
void MC_Bezier2 (float p0[], float p1 [], float p2[])
{
    float p[2],ps [2],u;
    ps[0] = p0[0]; ps[1] = p0[1]; 
    for(u=0.1;u<1.1;u+=0.1) 
    {
        Bezier2(p0, p1, p2, u,p);
        MC_line((int)ps[0],(int)ps[1],(int)p[0],(int)p[1]);
        ps[0] = p[0]; ps[1]=p[1]; 
    }
}

// MC에 3차 베지에 곡선 그리기
void MC_Bezier3(float p0[], float p1 [ ], float p2[], float p3[])
{
    float p[2],ps [2], u;
    ps[0] = p0[0]; ps[1]=p0[1]; 
    for(u=0.1;u<1.1;u+=0.1)
    {
        Bezier3(p0,p1,p2,p3,u,p);
        MC_line((int)ps[0], (int)ps [1], (int)p[0], (int)p[1]);
        ps[0]=p[0]; ps[1]=p[1]; 
    }
}

// =========================================================
// 텍스트 출력 함수 정의 ("문자.pdf" Page 15)
// (참고: 이 함수들은 MC 버퍼를 사용하지 않고 OLED에 직접 씁니다)
// =========================================================

// (x,y) 텍스트 좌표에 문자 c 출력
void GD_put_char(unsigned char x,unsigned char y,unsigned char c)
{
	OLED_page_addressing_mode(y,x*8);
	MT_OLED(SSD1306_SLA_W,(unsigned char*)ascii8x8v[c],8,GDDRAM);
}

// (x,y) 텍스트 좌표에 문자열 str 출력
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
// 초기화 및 테스트 루틴
// =========================================================

// I2C 통신 초기화 ("문자.pdf" Page 16)
void init_I2C(void)
{
	DDRD = 0x00;        // PD0(SCL), PD1(SDA) 입력
	PORTD = 0x03;       // 내부 풀업 활성화 PD0:SCL, PD1:SDA
	SFIOR &= ~(1<<PUD); // PUD=0: 풀업 활성화 (ATmega128용)
	set_SCL_400kHz();
}

// 텍스트 테스트 ("문자.pdf" Page 16)
void OLED_text_test(void)
{
	init_I2C();
	OLED_init();
    
    // 텍스트 함수는 버퍼를 사용하지 않고 직접 쓰기 때문에
    // OLED_clear()를 호출하면 안 됩니다.
	
	GD_ascii_all();
	_delay_ms(3000); // 3초간 아스키 테이블 표시
	
	GD_put_string(0,0, "Hello, World!");
    // 참고: GD_put_string은 화면을 지우지 않고 덮어씁니다.
}

// 그래픽 테스트 ("그래픽.pdf" Page 6)
void OLED_graphic_test(void)
{
	float p0[]={0,0},p1[]={40,63},p2[]={80,0},p3[]={127,63};
	
	init_I2C(); // I2C는 한 번만 초기화해도 되지만 PDF 구성을 따름
	OLED_init();
	OLED_clear(); // 그래픽 버퍼 사용 시작 (화면 지우기)

	// 1단계: 기본 도형
	MC_rectangle(0,0,20,20);
	MC_line(0,0,20,20);
	MC_line(20,0,0,20);
	MC_circle(10,10,10,0); // 외곽선 원
	MC_rectangle(21,21,41,41);
	MC_circle(31,31,10,1); // 채워진 원
	MC_display(); // 버퍼 내용을 화면에 전송
	_delay_ms(3000);
	
	OLED_clear(); // 화면 지우기
	
	// 2단계: 베지에 곡선
	MC_line(p0[0], p0[1],p1[0],p1[1]);
	MC_line(p1[0],p1[1], p2[0],p2[1]);
	MC_line(p2[0],p2[1],p3[0],p3[1]);
	MC_Bezier2(p0,p1, p2);
	MC_Bezier3(p0, p1, p2, p3);
	MC_display();
	_delay_ms(3000); 
}

// 스크롤 테스트 ("그래픽.pdf" Page 7)
void OLED_scroll_test(void)
{
    // 현재 화면(베지에 곡선)을 스크롤
	OLED_HS(0,0,7,0); // 우측 스크롤 시작
	_delay_ms(3000);
    OLED_deactivate_scroll(); // 스크롤 중지
}

// =========================================================
// 메인 함수
// =========================================================

int main(void)
{
	// PDF들의 테스트 루틴을 순차적으로 실행
	
	// 1. 텍스트 테스트 (직접 쓰기)
	OLED_text_test();
    // (내부 3초 딜레이)
    
	// 2. 그래픽 테스트 (버퍼 사용)
    // (이 테스트는 화면을 지우고 시작합니다)
	OLED_graphic_test();
    // (내부 3초 딜레이 2회)

	// 3. 스크롤 테스트
    // (현재 그래픽 화면을 스크롤합니다)
	OLED_scroll_test();
    // (내부 3초 딜레이)

	while(1)
	{
		// 모든 테스트 완료 후 무한 루프
	}
	return 0; 
}
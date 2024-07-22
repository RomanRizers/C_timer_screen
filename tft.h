//-----------------------------------------------------------------------------//  TFT: Experiments interfacing ATmega328 to an ST7735 1.8" LCD TFT display
//
// Author : Bruce E. Hall <bhall66@gmail.com>
//  Website : w8bh.net
// Version : 1.0
// Date : 04 May 2014
// Target : ATmega16A microcontroller
// Language : C, using AVR studio 6
// Size  : 3622 bytes
//
//  Fuse settings: 8 MHz osc with 65 ms Delay, SPI enable; *NO* clock/8
//
// Connections from LCD to DC Board (ATmega16A):
//
// TFT pin GND: GND
// TFT pin BL: +5V
// TFT pin RESET: PB6
// TFT pin D/C-LD: PB4 
// TFT pin CS-SD: n/c
// TFT pin CS-LD: GND
// TFT pin MOSI: PB5(MOSI)
// TFT pin SCK: PB7(SCK)
// TFT pin MISO: n/c
// TFT pin +5V: +5V
//
//  ---------------------------------------------------------------------------//  MISC ROUTINES

#define ClearBit(x,y) x &= ~_BV(y)  // equivalent to cbi(x,y)
#define SetBit(x,y) x |= _BV(y)  // equivalent to sbi(x,y)
//  ---------------------------------------------------------------------------//  ST7735 ROUTINES
#define SWRESET 0x01  // software reset
#define SLPOUT  0x11  // sleep out
#define DISPOFF 0x28  // display off
#define DISPON  0x29  // display on
#define CASET  0x2A  // column address set
#define RASET  0x2B  // row address set
#define RAMWR  0x2C  // RAM write 
#define MADCTL  0x36    // axis control
#define COLMOD  0x3A  // color mode
// ------------------------------------------------------------------------//    1.8" TFT display constants
#define XSIZE  128
#define YSIZE  160
#define XMAX  XSIZE-1
#define YMAX  YSIZE-1
//  -----------------------------------------------------------------------//   Color constants
#define BLACK  0x0000
#define BLUE  0x001F
#define RED  0xF800
#define GREEN  0x0400
#define LIME  0x07E0
#define CYAN  0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE  0xFFFF
//  ---------------------------------------------------------------------------//  INCLUDES
#include <avr/io.h>  // deal with port registers
#include <avr/interrupt.h>  // deal with interrupt calls
#include <avr/pgmspace.h>  // put character data into progmem
#include <util/delay.h>  // used for _delay_ms function
#include <string.h>  // string manipulation routines
#include <avr/sleep.h>  // used for sleep functions
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

//  ---------------------------------------------------------------------------//  TYPEDEFS
typedef uint8_t byte;  // I just like byte & sbyte better
typedef int8_t sbyte;

//  ---------------------------------------------------------------------------//  GLOBAL VARIABLES
const byte FONT_CHARS[96][5] PROGMEM =
{ 
{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // (space)
{ 0x00, 0x00, 0x5F, 0x00, 0x00 }, // !
{ 0x00, 0x07, 0x00, 0x07, 0x00 }, // "
{ 0x14, 0x7F, 0x14, 0x7F, 0x14 }, // #
{ 0x24, 0x2A, 0x7F, 0x2A, 0x12 }, // $
{ 0x23, 0x13, 0x08, 0x64, 0x62 }, // %
{ 0x36, 0x49, 0x55, 0x22, 0x50 }, // &
{ 0x00, 0x05, 0x03, 0x00, 0x00 }, // '
{ 0x00, 0x1C, 0x22, 0x41, 0x00 }, // (
{ 0x00, 0x41, 0x22, 0x1C, 0x00 }, // )
{ 0x08, 0x2A, 0x1C, 0x2A, 0x08 }, // *
{ 0x08, 0x08, 0x3E, 0x08, 0x08 }, // +
{ 0x00, 0x50, 0x30, 0x00, 0x00 }, // ,
{ 0x08, 0x08, 0x08, 0x08, 0x08 }, // -
{ 0x00, 0x60, 0x60, 0x00, 0x00 }, // .
{ 0x20, 0x10, 0x08, 0x04, 0x02 }, // /
{ 0x3E, 0x51, 0x49, 0x45, 0x3E }, // 0
{ 0x00, 0x42, 0x7F, 0x40, 0x00 }, // 1
{ 0x42, 0x61, 0x51, 0x49, 0x46 }, // 2
{ 0x21, 0x41, 0x45, 0x4B, 0x31 }, // 3
{ 0x18, 0x14, 0x12, 0x7F, 0x10 }, // 4
{ 0x27, 0x45, 0x45, 0x45, 0x39 }, // 5
{ 0x3C, 0x4A, 0x49, 0x49, 0x30 }, // 6
{ 0x01, 0x71, 0x09, 0x05, 0x03 }, // 7
{ 0x36, 0x49, 0x49, 0x49, 0x36 }, // 8
{ 0x06, 0x49, 0x49, 0x29, 0x1E }, // 9
{ 0x00, 0x36, 0x36, 0x00, 0x00 }, // :
{ 0x00, 0x56, 0x36, 0x00, 0x00 }, // ;
{ 0x00, 0x08, 0x14, 0x22, 0x41 }, // <
{ 0x14, 0x14, 0x14, 0x14, 0x14 }, // =
{ 0x41, 0x22, 0x14, 0x08, 0x00 }, // >
{ 0x02, 0x01, 0x51, 0x09, 0x06 }, // ?
{ 0x32, 0x49, 0x79, 0x41, 0x3E }, // @
{ 0x7E, 0x11, 0x11, 0x11, 0x7E }, // A
{ 0x7F, 0x49, 0x49, 0x49, 0x36 }, // B
{ 0x3E, 0x41, 0x41, 0x41, 0x22 }, // C
{ 0x7F, 0x41, 0x41, 0x22, 0x1C }, // D
{ 0x7F, 0x49, 0x49, 0x49, 0x41 }, // E
{ 0x7F, 0x09, 0x09, 0x01, 0x01 }, // F
{ 0x3E, 0x41, 0x41, 0x51, 0x32 }, // G
{ 0x7F, 0x08, 0x08, 0x08, 0x7F }, // H
{ 0x00, 0x41, 0x7F, 0x41, 0x00 }, // I
{ 0x20, 0x40, 0x41, 0x3F, 0x01 }, // J
{ 0x7F, 0x08, 0x14, 0x22, 0x41 }, // K
{ 0x7F, 0x40, 0x40, 0x40, 0x40 }, // L
{ 0x7F, 0x02, 0x04, 0x02, 0x7F }, // M
{ 0x7F, 0x04, 0x08, 0x10, 0x7F }, // N
{ 0x3E, 0x41, 0x41, 0x41, 0x3E }, // O
{ 0x7F, 0x09, 0x09, 0x09, 0x06 }, // P
{ 0x3E, 0x41, 0x51, 0x21, 0x5E }, // Q
{ 0x7F, 0x09, 0x19, 0x29, 0x46 }, // R
{ 0x46, 0x49, 0x49, 0x49, 0x31 }, // S
{ 0x01, 0x01, 0x7F, 0x01, 0x01 }, // T
{ 0x3F, 0x40, 0x40, 0x40, 0x3F }, // U
{ 0x1F, 0x20, 0x40, 0x20, 0x1F }, // V
{ 0x7F, 0x20, 0x18, 0x20, 0x7F }, // W
{ 0x63, 0x14, 0x08, 0x14, 0x63 }, // X
{ 0x03, 0x04, 0x78, 0x04, 0x03 }, // Y
{ 0x61, 0x51, 0x49, 0x45, 0x43 }, // Z
{ 0x00, 0x00, 0x7F, 0x41, 0x41 }, // [
{ 0x02, 0x04, 0x08, 0x10, 0x20 }, // "\"
{ 0x41, 0x41, 0x7F, 0x00, 0x00 }, // ] 
{ 0x04, 0x02, 0x01, 0x02, 0x04 }, // ^
{ 0x40, 0x40, 0x40, 0x40, 0x40 }, // _
{ 0x00, 0x01, 0x02, 0x04, 0x00 }, // `
{ 0x20, 0x54, 0x54, 0x54, 0x78 }, // a
{ 0x7F, 0x48, 0x44, 0x44, 0x38 }, // b
{ 0x38, 0x44, 0x44, 0x44, 0x20 }, // c
{ 0x38, 0x44, 0x44, 0x48, 0x7F }, // d
{ 0x38, 0x54, 0x54, 0x54, 0x18 }, // e
{ 0x08, 0x7E, 0x09, 0x01, 0x02 }, // f 
{ 0x08, 0x14, 0x54, 0x54, 0x3C }, // g
{ 0x7F, 0x08, 0x04, 0x04, 0x78 }, // h
{ 0x00, 0x44, 0x7D, 0x40, 0x00 }, // i
{ 0x20, 0x40, 0x44, 0x3D, 0x00 }, // j
{ 0x00, 0x7F, 0x10, 0x28, 0x44 }, // k
{ 0x00, 0x41, 0x7F, 0x40, 0x00 }, // l
{ 0x7C, 0x04, 0x18, 0x04, 0x78 }, // m
{ 0x7C, 0x08, 0x04, 0x04, 0x78 }, // n
{ 0x38, 0x44, 0x44, 0x44, 0x38 }, // o
{ 0x7C, 0x14, 0x14, 0x14, 0x08 }, // p
{ 0x08, 0x14, 0x14, 0x18, 0x7C }, // q
{ 0x7C, 0x08, 0x04, 0x04, 0x08 }, // r
{ 0x48, 0x54, 0x54, 0x54, 0x20 }, // s
{ 0x04, 0x3F, 0x44, 0x40, 0x20 }, // t
{ 0x3C, 0x40, 0x40, 0x20, 0x7C }, // u
{ 0x1C, 0x20, 0x40, 0x20, 0x1C }, // v
{ 0x3C, 0x40, 0x30, 0x40, 0x3C }, // w
{ 0x44, 0x28, 0x10, 0x28, 0x44 }, // x
{ 0x0C, 0x50, 0x50, 0x50, 0x3C }, // y
{ 0x44, 0x64, 0x54, 0x4C, 0x44 }, // z
{ 0x00, 0x08, 0x36, 0x41, 0x00 }, // {
{ 0x00, 0x00, 0x7F, 0x00, 0x00 }, // |
{ 0x00, 0x41, 0x36, 0x08, 0x00 }, // }
{ 0x08, 0x08, 0x2A, 0x1C, 0x08 }, // ->
{ 0x08, 0x1C, 0x2A, 0x08, 0x08 }}; // <-};

void msDelay(int delay)  // put into a routine
{  // to remove code inlining
	for (int i=0;i<delay;i++)  // at cost of timing accuracy
	_delay_ms(1);
}

unsigned long intsqrt(unsigned long val)
// calculate integer value of square root
{
	unsigned long mulMask = 0x0008000;
	unsigned long retVal  = 0;
	if (val > 0)
	{
		while (mulMask != 0)
		{
			retVal |= mulMask;
			if ((retVal*retVal) > val)
			retVal &= ~ mulMask;
			mulMask >>= 1;
		}
	}
return retVal; 
}
//  ---------------------------------------------------------------------------//  SPI ROUTINES
//
// b7 b6 b5 b4 b3 b2 b1 b0 
// SPCR: SPIE SPE DORD MSTR CPOL CPHA SPR1 SPR0
// 0 1 0 1 . 0 0 0 1
// 
// SPIE - enable SPI interrupt
// SPE - enable SPI
// DORD - 0=MSB first, 1=LSB first
// MSTR - 0=slave, 1=master
// CPOL - 0=clock starts low, 1=clock starts high
// CPHA - 0=read on rising-edge, 1=read on falling-edge
// SPRx - 00=osc/4, 01=osc/16, 10=osc/64, 11=osc/128
//
// SPCR = 0x50: SPI enabled as Master, mode 0, at 16/4 = 4 MHz
void OpenSPI() //start of spi transfer
{
	SPCR = 0x50;  // SPI enabled as Master, Mode0 at 4 MHz 
	SetBit(SPSR,SPI2X);  // double the SPI rate: 4-->8 MHz 
}
void CloseSPI() //start of spi transfer
{
	SPCR = 0x00;  // clear SPI enable bit
}

byte Xfer(byte data) //transfer routine
{
	SPDR = data;  // initiate transfer
	while (!(SPSR & 0x80));  // wait for transfer to complete
	return SPDR;
}
void WriteCmd (byte cmd) //write command to tft
{
	ClearBit(PORTB,4);  // B1=DC; 0=command, 1=data
	Xfer(cmd);
	SetBit(PORTB,4);  // return DC high 
}
void WriteByte (byte b) //write 8 bit data to tft
{
	Xfer(b);
}
void WriteWord (int w) //write 16 bit data to tft
{
	Xfer(w >> 8);  // write upper 8 bits
	Xfer(w & 0xFF);  // write lower 8 bits
}
void Write888 (long data, int count) //write 24 bit data to tft
{
	byte red = data>>16;  // red = upper 8 bits
	byte green = (data>>8) & 0xFF;  // green = middle 8 bits
	byte blue = data & 0xFF;  // blue = lower 8 bits
	for (;count>0;count--)
	{
		WriteByte(red);
		WriteByte(green);
		WriteByte(blue);
	}
}
void Write565 (int data, unsigned int count)
// send 16-bit pixel data to the controller
// note: inlined spi xfer for optimization
{
	WriteCmd(RAMWR);
	for (;count>0;count--)
	{
		SPDR = (data >> 8);  // write hi byte
		while (!(SPSR & 0x80));  // wait for transfer to complete
		SPDR = (data & 0xFF);  // write lo byte
		while (!(SPSR & 0x80));  // wait for transfer to complete
	}
}
void InitTFT() {
	SetupPorts();  // use PortB for LCD interface
	HardwareReset();
	OpenSPI();  // start communication to TFT
	InitDisplay();  // initialize TFT controller 
	ClearScreen();
}
void SetupPorts() //init ports
{
	DDRB |= 0b11110000;  //set B4-B7 as outputs
	SetBit(PORTB,6);  // start with TFT reset line inactive high
}
void HardwareReset() //reset tft
{
	ClearBit(PORTB,6);  // pull TFT reset low
	msDelay(1);  // 1mS is enough 
	SetBit(PORTB,6);  // return TFT reset high
	msDelay(150);  // wait 150mS for reset to finish
}
void InitDisplay() //itin tft
{
	HardwareReset();  // initialize display controller
	WriteCmd(SLPOUT);  // take display out of sleep mode
	msDelay(150);  // wait 150mS for TFT driver circuits
	WriteCmd(COLMOD);  // select color mode: 
	WriteByte(0x05);  // mode 5 = 16bit pixels (RGB565)
	WriteCmd(DISPON);  // turn display on!
}
void SetAddrWindow(byte x0, byte y0, byte x1, byte y1) //rectangular area
{
	WriteCmd(CASET);  // set column range (x0,x1)
	WriteWord(x0);
	WriteWord(x1);
	WriteCmd(RASET);  // set row range (y0,y1)
	WriteWord(y0);
	WriteWord(y1);
}
void ClearScreen() //clear screen
{
	SetAddrWindow(0,0,XMAX,YMAX);  // set window to entire display
	WriteCmd(RAMWR);
	for (unsigned int i=40960;i>0;--i)  // byte count = 128*160*2
	{
		SPDR = 0;  // initiate transfer of 0x00
		while (!(SPSR & 0x80));  // wait for xfer to finish
	} 
}
//  ---------------------------------------------------------------------------//  SIMPLE GRAPHICS ROUTINES
//
// note: many routines have byte parameters, to save space,
// but these can easily be changed to int params for larger displays.
void DrawPixel (byte x, byte y, int color) //draw the pixel
{
	SetAddrWindow(x,y,x,y);
	Write565(color,1);
}
void HLine (byte x0, byte x1, byte y, int color)
// draws a horizontal line in given color
{
	byte width = x1-x0+1;
	SetAddrWindow(x0,y,x1,y);
	Write565(color,width);
}
void VLine (byte x, byte y0, byte y1, int color)
// draws a vertical line in given color
{
	byte height = y1-y0+1;
	SetAddrWindow(x,y0,x,y1);
	Write565(color,height);
}
void Line (int x0, int y0, int x1, int y1, int color)
// an elegant implementation of the Bresenham algorithm 
{
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
	int err = (dx>dy ? dx : -dy)/2, e2;
	for(;;) 
	{
		DrawPixel(x0,y0,color);
		if (x0==x1 && y0==y1) break;
		e2 = err;
		if (e2 >-dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}
}
void DrawRect (byte x0, byte y0, byte x1, byte y1, int color)
// draws a rectangle in given color
{
	HLine(x0,x1,y0,color);
	HLine(x0,x1,y1,color);
	VLine(x0,y0,y1,color);
	VLine(x1,y0,y1,color);
}
void FillRect (byte x0, byte y0, byte x1, byte y1, int color) //filled rectangular
{
	byte width = x1-x0+1;
	byte height = y1-y0+1;
	SetAddrWindow(x0,y0,x1,y1);
	Write565(color,width*height);
}
void CircleQuadrant (byte xPos, byte yPos, byte radius, byte quad, int color)
// draws circle quadrant(s) centered at x,y with given radius & color
// quad is a bit-encoded representation of which cartesian quadrant(s) to draw.
// Remember that the y axis on our display is 'upside down':
// bit 0: draw quadrant I (lower right)
// bit 1: draw quadrant IV (upper right)
// bit 2: draw quadrant II (lower left)
// bit 3: draw quadrant III (upper left)
{
	int x, xEnd = (707*radius)/1000 + 1;
	for (x=0; x<xEnd; x++)
	{
		byte y = intsqrt(radius*radius - x*x);
		if (quad & 0x01)
		{
			DrawPixel(xPos+x,yPos+y,color);  // lower right
			DrawPixel(xPos+y,yPos+x,color);
		}
		if (quad & 0x02)
		{
			DrawPixel(xPos+x,yPos-y,color);  // upper right
			DrawPixel(xPos+y,yPos-x,color);
		}
		if (quad & 0x04)
		{
			DrawPixel(xPos-x,yPos+y,color);  // lower left
			DrawPixel(xPos-y,yPos+x,color);
		}
		if (quad & 0x08)
		{
			DrawPixel(xPos-x,yPos-y,color);  // upper left
			DrawPixel(xPos-y,yPos-x,color);
		}
	}
}
void Circle (byte xPos, byte yPos, byte radius, int color)
// draws circle at x,y with given radius & color
{
	CircleQuadrant(xPos,yPos,radius,0x0F,color); // do all 4 quadrants
}
void RoundRect (byte x0, byte y0, byte x1, byte y1, byte r, int color)
// draws a rounded rectangle with corner radius r.
// coordinates: top left = x0,y0; bottom right = x1,y1 
{
	HLine(x0+r,x1-r,y0,color);  // top side
	HLine(x0+r,x1-r,y1,color);  // bottom side
	VLine(x0,y0+r,y1-r,color);  // left side
	VLine(x1,y0+r,y1-r,color);  // right side
	CircleQuadrant(x0+r,y0+r,r,8,color);  // upper left corner
	CircleQuadrant(x1-r,y0+r,r,2,color);  // upper right corner
	CircleQuadrant(x0+r,y1-r,r,4,color);  // lower left corner
	CircleQuadrant(x1-r,y1-r,r,1,color);  // lower right corner
}
void FillCircle (byte xPos, byte yPos, byte radius, int color)
// draws filled circle at x,y with given radius & color
{
	long r2 = radius * radius;
	for (int x=0; x<=radius; x++)
	{
		byte y = intsqrt(r2-x*x);
		byte y0 = yPos-y;
		byte y1 = yPos+y;
		VLine(xPos+x,y0,y1,color);
		VLine(xPos-x,y0,y1,color);
	}
}
void Ellipse (int x0, int y0, int width, int height, int color)
// draws an ellipse of given width & height
// two-part Bresenham method
// note: slight discontinuity between parts on some (narrow) ellipses.
{
	int a=width/2, b=height/2;
	int x = 0, y = b;
	long a2 = (long)a*a*2;
	long b2 = (long)b*b*2;
	long error = (long)a*a*b;
	long stopY=0, stopX = a2*b;
	while (stopY <= stopX)
	{
		DrawPixel(x0+x,y0+y,color);
		DrawPixel(x0+x,y0-y,color);
		DrawPixel(x0-x,y0+y,color);
		DrawPixel(x0-x,y0-y,color);
		x++;
		error -= b2*(x-1);
		stopY += b2;
		if (error < 0)
		{
			error += a2*(y-1);
			y--;
			stopX -= a2;
		}
	}
	x=a; y=0; error = b*b*a;
	stopY = a*b2; stopX = 0;
	while (stopY >= stopX)
	{
		DrawPixel(x0+x,y0+y,color);
		DrawPixel(x0+x,y0-y,color);
		DrawPixel(x0-x,y0+y,color);
		DrawPixel(x0-x,y0-y,color);
		y++;
		error -= a2*(y-1);
		stopX += a2;
		if (error < 0)
		{
			error += b2*(x-1);
			x--;
			stopY -= b2;
		}
	}
} 
void FillEllipse(int xPos,int yPos,int width,int height, int color)
// draws a filled ellipse of given width & height
{
	int a=width/2, b=height/2;  // get x & y radii
	int x1, x0 = a, y = 1, dx = 0;
	long a2 = a*a, b2 = b*b;  // need longs: big numbers!
	long a2b2 = a2 * b2;
	HLine(xPos-a,xPos+a,yPos,color);  // draw centerline
	while (y<=b)  // draw horizontal lines...
	{
		for (x1= x0-(dx-1); x1>0; x1--)
		if (b2*x1*x1 + a2*y*y <= a2b2)
		break;
		dx = x0-x1;
		x0 = x1;
		HLine(xPos-x0,xPos+x0,yPos+y,color);
		HLine(xPos-x0,xPos+x0,yPos-y,color);
		y += 1;
	}
}
//  ---------------------------------------------------------------------------//  TEXT ROUTINES
// 
// Each ASCII character is 5x7, with one pixel space between characters
// So, character width = 6 pixels & character height = 8 pixels.
//
// In portrait mode: 
// Display width = 128 pixels, so there are 21 chars/row (21x6 = 126).
// Display height = 160 pixels, so there are 20 rows (20x8 = 160).
// Total number of characters in portait mode = 21 x 20 = 420.
//
// In landscape mode:
// Display width is 160, so there are 26 chars/row (26x6 = 156).
// Display height is 128, so there are 16 rows (16x8 = 128).
// Total number of characters in landscape mode = 26x16 = 416. 
byte curX,curY;  // current x & y cursor position
void GotoXY (byte x,byte y)
// position cursor on character x,y grid, where 0<x<20, 0<y<19.
{
	curX = x;
	curY = y;
}
void GotoLine(byte y)
// position character cursor to start of line y, where 0<y<19.
{
	curX = 0;
	curY = y;
}
void AdvanceCursor()
// moves character cursor to next position, assuming portrait orientation
{
	curX++;  // advance x position
	if (curX>20)  // beyond right margin?
	{
		curY++;  // go to next line
		curX = 0;  // at left margin
	}
	if (curY>19)  // beyond bottom margin?
	curY = 0;  // start at top again
}
void SetOrientation(int degrees)
// Set the display orientation to 0,90,180,or 270 degrees
{ 
	byte arg;
	switch (degrees)
	{
		case  90: arg = 0x60; break;
		case 180: arg = 0xC0; break;
		case 270: arg = 0xA0; break;
		default:  arg = 0x00; break;
	}
	WriteCmd(MADCTL);
	WriteByte(arg);
}
void PutCh (char ch, byte x, byte y, int color)
// write ch to display X,Y coordinates using ASCII 5x7 font
{
	int  pixel;
	byte row, col, bit, data, mask = 0x01;
	SetAddrWindow(x,y,x+4,y+6);
	WriteCmd(RAMWR);
	for (row=0; row<7;row++)
	{
		for (col=0; col<5;col++)
		{
			data = pgm_read_byte(&(FONT_CHARS[ch-32][col]));
			bit = data & mask;
			if (bit==0) pixel=BLACK;
			else  pixel=color;
			WriteWord(pixel);
		}
		mask <<= 1;
	}
}
void WriteChar(char ch, int color)
// writes character to display at current cursor position.
{
	PutCh(ch,curX*6, curY*8, color);
	AdvanceCursor();
}
void WriteString(char *text, int color)
// writes string to display at current cursor position.
{
	for (;*text;text++)  // for all non-nul chars
	WriteChar(*text,color);  // write the char
}
void WriteInt(int i)
// writes integer i at current cursor position
{
	char str[8];  // buffer for string result
	itoa(i,str,10);  // convert to string, base 10
	WriteString(str,WHITE); 
}
void WriteHex(int i)
// writes hexadecimal value of integer i at current cursor position
{
	char str[8];  // buffer for string result
	itoa(i,str,16);  // convert to base 16 (hex)
	WriteString(str,WHITE);
}
//  ---------------------------------------------------------------------------//  TEST ROUTINES
void PixelTest()
// draws 4000 pixels on the screen
{
	ClearScreen();
	for (int i=4000; i>0; i--)  // do a whole bunch: 
	{
		int x = rand() % XMAX;  // random x coordinate
		int y = rand() % YMAX;  // random y coordinate
		DrawPixel(x,y,YELLOW);  // draw pixel at x,y
	}
}
void LineTest()
// sweeps Line routine through all four quadrants.
{
	ClearScreen();
	int x,y,x0=64,y0=80;
	for (x=0;x<XMAX;x+=2) Line(x0,y0,x,0,YELLOW);
	for (y=0;y<YMAX;y+=2) Line(x0,y0,XMAX,y,CYAN);
	for (x=XMAX;x>0;x-=2) Line(x0,y0,x,YMAX,YELLOW);
	for (y=YMAX;y>0;y-=2) Line(x0,y0,0,y,CYAN);
}
void CircleTest()
// draw series of concentric circles
{
	ClearScreen();
	for(int radius=6;radius<60;radius+=2)
	Circle(60,80,radius,YELLOW);
}
void PortraitChars()
// Writes 420 characters (5x7) to screen in portrait mode
{
	ClearScreen();
	for (int i=420;i>0;i--)
	{
		byte x= i % 21;
		byte y= i / 21;
		char ascii = (i % 96)+32;
		PutCh(ascii,x*6,y*8,CYAN);
	}
}

//==============================================================================================================
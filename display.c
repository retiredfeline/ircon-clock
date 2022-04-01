#define STM8S103

#include <stm8s_gpio.h>

#include "clock.h"

#include "display.h"

static uint8_t hhmm_buffer[4];
static uint8_t digit_number;
const static uint8_t font[] =
#ifdef	TM1637	// normal 7-segment font
{ 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#else		// nixie driving font
// bits 0 and 1 are even/odd, bits 2-6 are 01,23,45,67,89
{ 0x05, 0x06, 0x09, 0x0A, 0x11, 0x12, 0x21, 0x22, 0x41, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
#endif

const static port_pin ser = { GPIOD, GPIO_PIN_1 };
const static port_pin srclk = { GPIOD, GPIO_PIN_2 };
const static port_pin rclk = { GPIOD, GPIO_PIN_3 };

#define	GPIOWriteValue(port,pin,val)	((val) ? GPIO_WriteHigh(port,pin) : GPIO_WriteLow(port,pin))

static void setBrightness(uint8_t);

void display_init(void)
{
#ifndef	DS3231
	GPIO_DeInit(GPIOB);
	GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_PP_HIGH_SLOW);        // LED
#endif
	GPIO_Init(ser.port, ser.pin, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_Init(srclk.port, srclk.pin, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_Init(rclk.port, rclk.pin, GPIO_MODE_OUT_PP_LOW_SLOW);
#ifdef	TM1637
	setBrightness(0x8f);
#endif
}

static void digits_update(uint8_t *buffer, uint8_t value)
{
	uint8_t	tens = value / 10;
	uint8_t units = value % 10;

	buffer[0] = font[tens];
	buffer[1] = font[units];
}

// taken from https://github.com/unfrozen/stm8_libs/blob/master/lib_delay.c
static void delay_usecs(uint8_t usecs)
{
	usecs;
__asm
	ld	a, (3, sp)
	dec	a
	clrw	x
	ld	xl,a
#ifdef STM8103
	sllw	x
#endif
	sllw	x
00001$:
	nop			; (1)
	decw	x		; (1)
	jrne	00001$		; (2)
__endasm;
}

#ifdef	TM1637

static void startXfer()
{
	GPIO_WriteHigh(srclk.port, srclk.pin);
	GPIO_WriteHigh(ser.port, ser.pin);
	delay_usecs(5);
	GPIO_WriteLow(ser.port, ser.pin);
	GPIO_WriteLow(srclk.port, srclk.pin);
	delay_usecs(5);
}

static void stopXfer()
{
	GPIO_WriteLow(srclk.port, srclk.pin);
	GPIO_WriteLow(ser.port, ser.pin);
	delay_usecs(5);
	GPIO_WriteHigh(srclk.port, srclk.pin);
	GPIO_WriteHigh(ser.port, ser.pin);
	delay_usecs(5);
}

static uint8_t writeByte(uint8_t value)
{
	uint8_t i;

	for (i = 0; i < 8; i++) {
		GPIO_WriteLow(srclk.port, srclk.pin);
		delay_usecs(5);
		GPIOWriteValue(ser.port, ser.pin, value & 0x1);
		delay_usecs(5);
		GPIO_WriteHigh(srclk.port, srclk.pin);
		delay_usecs(5);
		value >>= 1;
	}
	GPIO_WriteLow(srclk.port, srclk.pin);
	delay_usecs(5);
	GPIO_WriteHigh(ser.port, ser.pin);
	GPIO_WriteHigh(srclk.port, srclk.pin);
	delay_usecs(5);
	return 1;
}

static void setBrightness(uint8_t val)
{
	startXfer();
	writeByte(val);
	stopXfer();
}

void display_update(void)
{
	digits_update(&hhmm_buffer[0], TIME.hours);
	digits_update(&hhmm_buffer[2], TIME.minutes);
	startXfer();
	writeByte(0x40);
	stopXfer();
	startXfer();
	writeByte(0xc0);
	for (uint8_t i = sizeof(hhmm_buffer), *p = hhmm_buffer; i > 0; i--, p++)
		writeByte(*p);
	stopXfer();
}
	
#else

// send one translated byte, MSb first
static void display_digit(uint8_t digit)
{
	for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
		GPIOWriteValue(ser.port, ser.pin, digit & mask);
		delay_usecs(5);
		GPIO_WriteHigh(srclk.port, srclk.pin);
		delay_usecs(5);
		GPIO_WriteLow(srclk.port, srclk.pin);
		delay_usecs(5);
	}
}

// send load pulse to latch serial data to output register
static void display_load(void)
{
	GPIO_WriteHigh(rclk.port, rclk.pin);
	delay_usecs(5);
	GPIO_WriteLow(rclk.port, rclk.pin);
	delay_usecs(5);
}

void display_update(void)
{
	digits_update(&hhmm_buffer[0], TIME.hours);
	digits_update(&hhmm_buffer[2], TIME.minutes);
	for (uint8_t i = sizeof(hhmm_buffer), *p = hhmm_buffer; i > 0; i--, p++)
		display_digit(*p);
	display_load();
}

#endif

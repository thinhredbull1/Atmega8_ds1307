/*
ds1307 lib 0x01

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.

References: parts of the code taken from https://github.com/adafruit/RTClib
*/


#ifndef DS1307_H
#define DS1307_H
#define F_CPU 16000000UL
#include <util/delay.h>
#include "avr/pgmspace.h"
//definitions
#define DS1307_ADDR (0x68<<1) //device address

//path to i2c fleury lib
#define DS1307_I2CFLEURYPATH "i2cmaster.h" //define the path to i2c fleury lib
#define DS1307_I2CINIT 1 //init i2c

//functions
extern void ds1307_init();
extern uint8_t ds1307_setdate(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
extern void ds1307_getdate(volatile uint8_t *year,volatile uint8_t *month,volatile uint8_t *day, volatile uint8_t *hour,volatile uint8_t *minute,volatile uint8_t *second);

#endif


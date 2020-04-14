#ifndef __DS3231_H_
#define __DS3231_H_

#include "VIIC_C51.h"

#define DS3231_ADD 		0xD0

typedef struct {
	unsigned char year;
	unsigned char month;
	unsigned char date;
	unsigned char dow;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
} Time;

void SetDS3231PPS(unsigned char enable);
bit GetDS3231PPS();
void GetDS3231(Time *time);
void SetDS3231(Time *time);

#endif
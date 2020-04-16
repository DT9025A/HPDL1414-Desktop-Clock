/*********************
	DS3231»ù´¡Çý¶¯³ÌÐò
	by DT9025A
	2019/8/15
*********************/

#include "ds3231.h"

unsigned char HEX2BCD (unsigned char dat) {
    return (((dat / 10) << 4) + (dat % 10));
}

unsigned char BCD2HEX (unsigned char dat) {
    return ((dat >> 4) * 10 + (dat & 0x0f));
}

void SetDS3231PPS (unsigned char enable) {
    unsigned char temp;
    IRcvStr (DS3231_ADD, 0x0e, &temp, 1);
	temp &= 0xe3;
	if(enable == 0)
		temp |= 0x04;
	ISendStr (DS3231_ADD, 0x0e, &temp, 1);
}

//bit GetDS3231PPS() {
//    unsigned char temp;
//    IRcvStr (DS3231_ADD, 0x0e, &temp, 1);
//    if (temp & 0x1c == 0x1c)
//        return 1;
//    return 0;
//}

void GetDS3231 (Time *time) {
    unsigned char buf[7];

    IRcvStr (DS3231_ADD, 0, buf, 7);
    time->second = BCD2HEX (buf[0]);
    time->minute = BCD2HEX (buf[1]);
    time->hour = BCD2HEX (buf[2]);
    time->dow = BCD2HEX (buf[3]);
    time->date = BCD2HEX (buf[4]);
    time->month = BCD2HEX (buf[5]);
    time->year = BCD2HEX (buf[6]);
}

void SetDS3231 (Time *time) {
    unsigned char buf[7];
    
    buf[0] = HEX2BCD (time->second);
    buf[1] = HEX2BCD (time->minute);
    buf[2] = HEX2BCD (time->hour);
    buf[3] = HEX2BCD (time->dow);
    buf[4] = HEX2BCD (time->date);
    buf[5] = HEX2BCD (time->month);
    buf[6] = HEX2BCD (time->year);
	ISendStr (DS3231_ADD, 0, buf, 7);
}
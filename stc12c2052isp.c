/*************************
	面对STC12C2052系列的片上E2PROM驱动程序
	by DT9025A at 2020/1/15
**************************/

#include "stc12c2052isp.h"

void TRIGISP (unsigned int addrOffset, unsigned char cmd) {
    addrOffset += FLASH_START_ADDR;
    ISP_ADDRH = addrOffset >> 8;
    ISP_ADDRL = (unsigned char)addrOffset & 0xff;
    EA = 0;
    ISP_CONTR = WAIT_TIME | 0x80;
    ISP_CMD = cmd;
    ISP_TRIG = 0x46;
    ISP_TRIG = 0xB9;
    ISP_ADDRH = ISP_ADDRL = ISP_TRIG = ISP_CMD = ISP_CONTR = 0;
    EA = 1;
}

void ByteProgram (unsigned int addrOffset, unsigned char x) {
    ISP_DATA = x;
    TRIGISP (addrOffset, ISP_IAP_BYTE_PROGRAM);
}

void BufProgram (unsigned int addrOffset, unsigned int cnt, unsigned char *buf) {
    unsigned int i;
    for (i = 0; i < cnt; i++)
        ByteProgram (addrOffset + i, buf[i]);
}

unsigned char ByteRead (unsigned int addrOffset) {
    TRIGISP (addrOffset, ISP_IAP_BYTE_READ);
    return ISP_DATA;
}

void BufRead (unsigned int addrOffset, unsigned int cnt, unsigned char *buf) {
    unsigned int i;
    for (i = 0; i < cnt; i++)
        buf[i] = ByteRead (addrOffset + i);
}

void SectorErase (unsigned char sectorNumber) {
    unsigned int addrOffset;

    addrOffset = sectorNumber * 512;

    TRIGISP (addrOffset, ISP_IAP_SECTOR_ERASE);
}
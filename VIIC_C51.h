#ifndef _VIIC_51_H_
#define _VIIC_51_H_

#include <STC12C2052AD.H> //STC头文件
#include <intrins.h>

#define _Nop() _nop_() /*定义空指令*/
sbit SDA = P3 ^ 5; /*模拟I2C 数据传送位*/
sbit SCL = P3 ^ 7; /*模拟I2C 时钟控制位*/

//bit GetAck();
//bit ISendByte (unsigned char sla, unsigned char c);
bit ISendStr (unsigned char sla, unsigned char suba, unsigned char *s, unsigned char no);
//bit IRcvByte (unsigned char sla, unsigned char *c);
bit IRcvStr (unsigned char sla, unsigned char suba, unsigned char *s, unsigned char no);

#endif
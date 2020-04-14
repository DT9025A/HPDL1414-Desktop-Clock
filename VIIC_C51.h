#ifndef _VIIC_51_H_
#define _VIIC_51_H_

#include <STC12C2052AD.H> //STCͷ�ļ�
#include <intrins.h>

#define _Nop() _nop_() /*�����ָ��*/
sbit SDA = P3 ^ 5; /*ģ��I2C ���ݴ���λ*/
sbit SCL = P3 ^ 7; /*ģ��I2C ʱ�ӿ���λ*/

//bit GetAck();
//bit ISendByte (unsigned char sla, unsigned char c);
bit ISendStr (unsigned char sla, unsigned char suba, unsigned char *s, unsigned char no);
//bit IRcvByte (unsigned char sla, unsigned char *c);
bit IRcvStr (unsigned char sla, unsigned char suba, unsigned char *s, unsigned char no);

#endif
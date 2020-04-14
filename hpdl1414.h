#ifndef __HPDL_1414_H_
#define __HPDL_1414_H_

#include <STC12C2052AD.H>
#include <intrins.h>

#define MODELNUM 1

sbit W_R = P3 ^ 4;
sbit A1 = P3 ^ 0;
sbit A0 = P3 ^ 1;
//sbit W_R_1 = P3 ^ 2;

#define DATA_OUT P1

typedef unsigned char u8;
typedef unsigned int u16;

void dispChar (char x, u8 pos, u8 i);
void dispString (char *str);

#endif
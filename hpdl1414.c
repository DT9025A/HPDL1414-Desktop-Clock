/*********************
	HPDL1414 驱动程序
	by DT9025A at 2020/4/12
********************/

#include "hpdl1414.h"

//位选
void Digital (u8 d) {
    d = ~d;
    d &= 0x03;
    A1 = d >> 1;
    A0 = d & 1;
}

//W/R 参数为HPDL1414编号 从1开始 对应W_R
void W (u8 i) {
#if (MODELNUM == 2)
    if (i) {
        W_R = 1;
        W_R_1 = 0;
    } else {
        W_R = 0;
        W_R_1 = 1;
    }
#else
    W_R = i;
#endif
}

//清除W/R状态
void CWR() {
#if (MODELNUM == 2)
    W_R = W_R_1 = 1;
#else
    W_R = 1;
#endif
}

//显示字符
void dispChar (char x, u8 pos, u8 i) {
    if (x < ' ' || x > '_')
        return;
    Digital (pos);
    DATA_OUT = x;
    W (i);
    _nop_();
    _nop_();
    CWR ();
}

//显示字符串，超出显示范围则折回开头继续显示
void dispString (char *str) {
    u8 pos = 0;
    while (*str) {
        dispChar (*str++, pos % 4, pos / 4);
        if (pos == MODELNUM * 4 - 1)
            pos = 0;
        else
            pos += 1;
    }
}

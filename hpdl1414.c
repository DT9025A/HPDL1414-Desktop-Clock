#include "hpdl1414.h"

//λѡ
void Digital (u8 d) {
    d = ~d;
    d &= 0x03;
    A1 = d >> 1;
    A0 = d & 1;
}

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
    if (i == 0)
        W_R = 0;
    else
        W_R = 1;
#endif
}

void CWR() {
#if (MODELNUM == 2)
    W_R = W_R_1 = 1;
#else
    W_R  = 1;
#endif
}

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

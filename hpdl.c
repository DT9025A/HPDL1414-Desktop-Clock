/*********************
	基于STC12C4052,使用HPDL-1414和DS3231的桌面时钟
	by DT9025A at 2020/4/12

	V1.2 First Release
	KEIL4 C51 : Program Size: data=57.1 xdata=0 code=3816

	V1.3
	修正了PPS可能不同步的BUG
	更新了一些驱动，进一步减小体积
	KEIL4 C51 : Program Size: data=57.1 xdata=0 code=3804
********************/

#include <STC12C2052AD.H>
#include "hpdl1414.h"
#include "VIIC_C51.h"
#include "ds3231.h"
#include "stc12c2052isp.h"

//系统设置
typedef struct {
    unsigned int SysCycle;
    unsigned int TimeCycle;		//Cycle = Sec * 200
    unsigned int TempCycle;
    unsigned char DS3231PPS;
} SysConfigStruct;

//K2菜单
#define MonthDateMenu_MonthDate 0
#define MonthDateMenu_DayOfWeek 1
#define MonthDateMenu_Year      2

//按键检测
#define WAITKEY while(K1Pressed==0&&K2Pressed==0)
#define SINGLEPRESS_TICK 700

#define MDDELAY_TICK 400

//设置掩码
#define CONFIG_HEAD_MASK 0x99
#define CONFIG_END_MASK  0xAA

//外部声明
unsigned char DSS (void);

//全局变量
bit InConfig = 0, UpdateTime = 0, UpdateTemp = 0, K1Pressed = 0, K2Pressed = 0, DS3231PPS = 1, InEditingDOW = 0;
unsigned int sysTick = 0, lastK1Tick = 65535, lastK2Tick = 65535;
unsigned char MonthDateMenu = MonthDateMenu_MonthDate, buf[5];
Time time;
SysConfigStruct config = {2400, 2000, 400, 0};

//版本号
char code VERSION[] = {"V1.3"};

//IO
sbit K1 = P3 ^ 2;	//INT0
sbit K2 = P3 ^ 3;	//INT1

void Timer0_Init (void) {	//5毫秒@12.000MHz
    AUXR |= 0x80;		//定时器时钟1T模式
    TMOD &= 0xF0;		//设置定时器模式
    TMOD |= 0x01;		//设置定时器模式
    TL0 = 0xA0;		//设置定时初值
    TH0 = 0x15;		//设置定时初值
    TF0 = 0;		//清除TF0标志
    TR0 = 1;		//定时器0开始计时
}

//延迟500ms
void Delay500ms() {	//@12.000MHz
    unsigned char i, j, k;

    i = 23;
    j = 205;
    k = 120;
    do {
        do {
            while (--k);
        } while (--j);
    } while (--i);
}

//延迟5ms
void Delay5ms() {	//@12.000MHz
    unsigned char i, j;

    _nop_();
    _nop_();
    i = 59;
    j = 89;
    do {
        while (--j);
    } while (--i);
}

//读设置
bit LoadConfig () {
    unsigned char _buf[5];
    BufRead (16, 5, _buf);
    if (_buf[0] == CONFIG_HEAD_MASK && _buf[4] == CONFIG_END_MASK) {
        //掩码正确
        if (_buf[1] > 0)
            config.TimeCycle = _buf[1] * 200;
        if (_buf[2] > 0)
            config.TempCycle = _buf[2] * 200;
        config.SysCycle = config.TimeCycle + config.TempCycle;
        config.DS3231PPS = _buf[3];
        SetDS3231PPS (_buf[3]);
        return 1;
    }
    return 0;
}

//写设置
void SaveConfig () {
    unsigned char _buf[5];
    dispString ("SAVE");
    _buf[0]	= CONFIG_HEAD_MASK;
    _buf[1] = config.TimeCycle / 200;
    _buf[2] = config.TempCycle / 200;
    _buf[3] = config.DS3231PPS;
    _buf[4]	= CONFIG_END_MASK;
    SectorErase (0);
    BufProgram (16, 5, _buf);
}

//关EA后延时
void Delay1sWithEA() {
    EA = 0;
    Delay500ms();
    Delay500ms();
    EA = 1;
}

//在指定位置格式化两位数字
void FormatDigit2 (unsigned char x, unsigned char pos) {
    buf[pos] = x / 10 + '0';
    buf[pos + 1] = x % 10 + '0';
}

//按键扫描
unsigned char ScanKey() {
    WAITKEY;
    if (K1 == 0 && K2 == 0) {
        K1Pressed = K2Pressed = 0;
        while (K1 == 0 || K2 == 0);
        Delay5ms();
        while (K1 == 0 || K2 == 0);
        return 3;
    }
    if (K1Pressed) {
        K1Pressed = 0;
        return 1;
    }
    if (K2Pressed) {
        //ADD
        K2Pressed = 0;
        return 2;
    }
    return 0;
}

//显示周几
//if 比 switch 少10字节
void dispDOW() {
    if (time.dow == 1)
        dispString ("MON ");
    else if (time.dow == 2)
        dispString ("TUE ");
    else if (time.dow == 3)
        dispString ("WED ");
    else if (time.dow == 4)
        dispString ("THU ");
    else if (time.dow == 5)
        dispString ("FRI ");
    else if (time.dow == 6)
        dispString ("SAT ");
    else if (time.dow == 7)
        dispString ("SUN ");
    else
        dispString ("ERR ");
}

//显示PPS设置
void dispDS3231PPS() {
    if (config.DS3231PPS)
        dispString ("ENAB");
    else
        dispString ("DSAB");
}

//闪烁显示当前缓冲区内容
void flashCurrentBuf() {
    dispString (buf);
    Delay500ms();
    dispString ("    ");
    Delay500ms();
    dispString (buf);
    Delay500ms();
}

//设置某项的值
void SetValue (unsigned char *val, unsigned char lo, unsigned char up, char *title, char *buf, unsigned char arg2) {
    unsigned int lastKeyTick;
    unsigned char lastVal = *val;
    dispString (title);
    Delay1sWithEA();
    while (1) {
        if (InEditingDOW)
            dispDOW();
        else {
            FormatDigit2 (*val, arg2);
            dispString (buf);
        }
        lastKeyTick = sysTick;
        switch (ScanKey()) {
            case 1:
                if (lastKeyTick - sysTick > SINGLEPRESS_TICK) {
                    lastKeyTick = sysTick;
                    lastVal = *val;
                    if (*val > lo)
                        *val -= 1;
                }
                break;
            case 2:
                if (lastKeyTick - sysTick > SINGLEPRESS_TICK) {
                    lastKeyTick = sysTick;
                    lastVal = *val;
                    if (*val < up)
                        *val += 1;
                }
                break;
            case 3:
                *val = lastVal;
                if (InEditingDOW) {
                    dispDOW();
                    Delay500ms();
                    dispString ("    ");
                    Delay500ms();
                    dispDOW();
                    Delay500ms();
                } else {
                    FormatDigit2 (*val, arg2);
                    flashCurrentBuf();
                }
                return;
        }
    }
}

//设置
void SysConfig() {
    unsigned int lastKeyTick;
    unsigned char lastVal;
    dispString ("CONF");
    Delay1sWithEA();
    dispString ("TIME");
    WAITKEY; //等待按键
    if (K1Pressed) {
        //时间设置
        K1Pressed = 0;
        goto TIME_SET;
    }
    if (K2Pressed) {
        //下一个
        K2Pressed = 0;
        dispString ("DELA");
        WAITKEY;
        if (K1Pressed) {
            //延时
            K1Pressed = 0;
            goto DELAY_SET;
        }
        if (K2Pressed) {
            //下一个
            K2Pressed = 0;
            dispString ("LPPS");
            WAITKEY;
            if (K1Pressed) {
                //LED PPS
                K1Pressed = 0;
                goto LED_SET;
            }
            if (K2Pressed) {
                //退出
                K2Pressed = 0;
                goto CONFIG_OUT;
            }
        }
    }

LED_SET:
    while (1) {
        dispDS3231PPS();
        lastKeyTick = sysTick;
        InEditingDOW  = 0;
        switch (ScanKey()) {
            case 1:
                if (lastKeyTick - sysTick > SINGLEPRESS_TICK) {
                    lastKeyTick = sysTick;
                    lastVal = config.DS3231PPS;
                    config.DS3231PPS = 1;
                }
                break;
            case 2:
                if (lastKeyTick - sysTick > SINGLEPRESS_TICK) {
                    lastKeyTick = sysTick;
                    lastVal = config.DS3231PPS;
                    config.DS3231PPS = 0;
                }
                break;
            case 3:
                config.DS3231PPS = lastVal;
                dispDS3231PPS();
                SetDS3231PPS (config.DS3231PPS);
                Delay500ms();
                dispString ("    ");
                Delay500ms();
                dispDS3231PPS();
                Delay500ms();
                goto CONFIG_OUT;
        }
    }

DELAY_SET:
    buf[0] = ' ';
    buf[3] = 'S';
    InEditingDOW  = 0;

    lastVal = config.TimeCycle / 200;
    SetValue (&lastVal, 1, 150, "TICY", buf, 1);
    config.TimeCycle = lastVal * 200;

    lastVal = config.TempCycle / 200;
    SetValue (&lastVal, 1, 150, "TECY", buf, 1);
    config.TempCycle = lastVal * 200;

    config.SysCycle = config.TimeCycle + config.TempCycle;

    goto CONFIG_OUT;

TIME_SET:
    GetDS3231 (&time);
    //年
    buf[0] = '2';
    buf[1] = '0';
    InEditingDOW  = 0;
    SetValue (&time.year, 0, 99, "YEAR", buf, 2);
    //月
    buf[0] = buf[3] = ' ';
    InEditingDOW  = 0;
    SetValue (&time.month, 0, 12, "MNTH", buf, 1);
    //日
    InEditingDOW  = 0;
    SetValue (&time.date, 0, 31, "DATE", buf, 1);
    //DOW
    InEditingDOW = 1;
    SetValue (&time.dow, 1, 7, "DOW", buf, 1);
    //HH
    InEditingDOW  = 0;
    SetValue (&time.hour, 0, 23, "HOUR", buf, 1);
    //MM
    InEditingDOW  = 0;
    SetValue (&time.minute, 0, 59, "MIN ", buf, 1);
    time.second = 0;

    dispString ("C  S");
    WAITKEY;
    if (K2Pressed) {
        SetDS3231 (&time);
        dispString ("SET ");
    } else if (K1Pressed) {
        dispString ("CANL");
    }
    Delay500ms();

CONFIG_OUT:
    sysTick = config.SysCycle - 1;
    InConfig = 0;
    InEditingDOW  = 0;
    K2Pressed = K1Pressed = 0;
    SaveConfig();
    Delay500ms();
}

//中断初始化
void Interrupt_Init() {
    ET0 = 1; //定时器0
    EX0 = 1; //INT0
    IT0 = 0; //LOW
    EX1 = 1; //INT1
    IT1 = 0; //LOW
    EA = 1; //开放所有中断
}

void main() {
    unsigned char temperature = 0;

    Interrupt_Init();
    Timer0_Init();

    dispString (VERSION);
    Delay500ms();

    if (LoadConfig())
        dispString ("LOAD");
    else {
        dispString ("FAIL");
        Delay500ms();
        SaveConfig();
    }
    Delay500ms();
    buf[4] = 0;

    while (1) {
        //主循环
        if (config.DS3231PPS == 1 && DS3231PPS == 0) {
            SetDS3231PPS (1);
            DS3231PPS = 1;
        }
        if (InConfig)
            continue;
        if (UpdateTemp) {
            //更新温度
            UpdateTemp = 0;

            MonthDateMenu = MonthDateMenu_MonthDate;

            do {
                temperature = DSS();
            } while (temperature == 85);

            buf[0] = ' ';
            FormatDigit2 (temperature, 1);

            buf[3] = 'C';
            dispString (buf);
            UpdateTime = 0;
        }
        if (UpdateTime) {
            //更新时间
            UpdateTime = 0;

            MonthDateMenu = MonthDateMenu_MonthDate;
            GetDS3231 (&time);
            FormatDigit2 (time.hour, 0);
            FormatDigit2 (time.minute, 2);
            dispString (buf);
        }
        if (K1Pressed) {
            //进入设置
            K1Pressed = 0;
            InConfig = 1;
            SysConfig();
        }
        if (K2Pressed) {
            //年月日星期显示
            K2Pressed = 0;
            if (MonthDateMenu == MonthDateMenu_Year) {
                SetDS3231PPS (config.DS3231PPS);
                MonthDateMenu = MonthDateMenu_MonthDate;
                //年
                buf[0] = '2';
                buf[1] = '0';
                FormatDigit2 (time.year, 2);
                dispString (buf);
            } else if (MonthDateMenu == MonthDateMenu_DayOfWeek) {
                MonthDateMenu = MonthDateMenu_Year;
                //星期
                dispDOW();
            } else if (MonthDateMenu == MonthDateMenu_MonthDate) {
                MonthDateMenu = MonthDateMenu_DayOfWeek;
                //月日
                SetDS3231PPS (0);
                DS3231PPS = 0;
                FormatDigit2 (time.month, 0);
                FormatDigit2 (time.date, 2);
                dispString (buf);
            }
        }
    }
}

//系统systick
void Timer0_IRQHandler() interrupt 1 {
    if (sysTick == 0) {
        UpdateTime = 1;
    } else if (sysTick == config.TimeCycle) {
        UpdateTemp = 1;
    } else if (sysTick == config.SysCycle) {
        sysTick = 0;
        return;
    }
    sysTick += 1;
}

//K1中断
void INT0_IRQHandler() interrupt 0 {
    if (K1Pressed == 0 && lastK1Tick != 65535) {
        if (sysTick - lastK1Tick >= 75) {
            K1Pressed = 1;
            lastK1Tick = 65535;
        }
    } else {
        lastK1Tick = sysTick;
    }
}

//K2中断
void INT1_IRQHandler() interrupt 2 {
    if (K2Pressed == 0 && lastK2Tick != 65535) {
        if (sysTick - lastK2Tick >= 75) {
            K2Pressed = 1;
            lastK2Tick = 65535;
        }
    } else {
        lastK2Tick = sysTick;
    }
}

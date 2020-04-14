# HPDL-1414 桌面时钟

### 简述

使用STC12C4052做主控，DS3231模块提供时间和作为支架，DS18B20提供温度数据。

左下角的LED为秒闪LED，PPS信号来自DS3231；右下角的电位器为HPDL1414亮度调整。

PCB使用杜洋工作室的 LEAF20-。

走线规划苦手，飞线走起来。

没有电容，没有滤波，生死全靠适配器（逃

提供时间设置，显示界面停留时间设置，秒闪设置，断电数据保存。

更新全靠标志位，按键全部绑中断，系统生死看T0心情。

---

### 显示效果

![](https://raw.githubusercontent.com/DT9025A/HPDL1414-Desktop-Clock/master/Images/front.jpg)



---

### 电路相关

反正是做着自己用的，也没啥要求，随便玩就行（

参考电路图如下

![](https://raw.githubusercontent.com/DT9025A/HPDL1414-Desktop-Clock/master/Images/refsch.jpg)

---

### 操作方式

详见[操作指南](https://github.com/DT9025A/HPDL1414-Desktop-Clock/blob/master/操作指南.md)
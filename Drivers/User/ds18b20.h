#ifndef __DS18B20_H
#define __DS18B20_H


unsigned char ds18b20_init(void);         /* 初始化DS18B20 */
unsigned char ds18b20_check(void);        /* 检测是否存在DS18B20 */
short ds18b20_get_temperature(void);/* 获取温度 */

//函数声明
//unsigned char ds18b20_init(void);
//float ds18b20_read_temperture(void);

#endif
















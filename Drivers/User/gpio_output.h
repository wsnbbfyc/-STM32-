#ifndef __GPIO_OUTPUT_H__
#define __GPIO_OUTPUT_H__

void GpioOutput_Init(void);
void GpioOutput_Control(unsigned short ledBit, unsigned char ledState);
void GpioOutput_ToggleControl(unsigned short num);
void GpioOutput_NumbetrControl(unsigned short num, unsigned char ledState);
void GpioOutput_GapControl(unsigned short ledBit);

#endif

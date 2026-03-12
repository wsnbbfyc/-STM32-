#ifndef __FLASH_H__
#define __FLASH_H__

unsigned char Flash_Init(void);
void Flash_SaveValue(unsigned char num, unsigned int value);
unsigned int Flash_ReadValue(unsigned char num);

#endif

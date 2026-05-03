#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f10x.h"

void Flash_Write(u32 addr,u32*buf,u32 len);
void Flash_Read(u32 addr,u32*buf,u32 len);

#endif

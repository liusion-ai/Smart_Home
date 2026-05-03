#include "flash.h"

void Flash_Read(u32 addr,u32*buf,u32 len)
{
    for(u32 i=0;i<len;i++) buf[i]=*(u32*)(addr+4*i);
}

void Flash_Write(u32 addr,u32*buf,u32 len)
{
    FLASH_Unlock();FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
    FLASH_ErasePage(addr);
    for(u32 i=0;i<len;i++) FLASH_ProgramWord(addr+4*i,buf[i]);
    FLASH_Lock();
}


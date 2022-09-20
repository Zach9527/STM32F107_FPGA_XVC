#ifndef __DRVEXFLASH__H__
#define __DRVEXFLASH__H__

//#include "Typedef.h"
#include "stm32f107.h"

#define  FLASH_WRITE_ENABLE_CMD         0x06
#define  FLASH_WRITE_DISABLE_CMD        0x04
#define  FLASH_READ_SR_CMD              0x05
#define  FLASH_WRITE_SR_CMD             0x01
#define  FLASH_READ_DATA                0x03
#define  FLASH_WRITE_PAGE               0x02
#define  FLASH_ERASE_PAGE               0x81
#define  FLASH_ERASE_SECTOR             0x20
#define	 FLASH_ERASE_BLOCK              0xD8
#define	 FLASH_ERASE_CHIP               0xC7
#define  FLASH_READ_ID            0x9f


#define PAGE_LEN        255  //256byte


uint32 Flash_ReadID(void);
void Flash_WaitBusy(void);

void Flash_WriteEnable(void);
void Flash_WriteDisable(void);
uint8 Flash_ReadSR(void);
void Flash_WriteSR(uint8 dat);
void Flash_ReadSomeByte(uint8* pBuffer, uint32 ReadAddr, uint16 NumByteToWrite);
void Flash_WritePage(uint8* pBuffer,uint32 WriteAddr,uint16 NumByteToWrite);
uint32 Flash_Write(uint8* pBuffer,uint32 WriteAddr,uint16 NumByteToWrite);
uint32 Flash_WriteNoCheck(uint8* pBuffer,uint32 WriteAddr,uint16 NumByteToWrite);

void Flash_EraseSector(uint32 Dst_Addr);
void Flash_EraseBlock(uint32 Dst_Addr);
void Flash_EraseChip(void);












#endif	//__DRVEXFLASH__H__

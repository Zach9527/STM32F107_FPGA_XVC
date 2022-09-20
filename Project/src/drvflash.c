
#include "drvflash.h"
#include "stm32f107.h"


uint32 Flash_ReadID(void)
{
    uint32 temp = 0;
    NSS_L;
    SPI_WriteByte(FLASH_READ_ID);
    temp |= SPI_ReadByte() << 16;
    temp |= SPI_ReadByte() << 8;
    temp |= SPI_ReadByte();
    NSS_H;
    return temp;
}

void Flash_WriteEnable(void)
{
    NSS_L;
    SPI_WriteByte(FLASH_WRITE_ENABLE_CMD);
    NSS_H;
}

void Flash_WriteDisable(void)
{
    NSS_L;
    SPI_WriteByte(FLASH_WRITE_DISABLE_CMD);
    NSS_H;
}

uint8 Flash_ReadSR(void)
{
    uint8 temp = 0;
    NSS_L;
    SPI_WriteByte(FLASH_READ_SR_CMD);
    temp = SPI_ReadByte();
    NSS_H;
    return temp;
}

void Flash_WriteSR(uint8 dat)
{
    NSS_L;
    SPI_WriteByte(FLASH_WRITE_SR_CMD);
    SPI_WriteByte(dat);
    NSS_H;
}
void Flash_ReadSomeByte(uint8* pBuffer, uint32 ReadAddr, uint16 NumByteToWrite)
{
    uint16 i = 0;
    int count = 0;
    NSS_L;
    SPI_WriteByte(FLASH_READ_DATA);
    SPI_WriteByte(ReadAddr >> 16);
    SPI_WriteByte(ReadAddr >> 8);
    SPI_WriteByte(ReadAddr);
    for( i = 0; i < NumByteToWrite; i++)
    {
        pBuffer[i] = SPI_ReadByte();
    }
    NSS_H;
}

void Flash_WritePage(uint8* pBuffer,uint32 WriteAddr,uint16 NumByteToWrite)
{
    uint16 i = 0;
    Flash_WriteEnable();
    NSS_L;
    SPI_WriteByte(FLASH_WRITE_PAGE);
    SPI_WriteByte(WriteAddr >> 16);
    SPI_WriteByte(WriteAddr >> 8);
    SPI_WriteByte(WriteAddr);
    for( i = 0; i < NumByteToWrite; i++)
    {
        SPI_WriteByte(pBuffer[i]);
    }
    NSS_H;
    Flash_WaitBusy();
}
extern int earseFlag;
extern void System_Periodic_Handle(void);

void Flash_WaitBusy(void)
{
    while((Flash_ReadSR()&0X01) == 0X01);

}

uint32 Flash_WriteNoCheck(uint8* pBuffer,uint32 WriteAddr,uint16 NumByteToWrite)
{
    uint16 pageremain;
    pageremain=256 - WriteAddr % 256; //单页剩余的字节数
    if(NumByteToWrite <= pageremain)
        pageremain = NumByteToWrite;//不大于256个字节
    while(1)
    {
        Flash_WritePage(pBuffer, WriteAddr, pageremain);
        if(NumByteToWrite == pageremain)
            return 1;
            //break;//写入结束了
        else //NumByteToWrite>pageremain
        {
            pBuffer += pageremain;
            WriteAddr += pageremain;
            NumByteToWrite -= pageremain;//减去已经写入的字节数
            if(NumByteToWrite > 256)
                pageremain = 256; //一次可以写入256个字节
            else 
                pageremain = NumByteToWrite;//不够256个字节了
        }
    }
}

uint32 Flash_Write(uint8* pBuffer,uint32 WriteAddr,uint16 NumByteToWrite) //一次写入大于4K
{
    uint32 secpos;
    uint16 secoff;
    uint16 secremain;
    uint16 i;
    secpos = WriteAddr / 4096;//扇区地址
    secoff = WriteAddr % 4096;//在扇区内的偏移
    secremain = 4096 - secoff;//扇区剩余空间大小
    if(NumByteToWrite <= secremain)
        secremain = NumByteToWrite;//不大于4096个字节
    while(1)
    {
        Flash_WriteNoCheck(pBuffer,WriteAddr,secremain);//写已经擦出了的，直接写入扇区剩余空间
        if(NumByteToWrite == secremain)
            return 1;//写入结束了
        else//写入未结束
        {
            secpos++;//扇区地址增加1
            secoff=0;//偏移地址为0
            pBuffer += secremain;  //ָ指针偏移
            WriteAddr += secremain;//写地址偏移
            NumByteToWrite -= secremain;//字节数递减
            if(NumByteToWrite > 4096)
                secremain = 4096;//下个扇区还是写不完
            else
                secremain = NumByteToWrite;//下个扇区可以写完了
        }
    };
}


void Flash_EraseSector(uint32 Dst_Addr)
{
    Dst_Addr*=4096;//4K
    Flash_WriteEnable();
    Flash_WaitBusy();
    NSS_L;
    SPI_WriteByte(FLASH_ERASE_SECTOR);
    SPI_WriteByte((uint8)((Dst_Addr)>>16));
    SPI_WriteByte((uint8)((Dst_Addr)>>8));
    SPI_WriteByte((uint8)Dst_Addr);
    NSS_H;
    Flash_WaitBusy();//等待擦除完成

}
void Flash_EraseBlock(uint32 Dst_Addr)
{
    Dst_Addr*=65535; //64K
    Flash_WriteEnable();
    Flash_WaitBusy();
    NSS_L;
    SPI_WriteByte(FLASH_ERASE_BLOCK);
    SPI_WriteByte((uint8)((Dst_Addr)>>16));
    SPI_WriteByte((uint8)((Dst_Addr)>>8));
    SPI_WriteByte((uint8)Dst_Addr);
    NSS_H;
    Flash_WaitBusy();//等待擦除完成
}

void Flash_EraseChip(void)
{
    Flash_WriteEnable();
    Flash_WaitBusy(); // MT25Q256 P66
    NSS_L;
    SPI_WriteByte(FLASH_ERASE_CHIP);
    NSS_H;
	//Flash_WaitBusy();
}  


#include "flash.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stm32f10x_flash.h"
#include "main.h"

//IP 信息保存在FLASH的最后那一页
//#define IP_INFO_FLASH_ADDR 0x082FF000
#define IP_INFO_FLASH_ADDR 0x0803F800 //stm32f10xxx参考手册32页互联型产品flash最后一页起始地址
#define TIME_OUT_CNT 0x50000 //超时时间
#define READ_CNT_FLASH_ADDR 0x0803F000
//flash中存储的IP信息内存数据镜像
IP_INFO_TYPE ipInfoFlash;

/*
 * 擦除flash
 * unsigned int pageAddr:Page地址，驱动中没有加任何偏移
* return:FLASH_OP_ERR表示失败/FLASH_OK表示成功
 */
int fmcEraseBlk1Pages(unsigned int pageAddr)
{
    unsigned int timeout;
    FlagStatus sta;
    volatile unsigned int *reg = (volatile unsigned int *)0x4002204c;
    
    /* unlock the flash program/erase controller */
    FLASH_Unlock();
    
    /* clear all pending flags */
    FLASH_ClearFlag(FLASH_FLAG_EOP);
    FLASH_ClearFlag(FLASH_FLAG_WRPRTERR);
    FLASH_ClearFlag(FLASH_FLAG_PGERR);
    
    FLASH_ErasePage(pageAddr);
    timeout = 0;
    do{
        sta = FLASH_GetFlagStatus(FLASH_FLAG_BSY);
        if(timeout++ > TIME_OUT_CNT)
        {
            printf("fmcEraseBlk1Pages timeout\r\n");
            /* lock the main FMC after the erase operation */
            FLASH_Lock();
            return FLASH_OP_ERR;
        }
    }while(SET == sta); //等待擦除动作完成
    timeout = 0;
    do{
        sta = FLASH_GetFlagStatus(FLASH_FLAG_EOP);
        if(timeout++ > TIME_OUT_CNT)
        {
            printf("sta=0x%08x\r\n", *reg);
            printf("fmcEraseBlk1Pages timeout\r\n");
            /* lock the main FMC after the erase operation */
            FLASH_Lock();
            return FLASH_OP_ERR;
        }
    }while(SET != sta); //等待擦除动作完成
    
    /* erase the flash pages */
    FLASH_ClearFlag(FLASH_FLAG_EOP);
    FLASH_ClearFlag(FLASH_FLAG_WRPRTERR);
    FLASH_ClearFlag(FLASH_FLAG_PGERR);
    
    
    /* lock the main FMC after the erase operation */
    FLASH_Lock();
    
    return FLASH_OK;
}

/*
 * flash Page编程
 * unsigned int pageAddr:Page地址，驱动中没有加任何偏移
 * unsigned char *data：需要写入的数据
 * unsigned int size：写入数据的长度，长度必须为4字节整数倍,不能超过4K字节
* return:FLASH_OP_ERR表示失败/FLASH_OK表示成功
 */
int fmcProgramBlk1Pages(unsigned int pageAddr, unsigned char *data, unsigned int size)
{
    unsigned int timeout;
    FlagStatus sta;
    unsigned int i, addr;
    unsigned int *p;
    
    /* unlock the flash program/erase controller */
    FLASH_Unlock();
    
    p = (unsigned int *)data;
    /* program flash */
    for(i = 0, addr = pageAddr; i < size/4; i++, addr += 4)
    {
        FLASH_ProgramWord(addr, p[i]);
        //fmc_page_erase(pageAddr);
        //判断
        timeout = 0;
        do{
            sta = FLASH_GetFlagStatus(FLASH_FLAG_BSY);
            if(timeout++ > TIME_OUT_CNT)
            {
                printf("fmcProgramBlk1 timeout\r\n");
                /* lock the main FMC after the erase operation */
                FLASH_Lock();
                return FLASH_OP_ERR;
            }
        }while(SET == sta); //等待擦除动作完成
        timeout = 0;
        do{
            sta = FLASH_GetFlagStatus(FLASH_FLAG_EOP);
            if(timeout++ > TIME_OUT_CNT)
            {
                printf("fmcProgramBlk1Pages timeout\r\n");
                /* lock the main FMC after the erase operation */
                FLASH_Lock();
                return FLASH_OP_ERR;
            }
        }while(SET != sta); //等待擦除动作完成
    
        FLASH_ClearFlag(FLASH_FLAG_EOP);
        FLASH_ClearFlag(FLASH_FLAG_WRPRTERR);
        FLASH_ClearFlag(FLASH_FLAG_PGERR);
    }
    
    /* lock the main FMC after the program operation */
    FLASH_Lock();
    
    return FLASH_OK;
}

/*
 * 数据和校验
 * uint8_t *buf：需要校验的数据
 * int size：数据大小
* return:返回校验和
 */
static uint32_t checkSumFun(uint8_t *buf, int size)
{
    uint32_t sum = 0;
    uint32_t val;
    int i;
    
    for(i = 0; i < size; i++)
    {
        val = buf[i];
        sum += val;
    }
    
    return sum;
}

/*
 * 保存写入fpga flash 文件大小
* return:FLASH_OP_ERR表示flash读写错误/FLASH_CHK_ERR表示数据校验错误/FLASH_OK表示成功
 */
int writeCntInfo(uint32* count)
{
    if(FLASH_OK != fmcEraseBlk1Pages(READ_CNT_FLASH_ADDR) )
    {
        return FLASH_OP_ERR;
    }
    if(FLASH_OK != fmcProgramBlk1Pages(READ_CNT_FLASH_ADDR, (uint8_t *)count,sizeof(uint32)) )
    {
        return FLASH_OP_ERR;
    }
    return FLASH_OK;
}

/*
 * 读取写入fpga flash 文件大小
* return:写入数据大小
 */
int readCntInfo(void)
{
    volatile  uint32_t *p32Flash;
    p32Flash = (volatile uint32_t *)READ_CNT_FLASH_ADDR;
    return *p32Flash;
}




/*
 * 保存网口IP信息
 * IP_INFO_TYPE *ipInfo:写入的IP信息数据结构体
* return:FLASH_OP_ERR表示flash读写错误/FLASH_CHK_ERR表示数据校验错误/FLASH_OK表示成功
 */
int writeIpInfo(IP_INFO_TYPE *ipInfo)
{
    int uint32Num;
    
    ipInfo->head = IP_INFO_HEAD;
    ipInfo->tail = IP_INFO_TAIL;
    
    ipInfo->checkSum = checkSumFun((uint8_t *)ipInfo, sizeof(IP_INFO_TYPE)-sizeof(ipInfo->checkSum));
    if(FLASH_OK != fmcEraseBlk1Pages(IP_INFO_FLASH_ADDR) )
    {
        return FLASH_OP_ERR;
    }
    
    uint32Num = sizeof(IP_INFO_TYPE)/sizeof(uint32_t);
    if(FLASH_OK != fmcProgramBlk1Pages(IP_INFO_FLASH_ADDR, (uint8_t *)ipInfo, uint32Num * sizeof(uint32_t)) )
    {
        return FLASH_OP_ERR;
    }
    
    return FLASH_OK;
}

/*
 * 读取网口IP信息
 * IP_INFO_TYPE *ipInfo:读取的IP信息数据结构体
* return:FLASH_OP_ERR表示flash读写错误/FLASH_CHK_ERR表示数据校验错误/FLASH_OK表示成功
 */
int readIpInfo(IP_INFO_TYPE *ipInfo)
{
    int i, uint32Num;
    IP_INFO_TYPE info;
    uint32_t *p32;
    volatile  uint32_t *p32Flash;
    uint32_t sum;
    
    p32 = (uint32_t *)&info;
    p32Flash = (volatile uint32_t *)IP_INFO_FLASH_ADDR;
    uint32Num = sizeof(IP_INFO_TYPE)/sizeof(uint32_t);
    for(i = 0; i < uint32Num; i++)
    {
        p32[i] = p32Flash[i];
    }
    
    sum = checkSumFun((uint8_t *)&info, sizeof(info)-sizeof(info.checkSum));
    
    if(info.checkSum != sum)
    {
        return FLASH_CHK_ERR;
    }
    
    if( (IP_INFO_HEAD != info.head)\
        || (IP_INFO_TAIL != info.tail) )
    {
        return FLASH_CHK_ERR;
    }
    
    *ipInfo = info;
    
    return FLASH_OK;
}

/*
 * 转换IP字符串为IP信息
 * char *ip：IP字符串
 * int len：IP字符串长度
 * MY_IP_ADDR *ipAddr：IP信息
* return:FLASH_OP_ERR表示flash读写错误/FLASH_CHK_ERR表示数据校验错误/FLASH_OK表示成功
 */
 #define SINGLE_IP_SEG_MAX IP_STR_MAX_LEN
 #define SINGLE_IP_SEG_NUM 4
int strToIp(char *ip, int len, MY_IP_ADDR *ipAddr)
{
    int i;
    char ipStr[IP_STR_MAX_LEN];
    char ipSub[SINGLE_IP_SEG_NUM][SINGLE_IP_SEG_MAX];
    char *ipStrp;
        
    if(len > IP_STR_MAX_LEN)
    {
        return FLASH_CHK_ERR;
    }
    
    memset(ipStr, 0, IP_STR_MAX_LEN);
    strncpy(ipStr, ip, len);
    memset(ipSub, 0, SINGLE_IP_SEG_NUM * SINGLE_IP_SEG_MAX * sizeof(ipSub[0][0]) );
    
    for(i = 0; i < IP_STR_MAX_LEN; i++)
    {
        if('.' == ipStr[i] )
        {
            ipStr[i] = 0;
        }
    }
    ipStr[IP_STR_MAX_LEN-1] = 0;
    
    ipStrp = ipStr;
    for(i = 0; i < SINGLE_IP_SEG_NUM; i++)
    {
        strcpy(ipSub[i], ipStrp);
        if(strlen(ipSub[i]) <= 0)
        {
            strcpy(ipSub[i], "0");
        }
        if(strlen(ipSub[i]) > 3)
        {
            strcpy(ipSub[i], "255");
        }
        
        ipStrp += strlen(ipStrp) + 1;
        if(ipStrp >= ipStr+IP_STR_MAX_LEN)
        {
            ipStrp = ipStr + IP_STR_MAX_LEN - 1;
        }
    }
    
    ipAddr->addr_0 = atoi(ipSub[0]);
    ipAddr->addr_1 = atoi(ipSub[1]);
    ipAddr->addr_2 = atoi(ipSub[2]);
    ipAddr->addr_3 = atoi(ipSub[3]);
    
    return FLASH_OK;
}

static uint8_t toHex(char h, char l)
{
    uint8_t val;
    uint8_t val_h = 0xf, val_l = 0xf;
    
    if( (h <= 'f') && (h >= 'a') )
    {
        val_h = h-'a' + 0xa;
    }
    if( (h <= '9') && (h >= '0') )
    {
        val_h = h-'0' + 0x0;
    }
    if( (h <= 'F') && (h >= 'A') )
    {
        val_h = h-'A' + 0xa;
    }
    
    if( (l <= 'f') && (l >= 'a') )
    {
        val_l = l-'a' + 0xa;
    }
    if( (l <= '9') && (l >= '0') )
    {
        val_l = l-'0' + 0x0;
    }
    if( (l <= 'F') && (l >= 'A') )
    {
        val_l = l-'A' + 0xa;
    }
    
    val = val_h<<4 | val_l;
    return val;
}

/*
 * 转换MAC地址字符串为MAC信息数据
 * char *mac：mac字符串
 * int len：MAC字符串长度
 * MY_MAC_ADDR *macAddr：MAC信息
* return:FLASH_OP_ERR表示flash读写错误/FLASH_CHK_ERR表示数据校验错误/FLASH_OK表示成功
 */
 #define SINGLE_MAC_SEG_MAX MAC_STR_MAX_LEN
 #define SINGLE_MAC_SEG_NUM 6
int strToMac(char *mac, int len, MY_MAC_ADDR *macAddr)
{
    int i;
    char macStr[MAC_STR_MAX_LEN];
    char macSub[SINGLE_MAC_SEG_NUM][SINGLE_MAC_SEG_MAX];
    char *macStrp;
        
    if(len > MAC_STR_MAX_LEN)
    {
        return FLASH_CHK_ERR;
    }
    memset(macStr, 0, MAC_STR_MAX_LEN);	
    strncpy(macStr, mac, len);
    memset(macSub, 0, SINGLE_MAC_SEG_NUM * SINGLE_MAC_SEG_MAX * sizeof(macSub[0][0]) );
    
    for(i = 0; i < MAC_STR_MAX_LEN; i++)
    {
        if(':' == macStr[i] )
        {
            macStr[i] = 0;
        }
    }
    macStr[MAC_STR_MAX_LEN-1] = 0;
    
    macStrp = macStr;
    for(i = 0; i < SINGLE_MAC_SEG_NUM; i++)
    {
        strcpy(macSub[i], macStrp);
        if(strlen(macSub[i]) <= 0)
        {
            strcpy(macSub[i], "0");
        }
        if(strlen(macSub[i]) == 1)
        {
            macSub[i][1] = macSub[i][0] ;
            macSub[i][0] = '0';
        }
        if(strlen(macSub[i]) > 2)
        {
            macSub[i][0] = 'f';
            macSub[i][1] = 'f';
        }
        
        macStrp += strlen(macStrp) + 1;
        if(macStrp >= macStr+MAC_STR_MAX_LEN)
        {
            macStrp = macStr + MAC_STR_MAX_LEN - 1;
        }
    }
    
    macAddr->mac_0 = toHex(macSub[0][0], macSub[0][1]);
    macAddr->mac_1 = toHex(macSub[1][0], macSub[1][1]);
    macAddr->mac_2 = toHex(macSub[2][0], macSub[2][1]);
    macAddr->mac_3 = toHex(macSub[3][0], macSub[3][1]);
    macAddr->mac_4 = toHex(macSub[4][0], macSub[4][1]);
    macAddr->mac_5 = toHex(macSub[5][0], macSub[5][1]);
    
    return FLASH_OK;
}

/*
 * 转换PORT字符串为port信息数据
 * char *port：port字符串
 * int len：port字符串长度
 * uint16_t *portInfo：端口信息
* return:FLASH_OP_ERR表示flash读写错误/FLASH_CHK_ERR表示数据校验错误/FLASH_OK表示成功
 */
int strToPort(char *port, int len, uint16_t *portInfo)
{
    int val;
    
    if(len > 5)
    {
        return FLASH_CHK_ERR;
    }
    
    val = atoi(port);
    val = val > 65535 ? 65535 : val;
    
    *portInfo = (uint16_t)val;
    return FLASH_OK;
}


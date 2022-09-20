#ifndef __TCP_SERVER_H
#define __TCP_SERVER_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f107.h"
#define TCP_RECV_BUF_MAX_SIZE 2048
#define TCPHEADFRAME 0xaaaa5555
#define TCPTAILFRAME 0x5555aaaa

/** @defgroup tcpserver_Exported_Functions
  * @{
  */
#define XVC_DATA0_PIN GPIO_Pin_0
#define XVC_DATA1_PIN GPIO_Pin_1
#define XVC_DATA2_PIN GPIO_Pin_2
#define XVC_DATA3_PIN GPIO_Pin_3
#define XVC_DATA4_PIN GPIO_Pin_4
#define XVC_DATA5_PIN GPIO_Pin_5
#define XVC_DATA6_PIN GPIO_Pin_6
#define XVC_DATA7_PIN GPIO_Pin_7
//地址线
#define XVC_ADDR0_PIN GPIO_Pin_8
#define XVC_ADDR1_PIN GPIO_Pin_9
#define XVC_ADDR2_PIN GPIO_Pin_10
#define XVC_ADDR3_PIN GPIO_Pin_11
#define XVC_ADDR4_PIN GPIO_Pin_12
#define XVC_ADDR5_PIN GPIO_Pin_13
#define XVC_ADDR6_PIN GPIO_Pin_14
#define XVC_ADDR7_PIN GPIO_Pin_15


#define ICD_MAX_SIZE 1024 //ICD最大长度为1K
#define ICD_DAT_MAX_SIZE 896 //ICD最大有效数据载荷896
#define ICD_FILEDAT_MAX_SIZE (ICD_DAT_MAX_SIZE-8) //ICD最大有效的文件数据载荷896
#define ICD_CMD_EARSE 0x00000001 //擦除指令
#define ICD_CMD_WRITE 0x00000002 //数据写入指令
#define ICD_CMD_READ 0x00000003 //数据读取指令
#define ICD_CMD_PARA_GET 0x00000004 //参数获取指令
#define ICD_CMD_PARA_SET 0x00000005 //参数设置取指令
#define ICD_CMD_STATE_GET 0x00000006 //状态获取
//擦除指令ICD定义
typedef struct {
    uint32_t head;//包头
    uint32_t cmd;//表示该条ICD为什么命令
    uint32_t size;//表示数据字段的字节个数
    uint32_t fpgaCs;//FPGA片选
    uint32_t addr;//擦除的地址
    uint32_t spaceSize;//擦除空间大小
    uint32_t tail;//包尾
    uint32_t checkSum;//和校验
} ICD_EARSE_CMD_T;

typedef struct {
    uint32_t head;//包头
    uint32_t cmd;//表示该条ICD为什么命令
    uint32_t size;//表示数据字段的字节个数
    uint8_t  dat[1];//数据段
    uint32_t tail;//包尾
    uint32_t checkSum;//和校验
} ICD_COMMON_HEAD_T;
typedef struct {
    uint32_t head;//包头
    uint32_t cmd;//表示该条ICD为什么命令
    uint32_t size;//表示数据字段的字节个数
    uint32_t opRet;//操作结果 0x0000_0001:表示擦除成功，其他:表示擦除失败
    uint32_t tail;//包尾
    uint32_t checkSum;//和校验
} ICD_COMMON_RSP_T;

typedef struct {
    uint32_t head;//包头
    uint32_t cmd;//表示该条ICD为什么命令
    uint32_t size;//表示数据字段的字节个数
    uint32_t fpgaCs;//FPGA片选
    uint32_t ackCnt;//在一个写流程中，第几次请求写操作
    uint32_t tail;//包尾
    uint32_t checkSum;//和校验
} ICD_READ_CMD_T;

typedef struct {
    uint32_t head;//包头
    uint32_t cmd;//表示该条ICD为什么命令
    uint32_t size;//表示数据字段的字节个数
    uint8_t  dat[ICD_DAT_MAX_SIZE];//数据段
    uint32_t tail;//包尾
    uint32_t checkSum;//和校验
} ICD_READ_RSP_T;
//服务器参数获取响应ICD定义   或者    服务器参数设置响应ICD定义
typedef struct {
    uint32_t head;//包头
    uint32_t cmd;//表示该条ICD为什么命令
    uint32_t size;//表示数据字段的字节个数
    uint8_t  ip[4];//IP地址
    uint16_t port;//端口
    uint8_t  mac[6];//MAC地址
    uint32_t tail;//包尾
    uint32_t checkSum;//和校验
} ICD_PARA_T;

typedef struct {
    uint32_t head;//包头
    uint32_t cmd;//表示该条ICD为什么命令
    uint32_t size;//表示数据字段的字节个数
    uint32_t fpgaCs;//FPGA片选
    uint32_t ackCnt;//在一个写流程中，第几次请求写操作
    uint8_t dat[ICD_FILEDAT_MAX_SIZE];//需要写入的数据
    uint32_t tail;//包尾
    uint32_t checkSum;//和校验
} ICD_WRITE_CMD_T;


//服务器参数设置响应ICD定义
typedef struct {
    uint32_t head;//包头
    uint32_t cmd;//表示该条ICD为什么命令
    uint32_t size;//表示数据字段的字节个数
    uint32_t result;
    uint32_t tail;//包尾
    uint32_t checkSum;//和校验
} ICD_PARA_R;

typedef struct 
{
    uint32 frameHead;
    uint32 cmd;
    uint32 size;
    uint32 fpgaCs;
}ICD_COMMON_CMD;

typedef struct
{
    uint32_t head;//包头
    uint32_t cmd;//表示该条ICD为什么命令
    uint32_t size;//表示数据字段的字节个数
    uint8_t  dat[48];//数据段
    uint32_t tail;//包尾
    uint32_t checkSum;//和校验
}ICD_STATE_RSP_T;
void Tcp_Server_Init(void);
void XVC_Interface(struct tcp_pcb * tpcb, uint8 * recv,uint16_t length);

uint32 check_sum(uint8* pbuf,int len);
int flashChoice(int select);
void ipaddrGet(uint32 ip, uint8 *str);
void macaddrGet(uint8 *mac, uint8 *str);
int ipaddrSet(uint8_t *cmd);
int macaddrSet(uint8_t *cmd);
int portSet(uint16 port);
void setAddressLine(uint8 addr);
void setDataLine(uint8 data);
uint8 getDataLine(void);
void fpgaStateGet(uint8* sbuf);

/**
  * @}
  */


#endif /* __TCP_SERVER_H */




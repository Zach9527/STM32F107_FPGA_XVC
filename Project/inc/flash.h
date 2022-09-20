#ifndef __FLASH_H_
#define __FLASH_H_

#ifdef cplusplus
 extern "C" {
#endif

#include "stm32f10x.h"
#include "netconf.h"

#define IP_STR_MAX_LEN 16 //必须为4的整数倍
#define MAC_STR_MAX_LEN 24 //必须为4的整数倍
#define PORT_STR_MAX_LEN 8 //必须为4的整数倍

#define FLASH_OK 0
#define FLASH_OP_ERR (1) //flash读写错误
#define FLASH_CHK_ERR (2) //数据校验错误

#define IP_INFO_HEAD 0x1234AABB
#define IP_INFO_TAIL 0xEEFF1234
#define DATA_VALID_VAL 0xAAAA5555
//该结构体为4字节对齐
//用于保存网口ip相关信息
typedef struct {
	uint32_t head;
	char ip[IP_STR_MAX_LEN];
	uint32_t ipValid;//字段是否有效
	char mask[IP_STR_MAX_LEN];
	uint32_t maskValid;//字段是否有效
	char gw[IP_STR_MAX_LEN];
	uint32_t gwValid;//字段是否有效
	char mac[MAC_STR_MAX_LEN];
	uint32_t macValid;//字段是否有效
	char port[PORT_STR_MAX_LEN];
	uint32_t portValid;//字段是否有效
	uint32_t tail;
	uint32_t checkSum;//校验和
} IP_INFO_TYPE;

//flash中存储的ip信息内存数据镜像
extern IP_INFO_TYPE ipInfoFlash;

/*
 * 擦除flash
 * unsigned int pageAddr:Page地址，驱动中没有加任何偏移
* return:FLASH_OP_ERR表示失败/FLASH_OK表示成功
 */
int fmcEraseBlk1Pages(unsigned int pageAddr);

/*
 * flash Page编程
 * unsigned int pageAddr:Page地址，驱动中没有加任何偏移
 * unsigned char *data需要写入的数据
 * unsigned int size写入的数据长度，必须为4字节的整数倍，不超过4K
* return:FLASH_OP_ERR失败/FLASH_OK成功
 */
int fmcProgramBlk1Pages(unsigned int pageAddr, unsigned char *data, unsigned int size);

/*
 * 保存网口IP信息
 * IP_INFO_TYPE *ipInfo:写入的ip信息数据结构体
* return:FLASH_OP_ERR flash读写错误/FLASH_CHK_ERR flash数据校验错误/FLASH_OK表示成功
 */

int writeIpInfo(IP_INFO_TYPE *ipInfo);

/*
 * 读取网口IP信息
 * IP_INFO_TYPE *ipInfo:写入的ip信息数据结构体
* return:FLASH_OP_ERR flash读写错误/FLASH_CHK_ERR flash数据校验错误/FLASH_OK表示成功
 */

int readIpInfo(IP_INFO_TYPE *ipInfo);

/*
 * 转换IP字符串为IP信息
 * char *ip IP字符串
 * int len  IP字符串长度
 * MY_IP_ADDR *ipAddr  IP信息
* return:FLASH_OP_ERR flash读写错误/FLASH_CHK_ERR flash数据校验错误/FLASH_OK表示成功
 */
int strToIp(char *ip, int len, MY_IP_ADDR *ipAddr);

/*
 * 转换MAC地址字符串为MAC信息数据
 * char *mac mac字符串
 * int len mac字符串长度
 * MY_MAC_ADDR *macAddr  mac信息
* return:FLASH_OP_ERR flash读写错误/FLASH_CHK_ERR flash数据校验错误/FLASH_OK表示成功
 */

int strToMac(char *mac, int len, MY_MAC_ADDR *macAddr);

/*
 *转换port地址字符串为port信息数据
 * char *port  port字符串
 * int len  port字符串长度
 * uint16_t *portInfo  port信息
* return:FLASH_OP_ERR flash读写错误/FLASH_CHK_ERR flash数据校验错误/FLASH_OK表示成功
 */
int strToPort(char *port, int len, uint16_t *portInfo);
int readCntInfo(void);
int writeCntInfo(uint32* count);


#ifdef cplusplus
}
#endif
#endif /* IPMB_H_ */


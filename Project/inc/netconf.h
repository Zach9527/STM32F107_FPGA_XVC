																			   /**
  ******************************************************************************
  * @file    netconf.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   This file contains all the functions prototypes for the netconf.c 
  *          file.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NETCONF_H
#define __NETCONF_H
#include "stm32f107.h"
#ifdef __cplusplus
 extern "C" {
#endif

typedef struct {
	uint8 addr_0;
	uint8 addr_1;
	uint8 addr_2;
	uint8 addr_3;
} MY_IP_ADDR;
//���ڱ���MAC�����Ϣ
typedef struct {

	uint8 mac_0;
	uint8 mac_1;
	uint8 mac_2;
	uint8 mac_3;
	uint8 mac_4;
	uint8 mac_5;
} MY_MAC_ADDR;

typedef struct {
	MY_IP_ADDR ip;
	MY_IP_ADDR mask;
	MY_IP_ADDR gw;
	MY_MAC_ADDR mac;
	uint16 serverPort;
} NET_CONFIG_TYPE;



extern const NET_CONFIG_TYPE defaultNetCfgInfo;
extern NET_CONFIG_TYPE netCfgInfo;
/* MAC address: MAC_ADDR0:MAC_ADDR1:MAC_ADDR2:MAC_ADDR3:MAC_ADDR4:MAC_ADDR5 */
#define MAC_0   netCfgInfo.mac.mac_0
#define MAC_1   netCfgInfo.mac.mac_1
#define MAC_2   netCfgInfo.mac.mac_2
#define MAC_3   netCfgInfo.mac.mac_3
#define MAC_4   netCfgInfo.mac.mac_4
#define MAC_5   netCfgInfo.mac.mac_5
 
/* static IP address: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0   netCfgInfo.ip.addr_0
#define IP_ADDR1   netCfgInfo.ip.addr_1
#define IP_ADDR2   netCfgInfo.ip.addr_2
#define IP_ADDR3   netCfgInfo.ip.addr_3
   
/* net mask */
#define NETMASK_ADDR0   netCfgInfo.mask.addr_0
#define NETMASK_ADDR1   netCfgInfo.mask.addr_1
#define NETMASK_ADDR2   netCfgInfo.mask.addr_2
#define NETMASK_ADDR3   netCfgInfo.mask.addr_3

/* gateway address */
#define GW_ADDR0   netCfgInfo.gw.addr_0
#define GW_ADDR1   netCfgInfo.gw.addr_1
#define GW_ADDR2   netCfgInfo.gw.addr_2
#define GW_ADDR3   netCfgInfo.gw.addr_3

//����TCP�˿�
#define TCP_Port  netCfgInfo.serverPort

/* Includes ------------------------------------------------------------------*/
void LwIP_Init(void);
void LwIP_Pkt_Handle(void);
void LwIP_Periodic_Handle(__IO uint32_t localtime);
void Eth_Link_ITHandler(void);


#ifdef __cplusplus
}
#endif

#endif /* __NETCONF_H */


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/


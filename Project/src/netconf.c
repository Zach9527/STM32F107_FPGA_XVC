/**
  ******************************************************************************
  * @file    netconf.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   Network connection configuration
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

/* Includes ------------------------------------------------------------------*/
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "main.h"
#include "netconf.h"
#include <stdio.h>
#include "stm32_eth.h"
#include "stm32f107.h"
#include "netif.h"

/* Private typedef -----------------------------------------------------------*/
#define LCD_DELAY             3000
#define KEY_DELAY 			  3000
#define LCD_TIMER_MSECS       250
#define MAX_DHCP_TRIES        4
#define SELECTED              1
#define NOT_SELECTED		  (!SELECTED)
#define CLIENTMAC6            2
//#define CLIENTMAC6            3
//#define CLIENTMAC6            4

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
struct netif netif;
__IO uint32_t TCPTimer = 0;
__IO uint32_t ARPTimer = 0;

#ifdef LWIP_DHCP
__IO uint32_t DHCPfineTimer = 0;
__IO uint32_t DHCPcoarseTimer = 0;
static uint32_t IPaddress = 0;
#endif

__IO uint32_t DisplayTimer = 0;
uint8_t LedToggle = 4;
uint8_t	Server = 0;
uint8_t EthInitStatus;          //��ʼ��״̬

//Ĭ�ϵ�IP��ַ��MAC��ַ��TCP�������˿� �������޸�
const NET_CONFIG_TYPE defaultNetCfgInfo = {
	{192, 168, 1, 78},//IP
	{255, 255, 255, 0},//mask
	{192, 168, 1, 1},//gateway
	{0x02, 0x0A, 0x0F, 0x0E, 0x0E, 0x06},//MAC
	888,

};
//IP��ַ��MAC��ַ��TCP�������˿� �����޸�
NET_CONFIG_TYPE netCfgInfo = {
	{192, 168, 1, 78},//IP
	{255, 255, 255, 0},//mask
	{192, 168, 1, 1},//gateway
	{0x02, 0x0A, 0x0F, 0x0E, 0x0E, 0x06},//MAC
	888,
};

/* Private function prototypes -----------------------------------------------*/
extern void client_init(void);
extern void server_init(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
void LwIP_Init(void)
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
  //uint8_t macaddress[6]={0x02,0x0A,0x0F,0x0E,0x0E,0x06};

  /* Initializes the dynamic memory heap defined by MEM_SIZE.*/
  mem_init();

  /* Initializes the memory pools defined by MEMP_NUM_x.*/
  memp_init();


#if LWIP_DHCP
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;
#else
  IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
  IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
  IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);

#endif

  //Set_MAC_Address(macaddress);//mac地址

  /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
            struct ip_addr *netmask, struct ip_addr *gw,
            void *state, err_t (* init)(struct netif *netif),
            err_t (* input)(struct pbuf *p, struct netif *netif))
    
   Adds your network interface to the netif_list. Allocate a struct
  netif and pass a pointer to this structure as the first argument.
  Give pointers to cleared ip_addr structures when using DHCP,
  or fill them with sane numbers otherwise. The state pointer may be NULL.

  The init function pointer must point to a initialization function for
  your ethernet netif interface. The following code illustrates it's use.*/
  netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

  /*  Registers the default network interface.*/
  netif_set_default(&netif);


#if LWIP_DHCP
  /*  Creates a new DHCP client for this interface on the first call.
  Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
  the predefined regular intervals after starting the client.
  You can peek in the netif->dhcp struct for the actual DHCP status.*/
  dhcp_start(&netif);
#endif

  /*  When the netif is fully configured this function must be called.*/
  netif_set_up(&netif);

}

/**
  * @brief  Called when a frame is received
  * @param  None
  * @retval None
  */
void LwIP_Pkt_Handle(void)
{
  /* Read a received packet from the Ethernet buffers and send it to the lwIP for handling */
  ethernetif_input(&netif);
}

/**
  * @brief  LwIP periodic tasks
  * @param  localtime the current LocalTime value
  * @retval None
  */
void LwIP_Periodic_Handle(__IO uint32_t localtime)
{

  /* TCP periodic process every 250 ms */
  if (localtime - TCPTimer >= TCP_TMR_INTERVAL)
  {
    TCPTimer =  localtime;
    tcp_tmr();
  }
  /* ARP periodic process every 5s */
  if (localtime - ARPTimer >= ARP_TMR_INTERVAL)
  {
    ARPTimer =  localtime;
    etharp_tmr();
    Eth_Link_ITHandler(); 
  }

#if LWIP_DHCP
  /* Fine DHCP periodic process every 500ms */
  if (localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
  {
    DHCPfineTimer =  localtime;
    dhcp_fine_tmr();
  }
  /* DHCP Coarse periodic process every 60s */
  if (localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
  {
    DHCPcoarseTimer =  localtime;
    dhcp_coarse_tmr();
  }
#endif

}
#define PHY_ADDRESS       0x01 /* Relative to STM3210C-EVAL Board */

void Eth_Link_ITHandler(void)
{
/* Check whether the link interrupt has occurred or not */
    if((ETH_ReadPHYRegister(PHY_ADDRESS, PHY_BSR) & PHY_Linked_Status) == 0x00)
    {
        uint16_t status = ETH_ReadPHYRegister(PHY_ADDRESS, PHY_BSR);
        if(status & (PHY_AutoNego_Complete | PHY_Linked_Status))
        {
            if(EthInitStatus != 1)
            {/*if(EthInitStatus != ETH_SUCCESS)
                /*Reinit PHY*/
                ETH_Reinit();
            }
            else
            {
                /*set link up for re link callbalk function*/
                netif_set_link_up(&netif);
            }
        }
        else
        {
            netif_set_link_down(&netif);
        }
    } 
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

/**
  ******************************************************************************
  * @file    stm32f107.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   STM32F107 hardware configuration
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
#include "stm32_eth.h"
#include "stm32f107.h"
#include "flash.h"
#include "main.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DP83848_PHY        /* Ethernet pins mapped on STM3210C-EVAL Board */
#define PHY_ADDRESS       0x01 /* Relative to STM3210C-EVAL Board */

#define MII_MODE          /* MII mode for STM3210C-EVAL Board (MB784) (check jumpers setting) */
//#define RMII_MODE       /* RMII mode for STM3210C-EVAL Board (MB784) (check jumpers setting) */

extern UART_RX_BUF_TYPE rxbuf;
extern uint8_t EthInitStatus;   

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void GPIO_Configuration(void);
void NVIC_Configuration(void);
void ADC_Configuration(void);
void Ethernet_Configuration(void);

void XVC_PHY_RST(void)
{
    int i = 0;
    GPIO_ResetBits(GPIOB, GPIO_Pin_15);
    for(i = 20; i != 0; i--)//延时10个时钟周期
    {
    }
    GPIO_SetBits(GPIOB,GPIO_Pin_15);
    Delay(1);
    GPIO_ResetBits(GPIOB, GPIO_Pin_15);
    Delay(1);
    GPIO_SetBits(GPIOB,GPIO_Pin_15);
}

/**
  * @brief  Setup STM32 system (clocks, Ethernet, GPIO, NVIC) and STM3210C-EVAL resources.
  * @param  None
  * @retval None
  */
void System_Setup(void)
{
  RCC_ClocksTypeDef RCC_Clocks;

  /* Setup STM32 clock, PLL and Flash configuration) */
  SystemInit();
  /* SystTick configuration: an interrupt every 1ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.SYSCLK_Frequency / 10000);

  /* Update the SysTick IRQ priority should be higher than the Ethernet IRQ */
  /* The Localtime should be updated during the Ethernet packets processing */
  NVIC_SetPriority (SysTick_IRQn, 1);  
  /* Enable USART3 clock */

  /* Enable ETHERNET clock  */

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx |
                        RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);
  /* Enable GPIOs and clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
                         RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO 
                         ,ENABLE);

  /* NVIC configuration */
  NVIC_Configuration();  
  /*GPIO_PartialRemap_USART3 */
    /* Configure the GPIO ports */
  GPIO_Configuration();
    /* XVC INIT*/
  XVC_PHY_RST();
  XVC_Init();
  /* USART3 INIT */
  Usart3Init();
  /* soft spi init */
  SPI_softInit();
  /* Initialize STM3210C-EVAL's LEDs */
  STM_EVAL_LEDInit(LED1);
  STM_EVAL_LEDInit(LED2);  
  /* Turn on leds available on STM3210X-EVAL */
  //STM_EVAL_LEDOn(LED1);
  //STM_EVAL_LEDOn(LED2);

  /* Configure the Ethernet peripheral */
  Ethernet_Configuration();
}

/**
  * @brief  Configures the Ethernet Interface
  * @param  None
  * @retval None
  */
void Ethernet_Configuration(void)
{
  

  /* MII/RMII Media interface selection ------------------------------------------*/
#ifdef MII_MODE /* Mode MII with STM3210C-EVAL  */
  GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_MII);

  /* Get HSE clock = 25MHz on PA8 pin (MCO) */
  RCC_MCOConfig(RCC_MCO_HSE);   //时钟问题，调试的时候注意跟一下RCC_MCO_HSE

#elif defined RMII_MODE  /* Mode RMII with STM3210C-EVAL */
  GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_RMII);

  /* Set PLL3 clock output to 50MHz (25MHz /5 *10 =50MHz) */
  RCC_PLL3Config(RCC_PLL3Mul_10);
  /* Enable PLL3 */
  RCC_PLL3Cmd(ENABLE);
  /* Wait till PLL3 is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_PLL3RDY) == RESET)
  {}

  /* Get PLL3 clock on PA8 pin (MCO) */
  RCC_MCOConfig(RCC_MCO_PLL3CLK);
#endif

  /* Reset ETHERNET on AHB Bus */
  ETH_DeInit();

  /* Software reset */
  ETH_SoftwareReset();

  /* Wait for software reset */
  while (ETH_GetSoftwareResetStatus() == SET);
  
  ETH_Reinit();
  /* Enable the Ethernet Rx Interrupt */
  ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);

}
void ETH_Reinit(void)
{
        ETH_InitTypeDef ETH_InitStructure;
      /* ETHERNET Configuration ------------------------------------------------------*/
      /* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
      ETH_StructInit(&ETH_InitStructure);
    
      /* Fill ETH_InitStructure parametrs */
      /*------------------------   MAC   -----------------------------------*/
      ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable  ;//开启网络自适应功能
      ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;//关闭反馈
      ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;//关闭重传功能
      ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;//关闭自动去除PDA/CRC功能
      ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable; //关闭接收所有的帧
      ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;//允许接收所有广播帧
      ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;//关闭混合模式的地址过滤
      ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;//对于组播地址使用完美地址过滤
      ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;//对单播地址使用完美地址过滤
#ifdef CHECKSUM_BY_HARDWARE//判断是否开启硬件校验，关闭软件校验
      ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;//开启ipv4/tcp/udp/icmp的帧校验和卸载
#endif
    
      /*------------------------   DMA   -----------------------------------*/  
      
      /* When we use the Checksum offload feature, we need to enable the Store and Forward mode: 
      the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum, 
      if the checksum is OK the DMA can handle the frame otherwise the frame is dropped */
      ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable; //开启丢弃tcp/ip错误帧
      ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;         //开启接收数据的存储转发模式
      ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;     //开启转发数据的存储转发模式
      ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;       //禁止转发错误帧
      ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;   //不转发过小的好帧
      ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;      //打开处理第二帧功能
      ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;      //开启DMA传输的地址对齐功能
      ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;    //开启固定突发功能
      ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;          
      ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;         
      ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
    
      /* Configure Ethernet */
      EthInitStatus = ETH_Init(&ETH_InitStructure, PHY_ADDRESS);

}


/**
  * @brief  Configures the different GPIO ports.
  * @param  None
  * @retval None
  */
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* ETHERNET pins configuration */
  /* AF Output Push Pull:
  - ETH_MII_MDIO / ETH_RMII_MDIO: PA2
  - ETH_MII_MDC / ETH_RMII_MDC: PC1
  - ETH_MII_TXD2: PC2
  - ETH_MII_TX_EN / ETH_RMII_TX_EN: PB11
  - ETH_MII_TXD0 / ETH_RMII_TXD0: PB12
  - ETH_MII_TXD1 / ETH_RMII_TXD1: PB13
  - ETH_MII_PPS_OUT / ETH_RMII_PPS_OUT: PB5
  - ETH_MII_TXD3: PB8 */

  /* Configure PA2 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure PC1, PC2 and PC3 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Configure PB5, PB8, PB11, PB12 and PB13 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_11 |
                                GPIO_Pin_12 | GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /**************************************************************/
  /*               For Remapped Ethernet pins                   */
  /*************************************************************/
  /* Input (Reset Value):
  - ETH_MII_CRS CRS: PA0
  - ETH_MII_RX_CLK / ETH_RMII_REF_CLK: PA1
  - ETH_MII_COL: PA3
  - ETH_MII_RX_DV / ETH_RMII_CRS_DV: PD8
  - ETH_MII_TX_CLK: PC3
  - ETH_MII_RXD0 / ETH_RMII_RXD0: PD9
  - ETH_MII_RXD1 / ETH_RMII_RXD1: PD10
  - ETH_MII_RXD2: PD11
  - ETH_MII_RXD3: PD12
  - ETH_MII_RX_ER: PB10 */

  /* ETHERNET pins remapp in STM3210C-EVAL board: RX_DV and RxD[3:0] */
  GPIO_PinRemapConfig(GPIO_Remap_ETH, ENABLE);

  /* Configure PA0, PA1 and PA3 as input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure PB10 as input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Configure PC3 as input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Configure PD8, PD9, PD10, PD11 and PD12 as input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOD, &GPIO_InitStructure); /**/


  /* MCO pin configuration------------------------------------------------- */
  /* Configure MCO (PA8) as alternate function push-pull */
  //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  //GPIO_Init(GPIOA, &GPIO_InitStructure);

  /*             software simulate spi gpio configure*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 |GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //模拟spi采用输出
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空
  GPIO_Init(GPIOD, &GPIO_InitStructure);



  //XVC_PHY_RST 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}


/**
  * @brief  Configures the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef   NVIC_InitStructure;

  /* Set the Vector Table base location at 0x08000000 */
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);

  /* 2 bit for pre-emption priority, 2 bits for subpriority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
  
  /* Enable the Ethernet global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);    
  
}

void delay_1us(void)
{
    int n = 72;
    while(n--);
}
void SPI_softInit(void)
{
    NSS_H;
    SCK_H;
    MOSI_L;
}
void SPI_WriteByte( uint8 write_dat )
{
    uint8 i;
    for( i = 0; i < 8; i++ )
    {
        if( (write_dat) & 0x80 )
        {
            GPIO_SetBits(SPI_MOSI_GPIO_PORT, SPI_MOSI_PIN) ;
        }
        else
        {
            GPIO_ResetBits(SPI_MOSI_GPIO_PORT, SPI_MOSI_PIN);
        }
        
        SCK_L; 
        write_dat = write_dat << 1;
        SCK_H; 
    }
}

uint8 SPI_ReadByte(void)
{
    uint8 read_dat = 0, i = 0;
    for(i = 0 ; i < 8; i++)
    {
        SCK_L;
        //delay_1us();
        read_dat <<= 1;
        if(GPIO_ReadInputDataBit(SPI_MISO_GPIO_PORT, SPI_MISO_PIN))
        {
            read_dat |= 1;
        }
        SCK_H;
        //delay_1us();
    }
    return read_dat;
}

void XVC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    //数据线
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5\
                                  | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure); /**/
    //PD0：片选，PD1：读写控制
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 ;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    //地址线
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13\
                                  | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure); /**/
    XVC_CS_H;//拉高片选
}

void Usart3Init(void)
{
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    GPIO_PinRemapConfig(GPIO_PartialRemap_USART3,ENABLE);
      //USART3_TX   GPIOC.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PC10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_Init(GPIOC, &GPIO_InitStructure);

  //USART3_RX 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PC11
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
    USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure); 
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART3, ENABLE);
}





ERR_STA writeUartBuf( uint8 *buf, uint8_t len)
{
	uint32  i, offset;
	if(UART_RX_BUF_SIZE < rxbuf.rx_cnt + len)
	{
		return STA_ERR;
	}
	
	offset = rxbuf.rx_cnt;
	for(i = 0; i < len; i++)
	{
		rxbuf.buf[offset + i] = buf[i];
	}
	
	rxbuf.rx_cnt += len;
	return STA_OK;
}

void setUartBufFlag()
{
	rxbuf.valid = STA_OK;
}
static uint8 readUartBuf( uint8 *buf)
{
	uint8 i;
	uint8 cnt;
	
	if(STA_OK == rxbuf.valid)
	{
		cnt = rxbuf.rx_cnt;
		for(i = 0; i < cnt; i++)
		{
			buf[i] = rxbuf.buf[i];
		}
		
		rxbuf.rx_cnt = 0;
		rxbuf.valid = STA_ERR;
	}
	else
	{
		cnt = 0;
	}
	return cnt;
}

uint8 serialRecv( uint8 *buf)
{
	uint8 size;
	
	size = readUartBuf(buf);
	
	return size;
}
uint8 serialSend( uint8 *buf, uint8 len)
{
	uint8 i;
	for(i = 0; i < len; i++)
	{
        USART_SendData(USART3, buf[i]);
        while(RESET == USART_GetFlagStatus(USART3, USART_FLAG_TXE));//判断是否发送成功
	}
	
	return len;
} 
void syncIpInfo(void *flashIpInfop, void *netConfp)
{
	IP_INFO_TYPE *flashIpInfo = (IP_INFO_TYPE *)flashIpInfop;
	NET_CONFIG_TYPE *netConf = (NET_CONFIG_TYPE *)netConfp;
	if(FLASH_OK != readIpInfo(flashIpInfo))
	{
		printf("\r\nreadIpInfo fail,setup default ip infomation\r\n");
		setDefaultIpInfo();
		return;
	}
	if(DATA_VALID_VAL == flashIpInfo->ipValid)
	{
	    memcpy(&netConf->ip,flashIpInfo->ip,4);
	}
	if(DATA_VALID_VAL == flashIpInfo->macValid)
	{
        memcpy(&netConf->mac,flashIpInfo->mac,6);
	}
	if(DATA_VALID_VAL == flashIpInfo->portValid)
	{
        memcpy(&netConf->serverPort,flashIpInfo->port,2);
	}
}

extern const NET_CONFIG_TYPE defaultNetCfgInfo;
extern IP_INFO_TYPE ipInfoFlash;

void setDefaultIpInfo(void)
{
	char ipInfoStr[IP_INFO_MAX_SIZE];
	
	memset(&ipInfoFlash, 0xff, sizeof(ipInfoFlash));
	//IP
	memcpy(ipInfoFlash.ip, &defaultNetCfgInfo.ip,4);
	ipInfoFlash.ipValid = DATA_VALID_VAL;
	//MAC
	memcpy(ipInfoFlash.mac, &defaultNetCfgInfo.mac,6);
	ipInfoFlash.macValid = DATA_VALID_VAL;
	//PORT
	memcpy(ipInfoFlash.port, &defaultNetCfgInfo.serverPort,2);
	ipInfoFlash.portValid = DATA_VALID_VAL;
}
/*!
    \brief    清楚回车或者换行符，将会车或者换行替换为0
    \param  	void
    \retval   void
*/
void replaceEnter(char *str)
{
	int i = 0;
	while(str[i])
	{
		if( ('\r' == str[i])\
				|| ('\n' == str[i]) )
		{
			str[i] = 0;
		}
		i++;
	}
}


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

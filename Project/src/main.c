/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   Main program body
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
#include "netconf.h"
#include "main.h"
#include "flash.h"
#include "lwip/tcp.h"
#include "tcpserver.h"
#include "stm32_eth.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SYSTEMTICK_PERIOD_MS  10
#define    __VERSION         "V1.00"

typedef void SERIAL_IDLE_FUNC(uint8 *);

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
uint32_t timingdelay;

int earseFlag = 0;
extern ICD_COMMON_RSP_T commonRsp ;
extern struct tcp_pcb* earsePcb;

/* Private function prototypes -----------------------------------------------*/
void System_Periodic_Handle(void);

/* Private functions ---------------------------------------------------------*/


int fputc(int ch, FILE *f)
{
	USART_SendData(USART3, (uint8_t) ch);

	while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
   
    return ch;
}

 uint32 flashCheckOut(void)
 {
     uint8 dat[16],i= 0;
     Flash_ReadSomeByte(dat,0,4);
     Flash_ReadSomeByte(dat+4,0x1ffffff,4);
     for(i = 0 ;i < 8 ;i++)
     {
         if(dat[i] != 0xff)
         {
             return 0;
         }
     }
     return 1;
 }

 void printf_version(void)
{
  printf("\nversion:%s\r\n", __VERSION);
  printf("%s/%s\r\n", __DATE__, __TIME__);
}
//uint8 ssbuf[128] = {0};
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
    /* Setup STM32 system (clocks, Ethernet, USART,GPIO, NVIC)  */
    System_Setup();
    syncIpInfo(&ipInfoFlash, &netCfgInfo);
    printf_version();
    //read flash ip
    /* Initilaize the LwIP satck */
    LwIP_Init();
    /* Setup TCP Server */
    Tcp_Server_Init();
    /* Infinite loop */
    //fpgaStateGet(ssbuf);
    while (1)
    {
    /* Periodic tasks */
       System_Periodic_Handle();
       if(earseFlag)
       {
            if((Flash_ReadSR()&0X01) != 0X01)//spi flash 擦除完成响应
            {
                earseFlag = 0;
                commonRsp.head = TCPHEADFRAME;
                commonRsp.cmd = ICD_CMD_EARSE;
                commonRsp.size = 4;
                commonRsp.opRet = flashCheckOut();
                commonRsp.tail = TCPTAILFRAME;
                commonRsp.checkSum = check_sum((uint8*)(&commonRsp),sizeof(ICD_COMMON_RSP_T)-4);
                tcp_write(earsePcb,(uint8*)(&commonRsp),sizeof(ICD_COMMON_RSP_T),1);
            }
       }
    }
}

/**
  * @brief  Inserts a delay time.
  * @param  nCount: number of 10ms periods to wait for.
  * @retval None
  */
void Delay(uint32_t nCount)
{
  /* Capture the current local time */
  timingdelay = LocalTime + nCount;  

  /* wait until the desired delay finish */  
  while(timingdelay > LocalTime)
  {     
  }
}

/**
  * @brief  Updates the system local time
  * @param  None
  * @retval None
  */
void Time_Update(void)
{
  LocalTime += SYSTEMTICK_PERIOD_MS;
}

/**
  * @brief  Handles the periodic tasks of the system
  * @param  None
  * @retval None
  */
void System_Periodic_Handle(void)
{
  /* LwIP periodic services are done here */
  LwIP_Periodic_Handle(LocalTime);
}







/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

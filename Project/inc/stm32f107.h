/**
  ******************************************************************************
  * @file    stm32f107.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   This file contains all the functions prototypes for the STM32F107 
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
#ifndef __STM32F107_H
#define __STM32F107_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "stm32_eval.h"
#include <string.h>
#include <stdio.h>
#define XVC_CS_PIN GPIO_Pin_0
#define XVC_RW_PIN GPIO_Pin_1

#define SPI_SCK_PIN                     GPIO_Pin_2
#define SPI_SCK_GPIO_PORT               GPIOD
#define SPI_NSS_PIN                     GPIO_Pin_3
#define SPI_NSS_GPIO_PORT               GPIOD
#define SPI_MOSI_PIN                    GPIO_Pin_4
#define SPI_MOSI_GPIO_PORT              GPIOD
#define SPI_MISO_PIN                    GPIO_Pin_5
#define SPI_MISO_GPIO_PORT              GPIOD
#if 0
 #define SPI_SCK_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPI_MISO_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPI_MOSI_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPI_NSS_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOD_CLK_ENABLE()
#endif
#define MOSI_H  GPIO_SetBits(SPI_MOSI_GPIO_PORT, SPI_MOSI_PIN)  
#define MOSI_L  GPIO_ResetBits(SPI_MOSI_GPIO_PORT, SPI_MOSI_PIN)  
#define SCK_H   GPIO_SetBits(SPI_SCK_GPIO_PORT, SPI_SCK_PIN)  
#define SCK_L   GPIO_ResetBits(SPI_SCK_GPIO_PORT, SPI_SCK_PIN)  
#define MISO    GPIO_ReadInputDataBit(SPI_MISO_GPIO_PORT, SPI_MISO_PIN) 
#define NSS_H   GPIO_SetBits(SPI_NSS_GPIO_PORT, SPI_NSS_PIN)  
#define NSS_L   GPIO_ResetBits(SPI_NSS_GPIO_PORT, SPI_NSS_PIN) 

#define XVC_CS_H GPIO_SetBits(GPIOD, GPIO_Pin_0)
#define XVC_CS_L GPIO_ResetBits(GPIOD, GPIO_Pin_0)
#define XVC_R GPIO_SetBits(GPIOD, GPIO_Pin_1)
#define XVC_W GPIO_ResetBits(GPIOD, GPIO_Pin_1)

#define NULL ((void *)0)
#define STA_OK 0
#define STA_ERR 1
#define UART_RX_BUF_SIZE 64
#define IP_INFO_MAX_SIZE 128

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef uint16 ERR_STA;

typedef struct UART_RX_BUF_TYPE_t
{
	uint8 buf[UART_RX_BUF_SIZE]; //接收缓冲
	uint8 rx_cnt;//接收数据的个数
	ERR_STA valid;//是否接收到完整数据包
}UART_RX_BUF_TYPE;

void SPI_WriteByte( uint8 write_dat );
uint8 SPI_ReadByte(void);
void SPI_softInit(void);
void System_Setup(void);
void XVC_Init(void);
ERR_STA writeUartBuf( uint8 *buf, uint8_t len);
void Usart3Init(void);
void setUartBufFlag(void);
static uint8 readUartBuf( uint8 *buf);
uint8 serialRecv( uint8 *buf);
void syncIpInfo(void *flashIpInfop, void *netConfp);
uint8 serialSend( uint8 *buf, uint8 len);
void setDefaultIpInfo(void);
void XVC_PHY_RST(void);
void replaceEnter(char *str);
void ETH_Reinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F10F107_H */


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

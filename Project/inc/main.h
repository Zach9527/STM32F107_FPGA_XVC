																			   /**
  ******************************************************************************
  * @file    main.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   This file contains all the functions prototypes for the main.c 
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f107.h"
#include "drvflash.h"
#define PP_HTONS(x) ((((x) & 0x00ffUL) << 8) | (((x) & 0xff00UL) >> 8))
#define PP_NTOHS(x) PP_HTONS(x)
#define PP_HTONL(x) ((((x) & 0x000000ffUL) << 24) | \
                     (((x) & 0x0000ff00UL) <<  8) | \
                     (((x) & 0x00ff0000UL) >>  8) | \
                     (((x) & 0xff000000UL) >> 24))
#define PP_NTOHL(x) PP_HTONL(x)
typedef struct
{
    uint8 yy_h;
    uint8 yy_l;
    uint8 mm;
    uint8 dd;
    uint8 hh;
    uint8 ss;
}compileTime;
typedef struct 
{
    uint8 fpga1;//0x10
    uint8 fpga2;//0x11
    uint8 fpga3;//0x12
}FpgaState;

typedef struct
{
    uint8 ctrl_h;//0x14
    uint8 ctrl_l;//0x15
    uint8 output_h;//0x16
    uint8 output_l;//0x17
}powerCtl;

typedef struct
{
    //内核电压电源温度
    uint8 LTM4630_0_h;//0x20
    uint8 LTM4630_0_l;//0x21
    uint8 LTM4630_1_h;//0x22
    uint8 LTM4630_1_l;//0x23
    uint8 LTM4630_2_h;//0x24
    uint8 LTM4630_2_l;//0x25
    //正面温度
    uint8 Temp_TOP_h;//0x26
    uint8 Temp_TOP_l;//0x27
    //背面温度
    uint8 Temp_BOT_h;//0x28
    uint8 Temp_BOT_l;//0x29
    //fpga
    uint8 Temp_FPGA1_h;//0x2a
    uint8 Temp_FPGA1_l;//0x2b
    uint8 Temp_FPGA2_h;//0x2c
    uint8 Temp_FPGA2_l;//0x2d
    uint8 Temp_FPGA3_h;//0x2e
    uint8 Temp_FPGA3_l;//0x2f
}Temperature;
typedef struct
{
    //+1V0_FPGA1电压  
    uint8 FPGA1_1V0_h;//0x30
    uint8 FPGA1_1V0_l;//0x31
    //+1V0_FPGA2_3电压
    uint8 FPGA2_3_1V0_h;//0x32
    uint8 FPGA2_3_1V0_l;//0x33
    //+1V0_FPGA1_MGTAVCC电压
    uint8 FPGA1_1V0_MGTA_h;//0x34
    uint8 FPGA1_1V0_MGTA_l;//0x35
    //+1V0_FPGA2_3_MGTAVCC电压
    uint8 FPGA2_3_1V0_MGTA_h;//0x36
    uint8 FPGA2_3_1V0_MGTA_l;//0x37
    //+1V2_FPGA1_MGTAVTT电压
    uint8 FPGA1_1V2_MGTA_h;//0x38
    uint8 FPGA1_1V2_MGTA_l;//0x39
    //+1V2_FPGA2_3_MGTAVTT电压
    uint8 FPGA2_3_1V2_MGTA_h;//0x3a
    uint8 FPGA2_3_1V2_MGTA_l;//0x3b
    //+1V8_FPGA1电压
    uint8 FPGA1_1V8_h;//0x3c
    uint8 FPGA1_1V8_l;//0x3d
    //+1V8电压
    uint8 _1V8_h;//0x3e
    uint8 _1V8_l;//0x3f
}Voltage;

typedef struct
{
    uint8 clkChoose;
    uint8 clkConfig;
    uint8 setupCtrl;
}ClockInfo;

/* Exported function prototypes ----------------------------------------------*/
void Time_Update(void);
void Delay(uint32_t nCount);




#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/


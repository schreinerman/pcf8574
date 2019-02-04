/**
 *******************************************************************************
 ** Created by Manuel Schreiner
 **
 ** Copyright © 2019 io-expert.com. All rights reserved.
 **
 ** 1. Redistributions of source code must retain the above copyright notice,
 **    this condition and the following disclaimer.
 **
 ** This software is provided by the copyright holder and contributors "AS IS"
 ** and any warranties related to this software are DISCLAIMED.
 ** The copyright owner or contributors be NOT LIABLE for any damages caused
 ** by use of this software.

 *******************************************************************************
 */

/**
 *******************************************************************************
 **\file pcf8574.c
 **
 ** PCF8574 I2C port expander driver
 ** A detailed description is available at
 ** @link Pcf8574Group file description @endlink
 **
 ** History:
 ** - 2019-2-3  1.00  Manuel Schreiner
 *******************************************************************************
 */

#if !defined(__PCF8574_H__)
#define __PCF8574_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 *******************************************************************************
 ** \defgroup Pcf8574Group PCF8574 I2C port expander driver
 **
 ** Provided functions of Pcf8574:
 **
 **
 *******************************************************************************
 */

//@{

/**
 *******************************************************************************
 ** \page pcf8574_module_includes Required includes in main application
 ** \brief Following includes are required
 ** @code
 ** #include "pcf8574.h"
 ** @endcode
 **
 *******************************************************************************
 */

/**
 *******************************************************************************
 ** \page pcf8574_module_example Example of PCF8574 usage
 ** \brief Following code gives some short example
 ** @code
 ** #include "pcf8574.h"
 **
 ** int I2CWrite(void* pHandle, uint32_t u32Address, uint8_t* pu8Data, uint32_t u32Len)
 ** {
 **     HAL_I2cWrite(pHandle,u32Address,pu8Data,u32Len);
 ** }
 ** 
 ** int I2CRead(void* pHandle, uint32_t u32Address, uint8_t* pu8Data, uint32_t u32Len)
 ** {
 **     HAL_I2cRead(pHandle,u32Address,pu8Data,u32Len);
 ** }
 **
 ** stc_pcf8574_list_item_t stcPcf8574ListItm;
 ** stc_pcf8574_handle_t stcPcf8574 =
 ** {
 **     I2C0,        //pI2cHandle;
 **     (0x70>>1),   //u32Address;
 **     0,           //uint8_t u8CurrentValues;
 **     I2CRead,     //pfnRead;
 **     I2CWrite     //pfnWrite;
 ** };
 ** 
 ** void GpioCallback(void)
 ** {
 **     Pcf8574_ExtIrqHandle();
 ** }
 **
 ** void Pcf8574Gpio0FallingEdgeCallback(void* pHandle, uint8_t u8Pin)
 ** {
 **     //handle callback of PCF8574, GPIO0, falling edge
 ** }
 **
 ** int main(void)
 ** {
 **     uint8_t u8Data = 0;
 **     Board_Init();        //Init I2C
 **
 **
 **     PCF8575_ZERO_CALLBACKS(&stcPcf8574);          //zero initialize all callbacks
 **     Pcf8574_Init(&stcPcf8574,&stcPcf8574ListItm); //init PCF8574
 **
 **     //Optional: Register pin connected to INT as falling edge interrupt at MCUs HAL
 **     //          with GpioCallback as callback
 **     HAL_RegisterGpioIrq(PCF8574_INT_PIN, FallingEdge, GpioCallback);   
 **
 **     //Optional: Register falling edge callback for GPIO0 of PCF8574
 **     //          requires calling Pcf8574_ExtIrqHandle() in GpioCallback
 **     Pcf8574_InitCallback(&stcPcf8574, 0, Pcf8574FallingEdge, Pcf8574Gpio0FallingEdgeCallback);
 **
 **     Pcf8574_Write(&stcPcf8574,0xFF);              //write 0xFF to all GPIOs
 **     u8Data = Pcf8574_Read(&stcPcf8574);           //read GPIOs into u8Data
 **     while(1)
 **     {
 **         //main applicaton
 **     }
 ** }
 ** @endcode
 **
 *******************************************************************************
 */

/**
 *******************************************************************************
 ** (Global) Include files
 *******************************************************************************
 */

#include "base_types.h"
#include "string.h"     //used for memset

/**
 *******************************************************************************
 ** Global pre-processor symbols/macros ('#define') 
 *******************************************************************************
 */

 #define PCF8574_CONFIG_INIT(x) memset(&(x),0,sizeof((x)))
 #define PCF8575_ZERO_CALLBACKS(pHandle) memset(&((pHandle)->astcCallbacks[0]),0,sizeof(((pHandle)->astcCallbacks)))

/**
 *******************************************************************************
 ** Global type definitions ('typedef') 
 *******************************************************************************
 */

typedef struct stc_pcf8574_list_item stc_pcf8574_list_item_t;

struct stc_pcf8574_list_item
{
  void* Handle;
  struct stc_pcf8574_list_item* pstcNext;
};

/**
 ** \brief I2C read function
 */
typedef int (*pfn_pcf8574_i2c_read_t)  (void* pHandle, uint32_t u32Address, uint8_t* pu8Data, uint32_t u32Len); 

/**
 ** \brief I2C write function
 */
typedef int (*pfn_pcf8574_i2c_write_t)  (void* pHandle, uint32_t u32Address, uint8_t* pu8Data, uint32_t u32Len); 

/**
 ** \brief Pin change callback
 */
typedef void (*pfn_pcf8574_callback_t)  (void* pHandle, uint8_t u8Pin); 

/**
 ** \brief IRQ trigger type enumeration
 */
typedef enum en_pcf8574_irq_trigger
{
   Pcf8574IrqNone = 0,
   Pcf8574RisingEdge,
   Pcf8574FallingEdge,
   Pcf8574RisingFallingEdge
} en_pcf8574_irq_trigger_t;

/**
 ** \brief IRQ struct
 */
typedef struct stc_pcf8574_irq
{
    en_pcf8574_irq_trigger_t enType;
    pfn_pcf8574_callback_t pfnCallback;
} stc_pcf8574_irq_t;

/**
 ** \brief PCF8574 handle
 */
typedef struct stc_pcf8574_handle
{
    void* pI2cHandle;
    uint32_t u32Address;
    uint8_t u8CurrentValues;
    pfn_pcf8574_i2c_read_t pfnRead;
    pfn_pcf8574_i2c_write_t pfnWrite;
    stc_pcf8574_irq_t astcCallbacks[8];
} stc_pcf8574_handle_t;




/**
 *******************************************************************************
 ** Global variable declarations ('extern', definition in C source)
 *******************************************************************************
 */

/**
 *******************************************************************************
 ** Global function prototypes ('extern', definition in C source) 
 *******************************************************************************
 */

en_result_t Pcf8574_Init(stc_pcf8574_handle_t* pstcHandle, stc_pcf8574_list_item_t* pstcListItemOut);
en_result_t Pcf8574_Deinit(stc_pcf8574_handle_t* pstcHandle, stc_pcf8574_list_item_t* pstcListItemOut);
en_result_t Pcf8574_InitCallback(stc_pcf8574_handle_t* pstcHandle, uint8_t u8Bit, en_pcf8574_irq_trigger_t enType, pfn_pcf8574_callback_t pfnCallback);
en_result_t Pcf8574_DeinitCallback(stc_pcf8574_handle_t* pstcHandle, uint8_t u8Bit);
uint8_t Pcf8574_Read(stc_pcf8574_handle_t* pstcHandle);
void Pcf8574_Write(stc_pcf8574_handle_t* pstcHandle, uint8_t u8Value);
void Pcf8574_ExecuteIrqHandle(stc_pcf8574_handle_t* pHandle);
void Pcf8574_ExtIrqHandle(void);

//@} // Pcf8574Group

#ifdef __cplusplus
}
#endif

#endif /* __PCF8574_H__ */

/**
 *******************************************************************************
 ** EOF (not truncated)
 *******************************************************************************
 */


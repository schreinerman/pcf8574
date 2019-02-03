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
 ** (Global) Include files
 *******************************************************************************
 */

#include "base_types.h"

/**
 *******************************************************************************
 ** Global pre-processor symbols/macros ('#define') 
 *******************************************************************************
 */

 #define PCF8574_CONFIG_INIT(x) memset(&(x),0,sizeof((x)))

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
 ** \brief IRQ callback
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
uint8_t Pcf8574_Read(stc_pcf8574_handle_t* pstcHandle);
void Pcf8574_Write(stc_pcf8574_handle_t* pstcHandle, uint8_t u8Value);
void ExecuteIrqHandle(stc_pcf8574_handle_t* pHandle);
void ExtIrqHandle(void);

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


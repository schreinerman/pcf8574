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

#define __PCF8574_C__

/**
 *******************************************************************************
 ** Include files
 *******************************************************************************
 */


#include "base_types.h"
#include "pcf8574.h"

/**
 *******************************************************************************
 ** Local pre-processor symbols/macros ('#define') 
 *******************************************************************************
 */

/**
 *******************************************************************************
 ** Global variable definitions (declared in header file with 'extern') 
 *******************************************************************************
 */

/**
 *******************************************************************************
 ** Local type definitions ('typedef') 
 *******************************************************************************
 */

/**
 *******************************************************************************
 ** Local variable definitions ('static') 
 *******************************************************************************
 */

static stc_pcf8574_list_item_t* pstcListRoot = NULL;

/**
 *******************************************************************************
 ** Local function prototypes ('static') 
 *******************************************************************************
 */

/**
 *******************************************************************************
 ** Function implementation - global ('extern') and local ('static') 
 *******************************************************************************
 */


/**
 ** \brief Add item to the linked list
 **
 ** \param pstcListItem Pointer of list item to add
 **
 */
static void ListItemAdd(stc_pcf8574_list_item_t* pstcListItem)
{
    stc_pcf8574_list_item_t* pstcCurrent = pstcListRoot;
    if (pstcListRoot == NULL) 
    {
        pstcListRoot = pstcListItem;
        pstcListRoot->pstcNext = NULL;
        return;
    }
    while(pstcCurrent->pstcNext != NULL)
    {
        pstcCurrent = pstcCurrent->pstcNext;
    }
    pstcCurrent->pstcNext = pstcListItem;
    pstcListItem->pstcNext = NULL;
}

/**
 ** \brief Remove item to the linked list
 **
 ** \param pstcListItem Pointer of list item to remove
 **
 */
static void ListItemRemove(stc_pcf8574_list_item_t* pstcListItem)
{
    stc_pcf8574_list_item_t* pstcCurrent = pstcListRoot;
    stc_pcf8574_list_item_t* pstcLast = NULL;
    if (pstcListItem == pstcListRoot)
    {
        pstcListRoot = pstcListItem->pstcNext;
        pstcListItem->pstcNext = NULL;
        return;
    }
    while(pstcCurrent != pstcListItem)
    {
        pstcLast = pstcCurrent;
        pstcCurrent = pstcCurrent->pstcNext;
        if (pstcCurrent == NULL) return;
    }
    if (pstcLast != NULL)
    {
        pstcLast->pstcNext = pstcCurrent->pstcNext;
        pstcListItem->pstcNext = NULL;
    }
}


/**
 ** \brief Init PCF8574 handle
 **
 ** \param pstcHandle Pointer of handle
 **
 ** \param pstcListItemOut Pointer to optional list item to add (will be configured automatically)
 **
 ** \returns Ok on success
 */
en_result_t Pcf8574_Init(stc_pcf8574_handle_t* pstcHandle, stc_pcf8574_list_item_t* pstcListItemOut)
{
    if (pstcHandle == NULL)
    {
        return ErrorUninitialized;
    }
    if (pstcListItemOut != NULL)
    {
        pstcListItemOut->Handle = pstcHandle;
        ListItemAdd(pstcListItemOut);
    }
    pstcHandle->pfnRead(pstcHandle->pI2cHandle,pstcHandle->u32Address,&pstcHandle->u8CurrentValues,1);
    return Ok;
}

/**
 ** \brief Deinit PCF8574 handle
 **
 ** \param pstcHandle Pointer of handle
 **
 ** \param pstcListItemOut Pointer to optional list item to add (will be configured automatically)
 **
 ** \returns Ok on success
 */
en_result_t Pcf8574_Deinit(stc_pcf8574_handle_t* pstcHandle, stc_pcf8574_list_item_t* pstcListItemOut)
{
    if (pstcHandle == NULL)
    {
        return ErrorUninitialized;
    }
    if (pstcListItemOut != NULL)
    {
        pstcListItemOut->Handle = pstcHandle;
        ListItemRemove(pstcListItemOut);
    }
    return Ok;
}

/**
 ** \brief Read from PCF8574 handle
 **
 ** \param pstcHandle Pointer of handle
 **
 ** \returns 8-bit (byte) with the read GPIOs
 */
uint8_t Pcf8574_Read(stc_pcf8574_handle_t* pstcHandle)
{
    uint8_t u8Tmp;
    if (pstcHandle == NULL)
    {
        return 0;
    }
    pstcHandle->pfnRead(pstcHandle->pI2cHandle,pstcHandle->u32Address,&u8Tmp,1);
    pstcHandle->u8CurrentValues = u8Tmp;
    return u8Tmp;
}

/**
 ** \brief Write to PCF8574 handle
 **
 ** \param pstcHandle Pointer of handle
 **
 ** \param u8Value Write 8-bit (byte) to GPIOs
 */
void Pcf8574_Write(stc_pcf8574_handle_t* pstcHandle, uint8_t u8Value)
{
    if (pstcHandle == NULL)
    {
        return;
    }
    pstcHandle->pfnWrite(pstcHandle->pI2cHandle,pstcHandle->u32Address,&u8Value,1);
}

/**
 ** \brief Execute IRQ handling caused by INT pin for a specific device
 **
 ** \param pstcHandle Handle
 */
void ExecuteIrqHandle(stc_pcf8574_handle_t* pstcHandle)
{
    uint8_t u8Tmp;
    uint8_t u8Changes;
    int i;
    if (pstcHandle == NULL)
    {
        return;
    }
    pstcHandle->pfnRead(pstcHandle->pI2cHandle,pstcHandle->u32Address,&u8Tmp,1);
    u8Changes = pstcHandle->u8CurrentValues ^ u8Tmp;
    pstcHandle->u8CurrentValues = u8Tmp;
    for(i = 0;i < 8;i++)
    {
        if (((1 << i) & u8Changes) != 0)
        {
            if (pstcHandle->astcCallbacks[i].pfnCallback != NULL)
            {
                if (pstcHandle->astcCallbacks[i].enType == Pcf8574RisingFallingEdge)
                {
                    pstcHandle->astcCallbacks[i].pfnCallback(pstcHandle,i);
                } else if ((((1 << i) & u8Changes) != 0) && 
                    (((1 << i) & u8Tmp) != 0) &&
                    (pstcHandle->astcCallbacks[i].enType == Pcf8574RisingEdge))
                {
                    pstcHandle->astcCallbacks[i].pfnCallback(pstcHandle,i);
                } else if ((((1 << i) & u8Changes) != 0) && 
                    (((1 << i) & u8Tmp) == 0) &&
                    (pstcHandle->astcCallbacks[i].enType == Pcf8574FallingEdge))
                {
                    pstcHandle->astcCallbacks[i].pfnCallback(pstcHandle,i);
                }
            }
        }
    }
}

/**
 ** \brief Execute IRQ handling caused by INT pin for all devices in the list
 */
void ExtIrqHandle(void)
{
    stc_pcf8574_list_item_t* pstcCurrent = pstcListRoot;
    if (pstcListRoot == NULL)
    while(pstcCurrent->pstcNext != NULL)
    {
        ExecuteIrqHandle(pstcCurrent->Handle);
        pstcCurrent = pstcCurrent->pstcNext;
    }
}

/**
 *******************************************************************************
 ** EOF (not truncated)
 *******************************************************************************
 */


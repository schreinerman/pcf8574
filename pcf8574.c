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
 ** - 2019-2-3   1.00  Manuel Schreiner First version
 ** - 2019-3-25  1.10  Manuel Schreiner Added rotady endcoder support
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

static stc_pcf8574_list_item_t* pstcPcf8574ListRoot = NULL;
static volatile boolean_t bLock = FALSE;
static volatile boolean_t bHandleIrq = FALSE;

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
    stc_pcf8574_list_item_t* pstcCurrent = pstcPcf8574ListRoot;
    if (pstcPcf8574ListRoot == NULL) 
    {
        pstcPcf8574ListRoot = pstcListItem;
        pstcPcf8574ListRoot->Next = NULL;
        return;
    }
    while(pstcCurrent->Next != NULL)
    {
        pstcCurrent = pstcCurrent->Next;
    }
    pstcCurrent->Next = pstcListItem;
    pstcListItem->Next = NULL;
}

/**
 ** \brief Remove item to the linked list
 **
 ** \param pstcListItem Pointer of list item to remove
 **
 */
static void ListItemRemove(stc_pcf8574_list_item_t* pstcListItem)
{
    stc_pcf8574_list_item_t* pstcCurrent = pstcPcf8574ListRoot;
    stc_pcf8574_list_item_t* pstcLast = NULL;
    if (pstcListItem == pstcPcf8574ListRoot)
    {
        pstcPcf8574ListRoot = pstcListItem->Next;
        pstcListItem->Next = NULL;
        return;
    }
    while(pstcCurrent != pstcListItem)
    {
        pstcLast = pstcCurrent;
        pstcCurrent = pstcCurrent->Next;
        if (pstcCurrent == NULL) return;
    }
    if (pstcLast != NULL)
    {
        pstcLast->Next = pstcCurrent->Next;
        pstcListItem->Next = NULL;
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
        pstcListItemOut->enType = Pcf8574ListTypeNone;
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
 ** \brief Init callback for a specific GPIO
 **
 ** \param pstcHandle Pointer of handle
 **
 ** \param u8Bit Bit to address  between 0..7
 **
 ** \param enType IRQ type, can be Pcf8574RisingEdge, Pcf8574FallingEdge, Pcf8574RisingFallingEdge
 **
 ** \param pfnCallback Callback of type pfn_pcf8574_callback_t, example: void Pcf8574Callback(void* pHandle, uint8_t u8Pin);
 **
 ** \returns Ok on success
 */
en_result_t Pcf8574_InitCallback(stc_pcf8574_handle_t* pstcHandle, uint8_t u8Bit, en_pcf8574_irq_trigger_t enType, pfn_pcf8574_callback_t pfnCallback)
{
    if (pstcHandle == NULL)
    {
        return ErrorUninitialized;
    }
    if (pfnCallback == NULL)
    {
        return ErrorUninitialized;
    }
    if (u8Bit > 7)
    {
        return ErrorInvalidParameter;
    }
    pstcHandle->astcCallbacks[u8Bit].enType = enType;
    pstcHandle->astcCallbacks[u8Bit].pfnCallback = pfnCallback;
    return Ok;
}

/**
 ** \brief Deinit callback for a specific GPIO
 **
 ** \param pstcHandle Pointer of handle
 **
 ** \param u8Bit Bit to address  between 0..7
 **
 ** \returns Ok on success
 */
en_result_t Pcf8574_DeinitCallback(stc_pcf8574_handle_t* pstcHandle, uint8_t u8Bit)
{
    if (pstcHandle == NULL)
    {
        return ErrorUninitialized;
    }
    if (u8Bit > 7)
    {
        return ErrorInvalidParameter;
    }
    pstcHandle->astcCallbacks[u8Bit].enType = Pcf8574IrqNone;
    pstcHandle->astcCallbacks[u8Bit].pfnCallback = NULL;
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
 ** \brief Init PCF8574 handle
 **
 ** \param pstcHandle Pointer of handle
 **
 ** \param pstcListItemOut Pointer to optional list item to add (will be configured automatically)
 **
 ** \returns Ok on success
 */
en_result_t Pcf8574_InitRotaryEncoder(stc_pcf8574_rotaryencoder_t* pstcHandle, stc_pcf8574_list_item_t* pstcListItemOut)
{
    Pcf8574_Init(pstcHandle->pHandle,NULL);
    if (pstcListItemOut != NULL)
    {
        pstcListItemOut->Handle = pstcHandle;
        pstcListItemOut->enType = Pcf8574ListTypeEncoder;
        ListItemAdd(pstcListItemOut);
    }
    pstcHandle->pHandle->u8CurrentValues |= (1 << pstcHandle->A) | (1 << pstcHandle->B);
    Pcf8574_Write(pstcHandle->pHandle,pstcHandle->pHandle->u8CurrentValues);
    Pcf8574_Read(pstcHandle->pHandle);
}


/**
 ** \brief Execute IRQ handling caused by INT pin for a specific device
 **
 ** \param pstcHandle Handle
 */
void Pcf8574_ExecuteIrqHandle(stc_pcf8574_handle_t* pstcHandle)
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
 ** \brief Process type rotary encoder handle
 **
 ** \param pstcHandle Handle
 */
void Pcf8574_HandleRotaryEncoder(stc_pcf8574_rotaryencoder_t* pstcHandle)
{
    uint8_t u8Tmp,u8Changed;
    pstcHandle->pHandle->pfnRead(pstcHandle->pHandle->pI2cHandle,pstcHandle->pHandle->u32Address,&u8Tmp,1);
    u8Changed = pstcHandle->u8OldData ^ u8Tmp;
    pstcHandle->u8OldData = u8Tmp;
    if ((u8Changed & (1 << pstcHandle->Btn)) && ((u8Tmp & (1 << pstcHandle->Btn)) == 0))
    {
        pstcHandle->bButtonClicked = TRUE;
        pstcHandle->bButton = FALSE;
    } else if ((u8Changed & (1 << pstcHandle->Btn)) && ((u8Tmp & (1 << pstcHandle->Btn)) != 0))
    {
        pstcHandle->bButtonClicked = FALSE;
        pstcHandle->u32LastPressedTime = 0;
        pstcHandle->bButton = TRUE;
    }

    if ((u8Changed & (1 << pstcHandle->A)) && ((u8Tmp & (1 << pstcHandle->A)) == 0))
    {
        if (u8Tmp & (1 << pstcHandle->B))
        {
            pstcHandle->Counter++;
        } else
        {
            pstcHandle->Counter--;
        }
    }
}

/**
 ** \brief Execute IRQ handling caused by INT pin for all devices in the list
 */
void Pcf8574_ExtIrqHandle(void)
{
    stc_pcf8574_list_item_t* pstcCurrent = pstcPcf8574ListRoot;
    if (pstcPcf8574ListRoot == NULL) return;
    bHandleIrq = TRUE;
    if (bLock == FALSE)
    {
        bHandleIrq = FALSE;
        while(pstcCurrent != NULL)
        {
            if (pstcCurrent->enType == Pcf8574ListTypeNone)
            {
                Pcf8574_ExecuteIrqHandle(pstcCurrent->Handle);
            }
            if (pstcCurrent->enType == Pcf8574ListTypeEncoder)
            {
                Pcf8574_HandleRotaryEncoder(pstcCurrent->Handle);
            }
            pstcCurrent = pstcCurrent->Next;
        }
    }
}

/**
 ** \brief Lock interrupt handling
 */
void Pcf8574_LockIrq(void)
{
    bLock = TRUE;
}

/**
 ** \brief Unlock interrupt handling
 */
void Pcf8574_UnlockIrq(void)
{
    bLock = FALSE;
    if (bHandleIrq)
    {
        Pcf8574_ExtIrqHandle();
    }
}

/**
 ** \brief Called every ms for example via SysStick IRQ
 */
void Pcf8574_MsTickHandle(void)
{
    stc_pcf8574_list_item_t* pstcCurrent = pstcPcf8574ListRoot;
    if (pstcPcf8574ListRoot == NULL) return;
    
    while(pstcCurrent != NULL)
    {
        if (pstcCurrent->enType == Pcf8574ListTypeEncoder)
        {
            if (((stc_pcf8574_rotaryencoder_t*)pstcCurrent->Handle)->bButton)
            {
                ((stc_pcf8574_rotaryencoder_t*)pstcCurrent->Handle)->u32LastPressedTime++;
            }
        }
        pstcCurrent = pstcCurrent->Next;
    }
}

/**
 *******************************************************************************
 ** EOF (not truncated)
 *******************************************************************************
 */


# pcf8574
PCF8574 driver for embedded systems

Features:
- Support for multiple PCF8574 by linked list
- Read / Write 8-bit GPIO port expander
- Set callbacks for change of pin, falling edge or rising edge
- Support of INT pin, can be used to connect several INT pins together with one pull-up

Example code:
```
#include "pcf8574.h"

int I2CWrite(void* pHandle, uint32_t u32Address, uint8_t* pu8Data, uint32_t u32Len)
{
    HAL_I2cWrite(pHandle,u32Address,pu8Data,u32Len);
}

int I2CRead(void* pHandle, uint32_t u32Address, uint8_t* pu8Data, uint32_t u32Len)
{
    HAL_I2cRead(pHandle,u32Address,pu8Data,u32Len);
}

stc_pcf8574_list_item_t stcPcf8574ListItm;
stc_pcf8574_handle_t stcPcf8574 =
{
    I2C0,        //pI2cHandle;
    (0x70>>1),   //u32Address;
    0,           //uint8_t u8CurrentValues;
    I2CRead,     //pfnRead;
    I2CWrite     //pfnWrite;
};

void GpioCallback(void)
{
    Pcf8574_ExtIrqHandle();
}

void Pcf8574Gpio0FallingEdgeCallback(void* pHandle, uint8_t u8Pin)
{
    //handle callback of PCF8574, GPIO0, falling edge
}

int main(void)
{
    uint8_t u8Data = 0;
    Board_Init();        //Init I2C


    PCF8575_ZERO_CALLBACKS(&stcPcf8574);          //zero initialize all callbacks
    Pcf8574_Init(&stcPcf8574,&stcPcf8574ListItm); //init PCF8574

    //Optional: Register pin connected to INT as falling edge interrupt at MCUs HAL
    //          with GpioCallback as callback
    HAL_RegisterGpioIrq(PCF8574_INT_PIN, FallingEdge, GpioCallback);   

    //Optional: Register falling edge callback for GPIO0 of PCF8574
    //          requires calling Pcf8574_ExtIrqHandle() in GpioCallback
    Pcf8574_InitCallback(&stcPcf8574, 0, Pcf8574FallingEdge, Pcf8574Gpio0FallingEdgeCallback);

    Pcf8574_Write(&stcPcf8574,0xFF);              //write 0xFF to all GPIOs
    u8Data = Pcf8574_Read(&stcPcf8574);           //read GPIOs into u8Data
    while(1)
    {
        //main applicaton
    }
}
``

/**
 * @file hal_nrf_hw.c
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-07-22
 */

#include <stdint.h>
#include "hal_nrf.h"


void hal_nrf_hw_init()
{
    CSN_HIGH();

    SPI_InitTypeDef  SPI_InitStructure; 
    /* SPI configuration */ 
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //SPI1设置为两线全双工 
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	                   //设置SPI为主模式 
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                  //SPI发送接收8位帧结构 
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;	                //串行时钟在不操作时，时钟为LOW电平 
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	                //第二个时钟沿开始采样数据 
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;	                //NSS信号由软件（使用SSI位）管理 
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; //定义波特率预分频的值:波特率预分频值为8 
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	    //数据传输从MSB位开始 
    SPI_InitStructure.SPI_CRCPolynomial = 7;	    //CRC值计算的多项式 
    SPI_Init(NRF24_SPI, &SPI_InitStructure); 

    /* Enable SPI  */ 
    SPI_Cmd(NRF24_SPI, ENABLE);   //使能cxt->spi外设
    
    //EXTI config
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = NRF24_IRQ_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; //
    EXTI_Init(&EXTI_InitStructure);

    //Enable in NVIC
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = NRF24_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x1; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    NVIC_EnableIRQ(NRF24_IRQ);
}


uint8_t hal_nrf_rw(uint8_t reg)
{
    while(SPI_I2S_GetFlagStatus(NRF24_SPI, SPI_I2S_FLAG_TXE) == RESET); //Wait tx empty
    SPI_I2S_SendData(NRF24_SPI, reg); //

    while(SPI_I2S_GetFlagStatus(NRF24_SPI, SPI_I2S_FLAG_RXNE) == RESET); //Wait rx no empty
    return SPI_I2S_ReceiveData(NRF24_SPI);
}




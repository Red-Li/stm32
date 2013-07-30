/**
 * @file hal_nrf_hw.h
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-07-22
 */


#ifndef HAL_NRF_L01_H__
#define HAL_NRF_L01_H__

#include "hal_stm32.h"

/*
 * Custom micros 
 */

#define NRF24_CSN_GPIO GPIOB
#define NRF24_CSN_PIN  GPIO_Pin_12
#define NRF24_IRQ_GPIO GPIOA
#define NRF24_IRQ_PIN  GPIO_Pin_0
#define NRF24_CE_GPIO  GPIOB
#define NRF24_CE_PIN   GPIO_Pin_1
#define NRF24_SPI      SPI2

#define CSN_STATE() GPIO_ReadOutputDataBit(NRF24_CSN_GPIO, NRF24_CSN_PIN)
#define CE_STATE() GPIO_ReadOutputDataBit(NRF24_CE_GPIO, NRF24_CE_PIN)


/** Macro that set radio's CSN line LOW.
 *
 */
#define CSN_LOW() GPIO_WriteBit(NRF24_CSN_GPIO, NRF24_CSN_PIN, 0)

/** Macro that set radio's CSN line HIGH.
 *
 */
#define CSN_HIGH()  GPIO_WriteBit(NRF24_CSN_GPIO, NRF24_CSN_PIN, 1)

/** Macro that set radio's CE line LOW.
 *
 */
#define CE_LOW() GPIO_WriteBit(NRF24_CE_GPIO, NRF24_CE_PIN, 0)

/** Macro that set radio's CE line HIGH.
 *
 */
#define CE_HIGH() GPIO_WriteBit(NRF24_CE_GPIO, NRF24_CE_PIN, 1)

/** Macro for writing the radio SPI data register.
 *
 */
#define HAL_NRF_HW_SPI_WRITE(d) do{\
    while(SPI_I2S_GetFlagStatus(NRF24_SPI, SPI_I2S_FLAG_TXE) == RESET);\
    SPI_I2S_SendData(NRF24_SPI, (d)); \
}while(0)

/** Macro for reading the radio SPI data register.
 *
 */
#define HAL_NRF_HW_SPI_READ() SPI_I2S_ReceiveData(NRF24_SPI)

/** Macro specifyng the radio SPI busy flag.
 *
 */
#define HAL_NRF_HW_SPI_BUSY \
    (SPI_I2S_GetFlagStatus(NRF24_SPI, SPI_I2S_FLAG_RXNE) == RESET)
/**
 * Pulses the CE to nRF24L01p for at least 10 us
 */
#define CE_PULSE() do { \
    CE_HIGH();  \
    udelay(10);\
    CE_LOW();  \
} while(0)


//Only avaliable for 8051
#define data
#define pdata
#define xdata

#ifdef SET_BIT
#undef SET_BIT
#endif

#endif // HAL_NRF_L01_H__


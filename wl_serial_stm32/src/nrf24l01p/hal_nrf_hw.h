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

#define CSN_STATE() GPIO_ReadOutputDataBit(HAL_NRF_SPI_GPIO, HAL_NRF_SPI_CSN_PIN)
#define CE_STATE() GPIO_ReadOutputDataBit(HAL_NRF_CE_GPIO, HAL_NRF_CE_PIN)

#define NRF_IRQ_DISABLE()     disable_irq(HAL_NRF_IRQ_CHN)
#define NRF_IRQ_ENABLE()      enable_irq(HAL_NRF_IRQ_CHN)

/** Macro that set radio's CSN line LOW.
 *
 */
#define CSN_LOW() GPIO_WriteBit(HAL_NRF_SPI_GPIO, HAL_NRF_SPI_CSN_PIN, 0)

/** Macro that set radio's CSN line HIGH.
 *
 */
#define CSN_HIGH()  GPIO_WriteBit(HAL_NRF_SPI_GPIO, HAL_NRF_SPI_CSN_PIN, 1)

/** Macro that set radio's CE line LOW.
 *
 */
#define CE_LOW() GPIO_WriteBit(HAL_NRF_CE_GPIO, HAL_NRF_CE_PIN, 0)

/** Macro that set radio's CE line HIGH.
 *
 */
#define CE_HIGH() GPIO_WriteBit(HAL_NRF_CE_GPIO, HAL_NRF_CE_PIN, 1)

/** Macro for writing the radio SPI data register.
 *
 */
#define HAL_NRF_HW_SPI_WRITE(d) do{\
    while(SPI_I2S_GetFlagStatus(HAL_NRF_SPI, SPI_I2S_FLAG_TXE) == RESET);\
    SPI_I2S_SendData(HAL_NRF_SPI, (d)); \
}while(0)

/** Macro for reading the radio SPI data register.
 *
 */
#define HAL_NRF_HW_SPI_READ() SPI_I2S_ReceiveData(HAL_NRF_SPI)

/** Macro specifyng the radio SPI busy flag.
 *
 */
#define HAL_NRF_HW_SPI_BUSY \
    (SPI_I2S_GetFlagStatus(HAL_NRF_SPI, SPI_I2S_FLAG_RXNE) == RESET)
/**
 * Pulses the CE to nRF24L01p for at least 10 us
 */
#define CE_PULSE() do { \
    CE_HIGH();  \
    udelay(10);\
    CE_LOW();  \
} while(0)



void hal_nrf_hw_init();

#endif // HAL_NRF_L01_H__


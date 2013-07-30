/**
 * @file hal_nrf_hw.c
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-07-22
 */

#include <stdint.h>
#include "hal_nrf.h"

uint8_t hal_nrf_rw(uint8_t reg)
{
    while(SPI_I2S_GetFlagStatus(NRF24_SPI, SPI_I2S_FLAG_TXE) == RESET); //Wait tx empty
    SPI_I2S_SendData(NRF24_SPI, reg); //

    while(SPI_I2S_GetFlagStatus(NRF24_SPI, SPI_I2S_FLAG_RXNE) == RESET); //Wait rx no empty
    return SPI_I2S_ReceiveData(NRF24_SPI);
}

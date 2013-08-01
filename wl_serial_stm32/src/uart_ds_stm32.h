/**
 * @file uart_ds_stm32.h
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-08-01
 */

#ifndef UART_DS_STM32_H
#define UART_DS_STM32_H

#include "cbuffer.h"
#include "hal_stm32.h"

#define DS_DEFAULT_BAUDRATE     115200
#define DS_TX_BUFFER_SIZE       512
#define DS_RX_BUFFER_SIZE       64


#define DS_FLAG_TX_DMA      0x1



typedef struct ds_s{
    USART_InitTypeDef uart_init_struct;
    USART_TypeDef *uart;
    uint16_t baudrate;
    IRQn_Type   irq;

    DMA_InitTypeDef dma_init_struct;
    DMA_Channel_TypeDef *dma;
    IRQn_Type  dma_irq;

    uint8_t flags;
    
    cbuffer_t tx_cb;


    uint32_t count_recv;
    uint32_t count_recv_drop;
    uint32_t count_send;
    uint32_t count_send_fail;
}ds_t;


extern ds_t g_ds;
#define DS (&g_ds)


int ds_init(ds_t *ds);
void ds_start(ds_t *ds); 
void ds_stop(ds_t *ds);

int ds_send(ds_t *ds, uint8_t *data, uint8_t len);
int ds_recv(ds_t *ds, uint8_t *buf, uint8_t size);

void ds_set_baudrate(ds_t *ds, uint32_t baudrate);
uint32_t ds_get_baudrate(ds_t *ds);


#endif

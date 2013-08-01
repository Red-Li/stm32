/**
 * @file uart_ds_stm32.c
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-08-01
 */


#include "utils.h"
#include "uart_ds_stm32.h"


static uint8_t s_ds_tx_buffer[DS_TX_BUFFER_SIZE];
static uint8_t s_ds_rx_fifo[DS_RX_BUFFER_SIZE];
static uint8_t s_ds_rx_fifo_pos = 0;

int ds_init(ds_t *ds)
{
    USART_DeInit(ds->uart);
    USART_StructInit(&ds->uart_init_struct);
    ds_set_baudrate(ds, DS_DEFAULT_BAUDRATE);

    DMA_DeInit(ds->dma);
    DMA_StructInit(&ds->dma_init_struct);
    ds->dma_init_struct.DMA_PeripheralBaseAddr = (uint32_t)(&ds->uart->DR);
    ds->dma_init_struct.DMA_DIR = DMA_DIR_PeripheralDST;
    ds->dma_init_struct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    ds->dma_init_struct.DMA_Priority = DMA_Priority_High;

    DMA_ITConfig(ds->dma, DMA_IT_TC|DMA_IT_TE, ENABLE);
    DMA_Cmd(ds->dma, DISABLE);

    CBUFFER_INIT(&ds->tx_cb, 
            s_ds_tx_buffer,
            DS_TX_BUFFER_SIZE, 0);

    return 0;
}
void ds_start(ds_t *ds)
{
    enable_irq(ds->dma_irq);
    enable_irq(ds->irq);

    USART_ITConfig(ds->uart, USART_IT_RXNE, ENABLE);
    USART_DMACmd(ds->uart, USART_DMAReq_Tx, ENABLE);
    USART_Cmd(ds->uart, ENABLE);
}
void ds_stop(ds_t *ds)
{
    USART_DMACmd(ds->uart, USART_DMAReq_Tx, DISABLE);
    USART_Cmd(ds->uart, DISABLE);

    USART_ITConfig(ds->uart, USART_IT_RXNE, DISABLE);

    disable_irq(ds->dma_irq);
    disable_irq(ds->irq);

}


static void ds_tx_dma_start(ds_t *ds)
{
    if(!CBUFFER_EMPTY(&ds->tx_cb) && !(ds->flags & DS_FLAG_TX_DMA)){
        cbuffer_size_t csize = CBUFFER_CDATA_LENGTH(&ds->tx_cb);
        while(csize != CBUFFER_CDATA_LENGTH(&ds->tx_cb))
            csize = CBUFFER_CDATA_LENGTH(&ds->tx_cb);
        
        ds->dma_init_struct.DMA_BufferSize = csize;
        ds->dma_init_struct.DMA_MemoryBaseAddr = (uint32_t)CBUFFER_DATA(&ds->tx_cb);

        DMA_Cmd(ds->dma, DISABLE);
        DMA_Init(ds->dma, &ds->dma_init_struct);
        FLAG_SET(ds->flags, DS_FLAG_TX_DMA);
        DMA_Cmd(ds->dma, ENABLE);
    }
}


int ds_send(ds_t *ds, uint8_t *data, uint8_t len)
{
    int n = CBUFFER_WRITE(&ds->tx_cb, data, len);
    ds_tx_dma_start(ds);
    
    return n;
}


int ds_recv(ds_t *ds, uint8_t *buf, uint8_t size)
{
    uint8_t pos = s_ds_rx_fifo_pos, i = 0;
    ASSERT(size);

    if(pos){
        pos = pos < size ? pos : size;   
        memcpy(buf, s_ds_rx_fifo, pos);
        

        disable_irq(ds->irq);
        while(pos < s_ds_rx_fifo_pos)
            s_ds_rx_fifo[i++] = s_ds_rx_fifo[pos++];
        s_ds_rx_fifo_pos = i;
        enable_irq(ds->irq);
    }

    return pos - i;
}


void ds_set_baudrate(ds_t *ds, uint32_t baudrate)
{

    ds->uart_init_struct.USART_BaudRate = baudrate;
    USART_Init(ds->uart, &ds->uart_init_struct);
}

uint32_t ds_get_baudrate(ds_t *ds)
{
    return ds->uart_init_struct.USART_BaudRate;
}



void ds_uart_dma_interrupt_handler()
{
    ASSERT(DS->dma->CPAR == (uint32_t)(&DS->uart->DR));

    if(DMA_GetITStatus(DMA1_IT_TE4)){ //Error
        DS->count_send_fail += DS->dma_init_struct.DMA_BufferSize;
        CBUFFER_FREE(&DS->tx_cb, DS->dma_init_struct.DMA_BufferSize);
        FLAG_CLR(DS->flags, DS_FLAG_TX_DMA);
        ds_tx_dma_start(DS);

        DMA_ClearITPendingBit(DMA1_IT_TE4);
    }
    else if(DMA_GetITStatus(DMA1_IT_TC4)){
        DS->count_send += DS->dma_init_struct.DMA_BufferSize;
        CBUFFER_FREE(&DS->tx_cb, DS->dma_init_struct.DMA_BufferSize);
        FLAG_CLR(DS->flags, DS_FLAG_TX_DMA);
        ds_tx_dma_start(DS);

        DMA_ClearITPendingBit(DMA1_IT_TC4);
    }
    else{
        ASSERT(0);
    }
    
}


void ds_uart_interrupt_handler()
{
    while(USART_GetFlagStatus(DS->uart, USART_FLAG_RXNE) == SET){
        s_ds_rx_fifo[s_ds_rx_fifo_pos++] = USART_ReceiveData(DS->uart) & 0xff;
        if(s_ds_rx_fifo_pos >= DS_RX_BUFFER_SIZE){ //FIFO overflow
            --s_ds_rx_fifo_pos;
            ++DS->count_recv_drop;
        }
    }
    
    USART_ClearFlag(DS->uart, USART_FLAG_RXNE);
    USART_ClearITPendingBit(DS->uart, USART_IT_RXNE);
}



ds_t g_ds = {
    .uart = USART1,
    .irq = USART1_IRQn,

    .dma = DMA1_Channel4,
    .dma_irq = DMA1_Channel4_IRQn,

    .flags = 0,

    .count_recv = 0,
    .count_recv_drop = 0,
    .count_send = 0,
    .count_send_fail = 0
};

/**
 * @file utils.c
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-06-12
 */


#include <stdint.h>

#include "utils.h"
#include "hal_stm32.h"

static void raw_print_func(void *priv, char c)
{
    if(c == '\n'){
        while(USART_GetFlagStatus((USART_TypeDef*)priv, USART_FLAG_TXE) == RESET);          
        USART_SendData((USART_TypeDef*)priv, '\r');
    }
    while(USART_GetFlagStatus((USART_TypeDef*)priv, USART_FLAG_TXE) == RESET);          
    USART_SendData((USART_TypeDef*)priv, c);
}


void global_init(void) 
{
    hal_init();

    //Using RAW USART putc()
    init_printf((void*)HAL_DEBUG_UART, raw_print_func);
}



void _exit(int code)
{
    while(1);
}


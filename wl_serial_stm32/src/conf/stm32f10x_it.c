/**
 * @file stm32f10x_it.c
 * @brief define the interrput handler
 * @author Red Li
 * @version 
 * @date 2013-06-13
 */

#include "hal_stm32.h"

#define STM32_MAP_INT_FUNC(from, to)\
    extern void to(void);\
    inline void from(void){\
        to();\
    }

#if defined(HAL_BOARD_DOFLY)
STM32_MAP_INT_FUNC(SysTick_Handler, systick_interrupt_handler)
STM32_MAP_INT_FUNC(EXTI0_IRQHandler, wls_nrf24_interrupt_handler)
STM32_MAP_INT_FUNC(EXTI2_IRQHandler, management_interrupt_handler)
STM32_MAP_INT_FUNC(USART3_IRQHandler, ds_uart_interrupt_handler)
STM32_MAP_INT_FUNC(DMA1_Channel2_IRQHandler, ds_uart_dma_interrupt_handler)
STM32_MAP_INT_FUNC(TIM3_IRQHandler, hal_timer_interrupt_handler)

#elif defined(HAL_BOARD_OURSTM)
STM32_MAP_INT_FUNC(SysTick_Handler, systick_interrupt_handler)
STM32_MAP_INT_FUNC(EXTI0_IRQHandler, wls_nrf24_interrupt_handler)
STM32_MAP_INT_FUNC(EXTI2_IRQHandler, management_interrupt_handler)
STM32_MAP_INT_FUNC(USART1_IRQHandler, ds_uart_interrupt_handler)
STM32_MAP_INT_FUNC(DMA1_Channel4_IRQHandler, ds_uart_dma_interrupt_handler)
STM32_MAP_INT_FUNC(TIM3_IRQHandler, hal_timer_interrupt_handler)

#else
#error "No board have been selected"
#endif

/**
 * @file stm32f10x_it.c
 * @brief define the interrput handler
 * @author Red Li
 * @version 
 * @date 2013-06-13
 */

#define STM32_MAP_INT_FUNC(from, to)\
    extern void to(void);\
    inline void from(void){\
        to();\
    }


STM32_MAP_INT_FUNC(SysTick_Handler, systick_interrupt_handler)
STM32_MAP_INT_FUNC(EXTI0_IRQHandler, wls_nrf24_interrupt_handler)

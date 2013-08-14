/**
 * @file hal_stm32.h
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-07-09
 */


#ifndef HAL_STM32_H
#define HAL_STM32_H

#include <stdint.h>
#include <stdlib.h>
#include "misc.h"
#include "stm32f10x.h"

//Board definition
//#define HAL_BOARD_DOFLY 1
#define HAL_BOARD_OURSTM

//Board resource select
#if defined(HAL_BOARD_DOFLY)
//UART
#define HAL_DEBUG_UART                          USART3
#define HAL_DEFAULT_UART_BAUD                   38400

#define HAL_DS_UART_GPIO                        GPIOB
#define HAL_DS_UART_RX_PIN                      GPIO_Pin_11
#define HAL_DS_UART_TX_PIN                      GPIO_Pin_10

#define HAL_DS_UART                             HAL_DEBUG_UART
#define HAL_DS_UART_DMA_CHN                     DMA1_Channel2
#define HAL_DS_UART_DMA_IRQ_CHN                 DMA1_Channel2_IRQn
#define HAL_DS_UART_DMA_TE                      DMA1_IT_TE2
#define HAL_DS_UART_DMA_TC                      DMA1_IT_TC2
#define HAL_DS_UART_RX_IRQ_CHN                  USART3_IRQn

//NRF24L01
#define HAL_NRF_SPI                             SPI2
#define HAL_NRF_SPI_GPIO                        GPIOB
#define HAL_NRF_SPI_CSN_PIN                     GPIO_Pin_12
#define HAL_NRF_SPI_SCK_PIN                     GPIO_Pin_13
#define HAL_NRF_SPI_MISO_PIN                    GPIO_Pin_14
#define HAL_NRF_SPI_MOSI_PIN                    GPIO_Pin_15
#define HAL_NRF_CE_GPIO                         GPIOA
#define HAL_NRF_CE_PIN                          GPIO_Pin_1
#define HAL_NRF_IRQ_GPIO                        GPIOA
#define HAL_NRF_IRQ_PIN                         GPIO_Pin_0

#define HAL_NRF_IRQ_CHN                         EXTI0_IRQn
#define HAL_NRF_EXTI_LINE                       EXTI_Line0
#define HAL_NRF_EXTI_PORT                       GPIO_PortSourceGPIOA
#define HAL_NRF_EXTI_PORT_SOURCE                GPIO_PinSource0

//management
#define HAL_MANAGEMENT_GPIO                     GPIOA
#define HAL_MANAGEMENT_PIN                      GPIO_Pin_2
#define HAL_MANAGEMENT_IRQ_CHN                  EXTI2_IRQn
#define HAL_MANAGEMENT_EXTI_LINE                EXTI_Line2
#define HAL_MANAGEMENT_EXTI_PORT                GPIO_PortSourceGPIOA
#define HAL_MANAGEMENT_EXTI_PORT_SOURCE         GPIO_PinSource2

#elif defined(HAL_BOARD_OURSTM)
//UART
#define HAL_DEBUG_UART                          USART1
#define HAL_DEFAULT_UART_BAUD                   38400

#define HAL_DS_UART_GPIO                        GPIOA
#define HAL_DS_UART_RX_PIN                      GPIO_Pin_10
#define HAL_DS_UART_TX_PIN                      GPIO_Pin_9
#define HAL_DS_UART_RX_IRQ_CHN                  USART1_IRQn

#define HAL_DS_UART                             HAL_DEBUG_UART
#define HAL_DS_UART_DMA_CHN                     DMA1_Channel4
#define HAL_DS_UART_DMA_IRQ_CHN                 DMA1_Channel4_IRQn
#define HAL_DS_UART_DMA_TE                      DMA1_IT_TE4
#define HAL_DS_UART_DMA_TC                      DMA1_IT_TC4

//NRF24L01
#define HAL_NRF_SPI                             SPI2
#define HAL_NRF_SPI_GPIO                        GPIOB
#define HAL_NRF_SPI_CSN_PIN                     GPIO_Pin_12
#define HAL_NRF_SPI_SCK_PIN                     GPIO_Pin_13
#define HAL_NRF_SPI_MISO_PIN                    GPIO_Pin_14
#define HAL_NRF_SPI_MOSI_PIN                    GPIO_Pin_15
#define HAL_NRF_CE_GPIO                         GPIOB
#define HAL_NRF_CE_PIN                          GPIO_Pin_1
#define HAL_NRF_IRQ_GPIO                        GPIOA
#define HAL_NRF_IRQ_PIN                         GPIO_Pin_0

#define HAL_NRF_IRQ_CHN                         EXTI0_IRQn
#define HAL_NRF_EXTI_LINE                       EXTI_Line0
#define HAL_NRF_EXTI_PORT                       GPIO_PortSourceGPIOA
#define HAL_NRF_EXTI_PORT_SOURCE                GPIO_PinSource0

//management
#define HAL_MANAGEMENT_GPIO                     GPIOA
#define HAL_MANAGEMENT_PIN                      GPIO_Pin_2
#define HAL_MANAGEMENT_IRQ_CHN                  EXTI2_IRQn
#define HAL_MANAGEMENT_EXTI_LINE                EXTI_Line2
#define HAL_MANAGEMENT_EXTI_PORT                GPIO_PortSourceGPIOA
#define HAL_MANAGEMENT_EXTI_PORT_SOURCE         GPIO_PinSource2

#else
#error "No board have been selected"
#endif







void hal_init();
typedef uint64_t hal_time_t;

#define sysclk_freq() SystemCoreClock
#define systick_freq() 1000  //about 1ms

#define systick_counter() (sysclk_freq() / systick_freq())
#define systick_current() (SysTick->VAL)

void systick_interrupt_handler();

hal_time_t hal_time(); //return us

void hal_srand(uint32_t seed);
uint32_t hal_rand();

#define hal_soft_reset() NVIC_SystemReset()

//return t1 - t0
#define hal_time_diff(t0, t1) ((t1) - (t0))

#define local_irq_enable()      __set_PRIMASK(0)
#define local_irq_disable()     __set_PRIMASK(1) 

#define enable_irq(irq)     NVIC_EnableIRQ(irq)
#define disable_irq(irq)    NVIC_DisableIRQ(irq)
static inline uint32_t get_irq_state(unsigned long IRQn)
{
    return NVIC->ISER[((uint32_t)(IRQn) >> 5)] & (1 << ((uint32_t)(IRQn) & 0x1F)) ? 1 : 0; /* enable interrupt */
}
static inline void disable_irq_save(unsigned long irq, uint32_t *flags)
{
    *flags = get_irq_state(irq);
    disable_irq(irq);
}

static inline void enable_irq_restore(unsigned long irq, uint32_t flags)
{
    if(flags) enable_irq(irq);
}

static inline void local_irq_save(unsigned long *flags)
{
    *flags = __get_PRIMASK(); //interrupt flag
    local_irq_disable();
}

static inline void local_irq_restore(unsigned long flags)
{
    __set_PRIMASK(flags);
}


//delay
void udelay(uint32_t us);
void mdelay(uint32_t ms);
#define usleep udelay
#define msleep mdelay

/******************************* Hardward device ********************/
#define HAL_PIN_TOTAL 0x8
void hal_pin_mode_change(uint8_t which, uint8_t mode_from, uint8_t mode_to);
void hal_gpio_set(uint8_t which, uint8_t enable);
void hal_pwm_set_duty_cycle(uint8_t which, uint8_t cycle_duty);

#define HAL_PWM_TOTAL 4
#define HAL_GPIO_TOTAL 4

#endif

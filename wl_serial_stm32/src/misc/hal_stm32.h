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

#define HAL_DEFAULT_UART_BAUD 38400

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
#define HAL_NUM_PWM 4
#define HAL_NUM_GPIO 4

#endif

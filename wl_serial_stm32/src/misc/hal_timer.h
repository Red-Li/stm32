/**
 * @file hal_timer.h
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-08-01
 */


#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include "utils.h"
#include "hal_stm32.h"

#include "l_list.h"

#define HAL_TIMER_CONTINUOUS      0x1
#define HAL_TIMER_RUNNING   0x2

typedef void (*hal_timer_callback_t)(void*);

typedef struct hal_timer_s{
    hal_timer_callback_t callback;
    void *callback_data;
    uint8_t flags;
    
    hal_time_t __next_active_time;
    uint32_t __interval;
    struct list_head __list;
}hal_timer_t;

//
void __hal_timer_tick();

void hal_timer_init(hal_timer_t *timer, hal_timer_callback_t callback, void *data);
void hal_timer_start(hal_timer_t *timer, uint32_t interval, uint8_t continuous);
void hal_timer_stop(hal_timer_t *timer);

#define hal_timer_is_running(timer) (FLAG_CHECK((timer)->flags, HAL_TIMER_RUNNING))

#endif

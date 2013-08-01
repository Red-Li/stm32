/**
 * @file hal_timer.c
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-08-01
 */

#include "hal_timer.h"

static LIST_HEAD(s_hal_timer_list);


void __hal_timer_tick()
{
    struct list_head *p, *q;
    hal_timer_t *timer;
    hal_time_t now = hal_time();

    list_for_each(p, &s_hal_timer_list){
        timer = container_of(p, hal_timer_t, __list);
        ASSERT(FLAG_CHECK(timer->flags, HAL_TIMER_RUNNING));


        if(timer->__next_active_time < now){
            timer->callback(timer->callback_data);

            if(FLAG_CHECK(timer->flags, HAL_TIMER_CONTINUOUS)){
                //Set next trigger time
                timer->__next_active_time += timer->__interval;
            }
            else{
                //Remove it from timer list 
                FLAG_CLR(timer->flags, HAL_TIMER_RUNNING);
                q = p->prev;
                list_del(p);
                p = q;
            }
        }
    }
}

void hal_timer_init(hal_timer_t *timer, hal_timer_callback_t callback, void *data)
{
    timer->callback = callback;
    timer->callback_data = data;
    timer->flags = 0;
    timer->__next_active_time = -1;
    timer->__interval = 0;
    
    INIT_LIST_HEAD(&timer->__list);
}

void hal_timer_start(hal_timer_t *timer, uint32_t interval, uint8_t continuous)
{
    uint32_t flags;
    if(continuous)
        FLAG_SET(timer->flags, HAL_TIMER_CONTINUOUS);

    timer->__interval = interval;
    timer->__next_active_time = hal_time() + interval;

    FLAG_SET(timer->flags, HAL_TIMER_RUNNING);

    local_irq_save(&flags);
    list_add_tail(&s_hal_timer_list, &timer->__list);
    local_irq_restore(flags);

}
void hal_timer_stop(hal_timer_t *timer)
{
    uint32_t flags;

    local_irq_save(&flags);
    list_del(&timer->__list);
    local_irq_restore(flags);

    timer->__interval = 0;
    timer->__next_active_time = -1;
    FLAG_CLR(timer->flags, HAL_TIMER_RUNNING);
}

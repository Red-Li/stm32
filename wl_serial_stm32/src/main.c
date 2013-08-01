/**
 * @file main.c
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-07-31
 */


#include <string.h>

#include "hal_stm32.h"
#include "hal_timer.h"
#include "utils.h"
#include "wl_serial.h"
#include "uart_ds_stm32.h"
#include "settings.h"

uint32_t count_recv = 0,
         count_recv_fail = 0,
         count_send = 0,
         count_send_fail = 0;

static uint8_t s_management_mode = 0;

static hal_timer_t s_uart_poll_timer;
static uint8_t uart_recv_buffer[32];


static uint8_t __send_cmd_data(uint8_t *data, uint8_t len)
{
    return len;
}

static uint8_t __send_data(uint8_t *data, uint8_t len)
{
    uint8_t handled = wls_send_to(WLS, data, len);
    count_send += handled;
    count_send_fail += len - handled;

    return handled;
}

void uart_poll_data(void *_ds)
{
    ds_t *ds = (ds_t*)_ds;
    uint8_t n = ds_recv(ds, uart_recv_buffer, 32),
            handled = 0;

    while(n){
        if(s_management_mode){
            handled = __send_cmd_data(uart_recv_buffer, n);
        }
        else{
            handled = __send_data(uart_recv_buffer, n);
        }

        if(handled != n)
            break;
        n = ds_recv(ds, uart_recv_buffer, 32);
    }

}



void handle_data_packet(void *priv, uint8_t port, uint8_t *data, uint8_t len)
{
    uint8_t handled = 0;
    
    if(!s_management_mode)
        handled = ds_send(DS, data, len);

    count_recv += handled;
    count_recv_fail += len - handled;
}

void handle_cmd_packet(void *priv, uint8_t port, uint8_t *data, uint8_t len)
{

}

void handle_cmd_ack_packet(void *priv, uint8_t port, uint8_t *data, uint8_t len)
{
}



void main_loop()
{
    uint32_t time_wait = 0;
    uint32_t max_delay = 2000;


    while(1){
        WLS->port.tx_buffer_timeout = 8000000UL * WLS_MAX_PD_SIZE / ds_get_baudrate(DS);
        
        if(wls_flush_tx(WLS) < 0){
            max_delay = max_delay >= 8000 ? 8000 : max_delay + 1000;
            time_wait = hal_rand() % max_delay;
        }
        else{
            max_delay = max_delay > 1000 ? max_delay - 1000 : max_delay;
            time_wait = 0;
        }

        time_wait += hal_rand() % 4000 + 2000;
        udelay(time_wait);
    }
}


int main(void)
{
    local_irq_disable();
    
    global_init();


    DBG("Global Init");

    ASSERT(wls_init(WLS) == 0);
    //Setup callbacks
    wls_callback_t cb;

    cb.callback = handle_data_packet;
    cb.callback_data = NULL;
    wls_set_callback(WLS, WLS_CB_DATA, cb);
    
    cb.callback = handle_cmd_packet;
    cb.callback_data = NULL;
    wls_set_callback(WLS, WLS_CB_CMD, cb);

    cb.callback = handle_cmd_ack_packet;
    cb.callback_data = NULL;
    wls_set_callback(WLS, WLS_CB_CMD_ACK, cb);

    ASSERT(ds_init(DS) == 0);

    //Load settings
    settings_load(settings);
    settings->dirty_flag = 0xffffffff;
    settings_commit(settings);

    local_irq_enable();
    
    //
    ds_start(DS);
    
    hal_timer_init(&s_uart_poll_timer, uart_poll_data, DS);
    hal_timer_start(&s_uart_poll_timer, 2000, 1); //Every 2ms

    //Start handle packet
    wls_start(WLS);

    main_loop();

    while(1);
    return 0;
}

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
#include "wl_cmd.h"
#include "settings.h"


#define MANAGEMENT_GPIO GPIOA
#define MANAGEMENT_PIN  GPIO_Pin_2
#define MANAGEMENT_IRQ_LINE EXTI_Line2
#define MANAGEMENT_SELECTED() (!GPIO_ReadInputDataBit(MANAGEMENT_GPIO, MANAGEMENT_PIN))

uint32_t count_recv = 0,
         count_recv_fail = 0,
         count_send = 0,
         count_send_fail = 0;

static uint8_t s_management_mode = 0;

static hal_timer_t s_uart_poll_timer;
static uint8_t uart_recv_buffer[32];


static uint8_t __send_cmd_data(uint8_t *data, uint8_t len)
{
    return wl_cmd_recv(data, len);
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


//Send out cmd result
int send_cmd_result(void *priv, uint8_t *data, uint8_t len)
{
    return wls_send_cmd_result((wls_t*)priv, data, len);
}


void handle_cmd_packet(void *priv, uint8_t port, uint8_t *data, uint8_t len)
{
    if(!s_management_mode){
        wl_cmd_set_handle_return(send_cmd_result, WLS);
        wl_cmd_reset();
        
        //Process command
        wl_cmd_recv(data, len);
        wl_cmd_set_handle_return(NULL, NULL);

        //Apply settings if needed
        settings_commit(settings);

        //Save settings if needed
        settings_save(settings);
    }
}

void handle_cmd_ack_packet(void *priv, uint8_t port, uint8_t *data, uint8_t len)
{
    //output result
    if(s_management_mode)
        ds_send(DS, data, len);
    //else drop it
}


//Send out remote cmd and receive result
int handle_remote_cmd(void *priv, uint8_t *data, uint8_t len)
{
    return wls_send_cmd((wls_t*)priv, data, len);
}



void management_detect()
{
    if(MANAGEMENT_SELECTED()){
        wl_cmd_set_handle_return((wl_cmd_callback_t)ds_send, DS);
        wl_cmd_reset();

        //Mark baud rate changed
        settings->dirty_flag |= WL_BAUD_MASK;
        ds_set_baudrate(DS, 9600);

        s_management_mode = 1;
#if 0
        //Check if it is reset by command
        if(RST_GetFlagStatus(RST_FLAG_WWDGF) == SET){
            ds_handle_data(DS, "OK", 2);
            RST_ClearFlag(RST_FLAG_WWDGF);
        }
#endif
    }
    else{
        wl_cmd_set_handle_return(NULL, NULL);
        wl_cmd_reset();

        //Apply settings if needed
        settings_commit(settings);

        //Save settings if needed
        settings_save(settings);

        s_management_mode = 0;
    }
}


void management_interrupt_handler()
{
    management_detect();
    EXTI_ClearITPendingBit(MANAGEMENT_IRQ_LINE); //clear it, make it can receive interrupt again
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

    //Init hal_rand
    hal_srand(-1);

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

    //Set handle remote command callback
    wl_cmd_set_handle_remote_cmd(handle_remote_cmd, WLS);

    ASSERT(ds_init(DS) == 0);

    //Load settings
    settings_load(settings);
    settings->dirty_flag = 0xffffffff;
    settings_commit(settings);
    
    management_detect();

    //
    ds_start(DS);

    local_irq_enable();


    hal_timer_init(&s_uart_poll_timer, uart_poll_data, DS);
    hal_timer_start(&s_uart_poll_timer, 2000, 1); //Every 2ms

    //Start handle packet
    wls_start(WLS);
    

    DBG("-->");
    main_loop();

    while(1);
    return 0;
}

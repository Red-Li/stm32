/**
 * @file main.c
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-07-31
 */


#include <string.h>

#include "hal_stm32.h"
#include "utils.h"
#include "wl_serial.h"
#include "settings.h"

uint32_t count_recv = 0,
         count_recv_fail = 0,
         count_send = 0,
         count_send_fail = 0;

void handle_data_packet(void *priv, uint8_t port, uint8_t *data, uint8_t len)
{
    count_recv += len;
}

void handle_cmd_packet(void *priv, uint8_t port, uint8_t *data, uint8_t len)
{
}

void handle_cmd_ack_packet(void *priv, uint8_t port, uint8_t *data, uint8_t len)
{
}



void send_data(void *priv, uint8_t *data, uint8_t len)
{
}


void main_loop()
{
    wls_addr_t laddr, raddr;
    uint8_t data[32]  = "hello";


    while(1){
        wls_send_to(WLS, data, 5);
        
        wls_flush_tx(WLS);
        mdelay(1000);
        
        hal_nrf_get_address(HAL_NRF_PIPE0, laddr);
        hal_nrf_get_address(HAL_NRF_TX, raddr);

        DBG("status:0x%x config:0x%x rf:0x%x rf_chn:0x%x fs:0x%x er:0x%x ea:0x%x RPD:0x%x AW:0x%x FEATURE:0x%x DYNPD:0x%x CE:%d laddr:%x:%x:%x:%x:%x raddr:%x:%x:%x:%x:%x", 
                hal_nrf_read_reg(STATUS), 
                hal_nrf_read_reg(CONFIG),
                hal_nrf_read_reg(RF_SETUP),
                hal_nrf_read_reg(RF_CH),
                hal_nrf_read_reg(FIFO_STATUS),
                hal_nrf_read_reg(EN_RXADDR),
                hal_nrf_read_reg(EN_AA),
                hal_nrf_read_reg(CD),
                hal_nrf_read_reg(SETUP_AW),
                hal_nrf_read_reg(FEATURE),
                hal_nrf_read_reg(DYNPD),
                CE_STATE(),
                laddr[0], laddr[1], laddr[2], laddr[3], laddr[4],
                raddr[0], raddr[1], raddr[2], raddr[3], raddr[4]
                );

    }
}



int main(void)
{
    local_irq_disable();
    
    global_init();

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

    //Load settings
    settings_load(settings);
    settings->dirty_flag = 0xffffffff;
    settings_commit(settings);

    local_irq_enable();
    
    //Start handle packet
    wls_start(WLS);
    
    main_loop();

    return 0;
}

/**
 * @file wl_serial.h
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-07-31
 */


#ifndef WL_SERIAL_H
#define WL_SERIAL_H

#include <string.h>

#include "hal_stm32.h"
#include "cbuffer.h"
#include "bit_field.h"
#include "hal_nrf.h"

#define WLS_DEBUG

#define WLS_BUF_SIZE   1024

#define WLS_HEADER_SIZE sizeof(wls_packet_header_t)
#define WLS_MAX_PD_SIZE (32 - WLS_HEADER_SIZE)

#define WLS_MAX_RETRIES 12

#define WLS_DEFAULT_RF_CH 0x40
#define WLS_DEFAULT_DATERATE HAL_NRF_2MBPS

#define WLS_GAMBLE()  (hal_rand() % 2 == 0)

#define WLS_RX_MODE()\
    hal_nrf_set_operation_mode(HAL_NRF_PRX);\
    CE_HIGH();

#define WLS_TX_MODE()\
    CE_LOW();\
    hal_nrf_set_operation_mode(HAL_NRF_PTX);

typedef uint8_t wls_counter_t;
typedef hal_nrf_datarate_t wls_speed_t;

#define WLS_PKT_DATA        0x0
#define WLS_PKT_CMD         0x1
#define WLS_PKT_CMD_ACK     0x2

/* 
 * wls port flags
 */ 

#define WLS_FLAG_PKT_LOADED     0x1
#define WLS_FLAG_PKT_TX         0x2
#define WLS_FLAG_PKT_ACK        0x4

#define WLS_FLAG_SET(flags, flag) ((flags) |= (flag))
#define WLS_FLAG_CLR(flags, flag) ((flags) &= ~(flag))

typedef struct wls_packet_header_s{
    wls_counter_t counter;
    uint8_t type;
}__attribute__((packed)) wls_packet_header_t ;

typedef struct{
    wls_packet_header_t header;
    uint8_t payload[WLS_MAX_PD_SIZE];
}__attribute__((packed)) wls_packet_t ;

typedef uint8_t wls_addr_t[5];


typedef struct wls_port_s{
    wls_addr_t local_addr;
    wls_addr_t target_addr;

    hal_time_t last_send_time;
    
    volatile wls_counter_t recv_counter; //last recved counter
    volatile wls_counter_t send_counter;
    
    uint8_t send_retries;
    volatile uint8_t flags;

    wls_packet_t active_packet;
    uint8_t active_packet_size;

    uint32_t tx_buffer_timeout;

    cbuffer_t cb; 
}wls_port_t;

/**
 * Callback related definitions
 *
 **/
typedef enum{
    WLS_CB_DATA,
    WLS_CB_CMD,
    WLS_CB_CMD_ACK
}wls_callback_type_t;


//(priv, port, data, len)
typedef struct wls_callback_s{
    void (*callback)(void *, uint8_t, uint8_t *, uint8_t);
    void *callback_data;
}wls_callback_t;


/**
 * Wireless serial toplevel struct
 */
typedef struct wls_s{
    wls_port_t port;
    
    wls_callback_t handle_data;
    wls_callback_t handle_cmd;
    wls_callback_t handle_cmd_ack;

    uint32_t count_send;
    uint32_t count_ack_send;
    uint32_t count_send_fail;
    uint32_t count_retry;
    uint32_t count_recv;
    uint32_t count_recv_drop;
}wls_t;


extern wls_t g_wls;
#define WLS (&g_wls)


int wls_init(wls_t *wls);
void wls_start(wls_t *wls);
void wls_stop(wls_t *wls);
int wls_send_to(wls_t *wls, uint8_t *data, uint8_t len);

int wls_send_cmd(wls_t *wls, uint8_t *_data, uint8_t len);

int wls_send_cmd_result(wls_t *wls, uint8_t *_data, uint8_t len);

void wls_set_callback(wls_t *wls, wls_callback_type_t type, wls_callback_t cb);

void wls_set_rf(wls_t *wls, uint8_t chn, wls_speed_t speed);
void wls_set_local_addr(wls_t *wls, wls_addr_t addr);
void wls_set_remote_addr(wls_t *wls, wls_addr_t addr);

int wls_flush_tx(wls_t *wls);

#endif //WL_SERIAL_H


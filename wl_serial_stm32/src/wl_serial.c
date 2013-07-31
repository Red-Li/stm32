/**
 * @file wl_serial.c
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-07-22
 */

#include "utils.h"
#include "wl_serial.h"
#include "hal_nrf.h"


/* --------------------------------------------------------------------------*/
/**
 * @brief Global wls context
 */
/* ----------------------------------------------------------------------------*/
wls_t g_wls;


/* --------------------------------------------------------------------------*/
/**
 * @brief 
 */
/* ----------------------------------------------------------------------------*/
static uint8_t s_wls_buffer[WLS_BUF_SIZE];


#ifdef WLS_DEBUG

static uint8_t nRF24L01_reg_read(uint8_t reg)
{
    uint8_t value;

    CSN_LOW();

    hal_nrf_rw(reg);
    value = hal_nrf_rw(reg);

    CSN_HIGH();

    return value;
}

static uint8_t nRF24L01_reg_write(uint8_t reg, uint8_t value)
{

    CSN_LOW();

    hal_nrf_rw(reg);
    value = hal_nrf_rw(value);

    CSN_HIGH();

    return value;
}

#endif

static int wls_nrf24_exist()
{
    uint8_t waddr[] = {0xaa, 0x55, 0xaa, 0xff, 0x33},
            raddr[5];
    hal_nrf_set_address(HAL_NRF_TX, waddr);

    uint8_t aw = hal_nrf_get_address(HAL_NRF_TX, raddr);
    while(aw && raddr[aw - 1] == waddr[aw - 1])
        --aw;

    return aw ? 0 : 1;
}

static void wls_nrf24_reset()
{
    hal_nrf_get_clear_irq_flags();
    hal_nrf_flush_tx();
    hal_nrf_flush_rx();
}

static int wls_nrf24_init(void)
{
    //Address width
    hal_nrf_set_address_width(HAL_NRF_AW_5BYTES);

    //RF Setup
    hal_nrf_set_rf_channel(WLS_DEFAULT_RF_CH);
    hal_nrf_set_datarate(WLS_DEFAULT_DATERATE);

    //Setup crc mode
    hal_nrf_set_crc_mode(HAL_NRF_CRC_16BIT);

    //interrupt configuration
    hal_nrf_set_irq_mode(HAL_NRF_TX_DS, false);
    hal_nrf_set_irq_mode(HAL_NRF_MAX_RT, false);
    hal_nrf_set_irq_mode(HAL_NRF_RX_DR, true);

    //
    hal_nrf_activate_features();
    hal_nrf_enable_dynamic_payload(true);
    hal_nrf_setup_dynamic_payload(0x1); //channel 0
    hal_nrf_enable_ack_payload(true);
    //hal_nrf_set_rx_payload_width(0, 0);

    //Open channels
    hal_nrf_open_pipe(HAL_NRF_PIPE0, true);

    //Power up and ready to rx
    hal_nrf_set_power_mode(HAL_NRF_PWR_UP);
    udelay(1500); //1.5ms
    hal_nrf_set_operation_mode(HAL_NRF_PRX);
    udelay(130);
    //
    CE_LOW();

    wls_nrf24_reset();

    return 0;
}

static int wls_load_one_data_packet(wls_t *wls);

//Should execute in interrupt handle
static void wls_handle_ack_payload(wls_t *wls)
{
    if(hal_nrf_get_operation_mode() == HAL_NRF_PTX)
        return;

    wls_port_t *port = &wls->port;
    
    //Check if we have receive ack for ack payload
    if((hal_nrf_get_irq_flags() & 0x2) && (port->flags & WLS_FLAG_PKT_ACK)){
        //ACK payload confirmed
        wls->count_ack_send += port->active_packet_size;
        WLS_FLAG_CLR(port->flags, WLS_FLAG_PKT_LOADED|WLS_FLAG_PKT_ACK);

        //Reset retries
        port->send_retries = 0;
        //Update packet send counter
        ++port->send_counter;

        //Update last send time
        port->last_send_time = hal_time();

        hal_nrf_clear_irq_flag(HAL_NRF_TX_DS);
    }

    //Load data to ack payload if needed
    if((port->flags & WLS_FLAG_PKT_LOADED) || wls_load_one_data_packet(wls) > 0){
        port->active_packet.header.counter = port->send_counter;
        hal_nrf_write_ack_payload(0, (uint8_t*)&port->active_packet, port->active_packet_size);
    }
}


static void wls_handle_packet(wls_t *wls, uint8_t _port, uint8_t *_data, uint8_t len)
{
    wls_packet_t *pkt = (wls_packet_t*)_data;
    wls_port_t *port = &wls->port;
    
    wls_handle_ack_payload(wls);

    if(len < WLS_HEADER_SIZE || pkt->header.counter == port->recv_counter){
        wls->count_recv_drop += len;
        return;
    }
    

#define WLS_CALLBACK_BODY(wls, cb) {\
    wls->count_recv += len;\
    (wls)->cb.callback((wls)->cb.callback_data, _port, _data, len - WLS_HEADER_SIZE);\
}

    uint8_t type = pkt->header.type;
    //Dispatch data
    if(type == WLS_PKT_DATA && wls->handle_data.callback)
        WLS_CALLBACK_BODY(wls, handle_data)
    else if(type == WLS_PKT_CMD && wls->handle_cmd.callback)
        WLS_CALLBACK_BODY(wls, handle_cmd)
    else if(type == WLS_PKT_CMD_ACK && wls->handle_cmd_ack.callback)
        WLS_CALLBACK_BODY(wls, handle_cmd_ack)
    else
        wls->count_recv_drop += len;

#undef WLS_CALLBACK_BODY
    

}



void wls_nrf24_interrupt_handler()
{
    static uint8_t buf[32];

    uint8_t ds;
    uint16_t ret, port;
    
    ds = hal_nrf_get_rx_data_source();
    while(ds < 7U){ //
        ret = hal_nrf_read_rx_payload(buf);
        port = (ret >> 8) & 0xff;

        ASSERT(port == 0);

        wls_handle_packet(WLS, port, buf, ret & 0xff);

        ds = hal_nrf_get_rx_data_source();
    }

    //Only clear the RX DR interrupt
    hal_nrf_clear_irq_flag(HAL_NRF_RX_DR);
}


int wls_init(wls_t *wls)
{
    hal_nrf_hw_init();
    //Check nrf24l01 is exist or not
    if(wls_nrf24_exist() == 0)
        return -1;
    
    memset(wls, 0, sizeof(*wls));
    
    CBUFFER_INIT(&wls->port.cb,
            s_wls_buffer,
            WLS_BUF_SIZE,
            0);
    
    return wls_nrf24_init();
}


int wls_send_to(wls_t *wls, uint8_t *_data, uint8_t len)
{
    ASSERT(len);
    
    return CBUFFER_WRITE(&wls->port.cb, _data, len);
}


static int wls_load_one_data_packet(wls_t *wls)
{
    wls_port_t *port = &wls->port;
    uint32_t diff = 0;

    if(CBUFFER_EMPTY(&port->cb))
        return 0;
    
    diff = hal_time_diff(port->last_send_time, hal_time());
    if(diff < port->tx_buffer_timeout)
        return 0;
    
    port->active_packet_size = 
        CBUFFER_READ(&port->cb, port->active_packet.payload, WLS_MAX_PD_SIZE)
        + WLS_HEADER_SIZE;
    port->active_packet.header.type = WLS_PKT_DATA;
    WLS_FLAG_CLR(port->flags, WLS_FLAG_PKT_LOADED);    

    return port->active_packet_size;
}


int wls_nrf24_send_packet(uint8_t *addr, uint8_t *payload, uint8_t length)
{
    wls_addr_t ori_addr;
    //Check if ready to send
    if(length == 0)
        return 0;
    if(hal_nrf_get_irq_flags() & 0x3)
        return -1; //TX IRQ not handle

    //Setup address
    hal_nrf_get_address(HAL_NRF_PIPE0, ori_addr);
    hal_nrf_set_address(HAL_NRF_TX, addr);
    hal_nrf_set_address(HAL_NRF_PIPE0, addr);

    //setup restransmit
    hal_nrf_set_auto_retr(0x3, 0x20);//100us, 3
    hal_nrf_write_tx_payload(payload, length);

    //Pluse to send out data
    CE_PULSE();
    
    uint8_t status = hal_nrf_get_clear_irq_flags();
    while((status & 0x3) == 0) //Wait for transmit done
        status = hal_nrf_get_clear_irq_flags();
    
    if(status & 0x20)
        return length;
    else
        return -1;
}



static int wls_send_active_packet(wls_t *wls)
{
    wls_port_t *port = &wls->port;
    ASSERT(port->flags & WLS_FLAG_PKT_LOADED);
    ASSERT(!(port->flags & WLS_FLAG_PKT_ACK));

    WLS_FLAG_SET(port->flags, WLS_FLAG_PKT_TX);

    if(hal_nrf_get_irq_flags() & 0x3)
        return -1; //TX IRQ not handle

    //
    port->active_packet.header.counter = port->send_counter;

    //Setup address
    hal_nrf_set_address(HAL_NRF_PIPE0, port->target_addr);

    //setup restransmit
    hal_nrf_set_auto_retr(0x3, 0x20);//100us, 3
    hal_nrf_write_tx_payload((uint8_t*)&port->active_packet, port->active_packet_size);

    //Pluse to send out data
    CE_PULSE();
    

    uint8_t status;
    do{
        NRF24_IRQ_DISABLE();
        status = hal_nrf_get_irq_flags();
        NRF24_IRQ_ENABLE();
    }while((status & 0x30) == 0); //Wait for transmit done


    //
    hal_nrf_set_address(HAL_NRF_PIPE0, port->local_addr);
    
    //
    if(status & 0x20){ //Send succ
        wls->count_send += port->active_packet_size;
        WLS_FLAG_CLR(port->flags, WLS_FLAG_PKT_LOADED|WLS_FLAG_PKT_TX);

        //Reset retries
        port->send_retries = 0;
        //Update packet send counter
        ++port->send_counter;

        //Update last send time
        port->last_send_time = hal_time();

        hal_nrf_clear_irq_flag(HAL_NRF_TX_DS);
        return port->active_packet_size;
    }
    else if(port->send_retries < WLS_MAX_RETRIES){ //Need retry

        ++port->send_retries;
        WLS_FLAG_CLR(port->flags, WLS_FLAG_PKT_TX);
        
        hal_nrf_clear_irq_flag(HAL_NRF_MAX_RT);
        return -1;
    }
    else{ //Retry failed

        wls->count_send_fail += port->active_packet_size;

        WLS_FLAG_CLR(port->flags, WLS_FLAG_PKT_LOADED|WLS_FLAG_PKT_TX);
        
        //Reset retries
        port->send_retries = 0;
        //Update packet send counter
        ++port->send_counter;

        //Update last send time
        port->last_send_time = hal_time();

        hal_nrf_clear_irq_flag(HAL_NRF_MAX_RT);
        return 0;
    }
}



int wls_flush_tx(wls_t *wls)
{
    NRF24_IRQ_DISABLE();
    if(hal_nrf_get_carrier_detect() && !WLS_GAMBLE()){
        NRF24_IRQ_ENABLE();
        return -1;
    }
    //
    wls_port_t *port = &wls->port;

    //Make sure active packet handled by this thread
    WLS_FLAG_CLR(port->flags, WLS_FLAG_PKT_ACK);
    hal_nrf_set_operation_mode(HAL_NRF_PTX);

    NRF24_IRQ_ENABLE();

retry:

    if(!(port->flags & WLS_FLAG_PKT_LOADED) 
            && !wls_load_one_data_packet(wls))
        goto done;

    if(wls_send_active_packet(wls) || WLS_GAMBLE())
        goto retry;

done:
    hal_nrf_set_operation_mode(HAL_NRF_PRX);
    return 0;
}


void wls_set_callback(wls_t *wls, wls_callback_type_t type, wls_callback_t cb)
{

#define CB_CP(cb_from, cb_to)\
    (cb_to)->callback = (cb_from)->callback;\
    (cb_to)->callback_data = (cb_from)->callback_data;

    switch(type){
        case WLS_CB_DATA:
            CB_CP(&cb, &wls->handle_data)
            break;

        case WLS_CB_CMD:
            CB_CP(&cb, &wls->handle_cmd)
            break;

        case WLS_CB_CMD_ACK:
            CB_CP(&cb, &wls->handle_cmd_ack)
            break;

        default:
            ASSERT(0);
    }

#undef CB_CP
}


void wls_set_local_addr(wls_t *wls, wls_addr_t addr)
{
    wls_port_t *port = &wls->port;

    NRF24_IRQ_DISABLE();
    WLS_FLAG_CLR(port->flags, WLS_FLAG_PKT_LOADED|WLS_FLAG_PKT_TX|WLS_FLAG_PKT_ACK);
    hal_nrf_set_address(HAL_NRF_PIPE0, addr);
    NRF24_IRQ_ENABLE();

    memcpy(wls->port.local_addr, addr, sizeof(wls_addr_t));
}

void wls_set_remote_addr(wls_t *wls, wls_addr_t addr)
{
    wls_port_t *port = &wls->port;

    NRF24_IRQ_DISABLE();
    WLS_FLAG_CLR(port->flags, WLS_FLAG_PKT_LOADED|WLS_FLAG_PKT_TX|WLS_FLAG_PKT_ACK);
    hal_nrf_set_address(HAL_NRF_TX, addr);
    NRF24_IRQ_ENABLE();

    memcpy(wls->port.target_addr, addr, sizeof(wls_addr_t));
}



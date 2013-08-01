/**
 * @file wl_cmd.c
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-07-18
 */

#include "uart_ds_stm32.h"
#include "wl_serial.h"
#include  "wl_cmd.h"
#include  "settings.h"


static int (*wl_cmd_handle_remote_cmd)(void *, uint8_t *, uint8_t);
static void *wl_cmd_handle_remote_cmd_data = NULL;
static int (*wl_cmd_handle_return)(void *, uint8_t *, uint8_t);
static void *wl_cmd_handle_return_data = NULL;

static int wl_cmd_need_recv = -3; //
static uint8_t wl_cmd_pos = 0;
static uint8_t wl_cmd_return = 0;
static uint8_t wl_cmd[WL_CMD_BUF_SIZE];
static uint8_t *wl_cmd_result = wl_cmd + 2;

enum{
    WL_CMD_OP_SET_RF = 0x0,
    WL_CMD_OP_GET_RF = 0x1,
    WL_CMD_OP_SET_UART = 0x2,
    WL_CMD_OP_GET_UART = 0x3,
    WL_CMD_OP_SET_LOCAL_ADDR = 0x4,
    WL_CMD_OP_GET_LOCAL_ADDR = 0x5,
    WL_CMD_OP_SET_REMOTE_ADDR = 0x6,
    WL_CMD_OP_GET_REMOTE_ADDR = 0x7,
    WL_CMD_OP_SET_PWM_DUTY_CYCLE = 0x8,
    WL_CMD_OP_GET_PWM_DUTY_CYCLE = 0x9,
    WL_CMD_OP_SET_GPIO_STATE = 0xa,
    WL_CMD_OP_GET_GPIO_STATE = 0xb,

    WL_CMD_OP_GET_STATS = 0x31,

    WL_CMD_OP_RESET = 0x3F
};


#define WL_CMD_IS_REMOTE_CMD(op)  ((op) & 0x40)
#define WL_CMD_CLEAR_REMOTE_FLAG(op) ((op) & ~0x40)


void wl_cmd_set_handle_return(wl_cmd_callback_t handle_return, void *data)
{
    wl_cmd_handle_return_data = data;
    wl_cmd_handle_return = handle_return;
}

void wl_cmd_set_handle_remote_cmd(wl_cmd_callback_t handle_cmd, void *data)
{
    wl_cmd_handle_remote_cmd = handle_cmd;
    wl_cmd_handle_remote_cmd_data = data;
}


int wl_cmd_reset()
{
    wl_cmd_pos = 0;
    wl_cmd_need_recv = -3;
    wl_cmd_return = 0;
    return 0;
}

static int wl_cmd_handle_opcode(uint8_t op)
{
    switch(op & 0x3f){
        case WL_CMD_OP_SET_GPIO_STATE: //[idx, state]
        case WL_CMD_OP_SET_PWM_DUTY_CYCLE: //[idx, duty_cycle]
        case WL_CMD_OP_SET_RF: //[rf_chn, rf_speed]
            return 2;
        
        case WL_CMD_OP_GET_GPIO_STATE: //[idx]
        case WL_CMD_OP_GET_PWM_DUTY_CYCLE: //[idx]
        case WL_CMD_OP_SET_UART: //[baudrate]
        case WL_CMD_OP_GET_STATS: //[idx]
            return 1;

        case WL_CMD_OP_SET_REMOTE_ADDR:
        case WL_CMD_OP_SET_LOCAL_ADDR:
            return 5;

        case WL_CMD_OP_GET_RF:
        case WL_CMD_OP_GET_UART:
        case WL_CMD_OP_GET_LOCAL_ADDR:
        case WL_CMD_OP_GET_REMOTE_ADDR:
        case WL_CMD_OP_RESET:
            return 0;

        default:
            return -3;
    }
}

//[baudrate]
//baudrate = {1200銆�400銆�800銆�600銆�9200銆�8400銆�7600銆�15200
//230400, 460800}
//stopbits
static int wl_cmd_handle_setup_uart(uint8_t *cmd)
{
    uint32_t bs[] = {1200, 2400, 4800, 9600, 19200, 38400,
        57600, 115200, 230400, 460800};
    uint8_t op = cmd[0] & 0x3f;
    
    if(op == WL_CMD_OP_SET_UART){
        if(cmd[1] >= 10)
            return -1;

        settings->baudrate = bs[cmd[1]];
        settings->dirty_flag |= WL_BAUD_MASK;

        if(cmd[0] & 0x80)
            settings->save_flag |= WL_BAUD_MASK;
    }
    else{
        int i;
        //ASSERT settings->baudrate in bs
        for(i = 0; i < 10; i++)
            if(bs[i] == settings->baudrate)
                wl_cmd_result[0] = i; 
        
        wl_cmd_return = 1;
    }

    return 0;
}


//[FREQ, SPEED]
//[FREQ 0 - 125]
//[SPEED 0, 1, 2] = [250K, 1M, 2M]
static int wl_cmd_handle_setup_rf(uint8_t *cmd)
{
    uint8_t op = cmd[0] & 0x3f;
    
    if(op == WL_CMD_OP_SET_RF){
        uint8_t rf_map[3] = {0x2, 0x0, 0x1};
        if(cmd[1] > 125)
            return -1;
        if(cmd[2] > 2)
            return -1;

        settings->wls_rf_chn = cmd[1];
        settings->wls_rf_speed = rf_map[cmd[2]];
        settings->dirty_flag |= WL_NRF_CHN_MASK | WL_NRF_SPEED_MASK;

        if(cmd[0] & 0x80)
            settings->save_flag |= WL_NRF_CHN_MASK | WL_NRF_SPEED_MASK;
    }
    else{
        uint8_t rf_map[3] = {0x1, 0x2, 0x0};
      
        wl_cmd_result[0] = settings->wls_rf_chn;
        wl_cmd_result[1] = rf_map[settings->wls_rf_speed];
        wl_cmd_return = 2; 
    }
    
    return 0;
}

//[Address[5]]
static int wl_cmd_handle_set_local_addr(uint8_t *cmd)
{
    uint8_t op = cmd[0] & 0x3f;
    
    if(op == WL_CMD_OP_SET_LOCAL_ADDR){
        memcpy(settings->wls_local_addr, cmd + 1, sizeof(settings->wls_local_addr));
        settings->dirty_flag |= WL_LOCAL_ADDR_MASK;

        if(cmd[0] & 0x80)
            settings->save_flag |= WL_LOCAL_ADDR_MASK;
    }
    else{
        memcpy(wl_cmd_result, settings->wls_local_addr, sizeof(settings->wls_local_addr));
        wl_cmd_return = sizeof(settings->wls_local_addr);
    }
    return 0;
}

//[Address[5]]
static int wl_cmd_handle_set_remote_addr(uint8_t *cmd)
{
    uint8_t op = cmd[0] & 0x3f;

    if(op == WL_CMD_OP_SET_REMOTE_ADDR){
        memcpy(settings->wls_remote_addr, cmd + 1, sizeof(settings->wls_remote_addr));
        settings->dirty_flag |= WL_REMOTE_ADDR_MASK;

        if(cmd[0] & 0x80)
            settings->save_flag |= WL_REMOTE_ADDR_MASK;
    }
    else{
        memcpy(wl_cmd_result, settings->wls_remote_addr, sizeof(settings->wls_remote_addr));
        wl_cmd_return = sizeof(settings->wls_remote_addr);
    }

    return 0;
}


static int wl_cmd_handle_gpio_set(uint8_t *cmd)
{
    uint8_t op = cmd[0] & 0x3f;
    uint8_t idx = cmd[1];
    if(idx >= HAL_NUM_GPIO)
        return -1;

    if(op == WL_CMD_OP_SET_GPIO_STATE){
        if(cmd[2])
            settings->gpio_state |= 1 << idx;
        else
            settings->gpio_state &= ~(1 << idx);
        settings->dirty_flag |= WL_GPIO_MASK;
    }
    else{
        wl_cmd_result[0] = (settings->gpio_state >> idx) & 0x1;
        wl_cmd_return = 1;
    }
  
    return 0;

}

static int wl_cmd_handle_pwm_duty_cycle(uint8_t *cmd)
{
    uint8_t op = cmd[0] & 0x3f;

    if(op == WL_CMD_OP_SET_PWM_DUTY_CYCLE){
        uint8_t idx = cmd[1];
        uint32_t flags[HAL_NUM_PWM] = {
                    WL_PWM0_MASK,
                    WL_PWM1_MASK,
                    WL_PWM2_MASK,
                    WL_PWM3_MASK};

        if(cmd[2] > 100 || idx >= HAL_NUM_PWM)
            return -1;
        settings->pwm_duty_cycle[idx] = cmd[2];
        settings->dirty_flag |= flags[idx];
    }
    else{
        uint8_t idx = cmd[1]; 
        if(idx >= HAL_NUM_PWM)
            return -1;
        wl_cmd_result[0] = settings->pwm_duty_cycle[idx];
        wl_cmd_return = 1;
    }
    
    return 0;
}

static int wl_cmd_handle_get_stats(uint8_t *cmd)
{

    extern uint32_t count_recv,
           count_recv_fail,
           count_send,
           count_send_fail;
    extern wls_t g_wls;
    uint8_t idx = cmd[1];

    uint32_t vs[] = {count_recv,
                     count_recv_fail,
                     count_send,
                     count_send_fail,
                     WLS->count_recv,
                     WLS->count_recv_drop,
                     WLS->count_send,
                     WLS->count_ack_send,
                     WLS->count_send_fail,
                     WLS->count_retry,
                     DS->count_recv_drop,
                     DS->count_recv,
                     DS->count_send,
                     DS->count_send_fail
                    };
    uint8_t *data = wl_cmd_result;
    if(idx < 14){
        data[0] = vs[idx] & 0xff;
        data[1] = (vs[idx] >> 8) & 0xff;
        data[2] = (vs[idx] >> 16) & 0xff;
        data[3] = (vs[idx] >> 24) & 0xff;
        wl_cmd_return = 4;

        return 0;
    }
    else{
        wl_cmd_return = 0;
        return -1;
    }

}

static int wl_cmd_handle_reset(uint8_t *cmd)
{
    //WWDG_SWReset();
    while(1);

    return 0;
}


static void wl_cmd_output_result()
{
    if(wl_cmd_handle_return && wl_cmd_return)
        wl_cmd_handle_return(wl_cmd_handle_return_data, wl_cmd, wl_cmd_return);
}


static int wl_cmd_handle_local_cmd(uint8_t *cmd, uint8_t len)
{

    uint8_t op = cmd[2];
    int ret = -1;
    
    switch(op & 0x3f){
        case WL_CMD_OP_SET_RF:
        case WL_CMD_OP_GET_RF:
            ret = wl_cmd_handle_setup_rf(cmd + 2);
            break;

        case WL_CMD_OP_SET_UART:
        case WL_CMD_OP_GET_UART:
            ret = wl_cmd_handle_setup_uart(cmd + 2);
            break;

        case WL_CMD_OP_SET_LOCAL_ADDR:
        case WL_CMD_OP_GET_LOCAL_ADDR:
            ret = wl_cmd_handle_set_local_addr(cmd + 2);
            break;

        case WL_CMD_OP_SET_REMOTE_ADDR:
        case WL_CMD_OP_GET_REMOTE_ADDR:
            ret = wl_cmd_handle_set_remote_addr(cmd + 2);
            break;

        case WL_CMD_OP_SET_PWM_DUTY_CYCLE:
        case WL_CMD_OP_GET_PWM_DUTY_CYCLE:
            ret = wl_cmd_handle_pwm_duty_cycle(cmd + 2);
            break;

        case WL_CMD_OP_SET_GPIO_STATE:
        case WL_CMD_OP_GET_GPIO_STATE:
            ret = wl_cmd_handle_gpio_set(cmd + 2);
            break;

        case WL_CMD_OP_GET_STATS:
            ret = wl_cmd_handle_get_stats(cmd + 2);
            break;

        case WL_CMD_OP_RESET:
            ret = wl_cmd_handle_reset(cmd + 2);
            break;

        default:
            ret = -1;
    }
    
    if(ret == 0){
        wl_cmd[0] = 'O';
        wl_cmd[1] = 'K';
    }
    else{
        wl_cmd[0] = 'N';
        wl_cmd[1] = 'O';
    }

    return 2 + wl_cmd_return;
}


static void wl_cmd_handle_cmd()
{
    ASSERT(wl_cmd_need_recv >= 0 && wl_cmd_pos == wl_cmd_need_recv);
    uint8_t op = wl_cmd[2];
    
    if(WL_CMD_IS_REMOTE_CMD(op)){
        if(wl_cmd_handle_remote_cmd){
            //Clear remote flags
            wl_cmd[2] = WL_CMD_CLEAR_REMOTE_FLAG(op);
            wl_cmd_handle_remote_cmd(wl_cmd_handle_remote_cmd_data, wl_cmd, wl_cmd_pos + 3);

            wl_cmd_return = 0; //ack will receive through handle_cmd_ack_packet()
        }
    }
    else{
        wl_cmd_return = wl_cmd_handle_local_cmd(wl_cmd, wl_cmd_pos);
    }

}



int wl_cmd_recv(uint8_t *data, uint8_t len)
{
    int i, cpos;
    for(i = 0; i < len; i++){
        if(wl_cmd_need_recv < 0){
            cpos = 3 + wl_cmd_need_recv;
            if(cpos == 0 && data[i] == 0xAF){
                wl_cmd[cpos] = data[i];
                ++wl_cmd_need_recv;
            }
            else if(cpos == 1 && data[i] == 0xFA){
                wl_cmd[cpos] = data[i];
                ++wl_cmd_need_recv;
            }
            else if(cpos == 2){
                wl_cmd_need_recv = wl_cmd_handle_opcode(data[i]);
                wl_cmd[cpos] = data[i];
                wl_cmd_pos = 0;
            }
            else
                wl_cmd_reset();
        }
        else{
            wl_cmd[++wl_cmd_pos + 2] = data[i];
        }
        
        if(wl_cmd_need_recv >= 0 && wl_cmd_pos == wl_cmd_need_recv){
            wl_cmd_handle_cmd();
            wl_cmd_output_result();

            wl_cmd_reset();
        }

    }

    return len;
}

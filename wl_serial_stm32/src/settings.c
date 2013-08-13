/**
 * @file settings.c
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-07-19
 */

#include "settings.h"
#include "wl_serial.h"
#include "uart_ds_stm32.h"

#define SETTING_TAG 0xFEEFABBA
#define SETTING_ADDR (0x8000000 + 0xFC00) //63k

settings_t g_settings = {
    .tag = SETTING_TAG,
    .baudrate = HAL_DEFAULT_UART_BAUD,

    .wls_rf_chn = 0x40, 
    .wls_rf_speed = HAL_NRF_2MBPS, //2M
    .wls_local_addr = {0x0, 0x1, 0x2, 0x3, 0x4},
    .wls_remote_addr = {0x0, 0x1, 0x2, 0x3, 0x4},

    .pwm_duty_cycle = {0, 0, 0, 0},
    .gpio_state = 0,

    .dirty_flag = 0x0,
    .save_flag = 0x0
};

#if 0
#define SETTING_CP(st1, st2) do{\
    (st1)->tag = (st2)->tag;\
    (st1)->baudrate = (st2)->baudrate;\
    (st1)->nrf_chn = (st2)->nrf_chn;\
    (st1)->nrf_speed = (st2)->nrf_speed;\
    (st1)->local_addr[0] = (st2)->local_addr[0];\
    (st1)->local_addr[1] = (st2)->local_addr[1];\
    (st1)->local_addr[2] = (st2)->local_addr[2];\
    (st1)->local_addr[3] = (st2)->local_addr[3];\
    (st1)->local_addr[4] = (st2)->local_addr[4];\
    (st1)->remote_addr[0] = (st2)->remote_addr[0];\
    (st1)->remote_addr[1] = (st2)->remote_addr[1];\
    (st1)->remote_addr[2] = (st2)->remote_addr[2];\
    (st1)->remote_addr[3] = (st2)->remote_addr[3];\
    (st1)->remote_addr[4] = (st2)->remote_addr[4];\
}while(0)
#endif

int settings_load(settings_t *st)
{
    settings_t *ori = (settings_t*)SETTING_ADDR;
    //Check the data is correct or not
    if(ori->tag != SETTING_TAG){
        st->save_flag = 0xffffffff; //mark to save all settings
        settings_save(st);
    }
    else{
        memcpy(st, ori, SETTING_SIZE());
    }
    return 0;
}


int settings_commit(settings_t *st)
{
    if(st->dirty_flag & WL_BAUD_MASK)
        ds_set_baudrate(DS, st->baudrate);

    if(st->dirty_flag & (WL_NRF_CHN_MASK | WL_NRF_SPEED_MASK))
        wls_set_rf(WLS, st->wls_rf_chn, st->wls_rf_speed);

    if(st->dirty_flag & WL_LOCAL_ADDR_MASK)
        wls_set_local_addr(WLS, st->wls_local_addr);

    if(st->dirty_flag & WL_REMOTE_ADDR_MASK)
        wls_set_remote_addr(WLS, st->wls_remote_addr);
#if 0
    if(st->dirty_flag & WL_PWM0_MASK)
        hal_pwm_set_duty_cycle(0, st->pwm_duty_cycle[0]);

    if(st->dirty_flag & WL_PWM1_MASK)
        hal_pwm_set_duty_cycle(1, st->pwm_duty_cycle[1]);

    if(st->dirty_flag & WL_PWM2_MASK)
        hal_pwm_set_duty_cycle(2, st->pwm_duty_cycle[2]);

    if(st->dirty_flag & WL_PWM3_MASK)
        hal_pwm_set_duty_cycle(3, st->pwm_duty_cycle[3]);
    
    if(st->dirty_flag & WL_GPIO_MASK){
        uint8_t i;
        for(i = 0; i < WL_NUM_GPIO; i++)
            hal_gpio_set(i, (st->gpio_state >> i) & 0x1);
    }
#endif
    st->dirty_flag = 0x0;

    return 0;
}


int settings_save(settings_t *st)
{
    if(st->save_flag){
        uint16_t ssize = SETTING_SIZE() / 2; //dirty_flag and save_flag
        uint16_t i;
        uint16_t *data = (uint16_t*)st,
                 *dst = (uint16_t*)SETTING_ADDR;

retry:
        FLASH_Unlock();

        FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
        FLASH_ErasePage(SETTING_ADDR);
        //program
        for(i = 0; i < ssize; ++i)
            FLASH_ProgramHalfWord(SETTING_ADDR + i * 2, data[i]);

        FLASH_Unlock();

        //verify
        for(i = 0; i < ssize; ++i)
            if(dst[i] != data[i])
                goto retry;

    }
    st->save_flag = 0x0;

    return 0;
}

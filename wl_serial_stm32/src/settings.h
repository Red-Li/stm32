/**
 * @file settings.h
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-07-19
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include "hal_stm32.h"
#include "wl_serial.h"


#define WL_BAUD_MASK 0x1
#define WL_NRF_CHN_MASK 0x2
#define WL_NRF_SPEED_MASK 0x4
#define WL_LOCAL_ADDR_MASK 0x8
#define WL_REMOTE_ADDR_MASK 0x10

#define WL_PWM0_MASK 0x20
#define WL_PWM1_MASK 0x40
#define WL_PWM2_MASK 0x80
#define WL_PWM3_MASK 0x100

#define WL_GPIO_MASK 0x1000

typedef struct settings_s{
    uint32_t tag; //to verify the data in eeprom is valid or not
    uint32_t baudrate;

    uint8_t wls_rf_chn;
    wls_speed_t wls_rf_speed;
    wls_addr_t wls_local_addr;
    wls_addr_t wls_remote_addr;

    uint8_t pwm_duty_cycle[HAL_NUM_PWM]; //0 - 10
    uint8_t gpio_state; //maximum 8 GPIOs

    uint32_t dirty_flag;
    uint32_t save_flag;

}settings_t;

extern settings_t g_settings;
#define settings (&g_settings)

int settings_commit(settings_t *st);
int settings_load(settings_t *st);
int settings_save(settings_t *st);


#endif

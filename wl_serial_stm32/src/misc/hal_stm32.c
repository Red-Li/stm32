/**
 * @file hal_stm32.c
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-07-09
 */


#include <stdint.h>

#include "hal_stm32.h"

volatile static hal_time_t __hal_time = 0;

//Assign value to _estack in LD script

uint8_t *g_stack_lowest = (uint8_t*)-1;

static void NVIC_Configration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); 

    //DMA for uart
    NVIC_InitStructure.NVIC_IRQChannel = HAL_DS_UART_DMA_IRQ_CHN;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    //uart RX irq
    NVIC_InitStructure.NVIC_IRQChannel = HAL_DS_UART_RX_IRQ_CHN;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    //NRF RX interrupt
    NVIC_InitStructure.NVIC_IRQChannel = HAL_NRF_IRQ_CHN;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x4;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    //management interrupt
    NVIC_InitStructure.NVIC_IRQChannel = HAL_MANAGEMENT_IRQ_CHN;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //hal timer irq
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0xf;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_SetPriority(SysTick_IRQn, 0);
}


static void USART_Configration(void)
{
    //HAL_DEBUG_UART
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = HAL_DEFAULT_UART_BAUD;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    /* USART configuration */
    USART_Init(HAL_DEBUG_UART, &USART_InitStructure);
    /* Enable USART */
    USART_Cmd(HAL_DEBUG_UART, ENABLE);

}

static void RCC_Configuration(void)
{
#if defined(HAL_BOARD_DOFLY)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//DMA1

#elif defined(HAL_BOARD_OURSTM)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//DMA1
#else
#error "No board have been selected"
#endif
}


static void EXTI_Configuration(void)
{
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_DeInit();

    //NRF IRQ
    EXTI_InitStruct.EXTI_Line = HAL_NRF_EXTI_LINE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    //management IRQ
    EXTI_InitStruct.EXTI_Line = HAL_MANAGEMENT_EXTI_LINE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);
}

static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure USART3 Tx as push-pull */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = HAL_DS_UART_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(HAL_DS_UART_GPIO, &GPIO_InitStructure);
    /* Configure USART3 Rx as input floating */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = HAL_DS_UART_RX_PIN;
    GPIO_Init(HAL_DS_UART_GPIO, &GPIO_InitStructure);
    
    /* Configure SPI2 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    //SCK, MISO, MOSI
    GPIO_InitStructure.GPIO_Pin = HAL_NRF_SPI_SCK_PIN | HAL_NRF_SPI_MISO_PIN | HAL_NRF_SPI_MOSI_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(HAL_NRF_SPI_GPIO, &GPIO_InitStructure);
    //NSS 
    GPIO_InitStructure.GPIO_Pin = HAL_NRF_SPI_CSN_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(HAL_NRF_SPI_GPIO, &GPIO_InitStructure);
    //CE
    GPIO_InitStructure.GPIO_Pin = HAL_NRF_CE_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(HAL_NRF_CE_GPIO, &GPIO_InitStructure);
    
    //Configure IRQ PIN
    GPIO_InitStructure.GPIO_Pin = HAL_NRF_IRQ_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//input
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(HAL_NRF_IRQ_GPIO, &GPIO_InitStructure);
    //Enable extern interrupt
    GPIO_EXTILineConfig(HAL_NRF_EXTI_PORT, HAL_NRF_EXTI_PORT_SOURCE); //interrupt 0 

    //Configure management IRQ 
    GPIO_InitStructure.GPIO_Pin = HAL_MANAGEMENT_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//input
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(HAL_MANAGEMENT_GPIO, &GPIO_InitStructure);
    //Enable extern interrupt
    GPIO_EXTILineConfig(HAL_MANAGEMENT_EXTI_PORT, HAL_MANAGEMENT_EXTI_PORT_SOURCE); //interrupt 0 

}

hal_time_t hal_time()
{
    hal_time_t t0 = __hal_time;
    uint32_t elapse = systick_counter() - systick_current();

    while(t0 != __hal_time){
        t0 = __hal_time;
        elapse = systick_counter() - systick_current();
    } //Trick

    return t0 + elapse / (sysclk_freq() / 1000000); 
}


static uint32_t s_rand_seed = 0;
void hal_srand(uint32_t seed)
{
    if(seed == (uint32_t)-1)
        s_rand_seed = hal_time();
    else
        s_rand_seed = seed;
}

//http://stackoverflow.com/questions/6275593/how-to-write-you-own-random-number-algorithm
uint32_t hal_rand()
{
    return s_rand_seed = (s_rand_seed * 214013UL + 2531011UL)
        % 2147483647UL;
}


void systick_interrupt_handler()
{
    uint8_t tmp, *p = &tmp;
    __hal_time += 1000000 / systick_freq();
    if(p < g_stack_lowest)
        g_stack_lowest = p;
}

void _SysTick_Config()
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    SysTick_Config(systick_counter());
}


extern void __hal_timer_tick();
void hal_timer_interrupt_handler()
{
    __hal_timer_tick();
    TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update);
}


void hal_timer_config()
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_DeInit(TIM3);

    TIM_TimeBaseStructure.TIM_Period = 1000;//ARR鐨勫�
    TIM_TimeBaseStructure.TIM_Prescaler = 72; //sysclk_freq() / 1000000; //Every 1ms
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //鍚戜笂璁℃暟妯″紡
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE); //寮�惎鏃堕挓

}



void hal_init(void) 
{
    SystemInit();
    
    SystemCoreClockUpdate();


    _SysTick_Config();
    //
    RCC_Configuration();

    EXTI_Configuration();

    GPIO_Configuration();

    USART_Configration();
    
    hal_timer_config();

    NVIC_Configration();

}




void udelay(uint32_t us)
{
    hal_time_t now = hal_time();
    while(hal_time() < now + us);
}

void mdelay(uint32_t ms)
{
    hal_time_t now = hal_time();
    while(hal_time() < now + 1000UL * ms);
}

//
//
static void hal_pin_management_enable(void *data)
{
    //GPIO config
    GPIO_InitTypeDef GPIO_InitStructure;
    //Configure management IRQ 
    GPIO_InitStructure.GPIO_Pin = HAL_MANAGEMENT_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(HAL_MANAGEMENT_GPIO, &GPIO_InitStructure);
    //Enable extern interrupt
    GPIO_EXTILineConfig(HAL_MANAGEMENT_EXTI_PORT, HAL_MANAGEMENT_EXTI_PORT_SOURCE); //interrupt 0 
    
    //EXTI config
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Line = HAL_MANAGEMENT_EXTI_LINE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);
}

static void hal_pin_management_disable(void *data)
{
    //EXTI config
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Line = HAL_MANAGEMENT_EXTI_LINE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStruct.EXTI_LineCmd = DISABLE;
    EXTI_Init(&EXTI_InitStruct);
}


static void hal_pin_pwm_enable(void *data)
{

}

static void hal_pin_pwm_disable(void *data)
{
}

static void hal_pin_gpio_enable(void *data)
{
    uint8_t idx = (uint8_t)(data - (void*)0);
    GPIO_TypeDef *gpio = NULL;
    uint16_t pin = 0;

    if(idx < 2){
        gpio = GPIOA;
        pin = idx == 0 ? GPIO_Pin_4 : GPIO_Pin_5;
    }
    else if(idx < 4){
        gpio = GPIOB;
        pin = idx == 3 ? GPIO_Pin_10 : GPIO_Pin_11;
    }
    //else
    //  ASSERT(0);
    
    if(gpio){
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Pin = pin;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(gpio, &GPIO_InitStructure);
    }

}

static void hal_pin_gpio_disable(void *data)
{
}

static void hal_pin_uart_tx_enable(void *data)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure USART3 Tx as push-pull */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = HAL_DS_UART_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(HAL_DS_UART_GPIO, &GPIO_InitStructure);

    //Enable Tx
    HAL_DS_UART->CR1 |= USART_Mode_Tx;
}

static void hal_pin_uart_tx_disable(void *data)
{
    //Disable Tx
    HAL_DS_UART->CR1 &= ~USART_Mode_Tx;
}

static void hal_pin_uart_rx_enable(void *data)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure USART3 Rx as input floating */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = HAL_DS_UART_RX_PIN;
    GPIO_Init(HAL_DS_UART_GPIO, &GPIO_InitStructure);

    //Enable Rx
    HAL_DS_UART->CR1 |= USART_Mode_Rx;
}

static void hal_pin_uart_rx_disable(void *data)
{
    //Enable Tx
    HAL_DS_UART->CR1 &= ~USART_Mode_Rx;
}

typedef void (*hal_pin_mode_cb_t)(void*);

typedef struct hal_pin_mode_op_s{
    hal_pin_mode_cb_t enable;
    void              *enable_data;
    hal_pin_mode_cb_t disable;
    void              *disable_data;
}hal_pin_mode_op_t;


const static hal_pin_mode_op_t __hal_mode_op_map[HAL_PIN_TOTAL][4] = {
    { //PIN0
        {hal_pin_management_enable, NULL, hal_pin_management_disable, NULL},
        {hal_pin_pwm_enable, (void*)0, hal_pin_pwm_disable, (void*)0},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL}
    },
    { //Pin1
        {hal_pin_pwm_enable, (void*)1, hal_pin_pwm_disable, (void*)1},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL}
    },
    { //Pin2
        {hal_pin_gpio_enable, (void*)0, hal_pin_gpio_disable, (void*)0},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL}
    },
    { //Pin3
        {hal_pin_gpio_enable, (void*)1, hal_pin_gpio_disable, (void*)1},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL}
    },
    { //Pin4
        {hal_pin_pwm_enable, (void*)2, hal_pin_pwm_disable, (void*)2},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL}
    },
    { //Pin5
        {hal_pin_pwm_enable, (void*)3, hal_pin_pwm_disable, (void*)3},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL}
    },
    { //Pin6
        {hal_pin_uart_tx_enable, NULL, hal_pin_uart_tx_disable, NULL},
        {hal_pin_gpio_enable, (void*)2, hal_pin_gpio_disable, (void*)2},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL}
    },
    { //Pin7
        {hal_pin_uart_rx_enable, NULL, hal_pin_uart_rx_disable, NULL},
        {hal_pin_gpio_enable, (void*)3, hal_pin_gpio_disable, (void*)3},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL}
    },
};


void hal_pin_mode_change(uint8_t which, uint8_t mode_from, uint8_t mode_to)
{
    //ASSERT
    while(which >= HAL_PIN_TOTAL || mode_from >= 4 || mode_to >= 4);
    
    hal_pin_mode_op_t *op = (hal_pin_mode_op_t*)&__hal_mode_op_map[which];

    if(op[mode_from].disable)
        op[mode_from].disable(op[mode_from].disable_data);
    
    if(op[mode_to].enable)
        op[mode_to].enable(op[mode_to].enable_data);

}

void hal_gpio_set(uint8_t which, uint8_t enable)
{
    GPIO_TypeDef *gpio = NULL;
    uint16_t pin = 0;

    if(which < 2){
        gpio = GPIOA;
        pin = which == 0 ? GPIO_Pin_4 : GPIO_Pin_5;
    }
    else if(which < 4){
        gpio = GPIOB;
        pin = which == 3 ? GPIO_Pin_10 : GPIO_Pin_11;
    }
    //else
    //  ASSERT(0);
    if(gpio)
        GPIO_WriteBit(gpio, pin, enable);
}

void hal_pwm_set_duty_cycle(uint8_t which, uint8_t cycle_duty)
{
}
